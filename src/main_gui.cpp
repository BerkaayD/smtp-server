// main_gui.cpp - Dear ImGui ile masaustu uygulamasi giris noktasi
// Mevcut main.cpp'ye dokunmadan ayri bir executable olusturur.
// Backend: Win32 + OpenGL3 (MinGW uyumlu)

#include <windows.h>
#include <dwmapi.h>
#include <GL/gl.h>
#include <filesystem>
#include <string>
#include <iostream>
#include <thread>
#include <chrono>

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_opengl3.h"

#include "utils.h"
#include "logger.h"
#include "auth.h"
#include "mail_store.h"
#include "smtp_server.h"
#include "imap_server.h"
#include "gui_app.h"

namespace fs = std::filesystem;

// Forward declaration
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Globals
static HWND   g_hWnd = NULL;
static HDC    g_hDC  = NULL;
static HGLRC  g_hRC  = NULL;

static bool CreateDeviceWGL(HWND hWnd) {
    g_hDC = ::GetDC(hWnd);
    PIXELFORMATDESCRIPTOR pfd = {};
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;
    pfd.cStencilBits = 8;
    int pf = ::ChoosePixelFormat(g_hDC, &pfd);
    if (pf == 0) return false;
    if (::SetPixelFormat(g_hDC, pf, &pfd) == FALSE) return false;
    g_hRC = ::wglCreateContext(g_hDC);
    if (!g_hRC) return false;
    ::wglMakeCurrent(g_hDC, g_hRC);
    return true;
}

static void CleanupDeviceWGL() {
    ::wglMakeCurrent(NULL, NULL);
    if (g_hRC) ::wglDeleteContext(g_hRC);
    if (g_hDC && g_hWnd) ::ReleaseDC(g_hWnd, g_hDC);
}

static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;
    switch (msg) {
    case WM_SIZE:
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
    // Calisma dizinini belirle
    std::string exePath = fs::current_path().string();
    std::string dataDir = exePath + "\\data";
    std::string logFile = dataDir + "\\server.log";
    fs::create_directories(dataDir);
    fs::create_directories(dataDir + "\\mailboxes");

    // Logger baslat
    Logger::getInstance().init(logFile, false);
    LOG_INFO("=== Ulusan Sigorta Mail Sunucusu (GUI) Baslatiliyor ===");

    // Winsock baslat
    if (!utils::initWinsock()) {
        MessageBoxA(NULL, "Winsock baslatilamadi!", "Hata", MB_OK | MB_ICONERROR);
        return 1;
    }

    // Backend modulleri
    static const int SMTP_PORT = 587;
    static const int IMAP_PORT = 143;
    static const std::string DOMAIN = "ulusansigorta.com.tr";

    AuthManager auth(dataDir);
    MailStore store(dataDir);
    auth.createDefaultAdmin();
    store.createMailbox("admin");

    if (!auth.userExists("ulusan")) {
        auth.registerUser("ulusan", "ulusan123", UserRole::ADMIN, "Umut Baran Ulusan");
        store.createMailbox("ulusan");
    }
    if (!auth.userExists("berkay")) {
        auth.registerUser("berkay", "berkay123", UserRole::USER, "Berkay Demirci");
        store.createMailbox("berkay");
    }
    if (!auth.userExists("destek")) {
        auth.registerUser("destek", "destek123", UserRole::USER, "Teknik Destek");
        store.createMailbox("destek");
    }
    store.addForwardingRule("destek", "ulusan");
    store.addForwardingRule("destek", "berkay");

    SmtpServer smtp(SMTP_PORT, auth, store);
    ImapServer imap(IMAP_PORT, auth, store);
    smtp.start();
    imap.start();

    // Win32 pencere olustur
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(wc);
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = ::LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = L"UlusanMailGUI";
    ::RegisterClassExW(&wc);

    g_hWnd = ::CreateWindowExW(0, wc.lpszClassName,
        L"Ulusan Sigorta - Mail Sunucusu",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1280, 800,
        NULL, NULL, hInstance, NULL);

    if (!CreateDeviceWGL(g_hWnd)) {
        MessageBoxA(NULL, "OpenGL olusturulamadi!", "Hata", MB_OK | MB_ICONERROR);
        CleanupDeviceWGL();
        ::DestroyWindow(g_hWnd);
        return 1;
    }

    ::ShowWindow(g_hWnd, SW_SHOWDEFAULT);
    ::UpdateWindow(g_hWnd);

    // Koyu baslik cubugu (Windows 10+)
    BOOL darkMode = TRUE;
    ::DwmSetWindowAttribute(g_hWnd, 20, &darkMode, sizeof(darkMode));

    // ImGui baslat
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = NULL;

    ImGui_ImplWin32_InitForOpenGL(g_hWnd);
    ImGui_ImplOpenGL3_Init();

    // Font yukle - Segoe UI (Turkce karakter destegi)
    ImFontConfig fontCfg;
    fontCfg.OversampleH = 2;
    fontCfg.OversampleV = 2;
    // Glyph ranges: Basic Latin + Latin-1 Supplement + Latin Extended-A (Turkce)
    static const ImWchar glyphRanges[] = { 0x0020, 0x017F, 0 };
    const char* fontPath = "C:\\Windows\\Fonts\\segoeui.ttf";
    ImFont* font = io.Fonts->AddFontFromFileTTF(fontPath, 16.0f, &fontCfg, glyphRanges);
    if (!font) font = io.Fonts->AddFontDefault();

    // GUI uygulama nesnesi
    GuiApp guiApp(auth, store, smtp, imap, DOMAIN);

    // Ana dongu
    bool running = true;
    while (running) {
        MSG msg;
        while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT) running = false;
        }
        if (!running) break;

        // Frame baslat
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        guiApp.render();

        // Render
        ImGui::Render();
        RECT rect;
        ::GetClientRect(g_hWnd, &rect);
        glViewport(0, 0, rect.right - rect.left, rect.bottom - rect.top);
        glClearColor(0.059f, 0.067f, 0.090f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        ::SwapBuffers(g_hDC);
    }

    // Temizlik
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    CleanupDeviceWGL();
    ::DestroyWindow(g_hWnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    smtp.stop();
    imap.stop();
    utils::cleanupWinsock();
    LOG_INFO("=== Sunucu (GUI) kapatildi ===");

    return 0;
}

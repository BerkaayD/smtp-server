#include "gui_app.h"
#include "imgui.h"
#include "logger.h"
#include "utils.h"
#include <cstring>

// Renkler (gui_app.cpp ile ayni)
static const ImVec4 COL_ACCENT2  = ImVec4(0.231f, 0.510f, 0.965f, 1.0f);
static const ImVec4 COL_GREEN2   = ImVec4(0.063f, 0.725f, 0.506f, 1.0f);
static const ImVec4 COL_RED2     = ImVec4(0.937f, 0.267f, 0.267f, 1.0f);
static const ImVec4 COL_ORANGE2  = ImVec4(0.961f, 0.620f, 0.043f, 1.0f);
static const ImVec4 COL_DIMTEXT2 = ImVec4(0.392f, 0.455f, 0.545f, 1.0f);
static const ImVec4 COL_PURPLE2  = ImVec4(0.545f, 0.361f, 0.965f, 1.0f);

void GuiApp::renderLogin() {
    ImGuiViewport* vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(vp->WorkPos);
    ImGui::SetNextWindowSize(vp->WorkSize);
    ImGui::Begin("##Login", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);

    float winW = ImGui::GetWindowWidth();
    float winH = ImGui::GetWindowHeight();
    float cardW = 420, cardH = showRegister ? 420 : 360;
    float cx = (winW - cardW) * 0.5f;
    float cy = (winH - cardH) * 0.5f;

    ImGui::SetCursorPos(ImVec2(cx, cy));
    ImGui::BeginChild("##LoginCard", ImVec2(cardW, cardH), ImGuiChildFlags_Borders);

    ImGui::Dummy(ImVec2(0, 8));
    float tw = ImGui::CalcTextSize("ULUSAN SIGORTA").x;
    ImGui::SetCursorPosX((cardW - tw) * 0.5f);
    ImGui::TextColored(COL_ACCENT2, "ULUSAN SIGORTA");
    tw = ImGui::CalcTextSize("Mail Sunucusu").x;
    ImGui::SetCursorPosX((cardW - tw) * 0.5f);
    ImGui::TextColored(COL_DIMTEXT2, "Mail Sunucusu");
    ImGui::Dummy(ImVec2(0, 16));
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0, 12));

    if (!showRegister) {
        ImGui::Text("Kullanici Adi");
        ImGui::SetNextItemWidth(-1);
        ImGui::InputText("##lu", loginUser, sizeof(loginUser));
        ImGui::Dummy(ImVec2(0, 4));
        ImGui::Text("Sifre");
        ImGui::SetNextItemWidth(-1);
        ImGui::InputText("##lp", loginPass, sizeof(loginPass), ImGuiInputTextFlags_Password);
        ImGui::Dummy(ImVec2(0, 12));

        if (ImGui::Button("Giris Yap", ImVec2(-1, 40))) {
            if (auth.authenticate(loginUser, loginPass)) {
                loggedIn = true;
                currentUser = loginUser;
                currentRole = auth.getUserRole(currentUser);
                User* u = auth.getUser(currentUser);
                displayName = u ? u->displayName : currentUser;
                currentScreen = SCREEN_DASHBOARD;
                clearLoginFields();
            } else {
                loginError = "Kullanici adi veya sifre yanlis!";
            }
        }
        ImGui::Dummy(ImVec2(0, 4));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.118f, 0.133f, 0.176f, 1.0f));
        if (ImGui::Button("Kayit Ol", ImVec2(-1, 36))) {
            showRegister = true; loginError.clear();
        }
        ImGui::PopStyleColor();

        if (!loginError.empty()) {
            ImGui::Dummy(ImVec2(0, 4));
            ImGui::TextColored(COL_RED2, "%s", loginError.c_str());
        }
    } else {
        ImGui::Text("Kullanici Adi");
        ImGui::SetNextItemWidth(-1);
        ImGui::InputText("##ru", regUser, sizeof(regUser));
        ImGui::Text("Gorunen Ad");
        ImGui::SetNextItemWidth(-1);
        ImGui::InputText("##rd", regDisplayName, sizeof(regDisplayName));
        ImGui::Text("Sifre");
        ImGui::SetNextItemWidth(-1);
        ImGui::InputText("##rp", regPass, sizeof(regPass), ImGuiInputTextFlags_Password);
        ImGui::Text("Sifre (Tekrar)");
        ImGui::SetNextItemWidth(-1);
        ImGui::InputText("##rp2", regPass2, sizeof(regPass2), ImGuiInputTextFlags_Password);
        ImGui::Dummy(ImVec2(0, 8));

        if (ImGui::Button("Kayit Ol", ImVec2(-1, 40))) {
            if (strcmp(regPass, regPass2) != 0) {
                loginError = "Sifreler uyusmuyor!";
            } else if (auth.registerUser(regUser, regPass, UserRole::USER, regDisplayName)) {
                store.createMailbox(regUser);
                loginSuccess = std::string("Kayit basarili: ") + regUser + "@" + domain;
                showRegister = false;
                clearLoginFields();
            } else {
                loginError = "Kullanici adi zaten alinmis!";
            }
        }
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.118f, 0.133f, 0.176f, 1.0f));
        if (ImGui::Button("Geri", ImVec2(-1, 36))) {
            showRegister = false; loginError.clear();
        }
        ImGui::PopStyleColor();
        if (!loginError.empty()) ImGui::TextColored(COL_RED2, "%s", loginError.c_str());
    }
    if (!loginSuccess.empty()) ImGui::TextColored(COL_GREEN2, "%s", loginSuccess.c_str());

    ImGui::EndChild();
    ImGui::End();
}

void GuiApp::renderSidebar() {
    ImGui::BeginChild("##Sidebar", ImVec2(220, -1), ImGuiChildFlags_Borders);

    ImGui::Dummy(ImVec2(0, 4));
    ImGui::TextColored(COL_ACCENT2, "  ULUSAN SIGORTA");
    ImGui::TextColored(COL_DIMTEXT2, "  Mail Sunucusu v1.0");
    ImGui::Dummy(ImVec2(0, 4));
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0, 4));

    std::string userLabel = "  " + displayName;
    ImGui::Text("%s", userLabel.c_str());
    std::string emailLabel = "  " + currentUser + "@" + domain;
    ImGui::TextColored(COL_DIMTEXT2, "%s", emailLabel.c_str());
    if (currentRole == UserRole::ADMIN) ImGui::TextColored(COL_PURPLE2, "  [ADMIN]");
    ImGui::Dummy(ImVec2(0, 8));
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0, 8));

    int inboxCount = store.getMailCount(currentUser, false);
    int sentCount = store.getMailCount(currentUser, true);

    auto navBtn = [&](const char* label, Screen scr) {
        bool active = (currentScreen == scr);
        if (active) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.231f, 0.510f, 0.965f, 0.3f));
            ImGui::PushStyleColor(ImGuiCol_Text, COL_ACCENT2);
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.886f, 0.910f, 0.941f, 1.0f));
        }
        if (ImGui::Button(label, ImVec2(-1, 32))) {
            currentScreen = scr;
            if (scr == SCREEN_COMPOSE) clearComposeFields();
        }
        ImGui::PopStyleColor(2);
    };

    navBtn("  Dashboard", SCREEN_DASHBOARD);
    char inboxLabel[64]; snprintf(inboxLabel, 64, "  Gelen Kutusu (%d)", inboxCount);
    navBtn(inboxLabel, SCREEN_INBOX);
    char sentLabel[64]; snprintf(sentLabel, 64, "  Gonderilenler (%d)", sentCount);
    navBtn(sentLabel, SCREEN_SENT);
    navBtn("  Yeni E-posta", SCREEN_COMPOSE);

    if (currentRole == UserRole::ADMIN) {
        ImGui::Dummy(ImVec2(0, 8));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0, 4));
        ImGui::TextColored(COL_DIMTEXT2, "  YONETIM");
        ImGui::Dummy(ImVec2(0, 4));
        navBtn("  Kullanicilar", SCREEN_ADMIN_USERS);
        navBtn("  Sunucu Loglari", SCREEN_ADMIN_LOGS);
    }

    ImGui::Dummy(ImVec2(0, 8));
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0, 4));
    navBtn("  Ayarlar", SCREEN_SETTINGS);

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_Text, COL_RED2);
    if (ImGui::Button("  Cikis Yap", ImVec2(-1, 32))) {
        loggedIn = false;
        currentUser.clear();
        currentScreen = SCREEN_LOGIN;
        clearLoginFields();
    }
    ImGui::PopStyleColor(2);

    // Server status
    ImGui::Dummy(ImVec2(0, 8));
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0, 4));
    bool smtpOk = smtp.isRunning();
    bool imapOk = imap.isRunning();
    ImGui::TextColored(smtpOk ? COL_GREEN2 : COL_RED2, "  %s SMTP :587", smtpOk ? "[+]" : "[-]");
    ImGui::TextColored(imapOk ? COL_GREEN2 : COL_RED2, "  %s IMAP :143", imapOk ? "[+]" : "[-]");

    ImGui::EndChild();
}

#include "gui_app.h"
#include "imgui.h"
#include "logger.h"
#include "utils.h"
#include <cstring>
#include <algorithm>

// Renkler
static const ImVec4 COL_ACCENT  = ImVec4(0.231f, 0.510f, 0.965f, 1.0f);
static const ImVec4 COL_GREEN   = ImVec4(0.063f, 0.725f, 0.506f, 1.0f);
static const ImVec4 COL_RED     = ImVec4(0.937f, 0.267f, 0.267f, 1.0f);
static const ImVec4 COL_ORANGE  = ImVec4(0.961f, 0.620f, 0.043f, 1.0f);
static const ImVec4 COL_DIMTEXT = ImVec4(0.392f, 0.455f, 0.545f, 1.0f);
static const ImVec4 COL_CARD    = ImVec4(0.102f, 0.114f, 0.153f, 1.0f);
static const ImVec4 COL_PURPLE  = ImVec4(0.545f, 0.361f, 0.965f, 1.0f);

GuiApp::GuiApp(AuthManager& a, MailStore& s, SmtpServer& sm, ImapServer& im, const std::string& d)
    : auth(a), store(s), smtp(sm), imap(im), domain(d),
      currentScreen(SCREEN_LOGIN), loggedIn(false), currentRole(UserRole::USER),
      showRegister(false), composeMsgIsError(false), newRoleIdx(1),
      fontRegular(nullptr), fontLarge(nullptr), fontSmall(nullptr) {
    memset(loginUser, 0, sizeof(loginUser));
    memset(loginPass, 0, sizeof(loginPass));
    clearLoginFields();
    clearComposeFields();
    clearAdminFields();
    clearSettingsFields();
    setupTheme();
}

void GuiApp::clearLoginFields() {
    memset(loginUser, 0, sizeof(loginUser));
    memset(loginPass, 0, sizeof(loginPass));
    memset(regUser, 0, sizeof(regUser));
    memset(regPass, 0, sizeof(regPass));
    memset(regPass2, 0, sizeof(regPass2));
    memset(regDisplayName, 0, sizeof(regDisplayName));
    loginError.clear(); loginSuccess.clear();
}

void GuiApp::clearComposeFields() {
    memset(composeTo, 0, sizeof(composeTo));
    memset(composeSubject, 0, sizeof(composeSubject));
    memset(composeBody, 0, sizeof(composeBody));
    composeMsg.clear(); composeMsgIsError = false;
}

void GuiApp::clearAdminFields() {
    memset(newUsername, 0, sizeof(newUsername));
    memset(newPassword, 0, sizeof(newPassword));
    memset(newDisplayName, 0, sizeof(newDisplayName));
    newRoleIdx = 1; adminMsg.clear();
}

void GuiApp::clearSettingsFields() {
    memset(oldPassword, 0, sizeof(oldPassword));
    memset(newPw, 0, sizeof(newPw));
    memset(newPw2, 0, sizeof(newPw2));
    settingsMsg.clear();
}

void GuiApp::setupTheme() {
    ImGuiStyle& st = ImGui::GetStyle();
    st.WindowRounding = 8.0f;
    st.FrameRounding = 6.0f;
    st.ScrollbarRounding = 6.0f;
    st.GrabRounding = 4.0f;
    st.TabRounding = 6.0f;
    st.ChildRounding = 6.0f;
    st.PopupRounding = 6.0f;
    st.WindowPadding = ImVec2(16, 16);
    st.FramePadding = ImVec2(12, 8);
    st.ItemSpacing = ImVec2(10, 8);
    st.ScrollbarSize = 12.0f;
    st.WindowBorderSize = 0.0f;
    st.ChildBorderSize = 1.0f;

    ImVec4* c = st.Colors;
    c[ImGuiCol_WindowBg]          = ImVec4(0.059f, 0.067f, 0.090f, 1.0f);
    c[ImGuiCol_ChildBg]           = ImVec4(0.078f, 0.086f, 0.118f, 1.0f);
    c[ImGuiCol_PopupBg]           = ImVec4(0.102f, 0.114f, 0.153f, 1.0f);
    c[ImGuiCol_Border]            = ImVec4(0.176f, 0.200f, 0.282f, 0.5f);
    c[ImGuiCol_FrameBg]           = ImVec4(0.118f, 0.133f, 0.176f, 1.0f);
    c[ImGuiCol_FrameBgHovered]    = ImVec4(0.157f, 0.176f, 0.235f, 1.0f);
    c[ImGuiCol_FrameBgActive]     = ImVec4(0.196f, 0.220f, 0.294f, 1.0f);
    c[ImGuiCol_TitleBg]           = ImVec4(0.059f, 0.067f, 0.090f, 1.0f);
    c[ImGuiCol_TitleBgActive]     = ImVec4(0.078f, 0.086f, 0.118f, 1.0f);
    c[ImGuiCol_MenuBarBg]         = ImVec4(0.078f, 0.086f, 0.118f, 1.0f);
    c[ImGuiCol_ScrollbarBg]       = ImVec4(0.059f, 0.067f, 0.090f, 1.0f);
    c[ImGuiCol_ScrollbarGrab]     = ImVec4(0.176f, 0.200f, 0.282f, 1.0f);
    c[ImGuiCol_ScrollbarGrabHovered]=ImVec4(0.231f, 0.259f, 0.353f, 1.0f);
    c[ImGuiCol_ScrollbarGrabActive]= ImVec4(0.286f, 0.318f, 0.424f, 1.0f);
    c[ImGuiCol_CheckMark]         = COL_ACCENT;
    c[ImGuiCol_SliderGrab]        = COL_ACCENT;
    c[ImGuiCol_Button]            = ImVec4(0.231f, 0.510f, 0.965f, 0.8f);
    c[ImGuiCol_ButtonHovered]     = ImVec4(0.231f, 0.510f, 0.965f, 1.0f);
    c[ImGuiCol_ButtonActive]      = ImVec4(0.180f, 0.420f, 0.900f, 1.0f);
    c[ImGuiCol_Header]            = ImVec4(0.157f, 0.176f, 0.235f, 1.0f);
    c[ImGuiCol_HeaderHovered]     = ImVec4(0.196f, 0.220f, 0.294f, 1.0f);
    c[ImGuiCol_HeaderActive]      = ImVec4(0.231f, 0.510f, 0.965f, 0.6f);
    c[ImGuiCol_Separator]         = ImVec4(0.176f, 0.200f, 0.282f, 0.5f);
    c[ImGuiCol_Tab]               = ImVec4(0.118f, 0.133f, 0.176f, 1.0f);
    c[ImGuiCol_TabHovered]        = COL_ACCENT;
    c[ImGuiCol_TabSelected]       = ImVec4(0.196f, 0.220f, 0.294f, 1.0f);
    c[ImGuiCol_TableHeaderBg]     = ImVec4(0.118f, 0.133f, 0.176f, 1.0f);
    c[ImGuiCol_TableBorderStrong] = ImVec4(0.176f, 0.200f, 0.282f, 0.5f);
    c[ImGuiCol_TableBorderLight]  = ImVec4(0.176f, 0.200f, 0.282f, 0.3f);
    c[ImGuiCol_TableRowBg]        = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    c[ImGuiCol_TableRowBgAlt]     = ImVec4(1.0f, 1.0f, 1.0f, 0.02f);
    c[ImGuiCol_Text]              = ImVec4(0.886f, 0.910f, 0.941f, 1.0f);
    c[ImGuiCol_TextDisabled]      = COL_DIMTEXT;
}

void GuiApp::render() {
    if (!loggedIn) { renderLogin(); return; }
    ImGuiViewport* vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(vp->WorkPos);
    ImGui::SetNextWindowSize(vp->WorkSize);
    ImGui::Begin("##Main", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus);
    renderSidebar();
    ImGui::SameLine();
    ImGui::BeginGroup();
    float cw = ImGui::GetContentRegionAvail().x;
    float ch = ImGui::GetContentRegionAvail().y;
    ImGui::BeginChild("##Content", ImVec2(cw, ch), ImGuiChildFlags_None);
    switch (currentScreen) {
        case SCREEN_DASHBOARD:   renderDashboard(); break;
        case SCREEN_INBOX:       renderInbox(); break;
        case SCREEN_SENT:        renderSent(); break;
        case SCREEN_COMPOSE:     renderCompose(); break;
        case SCREEN_MAIL_VIEW:   renderMailView(); break;
        case SCREEN_ADMIN_USERS: renderAdminUsers(); break;
        case SCREEN_ADMIN_LOGS:  renderAdminLogs(); break;
        case SCREEN_SETTINGS:    renderSettings(); break;
        default: renderDashboard(); break;
    }
    ImGui::EndChild();
    ImGui::EndGroup();
    ImGui::End();
}

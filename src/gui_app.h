#ifndef GUI_APP_H
#define GUI_APP_H

#include <string>
#include <vector>
#include "auth.h"
#include "mail_store.h"
#include "smtp_server.h"
#include "imap_server.h"

struct ImFont;

/*
 * GuiApp - Dear ImGui Masaustu Arayuz Sinifi
 * Tum ekranlarin renderini ve kullanici etkilesimlerini yonetir.
 * Backend siniflarini (AuthManager, MailStore, vb.) dogrudan kullanir.
 */

class GuiApp {
public:
    GuiApp(AuthManager& auth, MailStore& store, SmtpServer& smtp, ImapServer& imap,
           const std::string& domain);

    // Her frame cagrilir
    void render();

    bool isLoggedIn() const { return loggedIn; }

private:
    // Ekran durumlari
    enum Screen {
        SCREEN_LOGIN,
        SCREEN_DASHBOARD,
        SCREEN_INBOX,
        SCREEN_SENT,
        SCREEN_COMPOSE,
        SCREEN_MAIL_VIEW,
        SCREEN_ADMIN_USERS,
        SCREEN_ADMIN_LOGS,
        SCREEN_SETTINGS
    };

    Screen currentScreen;

    // Backend referanslari
    AuthManager& auth;
    MailStore& store;
    SmtpServer& smtp;
    ImapServer& imap;
    std::string domain;

    // Kullanici durumu
    bool loggedIn;
    std::string currentUser;
    UserRole currentRole;
    std::string displayName;

    // Login input
    char loginUser[128];
    char loginPass[128];
    char regUser[128];
    char regPass[128];
    char regPass2[128];
    char regDisplayName[128];
    bool showRegister;
    std::string loginError;
    std::string loginSuccess;

    // Compose input
    char composeTo[128];
    char composeSubject[256];
    char composeBody[4096];
    std::string composeMsg;
    bool composeMsgIsError;

    // Mail view
    Mail viewingMail;

    // Admin - yeni kullanici
    char newUsername[128];
    char newPassword[128];
    char newDisplayName[128];
    int newRoleIdx;
    std::string adminMsg;

    // Settings
    char oldPassword[128];
    char newPw[128];
    char newPw2[128];
    std::string settingsMsg;

    // Fontlar
    ImFont* fontRegular;
    ImFont* fontLarge;
    ImFont* fontSmall;

    // Render fonksiyonlari
    void renderLogin();
    void renderSidebar();
    void renderDashboard();
    void renderInbox();
    void renderSent();
    void renderCompose();
    void renderMailView();
    void renderAdminUsers();
    void renderAdminLogs();
    void renderSettings();

    // Yardimci
    void setupTheme();
    void clearLoginFields();
    void clearComposeFields();
    void clearAdminFields();
    void clearSettingsFields();
};

#endif // GUI_APP_H

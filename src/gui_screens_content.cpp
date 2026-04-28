#include "gui_app.h"
#include "imgui.h"
#include "logger.h"
#include "utils.h"
#include <cstring>

static const ImVec4 C_ACC  = ImVec4(0.231f,0.510f,0.965f,1.0f);
static const ImVec4 C_GRN  = ImVec4(0.063f,0.725f,0.506f,1.0f);
static const ImVec4 C_RED  = ImVec4(0.937f,0.267f,0.267f,1.0f);
static const ImVec4 C_ORG  = ImVec4(0.961f,0.620f,0.043f,1.0f);
static const ImVec4 C_DIM  = ImVec4(0.392f,0.455f,0.545f,1.0f);
static const ImVec4 C_PUR  = ImVec4(0.545f,0.361f,0.965f,1.0f);
static const ImVec4 C_CARD = ImVec4(0.102f,0.114f,0.153f,1.0f);

static void StatCard(const char* title, int value, ImVec4 col, float w) {
    ImGui::PushStyleColor(ImGuiCol_ChildBg, C_CARD);
    ImGui::BeginChild(title, ImVec2(w, 90), ImGuiChildFlags_Borders);
    ImGui::Dummy(ImVec2(0,8));
    ImGui::SetCursorPosX(16);
    ImGui::TextColored(C_DIM, "%s", title);
    ImGui::SetCursorPosX(16);
    char buf[32]; snprintf(buf, 32, "%d", value);
    ImGui::TextColored(col, "%s", buf);
    ImGui::EndChild();
    ImGui::PopStyleColor();
}

void GuiApp::renderDashboard() {
    ImGui::TextColored(C_ACC, "Dashboard");
    ImGui::Dummy(ImVec2(0,4));
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0,12));

    int ic = store.getMailCount(currentUser, false);
    int sc = store.getMailCount(currentUser, true);
    int uc = auth.getUserCount();
    float cw = (ImGui::GetContentRegionAvail().x - 30) / 4.0f;

    StatCard("Gelen Kutusu", ic, C_ACC, cw);
    ImGui::SameLine(0, 10);
    StatCard("Gonderilenler", sc, C_GRN, cw);
    ImGui::SameLine(0, 10);
    StatCard("Kullanicilar", uc, C_PUR, cw);
    ImGui::SameLine(0, 10);
    long long mbSize = store.getMailboxSize(currentUser);
    ImGui::PushStyleColor(ImGuiCol_ChildBg, C_CARD);
    ImGui::BeginChild("DiskKul", ImVec2(cw, 90), ImGuiChildFlags_Borders);
    ImGui::Dummy(ImVec2(0,8));
    ImGui::SetCursorPosX(16);
    ImGui::TextColored(C_DIM, "Disk Kullanimi");
    ImGui::SetCursorPosX(16);
    char szBuf[32];
    if (mbSize < 1024) snprintf(szBuf, 32, "%lld B", mbSize);
    else if (mbSize < 1048576) snprintf(szBuf, 32, "%.1f KB", mbSize/1024.0);
    else snprintf(szBuf, 32, "%.1f MB", mbSize/1048576.0);
    ImGui::TextColored(C_ORG, "%s", szBuf);
    ImGui::EndChild();
    ImGui::PopStyleColor();

    ImGui::Dummy(ImVec2(0,16));
    ImGui::TextColored(C_DIM, "Sunucu Durumu");
    ImGui::Dummy(ImVec2(0,4));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, C_CARD);
    ImGui::BeginChild("SrvStat", ImVec2(-1, 80), ImGuiChildFlags_Borders);
    ImGui::Dummy(ImVec2(0,8));
    bool smOk = smtp.isRunning(), imOk = imap.isRunning();
    ImGui::SetCursorPosX(16);
    ImGui::TextColored(smOk?C_GRN:C_RED, "[%s] SMTP Sunucusu  -  Port 587  -  %s", smOk?"+":"-", smOk?"AKTIF":"KAPALI");
    ImGui::SetCursorPosX(16);
    ImGui::TextColored(imOk?C_GRN:C_RED, "[%s] IMAP Sunucusu  -  Port 143  -  %s", imOk?"+":"-", imOk?"AKTIF":"KAPALI");
    ImGui::EndChild();
    ImGui::PopStyleColor();

    ImGui::Dummy(ImVec2(0,16));
    ImGui::TextColored(C_DIM, "Hesap Bilgileri");
    ImGui::Dummy(ImVec2(0,4));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, C_CARD);
    ImGui::BeginChild("AccInfo", ImVec2(-1, 100), ImGuiChildFlags_Borders);
    ImGui::Dummy(ImVec2(0,8));
    ImGui::SetCursorPosX(16);
    ImGui::Text("Kullanici  : %s", currentUser.c_str());
    ImGui::SetCursorPosX(16);
    ImGui::Text("E-posta    : %s@%s", currentUser.c_str(), domain.c_str());
    ImGui::SetCursorPosX(16);
    ImGui::Text("Gorunen Ad : %s", displayName.c_str());
    ImGui::SetCursorPosX(16);
    ImGui::Text("Rol        : %s", currentRole==UserRole::ADMIN?"ADMIN":"USER");
    ImGui::EndChild();
    ImGui::PopStyleColor();
}

void GuiApp::renderInbox() {
    ImGui::TextColored(C_ACC, "Gelen Kutusu");
    ImGui::Dummy(ImVec2(0,4));
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0,8));

    auto mails = store.loadInbox(currentUser);
    if (mails.empty()) {
        ImGui::TextColored(C_DIM, "Gelen kutunuz bos.");
        return;
    }
    ImGui::Text("%d e-posta", (int)mails.size());
    ImGui::Dummy(ImVec2(0,4));

    if (ImGui::BeginTable("inbox_tbl", 4, ImGuiTableFlags_Borders|ImGuiTableFlags_RowBg|ImGuiTableFlags_Resizable)) {
        ImGui::TableSetupColumn("No", ImGuiTableColumnFlags_WidthFixed, 40);
        ImGui::TableSetupColumn("Gonderen", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Konu", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Islem", ImGuiTableColumnFlags_WidthFixed, 120);
        ImGui::TableHeadersRow();

        for (int i = 0; i < (int)mails.size(); i++) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn(); ImGui::Text("%d", i+1);
            ImGui::TableNextColumn(); ImGui::Text("%s", mails[i].from.c_str());
            ImGui::TableNextColumn(); ImGui::Text("%s", mails[i].subject.c_str());
            ImGui::TableNextColumn();
            ImGui::PushID(i);
            if (ImGui::SmallButton("Oku")) {
                viewingMail = mails[i];
                currentScreen = SCREEN_MAIL_VIEW;
            }
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.937f,0.267f,0.267f,0.7f));
            if (ImGui::SmallButton("Sil")) {
                store.deleteMail(currentUser, i, false);
            }
            ImGui::PopStyleColor();
            ImGui::PopID();
        }
        ImGui::EndTable();
    }
}

void GuiApp::renderSent() {
    ImGui::TextColored(C_ACC, "Gonderilenler");
    ImGui::Dummy(ImVec2(0,4));
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0,8));

    auto mails = store.loadSent(currentUser);
    if (mails.empty()) {
        ImGui::TextColored(C_DIM, "Gonderilen kutunuz bos.");
        return;
    }
    ImGui::Text("%d e-posta", (int)mails.size());
    ImGui::Dummy(ImVec2(0,4));

    if (ImGui::BeginTable("sent_tbl", 4, ImGuiTableFlags_Borders|ImGuiTableFlags_RowBg|ImGuiTableFlags_Resizable)) {
        ImGui::TableSetupColumn("No", ImGuiTableColumnFlags_WidthFixed, 40);
        ImGui::TableSetupColumn("Alici", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Konu", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Islem", ImGuiTableColumnFlags_WidthFixed, 60);
        ImGui::TableHeadersRow();
        for (int i = 0; i < (int)mails.size(); i++) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn(); ImGui::Text("%d", i+1);
            ImGui::TableNextColumn(); ImGui::Text("%s", mails[i].to.c_str());
            ImGui::TableNextColumn(); ImGui::Text("%s", mails[i].subject.c_str());
            ImGui::TableNextColumn();
            ImGui::PushID(1000+i);
            if (ImGui::SmallButton("Oku")) {
                viewingMail = mails[i];
                currentScreen = SCREEN_MAIL_VIEW;
            }
            ImGui::PopID();
        }
        ImGui::EndTable();
    }
}

void GuiApp::renderCompose() {
    ImGui::TextColored(C_ACC, "Yeni E-posta");
    ImGui::Dummy(ImVec2(0,4));
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0,8));

    ImGui::Text("Alici (kullanici adi)");
    ImGui::SetNextItemWidth(400);
    ImGui::InputText("##cto", composeTo, sizeof(composeTo));
    ImGui::Dummy(ImVec2(0,4));
    ImGui::Text("Konu");
    ImGui::SetNextItemWidth(-1);
    ImGui::InputText("##csub", composeSubject, sizeof(composeSubject));
    ImGui::Dummy(ImVec2(0,4));
    ImGui::Text("Mesaj");
    ImGui::InputTextMultiline("##cbody", composeBody, sizeof(composeBody), ImVec2(-1, 250));
    ImGui::Dummy(ImVec2(0,8));

    if (ImGui::Button("Gonder", ImVec2(160, 40))) {
        std::string toUser = utils::trim(composeTo);
        if (toUser.empty()) { composeMsg = "Alici bos olamaz!"; composeMsgIsError = true; }
        else if (!auth.userExists(toUser)) { composeMsg = "Kullanici bulunamadi: " + toUser; composeMsgIsError = true; }
        else {
            Mail mail;
            mail.messageId = utils::generateMessageId();
            mail.from = currentUser + "@" + domain;
            mail.to = toUser + "@" + domain;
            mail.subject = composeSubject;
            mail.body = composeBody;
            mail.date = utils::getRFC2822Date();
            store.saveMail(toUser, mail, false);
            store.saveMail(currentUser, mail, true);
            composeMsg = "E-posta basariyla gonderildi!";
            composeMsgIsError = false;
            LOG_INFO("GUI: Mail gonderildi: " + currentUser + " -> " + toUser);
            memset(composeTo, 0, sizeof(composeTo));
            memset(composeSubject, 0, sizeof(composeSubject));
            memset(composeBody, 0, sizeof(composeBody));
        }
    }
    if (!composeMsg.empty()) {
        ImGui::Dummy(ImVec2(0,4));
        ImGui::TextColored(composeMsgIsError ? C_RED : C_GRN, "%s", composeMsg.c_str());
    }
}

void GuiApp::renderMailView() {
    if (ImGui::Button("<< Geri", ImVec2(100, 30))) {
        currentScreen = SCREEN_INBOX;
        return;
    }
    ImGui::Dummy(ImVec2(0,4));
    ImGui::TextColored(C_ACC, "%s", viewingMail.subject.c_str());
    ImGui::Dummy(ImVec2(0,4));
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0,8));

    ImGui::PushStyleColor(ImGuiCol_ChildBg, C_CARD);
    ImGui::BeginChild("##mailmeta", ImVec2(-1, 100), ImGuiChildFlags_Borders);
    ImGui::Dummy(ImVec2(0,4));
    ImGui::SetCursorPosX(16);
    ImGui::Text("Gonderen  : %s", viewingMail.from.c_str());
    ImGui::SetCursorPosX(16);
    ImGui::Text("Alici     : %s", viewingMail.to.c_str());
    ImGui::SetCursorPosX(16);
    ImGui::Text("Tarih     : %s", viewingMail.date.c_str());
    ImGui::SetCursorPosX(16);
    ImGui::TextColored(C_DIM, "ID: %s", viewingMail.messageId.c_str());
    ImGui::EndChild();
    ImGui::PopStyleColor();

    ImGui::Dummy(ImVec2(0,8));
    ImGui::TextColored(C_DIM, "Mesaj Icerigi:");
    ImGui::Dummy(ImVec2(0,4));
    ImGui::BeginChild("##mailbody", ImVec2(-1, -1), ImGuiChildFlags_Borders);
    ImGui::TextWrapped("%s", viewingMail.body.c_str());
    ImGui::EndChild();
}

void GuiApp::renderAdminUsers() {
    ImGui::TextColored(C_ACC, "Kullanici Yonetimi");
    ImGui::Dummy(ImVec2(0,4));
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0,8));

    auto users = auth.listUsers();
    ImGui::Text("Kayitli Kullanicilar: %d", (int)users.size());
    ImGui::Dummy(ImVec2(0,4));

    if (ImGui::BeginTable("usr_tbl", 5, ImGuiTableFlags_Borders|ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("No", ImGuiTableColumnFlags_WidthFixed, 40);
        ImGui::TableSetupColumn("Kullanici", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("E-posta", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Rol", ImGuiTableColumnFlags_WidthFixed, 60);
        ImGui::TableSetupColumn("Islem", ImGuiTableColumnFlags_WidthFixed, 60);
        ImGui::TableHeadersRow();
        for (int i = 0; i < (int)users.size(); i++) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn(); ImGui::Text("%d", i+1);
            ImGui::TableNextColumn(); ImGui::Text("%s", users[i].username.c_str());
            ImGui::TableNextColumn(); ImGui::Text("%s", users[i].email.c_str());
            ImGui::TableNextColumn();
            ImGui::TextColored(users[i].role==UserRole::ADMIN ? C_PUR : C_DIM,
                             "%s", users[i].role==UserRole::ADMIN?"ADMIN":"USER");
            ImGui::TableNextColumn();
            ImGui::PushID(2000+i);
            if (users[i].username != currentUser) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.937f,0.267f,0.267f,0.7f));
                if (ImGui::SmallButton("Sil")) {
                    auth.deleteUser(users[i].username);
                    adminMsg = users[i].username + " silindi.";
                }
                ImGui::PopStyleColor();
            }
            ImGui::PopID();
        }
        ImGui::EndTable();
    }

    ImGui::Dummy(ImVec2(0,16));
    ImGui::TextColored(C_DIM, "Yeni Kullanici Ekle");
    ImGui::Dummy(ImVec2(0,4));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, C_CARD);
    ImGui::BeginChild("##adduser", ImVec2(-1, 180), ImGuiChildFlags_Borders);
    ImGui::Dummy(ImVec2(0,4));
    ImGui::Text("Kullanici Adi"); ImGui::SameLine(160);
    ImGui::SetNextItemWidth(200); ImGui::InputText("##nu", newUsername, sizeof(newUsername));
    ImGui::Text("Sifre"); ImGui::SameLine(160);
    ImGui::SetNextItemWidth(200); ImGui::InputText("##np", newPassword, sizeof(newPassword), ImGuiInputTextFlags_Password);
    ImGui::Text("Gorunen Ad"); ImGui::SameLine(160);
    ImGui::SetNextItemWidth(200); ImGui::InputText("##nd", newDisplayName, sizeof(newDisplayName));
    ImGui::Text("Rol"); ImGui::SameLine(160);
    const char* roles[] = {"ADMIN", "USER"};
    ImGui::SetNextItemWidth(200); ImGui::Combo("##nr", &newRoleIdx, roles, 2);
    ImGui::Dummy(ImVec2(0,4));
    if (ImGui::Button("Kullanici Ekle", ImVec2(200, 36))) {
        UserRole r = (newRoleIdx == 0) ? UserRole::ADMIN : UserRole::USER;
        if (auth.registerUser(newUsername, newPassword, r, newDisplayName)) {
            store.createMailbox(newUsername);
            adminMsg = std::string("Kullanici olusturuldu: ") + newUsername;
            clearAdminFields();
        } else {
            adminMsg = "Hata: Kullanici olusturulamadi!";
        }
    }
    ImGui::EndChild();
    ImGui::PopStyleColor();
    if (!adminMsg.empty()) ImGui::TextColored(C_GRN, "%s", adminMsg.c_str());
}

void GuiApp::renderAdminLogs() {
    ImGui::TextColored(C_ACC, "Sunucu Loglari");
    ImGui::Dummy(ImVec2(0,4));
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0,8));

    std::string logs = Logger::getInstance().readLogs(50);
    ImGui::BeginChild("##logs", ImVec2(-1, -1), ImGuiChildFlags_Borders);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.8f, 0.7f, 1.0f));
    ImGui::TextUnformatted(logs.c_str());
    ImGui::PopStyleColor();
    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) ImGui::SetScrollHereY(1.0f);
    ImGui::EndChild();
}

void GuiApp::renderSettings() {
    ImGui::TextColored(C_ACC, "Ayarlar");
    ImGui::Dummy(ImVec2(0,4));
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0,12));

    ImGui::TextColored(C_DIM, "Sifre Degistir");
    ImGui::Dummy(ImVec2(0,4));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, C_CARD);
    ImGui::BeginChild("##pwcard", ImVec2(450, 220), ImGuiChildFlags_Borders);
    ImGui::Dummy(ImVec2(0,8));
    ImGui::Text("Mevcut Sifre");
    ImGui::SetNextItemWidth(-16); ImGui::InputText("##op", oldPassword, sizeof(oldPassword), ImGuiInputTextFlags_Password);
    ImGui::Text("Yeni Sifre");
    ImGui::SetNextItemWidth(-16); ImGui::InputText("##np1", newPw, sizeof(newPw), ImGuiInputTextFlags_Password);
    ImGui::Text("Yeni Sifre (Tekrar)");
    ImGui::SetNextItemWidth(-16); ImGui::InputText("##np2", newPw2, sizeof(newPw2), ImGuiInputTextFlags_Password);
    ImGui::Dummy(ImVec2(0,8));
    if (ImGui::Button("Sifreyi Degistir", ImVec2(-16, 36))) {
        if (strcmp(newPw, newPw2) != 0) {
            settingsMsg = "Sifreler uyusmuyor!";
        } else if (auth.changePassword(currentUser, oldPassword, newPw)) {
            settingsMsg = "Sifre basariyla degistirildi!";
            clearSettingsFields();
        } else {
            settingsMsg = "Mevcut sifre yanlis!";
        }
    }
    ImGui::EndChild();
    ImGui::PopStyleColor();
    if (!settingsMsg.empty()) {
        ImGui::Dummy(ImVec2(0,4));
        ImGui::TextColored(settingsMsg.find("basari") != std::string::npos ? C_GRN : C_RED,
                         "%s", settingsMsg.c_str());
    }
}

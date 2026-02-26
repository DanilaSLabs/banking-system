#include "AppSession.h"
#include "imgui.h"
#include "imgui_stdlib.h"

void DrawForgot(AppSession& S) {
    ImGui::SeparatorText("Reset secret word");

    ImGui::InputText("Customer ID", &S.fId);
    ImGui::InputText("Email", &S.fEmail);
    ImGui::InputText("New secret word", &S.fNewSecret, ImGuiInputTextFlags_Password);

    if (ImGui::Button("Reset")) {
        if (!AppSession::validateID(S.fId) || !AppSession::validateEmail(S.fEmail) || S.fNewSecret.empty()) {
            S.fMsg = "Please fill fields correctly.";
        } else if (S.db.resetSecretWithEmail(S.fId, S.fEmail, S.fNewSecret)) {
            S.fMsg = "Secret word reset successful.";
        } else {
            S.fMsg = "Reset failed. Check ID / email.";
        }
    }

    if (!S.fMsg.empty()) ImGui::TextWrapped("%s", S.fMsg.c_str());
    if (ImGui::Button("Back")) S.page = Page::MainMenu;
}

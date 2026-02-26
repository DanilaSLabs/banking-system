#include "AppSession.h"
#include "imgui.h"
#include "imgui_stdlib.h"

void DrawLogin(AppSession& S) {
    ImGui::SeparatorText("Login");

    ImGui::InputTextWithHint("Customer ID", "digits only", &S.loginId);
    ImGui::InputTextWithHint("Secret word", "password", &S.loginSecret, ImGuiInputTextFlags_Password);
    ImGui::InputTextWithHint("Phone", "+357...", &S.loginPhone);

    if (ImGui::Button("Login")) {
        S.loginError.clear();

        if (!AppSession::validateID(S.loginId)) {
            S.loginError = "Invalid ID format.";
        } else if (!S.db.customerExists(S.loginId)) {
            S.loginError = "Customer not found.";
        } else if (S.loginSecret.empty()) {
            S.loginError = "Enter secret word.";
        } else if (!AppSession::validatePhone(S.loginPhone)) {
            S.loginError = "Invalid phone format.";
        } else if (!S.db.verifySecret(S.loginId, S.loginSecret)) {
            S.loginError = "Secret word mismatch.";
        } else if (!S.db.verifyPhone(S.loginId, S.loginPhone)) {
            S.loginError = "Phone mismatch.";
        } else {
            Customer loaded;
            if (!S.db.loadCustomer(S.loginId, loaded)) {
                S.loginError = "Failed to load profile.";
            } else {
                S.applySavingsInterestIfNeeded(loaded);
                S.db.addOrUpdateCustomer(loaded);
                S.current = loaded;

                S.tab = AppTab::Home;
                S.hideBalances = true;

                S.page = Page::Dashboard;
                S.ShowToast("Welcome back, " + S.current.getFullName() + "!");
            }
        }
    }

    if (!S.loginError.empty()) {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1,0.3f,0.3f,1), "%s", S.loginError.c_str());
    }

    if (ImGui::Button("Back")) S.page = Page::MainMenu;
}

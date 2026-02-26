#include "AppSession.h"
#include "imgui.h"
#include "imgui_stdlib.h"
#include <algorithm>

void DrawCreate(AppSession& S) {
    constexpr double DEFAULT_SAVINGS_RATE = 0.15;

    ImGui::SeparatorText("Create profile");

    ImGui::InputText("First name", &S.cFirstName);
    ImGui::InputText("Last name",  &S.cLastName);
    ImGui::InputInt("Age", &S.cAge);
    ImGui::InputText("Email", &S.cEmail);
    ImGui::InputText("Phone", &S.cPhone);
    ImGui::InputText("ID", &S.cId);
    ImGui::InputText("Secret word", &S.cSecret, ImGuiInputTextFlags_Password);

    ImGui::SeparatorText("Accounts");
    ImGui::RadioButton("1 account (Checking)", &S.cOpenCount, 1); ImGui::SameLine();
    ImGui::RadioButton("2 accounts (Checking + Savings)", &S.cOpenCount, 2);

    if (ImGui::Button("Create")) {
        if (S.cFirstName.empty() || S.cLastName.empty() ||
            !AppSession::validateEmail(S.cEmail) ||
            !AppSession::validatePhone(S.cPhone) ||
            !AppSession::validateID(S.cId) ||
            S.cSecret.empty()) {
            S.ShowToast("Please fill all fields correctly.");
            return;
        }

        if (S.cAge < 0) S.cAge = 0;
        if (S.cAge > 130) { S.ShowToast("Age looks incorrect."); return; }

        if (S.db.customerExists(S.cId)) {
            S.loginId = S.cId;
            S.loginPhone = S.cPhone;
            S.page = Page::Login;
            S.ShowToast("ID already exists. Please log in.");
            return;
        }

        Customer cust(S.cFirstName, S.cLastName, std::max(0, S.cAge),
                      S.cEmail, S.cId, S.cSecret, S.cPhone);

        // Checking
        {
            int accNum = S.db.generateUniqueAccountId();
            Account acc(accNum, "Checking", 0.0);
            cust.addAccount(acc);
        }

        // Savings (optional)
        if (S.cOpenCount == 2) {
            int acc2 = S.db.generateUniqueAccountId();
            Account sav(acc2, "Savings", 0.0);
            sav.setSavingsRate(DEFAULT_SAVINGS_RATE);
            sav.setLastSavedDate(AppSession::todayDate());
            cust.addAccount(sav);
        }

        S.db.addOrUpdateCustomer(cust);
        S.current = cust;
        S.tab = AppTab::Home;
        S.hideBalances = true;
        S.page = Page::Dashboard;
        S.ShowToast("Account created successfully.");
    }

    if (ImGui::Button("Back")) S.page = Page::MainMenu;
}

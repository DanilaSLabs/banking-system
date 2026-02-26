#include "AppSession.h"
#include "imgui.h"

void DrawMainMenu(AppSession& S) {
    ImGui::SeparatorText("ABC Banking");
    ImGui::TextWrapped("Welcome to the ABC Banking demo application.");

    if (ImGui::Button("Log in")) S.page = Page::Login;
    ImGui::SameLine();
    if (ImGui::Button("Create an account")) S.page = Page::Create;
    ImGui::SameLine();
    if (ImGui::Button("Forgot password")) S.page = Page::Forgot;

    ImGui::SeparatorText("Storage");
    ImGui::Text("DB file: data/database.json");
}

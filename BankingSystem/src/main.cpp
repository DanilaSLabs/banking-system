// main.cpp — тонкий запуск приложения + цикл ImGui
#include <iostream>
#include <string>

#include "AppSession.h"

// GLFW + ImGui
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#if __APPLE__
#  include <OpenGL/gl3.h>
#else
#  include <GL/gl.h>
#endif

// Экранные функции
void DrawMainMenu(AppSession& S);
void DrawLogin(AppSession& S);
void DrawCreate(AppSession& S);
void DrawForgot(AppSession& S);
void DrawDashboard(AppSession& S);

int main() {
    // --- GLFW ---
    if (!glfwInit()) return 1;
#if __APPLE__
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    GLFWwindow* window = glfwCreateWindow(1200, 800, "ABC Banking (ImGui)", NULL, NULL);
    if (!window) { glfwTerminate(); return 1; }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // --- ImGui ---
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 150");

    AppSession S; // всё состояние приложения

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // ===== Верхняя глобальная панель (всегда видна) =====
        if (ImGui::BeginMainMenuBar()) {
            // Если пользователь залогинен — показываем его и Logout
            // (проверка через непустой ID текущего пользователя)
            if (!S.current.getId().empty()) {
                ImGui::Text("User: %s (ID: %s)",
                            S.current.getFullName().c_str(),
                            S.current.getId().c_str());

                // прижимаем кнопку к правому краю
                float right = ImGui::GetIO().DisplaySize.x;
                float btn_w = 70.0f; // примерно под "Logout"
                float spacing = 12.0f;
                ImGui::SameLine(std::max(0.0f, right - btn_w - spacing));
                if (ImGui::Button("Logout")) {
                    // сохраняем изменения и сбрасываем сессию
                    S.db.addOrUpdateCustomer(S.current);
                    AppSession fresh;
                    S = fresh;
                    S.ShowToast("Logged out");
                }
            } else {
                ImGui::TextUnformatted("ABC Banking");
            }
            ImGui::EndMainMenuBar();
        }

        // ===== Главное окно: фиксируем позицию/размер ниже верхней панели =====
        const ImGuiViewport* vp = ImGui::GetMainViewport();
        float topbar_h = ImGui::GetFrameHeight(); // высота верхней панели

        ImGui::SetNextWindowPos(ImVec2(vp->Pos.x, vp->Pos.y + topbar_h), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(vp->Size.x, vp->Size.y - topbar_h), ImGuiCond_Always);

        ImGuiWindowFlags mainFlags =
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoCollapse;

        ImGui::Begin("Banking System", nullptr, mainFlags);

        switch (S.page) {
            case Page::MainMenu:  DrawMainMenu(S);  break;
            case Page::Login:     DrawLogin(S);     break;
            case Page::Create:    DrawCreate(S);    break;
            case Page::Forgot:    DrawForgot(S);    break;
            case Page::Dashboard: DrawDashboard(S); break;
        }

        ImGui::End();

        // ===== Рендер =====
        ImGui::Render();
        int w, h; glfwGetFramebufferSize(window, &w, &h);
        glViewport(0, 0, w, h);
        glClearColor(0.10f, 0.10f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    // --- Shutdown ---
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

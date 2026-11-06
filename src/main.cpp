#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <imgui-SFML.h>
#include <random>
#include <iostream>
#include <filesystem>

#include "global.h"
#include "player.h"
#include "gameStatus.h"
#include "tokenManager.h"

// Theme puki co czarno-biały
void ApplyTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 0.0f;
    style.FrameRounding = 0.0f;
    style.PopupRounding = 0.0f;
    style.ScrollbarRounding = 0.0f;

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
    colors[ImGuiCol_Border] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
    colors[ImGuiCol_Button] = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
    colors[ImGuiCol_ButtonActive] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
}

int main() {

    // główne okno
    sf::RenderWindow window(sf::VideoMode({GlobalConfig::DEFAULT_WINDOW_WIDTH, GlobalConfig::DEFAULT_WINDOW_HEIGHT }), GlobalConfig::WINDOW_TITLE, sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(60);
    if (!ImGui::SFML::Init(window)) {
        return -1;
    }

    // Theme
    ApplyTheme();
    ImGuiIO& io = ImGui::GetIO();
    float fontSize = GlobalConfig::FONT_SIZE; // Deklarujemy tylko raz!

    ImFont* customFont = io.Fonts->AddFontFromFileTTF(GlobalConfig::FONT_PATH.c_str(), fontSize);
    
    if (customFont != nullptr) {
        (void)ImGui::SFML::UpdateFontTexture();
    } else {
        std::cerr << "Nie udalo sie odczytac czcionki z katalogu (build). Ładuje się w takim razi czcionka standardowa" << std::endl;
    }


    // Inicjalizacja gracza (wczytuje dane w konstruktorze)
    Player player(500); 
    TokenManager tokenManager(10);
    GameStatus currentStatus = GameStatus::MAIN_MENU;
    sf::Clock deltaClock;

    // Do generatora liczb losowych
    std::random_device rd; // ziarno
    std::mt19937 gen(rd()); // wydajny algorytm
    std::uniform_int_distribution<> distrib(1, 100); // teraz distrib(gen) - zwraca losową liczbę (1-100)

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            ImGui::SFML::ProcessEvent(window, *event);

            // Close Button in Top Bar
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            // ESC - powrót do menu głównego z poziomu gry
            if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
                if (key->code == sf::Keyboard::Key::Escape) {
                    if (currentStatus != GameStatus::MAIN_MENU) {
                        currentStatus = GameStatus::MAIN_MENU;
                    }
                }
            }
        }

        ImGui::SFML::Update(window, deltaClock.restart());

        // Pieniądze - w grach u góry
        if (currentStatus != GameStatus::MAIN_MENU) {
            ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
            ImGui::Begin("Portfel Gracza", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
            ImGui::Text("Stan konta: %d $", player.getCash());
            
            // Czy my w ogóle mamy pieniądze 
            if (player.isBankrupt()) {
                ImGui::Separator();
                if (ImGui::Button("WEZ POZYCZKE (+200$)")) {
                    player.takeLoan(200);
                }
            }
            ImGui::End();
        }

        // Stany Gry
        switch (currentStatus) {
            // === MAIN MENU
            case GameStatus::MAIN_MENU: {
                ImGui::SetNextWindowPos(ImVec2(200, 150), ImGuiCond_FirstUseEver);
                ImGui::SetNextWindowSize(ImVec2(400, 320), ImGuiCond_FirstUseEver);
                ImGui::Begin("VCasino - Menu Glowne", nullptr, ImGuiWindowFlags_NoCollapse);
                
                ImGui::Text("%s", 
                    "============================================\n"
                    "|| ~~ || WITAJ W WIRTUALNYM KASYNIE || ~~ ||\n"
                    "============================================"
                );
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();
                // TODO 
                if (ImGui::Button("TEST GAME", ImVec2(-1, 40))) {
                    currentStatus = GameStatus::DEBUG;
                }

                if (ImGui::Button("Graj w Ruletke", ImVec2(-1, 40))) {
                    currentStatus = GameStatus::ROULETTE;
                }
                if (ImGui::Button("Graj w Gre 2 (Sloty)", ImVec2(-1, 40))) {
                    currentStatus = GameStatus::GAME_2;
                }
                if (ImGui::Button("Zapisz Stan Gry", ImVec2(-1, 40))) {
                    player.saveToFile();
                }
                if (ImGui::Button("Wyjscie", ImVec2(-1, 40))) {
                    window.close();
                }
                
                ImGui::End();
                break;
            }

            // === ROULETTE
            case GameStatus::ROULETTE: {
                ImGui::SetNextWindowPos(ImVec2(110, 80), ImGuiCond_FirstUseEver);
                ImGui::SetNextWindowSize(ImVec2(580, 450), ImGuiCond_FirstUseEver);
                ImGui::Begin("Ruletka", nullptr, ImGuiWindowFlags_NoCollapse);
                // ?
                ImGui::End();
                break;
            }

            // === GAME_2
            case GameStatus::GAME_2: {
                ImGui::SetNextWindowPos(ImVec2(110, 80), ImGuiCond_FirstUseEver);
                ImGui::SetNextWindowSize(ImVec2(580, 450), ImGuiCond_FirstUseEver);
                ImGui::Begin("GAME_2", nullptr, ImGuiWindowFlags_NoCollapse);
                // ?
                ImGui::End();
                break;
            }

            // === TEST_GAME
            case GameStatus::DEBUG: {
               
                // Nowe okno
                ImGui::SetNextWindowPos(ImVec2(200, 100), ImGuiCond_FirstUseEver);
                ImGui::SetNextWindowSize(ImVec2(580, 450), ImGuiCond_FirstUseEver);
               
                // Tytuł
                ImGui::Begin("Przykladowa Gra", nullptr, ImGuiWindowFlags_NoCollapse);

                ImGui::Text("Wybierz zeton do obstawiania:");
                ImGui::Separator();

                for (int chipValue : tokenManager.getAvailableTokens())
                {
                    std::string label = std::to_string(chipValue) + "$";
                   
                    if (tokenManager.getSelectedToken() == chipValue) {
                        label += "<==[Wybrales]";
                    }
                    
                    if (ImGui::Button(label.c_str(), ImVec2(150, 40))) {
                        tokenManager.setSelectedToken(chipValue);
                    }
                    ImGui::SameLine();
                }

                ImGui::NewLine();
                ImGui::Separator();
                ImGui::Text("Aktualny zeton: %d$", tokenManager.getSelectedToken());
                ImGui::Spacing();

                if (player.isBankrupt()) {
                    ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "JESTES BANKRUTEM!");
                    ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Wez pozyczke by grac dalej (W lewym gornym rogu)") ;
                } else {
                    int bet = tokenManager.getSelectedToken();
                    if (ImGui::Button("Postaw i zagraj (20% szans)", ImVec2(220, 40))) {
                        if (player.loseCash(bet)) {
                            // Loteria
                            int roll = distrib(gen);
                            if (roll <= 20) {
                                player.addCash(bet * 5); // DEBUG
                            }
                            // Jeśli roll > 20, to pieniądze i tak zostały już graczowi wzięte
                        }
                    }
                }

                ImGui::Spacing();
                if (ImGui::Button("Wroc do Menu (ESC)", ImVec2(-1, 30))) {
                    currentStatus = GameStatus::MAIN_MENU;
                }

                ImGui::End();
                break;
            }
        }

        // Czarne tło
        window.clear(sf::Color(15, 15, 15));
        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();
    return 0;
}
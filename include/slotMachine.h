#ifndef SLOT_MACHINE_H
#define SLOT_MACHINE_H

#include <SFML/Graphics.hpp>
#include <array>
#include <random>
#include <string>

class Player;
class TokenManager;

class SlotMachine {
    public:
        enum Symbol { SEVEN = 0, CHERRY = 1, BELL = 2, BAR = 3, SYMBOL_COUNT = 4 };

        SlotMachine();

        // Loads all textures from `dir` (e.g. "assets/"). Needs an open window (OpenGL context).
        bool load(const std::string& dir);

        // Mouse click in window coordinates - pulls the lever if it was hit
        void handleClick(sf::Vector2f mousePos, Player& player, int bet);

        // Call EVERY frame (also when the menu is shown) so a spin that is
        // still running always finishes and pays out.
        void update(sf::Time dt, Player& player);

        // SFML drawing (call between window.clear() and ImGui::SFML::Render())
        void draw(sf::RenderWindow& window);

        // ImGui overlay (bet chips, SPIN, payout table). Returns true when "Back" was pressed.
        bool drawUI(Player& player, TokenManager& tokens);

        bool isSpinning() const { return spinning; }

    private:
        enum class ReelState { Idle, Spinning, Stopping };

        struct Reel {
            float pos = 0.f;          // strip index shown in the middle row (continuous)
            ReelState state = ReelState::Idle;
            float startPos = 0.f;
            float targetPos = 0.f;
            float tweenT = 0.f;
            float tweenDur = 0.f;
            int targetSymbol = CHERRY;
        };

        bool trySpin(Player& player, int bet);
        void beginStopping(Reel& reel);
        void evaluate(Player& player);
        int  pickWeightedSymbol();
        sf::Transform machineTransform() const;

        bool loaded = false;
        bool spinning = false;
        int currentBet = 0;
        float spinTimer = 0.f;
        float leverTimer = 0.f;
        float winTimer = 0.f;
        std::string message = "Pociagnij za dzwignie lub kliknij SPIN!";
        enum class WinKind { None, Partial, Full };
        WinKind winKind = WinKind::None;
        std::array<bool, 3> matched = {false, false, false}; // which reels are highlighted

        std::array<Reel, 3> reels;

        sf::Texture texFrame;     // slot-machine4 (transparent windows)
        sf::Texture texReelBg;    // slot-machine5
        sf::Texture texLeverUp;   // slot-machine2
        sf::Texture texLeverDown; // slot-machine3
        std::array<sf::Texture, SYMBOL_COUNT> texSymbols;
        sf::RenderTexture reelTarget; // clips symbols to the reel windows

        std::mt19937 rng;
};

#endif
#include "slotMachine.h"
#include "player.h"
#include "tokenManager.h"

#include <imgui.h>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

namespace {
    // ---- Layout (all in ORIGINAL image pixels, images are 816x624) ----
    constexpr float REEL_X = 229.f;      // top-left of the 3 reel windows
    constexpr float REEL_Y = 246.f;
    constexpr float REEL_W = 368.f;      // total width of the 3 windows incl. gaps
    constexpr float REEL_H = 210.f;
    constexpr float REEL_STEP = 130.f;   // distance between reel centres
    constexpr float REEL_CENTER_X = 54.f;// centre of the first reel inside the target
    constexpr float CELL = 70.f;         // 3 rows * 70 = 210
    constexpr float SYMBOL_SCALE = 0.7f; // 100px symbol -> 70px
    const sf::FloatRect LEVER_RECT({673.f, 296.f}, {92.f, 270.f});

    // Where the machine is drawn in the 800x600 window
    constexpr float MACHINE_SCALE = 0.8f;
    constexpr float MACHINE_POS_X = 70.4f; // body is centred horizontally
    constexpr float MACHINE_POS_Y = 0.f;

    // ---- Game rules ----
    constexpr int N = 8; // strip length
    const int STRIP[N] = {
        SlotMachine::CHERRY, SlotMachine::BELL, SlotMachine::BAR, SlotMachine::CHERRY,
        SlotMachine::SEVEN, SlotMachine::BELL, SlotMachine::CHERRY, SlotMachine::BAR
    };
    // Probability weights per symbol, payout multiplier for 3 in a row,
    // and partial refund (% of bet) when exactly 2 reels show the same symbol
    // (any two of the three reels, they don't have to be neighbours).
    // 3-of-a-kind hits ~8% of spins, a pair ~58% of spins.
    // Theoretical return to player is ~92%.
    //                            SEVEN CHERRY BELL BAR
    const int WEIGHTS[4] =      {  15,    30,   30,  25 };
    const int PAYOUT_MULT[4] =  {  50,     4,    8,  15 };
    const int PAIR_PERCENT[4] = {  60,    25,   30,  40 };

    constexpr float SPIN_SPEED = 16.f;    // cells per second
    constexpr float FIRST_STOP = 1.0f;    // seconds until first reel starts braking
    constexpr float STOP_STEP  = 0.45f;   // delay between reels
    constexpr int   MIN_BRAKE_CELLS = 3;

    int wrap(int a, int n) { return ((a % n) + n) % n; }
    float easeOutQuad(float u) { return 1.f - (1.f - u) * (1.f - u); }
}

SlotMachine::SlotMachine() : rng(std::random_device{}()) {
    reels[0].pos = 0.f;
    reels[1].pos = 3.f;
    reels[2].pos = 5.f;
}

bool SlotMachine::load(const std::string& dir) {
    bool ok = true;
    auto loadTex = [&](sf::Texture& tex, const std::string& file) {
        if (!tex.loadFromFile(dir + file)) {
            std::cerr << "Nie udalo sie wczytac: " << dir + file << std::endl;
            ok = false;
        } else {
            tex.setSmooth(true);
        }
    };

    loadTex(texFrame,     "slot-machine4.png");
    loadTex(texReelBg,    "slot-machine5.png");
    loadTex(texLeverUp,   "slot-machine2.png");
    loadTex(texLeverDown, "slot-machine3.png");
    loadTex(texSymbols[SEVEN],  "slot-symbol1.png");
    loadTex(texSymbols[CHERRY], "slot-symbol2.png");
    loadTex(texSymbols[BELL],   "slot-symbol3.png");
    loadTex(texSymbols[BAR],    "slot-symbol4.png");

    if (!reelTarget.resize({static_cast<unsigned>(REEL_W), static_cast<unsigned>(REEL_H)})) {
        std::cerr << "Nie udalo sie utworzyc RenderTexture" << std::endl;
        ok = false;
    }

    loaded = ok;
    return ok;
}

sf::Transform SlotMachine::machineTransform() const {
    sf::Transform t;
    t.translate({MACHINE_POS_X, MACHINE_POS_Y});
    t.scale({MACHINE_SCALE, MACHINE_SCALE});
    return t;
}

int SlotMachine::pickWeightedSymbol() {
    std::discrete_distribution<int> dist(std::begin(WEIGHTS), std::end(WEIGHTS));
    return dist(rng);
}

bool SlotMachine::trySpin(Player& player, int bet) {
    if (!loaded || spinning) return false;

    if (bet <= 0 || player.getCash() < bet) {
        message = "Za malo srodkow na taka stawke!";
        winKind = WinKind::None;
        return false;
    }

    player.loseCash(bet); // bet is taken up front
    currentBet = bet;
    spinning = true;
    spinTimer = 0.f;
    leverTimer = 0.35f;
    winTimer = 0.f;
    winKind = WinKind::None;
    message = "Kreci sie...";

    // Outcome is decided now, the animation just lands on it
    for (auto& r : reels) {
        r.state = ReelState::Spinning;
        r.targetSymbol = pickWeightedSymbol();
    }
    return true;
}

void SlotMachine::handleClick(sf::Vector2f mousePos, Player& player, int bet) {
    sf::Vector2f p = machineTransform().getInverse().transformPoint(mousePos);
    if (LEVER_RECT.contains(p)) {
        trySpin(player, bet);
    }
}

void SlotMachine::beginStopping(Reel& r) {
    // collect strip positions that hold the wanted symbol
    std::vector<int> candidates;
    for (int j = 0; j < N; ++j)
        if (STRIP[j] == r.targetSymbol) candidates.push_back(j);

    std::uniform_int_distribution<size_t> pick(0, candidates.size() - 1);
    int targetIdx = candidates[pick(rng)];

    int base = static_cast<int>(std::ceil(r.pos)) + MIN_BRAKE_CELLS;
    int extra = wrap(targetIdx - base, N);

    r.startPos = r.pos;
    r.targetPos = static_cast<float>(base + extra);
    float dist = r.targetPos - r.startPos;
    r.tweenDur = 2.f * dist / SPIN_SPEED; // ease-out-quad keeps the speed continuous
    r.tweenT = 0.f;
    r.state = ReelState::Stopping;
}

void SlotMachine::evaluate(Player& player) {
    int s[3];
    for (int i = 0; i < 3; ++i)
        s[i] = STRIP[wrap(static_cast<int>(std::lround(reels[i].pos)), N)];

    spinning = false;
    matched = {false, false, false};
    winKind = WinKind::None;

    if (s[0] == s[1] && s[1] == s[2]) {
        // 3 of a kind - full win
        int win = currentBet * PAYOUT_MULT[s[0]];
        player.addCash(win);
        message = "WYGRANA! +" + std::to_string(win) + "$";
        winKind = WinKind::Full;
        matched = {true, true, true};
        winTimer = 2.5f;
        return;
    }

    // exactly 2 the same - partial refund
    int pairSymbol = -1;
    if (s[0] == s[1] || s[0] == s[2]) pairSymbol = s[0];
    else if (s[1] == s[2])            pairSymbol = s[1];

    if (pairSymbol >= 0) {
        int refund = static_cast<int>(std::lround(currentBet * PAIR_PERCENT[pairSymbol] / 100.0));
        if (refund > 0) {
            player.addCash(refund);
            message = "Para! Zwrot +" + std::to_string(refund) + "$ (stawka " + std::to_string(currentBet) + "$)";
            winKind = WinKind::Partial;
            for (int i = 0; i < 3; ++i) matched[i] = (s[i] == pairSymbol);
            winTimer = 1.5f;
            return;
        }
    }

    message = "Brak wygranej. Sprobuj jeszcze raz!";
}

void SlotMachine::update(sf::Time dtTime, Player& player) {
    float dt = dtTime.asSeconds();

    if (leverTimer > 0.f) leverTimer -= dt;
    if (winTimer > 0.f)   winTimer -= dt;
    if (!spinning) return;

    spinTimer += dt;

    bool allIdle = true;
    for (int i = 0; i < 3; ++i) {
        Reel& r = reels[i];

        if (r.state == ReelState::Spinning) {
            r.pos += SPIN_SPEED * dt;
            if (r.pos >= N) r.pos -= N;
            if (spinTimer >= FIRST_STOP + i * STOP_STEP) beginStopping(r);
        }
        else if (r.state == ReelState::Stopping) {
            r.tweenT += dt;
            float u = std::min(r.tweenT / r.tweenDur, 1.f);
            r.pos = r.startPos + (r.targetPos - r.startPos) * easeOutQuad(u);
            if (u >= 1.f) {
                r.pos = std::fmod(r.targetPos, static_cast<float>(N));
                r.state = ReelState::Idle;
            }
        }

        if (r.state != ReelState::Idle) allIdle = false;
    }

    if (allIdle) evaluate(player);
}

void SlotMachine::draw(sf::RenderWindow& window) {
    if (!loaded) return;

    sf::RenderStates states(machineTransform());

    // 1) reel background
    window.draw(sf::Sprite(texReelBg), states);

    // 2) symbols, clipped to the reel windows through a RenderTexture
    reelTarget.clear(sf::Color::Transparent);
    for (int i = 0; i < 3; ++i) {
        const Reel& r = reels[i];
        float cx = REEL_CENTER_X + i * REEL_STEP;
        int base = static_cast<int>(std::floor(r.pos));

        for (int k = base - 2; k <= base + 2; ++k) {
            float y = REEL_H / 2.f + (r.pos - static_cast<float>(k)) * CELL;
            sf::Sprite s(texSymbols[STRIP[wrap(k, N)]]);
            s.setOrigin({50.f, 50.f});
            s.setScale({SYMBOL_SCALE, SYMBOL_SCALE});
            s.setPosition({cx, y});
            reelTarget.draw(s);
        }
    }
    reelTarget.display();

    sf::Sprite reelSprite(reelTarget.getTexture());
    reelSprite.setPosition({REEL_X, REEL_Y});
    window.draw(reelSprite, states);

    // 3) payline (middle row)
    sf::RectangleShape line({REEL_W, 3.f});
    line.setPosition({REEL_X, REEL_Y + REEL_H / 2.f - 1.5f});
    line.setFillColor(sf::Color(255, 40, 40, 150));
    window.draw(line, states);

    // win highlight - blinking frame around the winning cells in the middle row
    if (winTimer > 0.f && (static_cast<int>(winTimer * 5.f) % 2 == 0)) {
        sf::Color col = (winKind == WinKind::Full) ? sf::Color(255, 220, 0) : sf::Color(120, 255, 140);
        for (int i = 0; i < 3; ++i) {
            if (!matched[i]) continue;
            sf::RectangleShape box({108.f - 6.f, CELL - 6.f});
            box.setPosition({REEL_X + i * REEL_STEP + 3.f, REEL_Y + CELL + 3.f});
            box.setFillColor(sf::Color(col.r, col.g, col.b, 50));
            box.setOutlineColor(col);
            box.setOutlineThickness(3.f);
            window.draw(box, states);
        }
    }

    // 4) machine frame (windows are transparent)
    window.draw(sf::Sprite(texFrame), states);

    // 5) lever
    window.draw(sf::Sprite(leverTimer > 0.f ? texLeverDown : texLeverUp), states);
}

bool SlotMachine::drawUI(Player& player, TokenManager& tokens) {
    bool back = false;
    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                                   ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                                   ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoSavedSettings;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);

    // ---- payout table (left side) ----
    ImGui::SetNextWindowPos(ImVec2(10, 100), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(165, 300), ImGuiCond_Always);
    ImGui::Begin("##slotsPayouts", nullptr, flags);
    ImGui::Text("WYPLATY (x stawka)");
    ImGui::Separator();
    ImGui::Text("7 7 7     x%d", PAYOUT_MULT[SEVEN]);
    ImGui::Text("BAR x3   x%d", PAYOUT_MULT[BAR]);
    ImGui::Text("Dzwon x3 x%d", PAYOUT_MULT[BELL]);
    ImGui::Text("Wisnie x3 x%d", PAYOUT_MULT[CHERRY]);
    ImGui::Spacing();
    ImGui::Text("ZWROT ZA PARE");
    ImGui::Separator();
    ImGui::Text("7 7       %d%%", PAIR_PERCENT[SEVEN]);
    ImGui::Text("BAR x2   %d%%", PAIR_PERCENT[BAR]);
    ImGui::Text("Dzwon x2 %d%%", PAIR_PERCENT[BELL]);
    ImGui::Text("Wisnie x2 %d%%", PAIR_PERCENT[CHERRY]);
    ImGui::Spacing();
    ImGui::TextDisabled("Linia: srodek");
    ImGui::End();

    // ---- bottom bar ----
    ImGui::SetNextWindowPos(ImVec2(0, 505), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(800, 95), ImGuiCond_Always);
    ImGui::Begin("##slotsBar", nullptr, flags);

    ImGui::AlignTextToFramePadding();
    ImGui::Text("Stawka:");
    ImGui::SameLine();
    for (int chip : tokens.getAvailableTokens()) {
        bool selected = (tokens.getSelectedToken() == chip);
        if (selected) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.85f, 0.85f, 0.85f, 1.f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.f, 0.f, 0.f, 1.f));
        }
        std::string label = std::to_string(chip) + "$";
        if (ImGui::Button(label.c_str(), ImVec2(70, 30)) && !spinning) {
            tokens.setSelectedToken(chip);
        }
        if (selected) ImGui::PopStyleColor(2);
        ImGui::SameLine();
    }

    ImGui::BeginDisabled(spinning || player.getCash() < tokens.getSelectedToken());
    if (ImGui::Button("SPIN", ImVec2(100, 30))) {
        trySpin(player, tokens.getSelectedToken());
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    if (ImGui::Button("Menu (ESC)", ImVec2(110, 30))) {
        back = true;
    }

    ImGui::Spacing();
    if (winKind == WinKind::Full)
        ImGui::TextColored(ImVec4(1.f, 0.85f, 0.f, 1.f), "%s", message.c_str());
    else if (winKind == WinKind::Partial)
        ImGui::TextColored(ImVec4(0.5f, 1.f, 0.55f, 1.f), "%s", message.c_str());
    else
        ImGui::Text("%s", message.c_str());

    ImGui::End();
    ImGui::PopStyleVar();

    return back;
}
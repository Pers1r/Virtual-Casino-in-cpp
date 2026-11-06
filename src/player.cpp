#include "player.h"
#include <fstream>
#include <algorithm>
#include <iostream>

Player::Player(int startCash) : cash(startCash) {
    loadFromFile();
}

Player::~Player() {
    saveToFile();
}

// Getter
int Player::getCash() const {
    return cash;
}

void Player::setCash(int _cash) {
    cash = _cash;
}

void Player::addCash(int amount) {
    cash += amount;
}

void Player::takeLoan(int amount) {
    if (isBankrupt()) {
        if (amount >0) {
            addCash(amount);
        } 
        else {
            std::cerr << "You can't take that loan - amount is under 0" << std::endl;
        }
    }
}

bool Player::isBankrupt() {
    return cash <= 0;
}

// Lose cash with check if player can continue game
bool Player::loseCash(int amount) {
    cash = std::max(cash-amount,0);
    if (cash == 0) {
        return false; // Player don't have money any more
    }
    else {
        return true; // Player can still play
    }
}

void Player::saveToFile() {
    std::ofstream file(saveFile);
    if (file.is_open()) {
        file << cash;
        file.close();
    }
}

void Player::loadFromFile() {
    std::ifstream file(saveFile);
    if (file.is_open()) {
        file >> cash;
        file.close();
    }
}
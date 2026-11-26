#ifndef TOKEN_MANAGER_H
#define TOKEN_MANAGER_H

#include <vector>
#include "token.h"

class TokenManager {
    private:
        int selectedValue;
        std::vector<Token> tokens;
    
    public:
        TokenManager(int defaultVal = 10) : selectedValue(defaultVal) {
            // Nominały: 1$, 10$, 50$, 100$
            tokens.emplace_back(1, sf::Vector2f(300.f, 550.f));
            tokens.emplace_back(10, sf::Vector2f(420.f, 550.f));
            tokens.emplace_back(50, sf::Vector2f(540.f, 550.f));
            tokens.emplace_back(100, sf::Vector2f(660.f, 550.f));
        }
    
        int getSelectedToken() const {
            return selectedValue;
        }
    
        void setSelectedToken(int val) {
            selectedValue = val;
        }
    
        std::vector<int> getAvailableTokens() const {
            std::vector<int> values;
            for (const auto& token : tokens) {
                values.push_back(token.getValue());
            }
            return values;
        }
    
    
        // === Może coś takiego się przyda
    
        // Click
        void handleMouseClick(sf::Vector2f mousePos) {
            for (const auto& token : tokens) {
                if (token.isClicked(mousePos)) {
                    selectedValue = token.getValue();
                    break;
                }
            }
        }
    
        // Rysowanie
        void draw(sf::RenderWindow& window, const sf::Font& font) const {
            for (const auto& token : tokens) {
                bool isSelected = (token.getValue() == selectedValue);
                token.draw(window, font, isSelected);
            }
        }
};

#endif
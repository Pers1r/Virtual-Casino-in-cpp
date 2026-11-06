#ifndef TOKEN_H
#define TOKEN_H

#include <SFML/Graphics.hpp>
#include <string>

class Token {
    private:
        int value;
        sf::Vector2f position;
        float radius;
        sf::CircleShape shape;

    public:
        Token(int val, sf::Vector2f pos, float rad = 30.f) : value(val), position(pos), radius(rad) {
            
            shape.setRadius(radius);
            shape.setOrigin({radius, radius}); // Środek
            shape.setPosition(position);
            shape.setFillColor(sf::Color::White);
            shape.setOutlineThickness(4.f);
            shape.setOutlineColor(sf::Color::Black);
        }

        int getValue() const {
            return value;
        }

        sf::Vector2f getPosition() const {
            return position;
        }

        // Click
        bool isClicked(sf::Vector2f mousePos) const {
            float dx = mousePos.x - position.x;
            float dy = mousePos.y - position.y;
            return (dx * dx + dy * dy <= radius * radius);
        }

        // Rysowanie
        void draw(sf::RenderWindow& window, const sf::Font& font, bool isSelected) const {
            sf::CircleShape currentShape = shape;
            if (isSelected) {
                currentShape.setOutlineColor(sf::Color::Magenta); // zmiana koloru i outlinu po kliknięciu
                currentShape.setOutlineThickness(6.f);
            } else {
                currentShape.setOutlineColor(sf::Color::Black);
                currentShape.setOutlineThickness(4.f);
            }

            window.draw(currentShape);

            // Teskst ? może nad albo coś takiego
            sf::Text text(font, std::to_string(value) + "$", 16);
            text.setFillColor(sf::Color::Black);

            // teskst w środku // TODO
            sf::FloatRect textBounds = text.getLocalBounds();
            text.setPosition({position.x - textBounds.size.x / 2.f, position.y - textBounds.size.y / 2.f - 4.f});

            window.draw(text);
        }
};

#endif
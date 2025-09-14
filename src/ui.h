#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include "hand.h"
#include "card.h"

namespace ui {

// Simple rectangular button with centered text
struct Button {
    sf::RectangleShape rect;
    sf::Text text;
};

// UI utilities
Button createButton(const std::string& label,
                    sf::Font& font,
                    sf::Vector2f size,
                    sf::Vector2f pos,
                    sf::Color color,
                    sf::Color textColor,
                    unsigned int textSize = 18);

void drawButton(sf::RenderWindow& window, const Button& btn);

bool isButtonClicked(const Button& btn, sf::Vector2i mouse);

// Rendering entrypoint: draws all game elements
void drawGameElements(sf::RenderWindow& window, sf::Font& font,
                      const Hand& player1Hand, const Hand& player2Hand,
                      const std::vector<Card>& communityCards, size_t cardsToShow,
                      int player1BetDisplay, int player2BetDisplay, int pendingStake, int pot,
                      int player1score, int player2score,
                      bool gameFinished, const std::string& winnerText,
                      const std::vector<Button*>& activeButtons,
                      double lastP2WinPercentage,
                      const std::string& player1Name, const std::string& player2Name,
                      unsigned int logicalWidth, unsigned int logicalHeight);

} // namespace ui
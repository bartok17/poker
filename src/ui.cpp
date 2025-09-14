#include "ui.h"
#include <SFML/Graphics.hpp>

namespace ui {

// Layout constants (render-only)
constexpr float CARD_X_SPACING = 120.f;
constexpr float HAND_START_X   = 800.f;
constexpr float P1_HAND_Y      = 800.f;
constexpr float P2_HAND_Y      = 200.f;
constexpr float COMMUNITY_Y    = 500.f;

// Internal helpers
static void drawHand(const Hand& hand, sf::RenderWindow& window, float y, bool showCards) {
    const auto& cards = hand.getCards();
    for (size_t i = 0; i < cards.size(); ++i) {
        float x = HAND_START_X + static_cast<float>(i) * CARD_X_SPACING;
        Card card = cards[i];
        card.setFaceUp(showCards);
        card.draw(window, x, y);
    }
}

static void drawCommunityCards(const std::vector<Card>& communityCards, size_t cardsToShow,
                               sf::RenderWindow& window, unsigned int logicalWidth) {
    // 110x (texture space) scaled later in Card::draw; spacing aligned with CARD_X_SPACING
    float communityStartX = (static_cast<float>(logicalWidth) - (5.f * 110.f - 10.f)) / 2.0f;
    for (size_t i = 0; i < communityCards.size(); ++i) {
        float x = communityStartX + static_cast<float>(i) * CARD_X_SPACING;
        Card card = communityCards[i];
        card.setFaceUp(i < cardsToShow);
        card.draw(window, x, COMMUNITY_Y);
    }
}

static void drawStakes(int player1BetDisplay, int player2BetDisplay, int pendingStake,
                       sf::RenderWindow& window, sf::Font& font,
                       const std::string& player1Name, const std::string& player2Name,
                       unsigned int logicalHeight) {
    sf::Text p1BetText(player1Name + " Bet: " + std::to_string(player1BetDisplay), font, 20);
    p1BetText.setFillColor(sf::Color::Yellow);
    p1BetText.setPosition(50, static_cast<float>(logicalHeight) - 80.f);
    window.draw(p1BetText);

    sf::Text p2BetText(player2Name + " Bet: " + std::to_string(player2BetDisplay), font, 20);
    p2BetText.setFillColor(sf::Color::Yellow);
    p2BetText.setPosition(50, 40);
    window.draw(p2BetText);

    sf::Text pendingText("Your Bet: " + std::to_string(pendingStake), font, 30);
    pendingText.setFillColor(sf::Color::White);
    pendingText.setPosition(50, static_cast<float>(logicalHeight) - 140.f);
    window.draw(pendingText);
}

// Public API
Button createButton(const std::string& label,
                    sf::Font& font,
                    sf::Vector2f size,
                    sf::Vector2f pos,
                    sf::Color color,
                    sf::Color textColor,
                    unsigned int textSize) {
    Button btn;
    btn.rect.setSize(size);
    btn.rect.setPosition(pos);
    btn.rect.setFillColor(color);
    btn.text.setString(label);
    btn.text.setFont(font);
    btn.text.setCharacterSize(textSize);
    btn.text.setFillColor(textColor);
    sf::FloatRect textRect = btn.text.getLocalBounds();
    btn.text.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
    btn.text.setPosition(pos.x + size.x / 2.0f, pos.y + size.y / 2.0f);
    return btn;
}

void drawButton(sf::RenderWindow& window, const Button& btn) {
    window.draw(btn.rect);
    window.draw(btn.text);
}

bool isButtonClicked(const Button& btn, sf::Vector2i mouse) {
    return btn.rect.getGlobalBounds().contains(static_cast<float>(mouse.x), static_cast<float>(mouse.y));
}

void drawGameElements(sf::RenderWindow& window, sf::Font& font,
                      const Hand& player1Hand, const Hand& player2Hand,
                      const std::vector<Card>& communityCards, size_t cardsToShow,
                      int player1BetDisplay, int player2BetDisplay, int pendingStake, int pot,
                      int player1score, int player2score,
                      bool gameFinished, const std::string& winnerText,
                      const std::vector<Button*>& activeButtons,
                      double /*lastP2WinPercentage*/,
                      const std::string& player1Name, const std::string& player2Name,
                      unsigned int logicalWidth, unsigned int logicalHeight) {
    // Hands
    drawHand(player1Hand, window, P1_HAND_Y, true);
    drawHand(player2Hand, window, P2_HAND_Y, gameFinished); // Show AI cards only after round ends

    // Board
    drawCommunityCards(communityCards, cardsToShow, window, logicalWidth);

    // Stakes and pot
    drawStakes(player1BetDisplay, player2BetDisplay, pendingStake, window, font, player1Name, player2Name, logicalHeight);

    sf::Text potText("Pot: " + std::to_string(pot), font, 28);
    potText.setFillColor(sf::Color::Cyan);
    potText.setPosition(900, 400);
    window.draw(potText);

    // Scores
    sf::Text scoreText1(player1Name + " Score: " + std::to_string(player1score), font, 20);
    scoreText1.setFillColor(sf::Color::White);
    scoreText1.setPosition(1500, 1000);
    window.draw(scoreText1);

    sf::Text scoreText2(player2Name + " Score: " + std::to_string(player2score), font, 20);
    scoreText2.setFillColor(sf::Color::White);
    scoreText2.setPosition(1500, 40);
    window.draw(scoreText2);

    // Round winner
    if (gameFinished) {
        sf::Text winAnnounceText(winnerText, font, 36);
        winAnnounceText.setFillColor(sf::Color::White);
        sf::FloatRect textRect = winAnnounceText.getLocalBounds();
        winAnnounceText.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
        winAnnounceText.setPosition(static_cast<float>(logicalWidth) / 2.0f, static_cast<float>(logicalHeight) / 2.0f + 200.f);
        window.draw(winAnnounceText);
    }

    // Active buttons
    for (const auto* btnPtr : activeButtons) {
        if (btnPtr) drawButton(window, *btnPtr);
    }
}

} // namespace ui
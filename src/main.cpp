#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <cctype>
#include "hand.h"
#include "card.h"
#include "comparer.h"
#include "deck.h"
#include "playerAI.h"


// Function declarations
void drawHand(const Hand& hand, sf::RenderWindow& window, sf::Font& font, float y, bool showCards);
void drawCommunityCards(const std::vector<Card>& communityCards, size_t cardsToShow, sf::RenderWindow& window, sf::Font& font);
void drawStakes(int player1stake, int player2stake, int pendingStake, sf::RenderWindow& window, sf::Font& font);


// Button helper functions
struct Button {
    sf::RectangleShape rect;
    sf::Text text;
};

Button createButton(const std::string& label, sf::Font& font, sf::Vector2f size, sf::Vector2f pos, sf::Color color, sf::Color textColor, unsigned int textSize = 18) {
    Button btn;
    btn.rect.setSize(size);
    btn.rect.setPosition(pos);
    btn.rect.setFillColor(color);
    btn.text.setString(label);
    btn.text.setFont(font);
    btn.text.setCharacterSize(textSize);
    btn.text.setFillColor(textColor);
    btn.text.setPosition(pos.x + (size.x - btn.text.getLocalBounds().width) / 2 - 5, pos.y + (size.y - textSize) / 2);
    return btn;
}

void drawButton(sf::RenderWindow& window, const Button& btn) {
    window.draw(btn.rect);
    window.draw(btn.text);
}

bool isButtonClicked(const Button& btn, sf::Vector2i mouse) {
    return btn.rect.getGlobalBounds().contains(static_cast<float>(mouse.x), static_cast<float>(mouse.y));
}

int main() {
    size_t cardsToShow = 0;
    bool inputActive = false;
    bool allInPhase = false;
    bool riverBettingPhase = false;

    Deck deck;
    deck.shuffle();

    Hand player1Hand;
    Hand player2Hand;

    int player1score = 2000;
    int player2score = 2000;
    int player1stake = 0;
    int player2stake = 0;
    int pot = 0;
    bool finalStakePhase = false;
    bool gameFinished = false;
    std::string winnerText;
    int winner = -1; // 0: player1, 1: player2, 2: draw
    bool player1Out = false;
    bool player2Out = false;

    playerAI ai;

    int lastCardsToShow = -1;
    double lastP2Win = 0.0;

    std::vector<Card> communityCards;
    auto startNewRound = [&]() {
        deck.reset();
        deck.shuffle();
        player1Hand = Hand();
        player2Hand = Hand();
        for (int i = 0; i < 2; ++i) {
            player1Hand.addCard(deck.draw());
            player2Hand.addCard(deck.draw());
        }
        communityCards.clear();
        for (int i = 0; i < 5; ++i) {
            communityCards.push_back(deck.draw());
        }
        cardsToShow = 0;
        player1stake = 50; // Blind
        player2stake = 50; // Blind
        pot = 100;         // Both players contribute 50 to the pot (test only)
        player1score -= 50;
        player2score -= 50;
        finalStakePhase = false;
        gameFinished = false;
        riverBettingPhase = false;
        winnerText.clear();
        winner = -1;
        player1Out = false;
        player2Out = false;
    };

    // Start the first round
    startNewRound();

    sf::RenderWindow window(sf::VideoMode(1920, 1080), "Poker Table");
    sf::Font font;
    if (!font.loadFromFile("/System/Library/Fonts/Supplemental/Arial.ttf")) {
        std::cerr << "Could not load font!" << std::endl;
        return 1;
    }

    // Button setup using helper
    Button submitButton   = createButton("Submit", font, {160, 60}, {300, 980}, sf::Color(100, 200, 100), sf::Color::Black);
    Button add10Button    = createButton("+10", font, {80, 60}, {500, 980}, sf::Color(200, 200, 255), sf::Color::Black);
    Button add50Button    = createButton("+50", font, {80, 60}, {600, 980}, sf::Color(200, 200, 255), sf::Color::Black);
    Button add100Button   = createButton("+100", font, {80, 60}, {700, 980}, sf::Color(200, 200, 255), sf::Color::Black);
    Button waitButton     = createButton("Wait", font, {80, 60}, {800, 980}, sf::Color(200, 255, 200), sf::Color::Black);
    Button passButton     = createButton("Pass", font, {80, 60}, {900, 980}, sf::Color(255, 200, 200), sf::Color::Black);
    Button resetButton    = createButton("Reset", font, {100, 60}, {1000, 980}, sf::Color(220, 220, 220), sf::Color::Black);
    Button nextRoundButton= createButton("Next Round", font, {200, 60}, {1200, 980}, sf::Color(255, 255, 100), sf::Color::Black);

    int pendingStake = 0;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            // Handle quick bet buttons
            if (event.type == sf::Event::MouseButtonPressed) {
                auto mouse = sf::Mouse::getPosition(window);

                if (!gameFinished && !player1Out && !player2Out) {
                    if (isButtonClicked(add10Button, mouse)) {
                        if (pendingStake + 10 >= player1score) {
                            pendingStake = player1score; // All in
                        } else {
                            pendingStake += 10;
                        }
                    } else if (isButtonClicked(add50Button, mouse)) {
                        if (pendingStake + 50 >= player1score) {
                            pendingStake = player1score; // All in
                        } else {
                            pendingStake += 50;
                        }
                    } else if (isButtonClicked(add100Button, mouse)) {
                        if (pendingStake + 100 >= player1score) {
                            pendingStake = player1score; // All in
                        } else {
                            pendingStake += 100;
                        }
                    } else if (isButtonClicked(waitButton, mouse)) {
                       
                        pendingStake = 0;
                        if (!finalStakePhase && !allInPhase && !riverBettingPhase) {
                            if (cardsToShow == 0)
                                cardsToShow = 3;
                            else if (cardsToShow == 3)
                                cardsToShow = 4;
                            else if (cardsToShow == 4) {
                                cardsToShow = 5;
                                riverBettingPhase = true; 
                            }
                        } else if (riverBettingPhase && !allInPhase) {
                            finalStakePhase = true; 
                            riverBettingPhase = false;
                        }
                    } else if (isButtonClicked(resetButton, mouse)) {
                        pendingStake = 0;
                    } else if (isButtonClicked(passButton, mouse)) {
                        // Pass
                        gameFinished = true;
                        winner = 1;
                        winnerText = "You passed. Player 2 wins the pot of " + std::to_string(pot) + "!";
                        player1score -= pot;
                        player2score += pot;
                    } else if (isButtonClicked(submitButton, mouse)) {
                        if (pendingStake > 0) {
                            // handle all in
                            if (pendingStake > player1score) pendingStake = player1score;
                            if (pendingStake > player2score) pendingStake = player2score;

                            // Subtract stake from scores
                            player1score -= pendingStake;
                            player2stake = std::min(pendingStake, player2score);
                            player2score -= player2stake;

                            // Add to pot
                            pot += pendingStake + player2stake;
                            player1stake = pendingStake;

                            // Check for all-in
                            if (player1score == 0 || player2score == 0) {
                                allInPhase = true;
                            }

                            pendingStake = 0;

                            // Advance phase or finish if all-in
                            if (!finalStakePhase && !allInPhase && !riverBettingPhase) {
                                if (cardsToShow == 0)
                                    cardsToShow = 3;
                                else if (cardsToShow == 3)
                                    cardsToShow = 4;
                                else if (cardsToShow == 4) {
                                    cardsToShow = 5;
                                    riverBettingPhase = true;
                                }
                            } else if (riverBettingPhase && !allInPhase) {
                                finalStakePhase = true;
                                riverBettingPhase = false;
                            }
                        }
                    }
                    // If all-in, reveal all remaining community cards and finish the round
                    if (allInPhase && !gameFinished) {
                        cardsToShow = 5;
                        finalStakePhase = true;
                        gameFinished = true;

                        Hand fullHand1 = player1Hand.combineHands(Hand{communityCards});
                        Hand fullHand2 = player2Hand.combineHands(Hand{communityCards});
                        int cmp = Comparer::compareHands(fullHand1, fullHand2);
                        if (cmp == 0) {
                            winnerText = "Player 1 wins the pot of " + std::to_string(pot) + "!";
                            winner = 0;
                            player1score += pot;
                        } else if (cmp == 1) {
                            winnerText = "Player 2 wins the pot of " + std::to_string(pot) + "!";
                            winner = 1;
                            player2score += pot;
                        } else {
                            winnerText = "It's a draw! Pot: " + std::to_string(pot);
                            winner = 2;
                            player1score += pot / 2;
                            player2score += pot / 2;
                        }
                        // Check possible victories
                        if (player1score <= 0) player1Out = true;
                        if (player2score <= 0) player2Out = true;
                    }
                } else if (gameFinished && isButtonClicked(nextRoundButton, mouse) && !player1Out && !player2Out) {
                    startNewRound();
                    allInPhase = false;
                }
            }
        }

        // Check for game over conditions
        if (finalStakePhase && !allInPhase && !gameFinished) {
            // Finish the round
            cardsToShow = 5;
            gameFinished = true;

            Hand fullHand1 = player1Hand.combineHands(Hand{communityCards});
            Hand fullHand2 = player2Hand.combineHands(Hand{communityCards});
            int cmp = Comparer::compareHands(fullHand1, fullHand2);
            if (cmp == 0) {
                winnerText = "Player 1 wins the pot of " + std::to_string(pot) + "!";
                winner = 0;
                player1score += pot;
            } else if (cmp == 1) {
                winnerText = "Player 2 wins the pot of " + std::to_string(pot) + "!";
                winner = 1;
                player2score += pot;
            } else {
                winnerText = "It's a draw! Pot: " + std::to_string(pot);
                winner = 2;
                // Refund: split the pot between both players
                player1score += pot / 2;
                player2score += pot / 2;
            }
            // check possible victories
            if (player1score <= 0) player1Out = true;
            if (player2score <= 0) player2Out = true;
        }

        // After the game loop, check for game over:
        if (player1Out || player2Out) {
            gameFinished = true;
            if (player1Out && player2Out)
                winnerText = "Both players are out of money! Game over.";
            else if (player1Out)
                winnerText = "Player 1 is out of money! Player 2 wins the game!";
            else if (player2Out)
                winnerText = "Player 2 is out of money! Player 1 wins the game!";
        }

        window.clear(sf::Color(0, 100, 0)); 

        if (static_cast<int>(cardsToShow) != lastCardsToShow) {
            std::vector<Card> visibleBoard(communityCards.begin(), communityCards.begin() + cardsToShow);
            // Update the AI's evaluation of player 2's hand           
            lastP2Win = ai.evaluateHand(player2Hand, visibleBoard) * 100.0;
            lastCardsToShow = static_cast<int>(cardsToShow);
        }
        sf::Text p2winText("Player 2 Win%: " + std::to_string(static_cast<int>(lastP2Win)) + "%", font, 28);
        p2winText.setFillColor(sf::Color::Magenta);
        p2winText.setPosition(1200, 40);
        window.draw(p2winText);

        // Always show player 1's cards
        drawHand(player1Hand, window, font, 900, true);

        // Show player 2's cards under certain conditions
        bool showEnemy = gameFinished && winner == 1;
        drawHand(player2Hand, window, font, 100, showEnemy);

        drawCommunityCards(communityCards, cardsToShow, window, font);
        drawStakes(player1stake, player2stake, pendingStake, window, font);

        // Draw pot
        sf::Text potText("Pot: " + std::to_string(pot), font, 22);
        potText.setFillColor(sf::Color::Cyan);
        potText.setPosition(900, 400);
        window.draw(potText);

        // Draw scores
        sf::Text scoreText1("Player 1 Score: " + std::to_string(player1score), font, 20);
        scoreText1.setFillColor(sf::Color::Green);
        scoreText1.setPosition(1500, 1000);
        window.draw(scoreText1);

        sf::Text scoreText2("Player 2 Score: " + std::to_string(player2score), font, 20);
        scoreText2.setFillColor(sf::Color::Green);
        scoreText2.setPosition(1500, 40);
        window.draw(scoreText2);

        // Draw winner info if game finished
        if (gameFinished) {
            sf::Text winText(winnerText, font, 28);
            winText.setFillColor(sf::Color::White);
            winText.setPosition(700, 700);
            window.draw(winText);
        }

        if (!gameFinished && !player1Out && !player2Out) {
            drawButton(window, submitButton);
            drawButton(window, add10Button);
            drawButton(window, add50Button);
            drawButton(window, add100Button);
            drawButton(window, waitButton);
            drawButton(window, passButton);
            drawButton(window, resetButton);
        } else if (gameFinished && !player1Out && !player2Out) {
            drawButton(window, nextRoundButton);
        }

        window.display();
    }

    return 0;
}

void drawHand(const Hand& hand, sf::RenderWindow& window, sf::Font& font, float y, bool showCards) {
    const auto& cards = hand.getCards();
    float handStartX = 600; 

    if (y == 900) { 
        float handY1 = 800;
        for (size_t i = 0; i < 2 && i < cards.size(); ++i) {
            float x = handStartX + i * 120; 
            Card card = cards[i];
            card.setFaceUp(showCards);
            card.draw(window, x, handY1);
        }
    } else if (y == 100) {
        float handY2 = 200; 
        for (size_t i = 0; i < 2 && i < cards.size(); ++i) {
            float x = handStartX + i * 120;
            Card card = cards[i];
            card.setFaceUp(showCards);
            card.draw(window, x, handY2);
        }
    }
}

void drawCommunityCards(const std::vector<Card>& communityCards, size_t cardsToShow, sf::RenderWindow& window, sf::Font& font) {
    float communityY = 500; 
    float communityStartX = 500;
    for (size_t i = 0; i < communityCards.size(); ++i) {
        float x = communityStartX + i * 120;
        float y = communityY;
        Card card = communityCards[i];
        card.setFaceUp(i < cardsToShow);
        card.draw(window, x, y);
    }
}

void drawStakes(int player1stake, int player2stake, int pendingStake, sf::RenderWindow& window, sf::Font& font) {
    sf::Text stakeText("Player 1 Stake: " + std::to_string(player1stake), font, 20);
    stakeText.setFillColor(sf::Color::Yellow);
    stakeText.setPosition(50, 1000);   
    window.draw(stakeText);

    sf::Text stakeText2("Player 2 Stake: " + std::to_string(player2stake), font, 20);
    stakeText2.setFillColor(sf::Color::Yellow);
    stakeText2.setPosition(50, 40);   
    window.draw(stakeText2);

    sf::Text pendingText("Pending: " + std::to_string(pendingStake), font, 30);
    pendingText.setFillColor(sf::Color::White);
    pendingText.setPosition(50, 940);  
    window.draw(pendingText);
}



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

const unsigned int LOGICAL_WIDTH = 1920;
const unsigned int LOGICAL_HEIGHT = 1080;

float AI_FOLD_THRESHOLD = 0.35;
float AI_RAISE_THRESHOLD = 0.55;
float AI_CALL_THRESHOLD = 0.55;
float AI_AGGRESSIVENESS = 0;

struct Button;
Button createButton(const std::string& label, sf::Font& font, sf::Vector2f size, sf::Vector2f pos, sf::Color color, sf::Color textColor, unsigned int textSize);
void drawButton(sf::RenderWindow& window, const Button& btn);
bool isButtonClicked(const Button& btn, sf::Vector2i mouse);

void drawHand(const Hand& hand, sf::RenderWindow& window, sf::Font& font, float y, bool showCards);
void drawCommunityCards(const std::vector<Card>& communityCards, size_t cardsToShow, sf::RenderWindow& window, sf::Font& font);
void drawStakes(int player1BetDisplay, int player2BetDisplay, int pendingStake, sf::RenderWindow& window, sf::Font& font);

void startNewRound(Deck& deck, Hand& player1Hand, Hand& player2Hand, std::vector<Card>& communityCards,
                   size_t& cardsToShow, int& player1score, int& player2score, 
                   int& player1BetDisplay, int& player2BetDisplay, int& pot,
                   bool& finalStakePhase, bool& gameFinished, bool& riverBettingPhase,
                   std::string& winnerText, int& winner, bool& allInPhase);

void advanceGamePhase(size_t& cardsToShow, bool& riverBettingPhase, bool& finalStakePhase,
                      int& player1BetDisplay, int& player2BetDisplay);

void determineAndSetWinner(Hand& player1Hand, Hand& player2Hand, const std::vector<Card>& communityCards, 
                           int& pot, int& player1score, int& player2score,
                           std::string& winnerText, int& winner, bool& player1Out, bool& player2Out);

void handlePlayerBetAction(playerAI& ai,
                           int& pendingStake, int& player1score, int& player2score,
                           int& player1BetDisplay, int& player2BetDisplay, int& pot,
                           bool& allInPhase, size_t& cardsToShow, bool& riverBettingPhase, bool& finalStakePhase,
                           const Hand& player2Hand, const std::vector<Card>& communityCards, 
                           std::string& winnerText, bool& gameFinished, int& winner);

void handlePlayerWaitAction(playerAI& ai, 
                            size_t& cardsToShow, bool& riverBettingPhase, bool& finalStakePhase,
                            int& player1score, int& player2score,
                            int& player1BetDisplay, int& player2BetDisplay, int& pot,
                            bool& allInPhase,
                            const Hand& player2Hand, const std::vector<Card>& communityCards, 
                            std::string& winnerText, bool& gameFinished, int& winner); 

void handlePlayerPassAction(bool& gameFinished, int& winner, std::string& winnerText, int& pot, int& player2score, int& player1score);

void updateAIInfo(playerAI& ai, const Hand& player2Hand, const std::vector<Card>& communityCards, size_t cardsToShow,
                  double& lastP2WinPercentage, int& lastCardsToShowState);

void drawGameElements(sf::RenderWindow& window, sf::Font& font,
                      const Hand& player1Hand, const Hand& player2Hand,
                      const std::vector<Card>& communityCards, size_t cardsToShow,
                      int player1BetDisplay, int player2BetDisplay, int pendingStake, int pot,
                      int player1score, int player2score,
                      bool gameFinished, const std::string& winnerText,
                      const std::vector<Button*>& activeButtons,
                      double lastP2WinPercentage);

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

int main() {
    AI_AGGRESSIVENESS = ((static_cast<float>(rand()) / RAND_MAX) * 0.2f) - 0.1f;
    AI_CALL_THRESHOLD += AI_AGGRESSIVENESS;
    AI_RAISE_THRESHOLD += AI_AGGRESSIVENESS;
    AI_FOLD_THRESHOLD += AI_AGGRESSIVENESS;

    size_t cardsToShow = 0;
    bool allInPhase = false, riverBettingPhase = false, finalStakePhase = false, gameFinished = false;
    Deck deck;
    Hand player1Hand, player2Hand;
    std::vector<Card> communityCards;
    int player1score = 2000, player2score = 2000, player1BetDisplay = 0, player2BetDisplay = 0, pot = 0, pendingStake = 0;
    std::string winnerText;
    int winner = -1;
    bool player1Out = false, player2Out = false, overallGameFinished = false;
    playerAI ai;
    double lastP2WinPercentage = 0.0;
    int lastCardsToShowState = -1;

    startNewRound(deck, player1Hand, player2Hand, communityCards, cardsToShow,
                  player1score, player2score, player1BetDisplay, player2BetDisplay, pot,
                  finalStakePhase, gameFinished, riverBettingPhase, winnerText, winner, allInPhase);

    sf::RenderWindow window(sf::VideoMode(LOGICAL_WIDTH, LOGICAL_HEIGHT), "Poker Table");
    window.setFramerateLimit(60);
    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) {
        if (!font.loadFromFile("C:/Windows/Fonts/arial.ttf") &&
            !font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf") &&
            !font.loadFromFile("/System/Library/Fonts/Supplemental/Arial.ttf")) {
            std::cerr << "Could not load font!" << std::endl;
            return 1;
        }
    }

    sf::View mainView(sf::FloatRect(0.f, 0.f, static_cast<float>(LOGICAL_WIDTH), static_cast<float>(LOGICAL_HEIGHT)));
    auto updateView = [&](sf::Vector2u newWindowSize) {
        float windowRatio = static_cast<float>(newWindowSize.x) / static_cast<float>(newWindowSize.y);
        float viewRatio = static_cast<float>(LOGICAL_WIDTH) / static_cast<float>(LOGICAL_HEIGHT);
        float sizeX = 1.f, sizeY = 1.f, posX = 0.f, posY = 0.f;
        bool horizontalSpacing = windowRatio >= viewRatio;
        if (horizontalSpacing) {
            sizeX = viewRatio / windowRatio;
            posX = (1.f - sizeX) / 2.f;
        } else {
            sizeY = windowRatio / viewRatio;
            posY = (1.f - sizeY) / 2.f;
        }
        mainView.setViewport(sf::FloatRect(posX, posY, sizeX, sizeY));
    };
    updateView(window.getSize());

    Button submitButton   = createButton("Submit", font, {160, 60}, {300, 980}, sf::Color(100, 200, 100), sf::Color::Black);
    Button add10Button    = createButton("+10", font, {80, 60}, {500, 980}, sf::Color(200, 200, 255), sf::Color::Black);
    Button add50Button    = createButton("+50", font, {80, 60}, {600, 980}, sf::Color(200, 200, 255), sf::Color::Black);
    Button add100Button   = createButton("+100", font, {80, 60}, {700, 980}, sf::Color(200, 200, 255), sf::Color::Black);
    Button waitButton     = createButton("Wait", font, {80, 60}, {800, 980}, sf::Color(200, 255, 200), sf::Color::Black);
    Button passButton     = createButton("Pass", font, {80, 60}, {900, 980}, sf::Color(255, 200, 200), sf::Color::Black);
    Button resetButton    = createButton("Reset Bet", font, {120, 60}, {1000, 980}, sf::Color(220, 220, 220), sf::Color::Black);
    Button nextRoundButton = createButton("Next Round", font, {200, 60}, {1200, 980}, sf::Color(255, 255, 100), sf::Color::Black);
    Button playAgainButton = createButton("Play Again", font, {200, 60}, {LOGICAL_WIDTH / 2.0f - 220, 980}, sf::Color(100, 255, 100), sf::Color::Black);
    Button quitButton      = createButton("Quit Game", font, {200, 60}, {LOGICAL_WIDTH / 2.0f + 20, 980}, sf::Color(255, 100, 100), sf::Color::Black);

    std::vector<Button*> activeButtons;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::Resized)
                updateView(sf::Vector2u(event.size.width, event.size.height));
            if (event.type == sf::Event::MouseButtonPressed) {
                sf::Vector2i pixelPos = sf::Mouse::getPosition(window);
                sf::Vector2f worldPos = window.mapPixelToCoords(pixelPos, mainView);

                if (overallGameFinished) {
                    if (isButtonClicked(playAgainButton, sf::Vector2i(worldPos.x, worldPos.y))) {
                        player1score = 2000; player2score = 2000;
                        player1Out = player2Out = overallGameFinished = gameFinished = false;
                        startNewRound(deck, player1Hand, player2Hand, communityCards, cardsToShow,
                                      player1score, player2score, player1BetDisplay, player2BetDisplay, pot,
                                      finalStakePhase, gameFinished, riverBettingPhase, winnerText, winner, allInPhase);
                        lastCardsToShowState = -1;
                    } else if (isButtonClicked(quitButton, sf::Vector2i(worldPos.x, worldPos.y))) {
                        window.close();
                    }
                } else if (!gameFinished && !player1Out && !player2Out) {
                    if (isButtonClicked(add10Button, sf::Vector2i(worldPos.x, worldPos.y)))
                        pendingStake = std::min(pendingStake + 10, player1score);
                    else if (isButtonClicked(add50Button, sf::Vector2i(worldPos.x, worldPos.y)))
                        pendingStake = std::min(pendingStake + 50, player1score);
                    else if (isButtonClicked(add100Button, sf::Vector2i(worldPos.x, worldPos.y)))
                        pendingStake = std::min(pendingStake + 100, player1score);
                    else if (isButtonClicked(waitButton, sf::Vector2i(worldPos.x, worldPos.y)))
                        handlePlayerWaitAction(ai, cardsToShow, riverBettingPhase, finalStakePhase,
                                               player1score, player2score, player1BetDisplay, player2BetDisplay, pot,
                                               allInPhase, player2Hand, communityCards,
                                               winnerText, gameFinished, winner);
                    else if (isButtonClicked(resetButton, sf::Vector2i(worldPos.x, worldPos.y)))
                        pendingStake = 0;
                    else if (isButtonClicked(passButton, sf::Vector2i(worldPos.x, worldPos.y))) {
                        handlePlayerPassAction(gameFinished, winner, winnerText, pot, player2score, player1score);
                        if (player1score <= 0) player1Out = true; 
                        if (player2score <= 0) player2Out = true;
                    } else if (isButtonClicked(submitButton, sf::Vector2i(worldPos.x, worldPos.y))) {
                        if (pendingStake > 0)
                            handlePlayerBetAction(ai, pendingStake, player1score, player2score,
                                                  player1BetDisplay, player2BetDisplay, pot,
                                                  allInPhase, cardsToShow, riverBettingPhase, finalStakePhase,
                                                  player2Hand, communityCards,
                                                  winnerText, gameFinished, winner);
                    }
                } else if (gameFinished && isButtonClicked(nextRoundButton, sf::Vector2i(worldPos.x, worldPos.y)) && !player1Out && !player2Out) {
                    startNewRound(deck, player1Hand, player2Hand, communityCards, cardsToShow,
                                  player1score, player2score, player1BetDisplay, player2BetDisplay, pot,
                                  finalStakePhase, gameFinished, riverBettingPhase, winnerText, winner, allInPhase);
                    lastCardsToShowState = -1;
                } else if (gameFinished && isButtonClicked(playAgainButton, sf::Vector2i(worldPos.x, worldPos.y))) {
                    player1score = 2000; player2score = 2000; pot = 0; pendingStake = 0;
                    winnerText.clear(); winner = -1;
                    player1Out = player2Out = gameFinished = allInPhase = riverBettingPhase = finalStakePhase = false;
                    cardsToShow = 0;
                    player1Hand = Hand(); player2Hand = Hand(); communityCards.clear();
                    deck.reset(); deck.shuffle();
                    startNewRound(deck, player1Hand, player2Hand, communityCards, cardsToShow,
                                  player1score, player2score, player1BetDisplay, player2BetDisplay, pot,
                                  finalStakePhase, gameFinished, riverBettingPhase, winnerText, winner, allInPhase);
                } else if (gameFinished && isButtonClicked(quitButton, sf::Vector2i(worldPos.x, worldPos.y))) {
                    window.close();
                }
            }
        }

        if (finalStakePhase && !gameFinished) {
            cardsToShow = 5;
            gameFinished = true;
            determineAndSetWinner(player1Hand, player2Hand, communityCards, pot, player1score, player2score,
                                  winnerText, winner, player1Out, player2Out);
        }
        // Game over check
        if (!overallGameFinished && (player1Out || player2Out)) {
            overallGameFinished = true;
            gameFinished = true;
            if (player1Out && player2Out)
                winnerText = "Game Over! It's a draw somehow!";
            else if (player1Out)
                winnerText = "Game Over! Player 2 Wins!";
            else if (player2Out)
                winnerText = "Game Over! Player 1 Wins!";
        }

        updateAIInfo(ai, player2Hand, communityCards, cardsToShow, lastP2WinPercentage, lastCardsToShowState);

        activeButtons.clear();
        if (overallGameFinished) {
            activeButtons.push_back(&playAgainButton);
            activeButtons.push_back(&quitButton);
        } else if (!gameFinished && !player1Out && !player2Out) {
            activeButtons.push_back(&submitButton);
            activeButtons.push_back(&add10Button);
            activeButtons.push_back(&add50Button);
            activeButtons.push_back(&add100Button);
            activeButtons.push_back(&waitButton);
            activeButtons.push_back(&passButton);
            activeButtons.push_back(&resetButton);
        } else if (gameFinished && (!player1Out && !player2Out)) {
            activeButtons.push_back(&nextRoundButton);
        } else if (gameFinished && (player1Out || player2Out)) {
            activeButtons.push_back(&playAgainButton);
            activeButtons.push_back(&quitButton);
        }

        window.clear(sf::Color(0, 100, 0));
        window.setView(mainView);

        drawGameElements(window, font, player1Hand, player2Hand, communityCards, cardsToShow,
                         player1BetDisplay, player2BetDisplay, pendingStake, pot, player1score, player2score,
                         gameFinished, winnerText, activeButtons, lastP2WinPercentage);

        window.display();
    }
    return 0;
}

void startNewRound(Deck& deck, Hand& player1Hand, Hand& player2Hand, std::vector<Card>& communityCards,
                   size_t& cardsToShow, int& player1score, int& player2score, 
                   int& player1BetDisplay, int& player2BetDisplay, int& pot,
                   bool& finalStakePhase, bool& gameFinished, bool& riverBettingPhase,
                   std::string& winnerText, int& winner, bool& allInPhase) {
    deck.reset();
    deck.shuffle();
    player1Hand = Hand(); 
    player2Hand = Hand();
    for (int i = 0; i < 2; ++i) {
        player1Hand.addCard(deck.draw());
        player2Hand.addCard(deck.draw());
    }
    communityCards.clear();
    for (int i = 0; i < 5; ++i)
        communityCards.push_back(deck.draw());

    cardsToShow = 0;
    int blindAmount = 50;
    player1BetDisplay = 0;
    player2BetDisplay = 0;
    int p1BlindPaid = std::min(blindAmount, player1score);
    player1score -= p1BlindPaid;
    player1BetDisplay = p1BlindPaid;
    int p2BlindPaid = std::min(blindAmount, player2score);
    player2score -= p2BlindPaid;
    player2BetDisplay = p2BlindPaid;
    pot = p1BlindPaid + p2BlindPaid;
    finalStakePhase = gameFinished = riverBettingPhase = allInPhase = false;
    winnerText.clear();
    winner = -1;
}

void advanceGamePhase(size_t& cardsToShow, bool& riverBettingPhase, bool& finalStakePhase,
                      int& player1BetDisplay, int& player2BetDisplay) {
    player1BetDisplay = player2BetDisplay = 0;
    if (cardsToShow == 0) cardsToShow = 3;
    else if (cardsToShow == 3) cardsToShow = 4;
    else if (cardsToShow == 4) { cardsToShow = 5; riverBettingPhase = true; }
    else if (riverBettingPhase) { finalStakePhase = true; riverBettingPhase = false; }
}

void determineAndSetWinner(Hand& player1Hand, Hand& player2Hand, const std::vector<Card>& communityCards, 
                           int& pot, int& player1score, int& player2score,
                           std::string& winnerText, int& winner, bool& player1Out, bool& player2Out) {
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
        player2score += pot - (pot / 2); 
    }
    pot = 0; 
    if (player1score <= 0) player1Out = true;
    if (player2score <= 0) player2Out = true;
}

void handlePlayerBetAction(playerAI& ai,
                           int& pendingStake, int& player1score, int& player2score,
                           int& player1BetDisplay, int& player2BetDisplay, int& pot,
                           bool& allInPhase, size_t& cardsToShow, bool& riverBettingPhase, bool& finalStakePhase,
                           const Hand& player2Hand, const std::vector<Card>& communityCards,
                           std::string& winnerText, bool& gameFinished, int& winner) {
    int p1TotalBetForStreet = pendingStake; 
    int p1AdditionalBet = p1TotalBetForStreet - player1BetDisplay;
    if (p1TotalBetForStreet < player2BetDisplay && p1TotalBetForStreet < (player1score + player1BetDisplay)) {
        p1TotalBetForStreet = player2BetDisplay; 
        p1AdditionalBet = p1TotalBetForStreet - player1BetDisplay;
    }
    p1AdditionalBet = std::min(p1AdditionalBet, player1score);
    if (p1AdditionalBet < 0) p1AdditionalBet = 0;
    player1score -= p1AdditionalBet;
    pot += p1AdditionalBet;
    player1BetDisplay += p1AdditionalBet;
    pendingStake = 0;
    if (player1score == 0) allInPhase = true;
    winnerText = "You bet to " + std::to_string(player1BetDisplay) + ".";

    if (gameFinished) return;
    int amountForAIToCall = player1BetDisplay - player2BetDisplay;
    std::vector<Card> visibleBoard;
    if (cardsToShow <= communityCards.size())
         visibleBoard.assign(communityCards.begin(), communityCards.begin() + std::min(cardsToShow, communityCards.size()));
    double winChance = ai.evaluateHand(player2Hand, visibleBoard); 

    int aiActionAmount = 0; 
    if (amountForAIToCall > 0) { 
        if (winChance < AI_FOLD_THRESHOLD && player2score > amountForAIToCall) { 
            gameFinished = true;
            winner = 0;
            winnerText += " Player 2 folds. You win!";
            player1score += pot; 
            pot = 0;
        } else if (winChance < AI_RAISE_THRESHOLD || amountForAIToCall >= player2score) { 
            aiActionAmount = std::min(amountForAIToCall, player2score);
            player2score -= aiActionAmount;
            pot += aiActionAmount;
            player2BetDisplay += aiActionAmount;
            winnerText += " Player 2 calls " + std::to_string(aiActionAmount) + ".";
            if (player2score == 0) allInPhase = true;
            if (!allInPhase && !gameFinished)
                advanceGamePhase(cardsToShow, riverBettingPhase, finalStakePhase, player1BetDisplay, player2BetDisplay);
        } else { 
            int aiCallPart = std::min(amountForAIToCall, player2score);
            int aiCanRaiseMax = player2score - aiCallPart;
            if (aiCanRaiseMax > 0) {
                int aiRaisePart = std::min({player1BetDisplay, pot / 2, aiCanRaiseMax}); 
                if (aiRaisePart <= 0) aiRaisePart = std::min(50, aiCanRaiseMax);
                aiActionAmount = aiCallPart + aiRaisePart;
                player2score -= aiActionAmount;
                pot += aiActionAmount;
                player2BetDisplay += aiActionAmount;
                winnerText += " Player 2 raises to " + std::to_string(player2BetDisplay) + ".";
                if (player2score == 0) allInPhase = true;
            } else { 
                aiActionAmount = std::min(amountForAIToCall, player2score);
                player2score -= aiActionAmount;
                pot += aiActionAmount;
                player2BetDisplay += aiActionAmount;
                winnerText += " Player 2 calls " + std::to_string(aiActionAmount) + ".";
                if (player2score == 0) allInPhase = true;
                if (!allInPhase && !gameFinished)
                    advanceGamePhase(cardsToShow, riverBettingPhase, finalStakePhase, player1BetDisplay, player2BetDisplay);
            }
        }
    } else {
        if (player1BetDisplay == player2BetDisplay) {
             winnerText += " Stakes are even.";
             if (!allInPhase && !gameFinished)
                advanceGamePhase(cardsToShow, riverBettingPhase, finalStakePhase, player1BetDisplay, player2BetDisplay);
        }
    }
    if ((player1score == 0 || player2score == 0) && !gameFinished)
        allInPhase = true;
    if (allInPhase && !gameFinished) {
        cardsToShow = 5;
        finalStakePhase = true;
        riverBettingPhase = false;
    }
}

void handlePlayerWaitAction(playerAI& ai, 
                            size_t& cardsToShow, bool& riverBettingPhase, bool& finalStakePhase,
                            int& player1score, int& player2score,
                            int& player1BetDisplay, int& player2BetDisplay, int& pot,
                            bool& allInPhase,
                            const Hand& player2Hand, const std::vector<Card>& communityCards,
                            std::string& winnerText, bool& gameFinished, int& winner) {
    if (player1BetDisplay < player2BetDisplay) { 
        int amountToCall = player2BetDisplay - player1BetDisplay;
        int p1ActualCall = std::min(amountToCall, player1score);
        player1score -= p1ActualCall;
        pot += p1ActualCall;
        player1BetDisplay += p1ActualCall;
        winnerText = "You call " + std::to_string(p1ActualCall) + ".";
        if (player1score == 0) allInPhase = true;
        if (!allInPhase && !gameFinished)
            advanceGamePhase(cardsToShow, riverBettingPhase, finalStakePhase, player1BetDisplay, player2BetDisplay);
    } else { 
        winnerText = "You check.";
        std::vector<Card> visibleBoard;
        if (cardsToShow <= communityCards.size())
            visibleBoard.assign(communityCards.begin(), communityCards.begin() + std::min(cardsToShow, communityCards.size()));
        double winChance = ai.evaluateHand(player2Hand, visibleBoard);
        if (winChance > AI_RAISE_THRESHOLD && player2score > 0) {
            int aiBetAmount = std::min({pot / 2, player2score / 2, player2score}); 
            if (aiBetAmount <= 0) aiBetAmount = std::min(50, player2score);
            if (aiBetAmount > 0) {
                player2score -= aiBetAmount;
                pot += aiBetAmount;
                player2BetDisplay += aiBetAmount;
                winnerText += " Player 2 bets " + std::to_string(aiBetAmount) + ".";
                if (player2score == 0) allInPhase = true;
            } else { 
                 winnerText += " Player 2 checks.";
                 if (!allInPhase && !gameFinished)
                    advanceGamePhase(cardsToShow, riverBettingPhase, finalStakePhase, player1BetDisplay, player2BetDisplay);
            }
        } else {
            winnerText += " Player 2 checks.";
            if (!allInPhase && !gameFinished)
                advanceGamePhase(cardsToShow, riverBettingPhase, finalStakePhase, player1BetDisplay, player2BetDisplay);
        }
    }
    if ((player1score == 0 || player2score == 0) && !gameFinished)
        allInPhase = true;
    if (allInPhase && !gameFinished) {
        cardsToShow = 5;
        finalStakePhase = true;
        riverBettingPhase = false;
    }
}

void handlePlayerPassAction(bool& gameFinished, int& winner, std::string& winnerText, int& pot, int& player2score, int& player1score) {
    gameFinished = true;
    winner = 1;
    winnerText = "You passed. Player 2 wins the pot of " + std::to_string(pot) + "!";
    player2score += pot;
    pot = 0;
}

void updateAIInfo(playerAI& ai, const Hand& player2Hand, const std::vector<Card>& communityCards, size_t cardsToShow,
                  double& lastP2WinPercentage, int& lastCardsToShowState) {
    if (static_cast<int>(cardsToShow) != lastCardsToShowState) {
        std::vector<Card> visibleBoard;
        if (cardsToShow > 0 && cardsToShow <= communityCards.size())
            visibleBoard.assign(communityCards.begin(), communityCards.begin() + cardsToShow);
        lastP2WinPercentage = ai.evaluateHand(player2Hand, visibleBoard) * 100.0;
        lastCardsToShowState = static_cast<int>(cardsToShow);
    }
}

void drawHand(const Hand& hand, sf::RenderWindow& window, sf::Font& font, float yPosCategory, bool showCards) {
    const auto& cards = hand.getCards();
    float handStartX = 800; 
    float cardOffsetY = (yPosCategory == 900) ? 800 : 200; 
    for (size_t i = 0; i < cards.size(); ++i) { 
        float x = handStartX + i * 120;
        Card card = cards[i]; 
        card.setFaceUp(showCards);
        card.draw(window, x, cardOffsetY);
    }
}

void drawCommunityCards(const std::vector<Card>& communityCards, size_t cardsToShow, sf::RenderWindow& window, sf::Font& font) {
    float communityY = 500;
    float communityStartX = (1920 - (5 * 110 - 10)) / 2.0f;
    for (size_t i = 0; i < communityCards.size(); ++i) {
        float x = communityStartX + i * 120; 
        Card card = communityCards[i];
        card.setFaceUp(i < cardsToShow);
        card.draw(window, x, communityY);
    }
}

void drawStakes(int player1BetDisplay, int player2BetDisplay, int pendingStake, sf::RenderWindow& window, sf::Font& font) {
    sf::Text p1BetText("P1 Bet: " + std::to_string(player1BetDisplay), font, 20);
    p1BetText.setFillColor(sf::Color::Yellow);
    p1BetText.setPosition(50, LOGICAL_HEIGHT - 80);
    window.draw(p1BetText);

    sf::Text p2BetText("P2 Bet: " + std::to_string(player2BetDisplay), font, 20);
    p2BetText.setFillColor(sf::Color::Yellow);
    p2BetText.setPosition(50, 40);
    window.draw(p2BetText);

    sf::Text pendingText("Your Bet: " + std::to_string(pendingStake), font, 30);
    pendingText.setFillColor(sf::Color::White);
    pendingText.setPosition(50, LOGICAL_HEIGHT - 140);
    window.draw(pendingText);
}

void drawGameElements(sf::RenderWindow& window, sf::Font& font,
                      const Hand& player1Hand, const Hand& player2Hand,
                      const std::vector<Card>& communityCards, size_t cardsToShow,
                      int player1BetDisplay, int player2BetDisplay, int pendingStake, int pot,
                      int player1score, int player2score,
                      bool gameFinished, const std::string& winnerText,
                      const std::vector<Button*>& activeButtons,
                      double lastP2WinPercentage) {
    // Uncomment for AI win% overlay
    /*
    sf::Text p2winText("Opponent Win%: " + std::to_string(static_cast<int>(lastP2WinPercentage)) + "%", font, 28);
    p2winText.setFillColor(sf::Color::Magenta);
    p2winText.setPosition(1200, 40);
    window.draw(p2winText);
    */
    drawHand(player1Hand, window, font, 900, true);
    drawHand(player2Hand, window, font, 100, gameFinished);
    drawCommunityCards(communityCards, cardsToShow, window, font);
    drawStakes(player1BetDisplay, player2BetDisplay, pendingStake, window, font);
    sf::Text potText("Pot: " + std::to_string(pot), font, 28);
    potText.setFillColor(sf::Color::Cyan);
    potText.setPosition(900, 400); 
    window.draw(potText);

    sf::Text scoreText1("Player 1 Score: " + std::to_string(player1score), font, 20);
    scoreText1.setFillColor(sf::Color::White);
    scoreText1.setPosition(1500, 1000);
    window.draw(scoreText1);

    sf::Text scoreText2("Player 2 Score: " + std::to_string(player2score), font, 20);
    scoreText2.setFillColor(sf::Color::White);
    scoreText2.setPosition(1500, 40);
    window.draw(scoreText2);

    if (gameFinished) {
        sf::Text winAnnounceText(winnerText, font, 36);
        winAnnounceText.setFillColor(sf::Color::White);
        sf::FloatRect textRect = winAnnounceText.getLocalBounds();
        winAnnounceText.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
        winAnnounceText.setPosition(LOGICAL_WIDTH / 2.0f, LOGICAL_HEIGHT / 2.0f + 200.f); 
        window.draw(winAnnounceText);
    }
    for (const auto& btnPtr : activeButtons)
        if (btnPtr) drawButton(window, *btnPtr);
}

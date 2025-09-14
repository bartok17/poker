#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <cctype>
#include <cstdlib> // For rand, srand
#include <ctime>   // For time
#include "hand.h"
#include "card.h"
#include "comparer.h"
#include "deck.h"
#include "playerAI.h"
#include "ui.h" // NEW

// -----------------------------------------------------------------------------
// Constants and globals
// -----------------------------------------------------------------------------
constexpr unsigned int LOGICAL_WIDTH  = 1920;
constexpr unsigned int LOGICAL_HEIGHT = 1080;

constexpr int DEFAULT_STACK = 2000;
constexpr int BLIND_AMOUNT  = 50;

// UI layout
constexpr float BUTTON_Y = 980.f;
constexpr float CARD_X_SPACING = 120.f;
constexpr float HAND_START_X   = 800.f;
constexpr float P1_HAND_Y      = 800.f;
constexpr float P2_HAND_Y      = 200.f;
constexpr float COMMUNITY_Y    = 500.f;

// AI thresholds (tuned elsewhere at runtime)
float AI_FOLD_THRESHOLD = 0.35f;
float AI_RAISE_THRESHOLD = 0.55f;
float AI_CALL_THRESHOLD = 0.55f;
float AI_AGGRESSIVENESS = 0.1f;

// -----------------------------------------------------------------------------
// Helpers
// -----------------------------------------------------------------------------
static std::vector<Card> makeVisibleBoard(const std::vector<Card>& communityCards, size_t cardsToShow) {
    std::vector<Card> visible;
    if (cardsToShow > 0 && cardsToShow <= communityCards.size()) {
        visible.assign(communityCards.begin(), communityCards.begin() + cardsToShow);
    }
    return visible;
}

static void enterAllInIfNeeded(bool gameFinished,
                               int player1score, int player2score,
                               bool& allInPhase,
                               size_t& cardsToShow, bool& riverBettingPhase, bool& finalStakePhase) {
    if ((player1score == 0 || player2score == 0) && !gameFinished) {
        allInPhase = true;
    }
    if (allInPhase && !gameFinished) {
        cardsToShow = 5;
        finalStakePhase = true;
        riverBettingPhase = false;
    }
}

// Stretches fold threshold based on bet/stack ratio and small randomness (same logic, centralized)
static float computeCurrentFoldThreshold(int amountForAIToCall, size_t cardsToShow, int player2score) {
    float current = AI_FOLD_THRESHOLD;

    if (cardsToShow > 0 && amountForAIToCall > 0 && player2score > 0) {
        float bet_to_stack_ratio = static_cast<float>(amountForAIToCall) / player2score;

        float calculated_increase = 0.0f;
        if (bet_to_stack_ratio > 0.5f) {
            calculated_increase = 0.2f;
        } else if (bet_to_stack_ratio > 0.25f) {
            calculated_increase = 0.1f;
        }

        float actual_increase = calculated_increase;
        if (calculated_increase > 0.0f) {
            float bluff_call_tendency_chance = 0.3f; // 30% chance to be less cautious
            if ((static_cast<float>(rand()) / RAND_MAX) < bluff_call_tendency_chance) {
                actual_increase *= ((static_cast<float>(rand()) / RAND_MAX) * 0.7f);
            }
        }

        current = std::min(current + actual_increase, 0.9f);
    }

    return current;
}

// -----------------------------------------------------------------------------
// Forward declarations (game flow)
// -----------------------------------------------------------------------------
void startNewRound(Deck& deck, Hand& player1Hand, Hand& player2Hand, std::vector<Card>& communityCards,
                   size_t& cardsToShow, int& player1score, int& player2score, 
                   int& player1BetDisplay, int& player2BetDisplay, int& pot,
                   bool& finalStakePhase, bool& gameFinished, bool& riverBettingPhase,
                   std::string& winnerText, int& winner, bool& allInPhase);

void advanceGamePhase(size_t& cardsToShow, bool& riverBettingPhase, bool& finalStakePhase,
                      int& player1BetDisplay, int& player2BetDisplay);

void determineAndSetWinner(Hand& player1Hand, Hand& player2Hand, const std::vector<Card>& communityCards, 
                           int& pot, int& player1score, int& player2score,
                           std::string& winnerText, int& winner, bool& player1Out, bool& player2Out,
                           const std::string& player1Name, const std::string& player2Name);

void handlePlayerBetAction(playerAI& ai,
                           int& pendingStake, int& player1score, int& player2score,
                           int& player1BetDisplay, int& player2BetDisplay, int& pot,
                           bool& allInPhase, size_t& cardsToShow, bool& riverBettingPhase, bool& finalStakePhase,
                           const Hand& player2Hand, const std::vector<Card>& communityCards,
                           std::string& winnerText, bool& gameFinished, int& winner,
                           const std::string& player1Name, const std::string& player2Name);

void handlePlayerWaitAction(playerAI& ai, 
                            size_t& cardsToShow, bool& riverBettingPhase, bool& finalStakePhase,
                            int& player1score, int& player2score,
                            int& player1BetDisplay, int& player2BetDisplay, int& pot,
                            bool& allInPhase,
                            const Hand& player2Hand, const std::vector<Card>& communityCards, 
                            std::string& winnerText, bool& gameFinished, int& winner,
                            const std::string& player1Name, const std::string& player2Name);

void handlePlayerPassAction(bool& gameFinished, int& winner, std::string& winnerText, int& pot, int& player2score, int& player1score, const std::string& player2Name);

void updateAIInfo(playerAI& ai, const Hand& player2Hand, const std::vector<Card>& communityCards, size_t cardsToShow,
                  double& lastP2WinPercentage, int& lastCardsToShowState);

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------
int main() {
    srand(static_cast<unsigned int>(time(nullptr))); // Seed for rand()

    // Slight variability per run
    AI_AGGRESSIVENESS = ((static_cast<float>(rand()) / RAND_MAX) * 0.2f) - 0.1f;
    AI_CALL_THRESHOLD  += AI_AGGRESSIVENESS;
    AI_RAISE_THRESHOLD += AI_AGGRESSIVENESS;
    AI_FOLD_THRESHOLD  += AI_AGGRESSIVENESS;

    std::string player1Name = "Player";
    std::vector<std::string> aiNames = {"Ben", "Ken", "Friederick", "Viper", "Jester", "Jonathan", "michał"};
    std::string player2Name = aiNames[rand() % aiNames.size()];

    size_t cardsToShow = 0;
    bool allInPhase = false, riverBettingPhase = false, finalStakePhase = false, gameFinished = false;
    Deck deck;
    Hand player1Hand, player2Hand;
    std::vector<Card> communityCards;

    int player1score = DEFAULT_STACK, player2score = DEFAULT_STACK;
    int player1BetDisplay = 0, player2BetDisplay = 0, pot = 0, pendingStake = 0;

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

    // Buttons (type changed to ui::Button; factory call namespaced)
    ui::Button submitButton    = ui::createButton("Submit",     font, {160, 60}, {300,  BUTTON_Y}, sf::Color(100, 200, 100), sf::Color::Black);
    ui::Button add10Button     = ui::createButton("+10",        font, { 80, 60}, {500,  BUTTON_Y}, sf::Color(200, 200, 255), sf::Color::Black);
    ui::Button add50Button     = ui::createButton("+50",        font, { 80, 60}, {600,  BUTTON_Y}, sf::Color(200, 200, 255), sf::Color::Black);
    ui::Button add100Button    = ui::createButton("+100",       font, { 80, 60}, {700,  BUTTON_Y}, sf::Color(200, 200, 255), sf::Color::Black);
    ui::Button waitButton      = ui::createButton("Wait",       font, { 80, 60}, {800,  BUTTON_Y}, sf::Color(200, 255, 200), sf::Color::Black);
    ui::Button passButton      = ui::createButton("Pass",       font, { 80, 60}, {900,  BUTTON_Y}, sf::Color(255, 200, 200), sf::Color::Black);
    ui::Button resetButton     = ui::createButton("Reset Bet",  font, {120, 60}, {1000, BUTTON_Y}, sf::Color(220, 220, 220), sf::Color::Black);
    ui::Button nextRoundButton = ui::createButton("Next Round", font, {200, 60}, {1200, BUTTON_Y}, sf::Color(255, 255, 100), sf::Color::Black);
    ui::Button playAgainButton = ui::createButton("Play Again", font, {200, 60}, {LOGICAL_WIDTH / 2.0f - 220, BUTTON_Y}, sf::Color(100, 255, 100), sf::Color::Black);
    ui::Button quitButton      = ui::createButton("Quit Game",  font, {200, 60}, {LOGICAL_WIDTH / 2.0f +  20, BUTTON_Y}, sf::Color(255, 100, 100), sf::Color::Black);

    std::vector<ui::Button*> activeButtons;

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
                    if (ui::isButtonClicked(playAgainButton, sf::Vector2i(worldPos.x, worldPos.y))) {
                        player1score = DEFAULT_STACK; player2score = DEFAULT_STACK;
                        player1Out = player2Out = overallGameFinished = gameFinished = false;
                        startNewRound(deck, player1Hand, player2Hand, communityCards, cardsToShow,
                                      player1score, player2score, player1BetDisplay, player2BetDisplay, pot,
                                      finalStakePhase, gameFinished, riverBettingPhase, winnerText, winner, allInPhase);
                        lastCardsToShowState = -1;
                    } else if (ui::isButtonClicked(quitButton, sf::Vector2i(worldPos.x, worldPos.y))) {
                        window.close();
                    }
                } else if (!gameFinished && !player1Out && !player2Out) {
                    if (ui::isButtonClicked(add10Button, sf::Vector2i(worldPos.x, worldPos.y)))
                        pendingStake = std::min(pendingStake + 10, player1score);
                    else if (ui::isButtonClicked(add50Button, sf::Vector2i(worldPos.x, worldPos.y)))
                        pendingStake = std::min(pendingStake + 50, player1score);
                    else if (ui::isButtonClicked(add100Button, sf::Vector2i(worldPos.x, worldPos.y)))
                        pendingStake = std::min(pendingStake + 100, player1score);
                    else if (ui::isButtonClicked(waitButton, sf::Vector2i(worldPos.x, worldPos.y)))
                        handlePlayerWaitAction(ai, cardsToShow, riverBettingPhase, finalStakePhase,
                                               player1score, player2score, player1BetDisplay, player2BetDisplay, pot,
                                               allInPhase, player2Hand, communityCards,
                                               winnerText, gameFinished, winner, player1Name, player2Name);
                    else if (ui::isButtonClicked(resetButton, sf::Vector2i(worldPos.x, worldPos.y)))
                        pendingStake = 0;
                    else if (ui::isButtonClicked(passButton, sf::Vector2i(worldPos.x, worldPos.y))) {
                        handlePlayerPassAction(gameFinished, winner, winnerText, pot, player2score, player1score, player2Name);
                        if (player1score <= 0) player1Out = true; 
                        if (player2score <= 0) player2Out = true;
                    } else if (ui::isButtonClicked(submitButton, sf::Vector2i(worldPos.x, worldPos.y))) {
                        if (pendingStake > 0)
                            handlePlayerBetAction(ai, pendingStake, player1score, player2score,
                                                  player1BetDisplay, player2BetDisplay, pot,
                                                  allInPhase, cardsToShow, riverBettingPhase, finalStakePhase,
                                                  player2Hand, communityCards,
                                                  winnerText, gameFinished, winner, player1Name, player2Name);
                    }
                } else if (gameFinished && ui::isButtonClicked(nextRoundButton, sf::Vector2i(worldPos.x, worldPos.y)) && !player1Out && !player2Out) {
                    startNewRound(deck, player1Hand, player2Hand, communityCards, cardsToShow,
                                  player1score, player2score, player1BetDisplay, player2BetDisplay, pot,
                                  finalStakePhase, gameFinished, riverBettingPhase, winnerText, winner, allInPhase);
                    lastCardsToShowState = -1;
                } else if (gameFinished && ui::isButtonClicked(playAgainButton, sf::Vector2i(worldPos.x, worldPos.y))) {
                    player1score = DEFAULT_STACK; player2score = DEFAULT_STACK; pot = 0; pendingStake = 0;
                    winnerText.clear(); winner = -1;
                    player1Out = player2Out = gameFinished = allInPhase = riverBettingPhase = finalStakePhase = false;
                    cardsToShow = 0;
                    player1Hand = Hand(); player2Hand = Hand(); communityCards.clear();
                    deck.reset(); deck.shuffle();
                    startNewRound(deck, player1Hand, player2Hand, communityCards, cardsToShow,
                                  player1score, player2score, player1BetDisplay, player2BetDisplay, pot,
                                  finalStakePhase, gameFinished, riverBettingPhase, winnerText, winner, allInPhase);
                } else if (gameFinished && ui::isButtonClicked(quitButton, sf::Vector2i(worldPos.x, worldPos.y))) {
                    window.close();
                }
            }
        }

        if (finalStakePhase && !gameFinished) {
            cardsToShow = 5;
            gameFinished = true;
            determineAndSetWinner(player1Hand, player2Hand, communityCards, pot, player1score, player2score,
                                  winnerText, winner, player1Out, player2Out, player1Name, player2Name);
        }

        // Game over check
        if (!overallGameFinished && (player1Out || player2Out)) {
            overallGameFinished = true;
            gameFinished = true;
            if (player1Out && player2Out)
                winnerText = "Game Over! It's a draw somehow!";
            else if (player1Out)
                winnerText = "Game Over! " + player2Name + " Wins!";
            else if (player2Out)
                winnerText = "Game Over! " + player1Name + " Wins!";
        }

        updateAIInfo(ai, player2Hand, communityCards, cardsToShow, lastP2WinPercentage, lastCardsToShowState);

        // Buttons to show
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

        // Render
        window.clear(sf::Color(0, 75, 0));
        window.setView(mainView);

        ui::drawGameElements(window, font,
                             player1Hand, player2Hand,
                             communityCards, cardsToShow,
                             player1BetDisplay, player2BetDisplay, pendingStake, pot,
                             player1score, player2score,
                             gameFinished, winnerText,
                             activeButtons,
                             lastP2WinPercentage,
                             player1Name, player2Name,
                             LOGICAL_WIDTH, LOGICAL_HEIGHT);

        window.display();
    }
    return 0;
}

// -----------------------------------------------------------------------------
// Game flow implementation
// -----------------------------------------------------------------------------
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
    player1BetDisplay = 0;
    player2BetDisplay = 0;

    const int p1BlindPaid = std::min(BLIND_AMOUNT, player1score);
    player1score -= p1BlindPaid;
    player1BetDisplay = p1BlindPaid;

    const int p2BlindPaid = std::min(BLIND_AMOUNT, player2score);
    player2score -= p2BlindPaid;
    player2BetDisplay = p2BlindPaid;

    pot = p1BlindPaid + p2BlindPaid;

    finalStakePhase = false;
    gameFinished = false;
    riverBettingPhase = false;
    allInPhase = false;

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
                           std::string& winnerText, int& winner, bool& player1Out, bool& player2Out,
                           const std::string& player1Name, const std::string& player2Name) {
    Hand fullHand1 = player1Hand.combineHands(Hand{communityCards});
    Hand fullHand2 = player2Hand.combineHands(Hand{communityCards});
    int cmp = Comparer::compareHands(fullHand1, fullHand2);

    if (cmp == 0) {
        winnerText = player1Name + " wins the pot of " + std::to_string(pot) + "!";
        winner = 0;
        player1score += pot;
    } else if (cmp == 1) {
        winnerText = player2Name + " wins the pot of " + std::to_string(pot) + "!";
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
                           std::string& winnerText, bool& gameFinished, int& winner,
                           const std::string& player1Name, const std::string& player2Name) {
    // Normalize to at least call if under-bet
    int p1TotalBetForStreet = pendingStake; 
    int p1AdditionalBet = p1TotalBetForStreet - player1BetDisplay;
    if (p1TotalBetForStreet < player2BetDisplay && p1TotalBetForStreet < (player1score + player1BetDisplay)) {
        p1TotalBetForStreet = player2BetDisplay; 
        p1AdditionalBet = p1TotalBetForStreet - player1BetDisplay;
    }
    p1AdditionalBet = std::max(0, std::min(p1AdditionalBet, player1score));

    player1score -= p1AdditionalBet;
    pot         += p1AdditionalBet;
    player1BetDisplay += p1AdditionalBet;
    pendingStake = 0;
    if (player1score == 0) allInPhase = true;

    winnerText = "You bet to " + std::to_string(player1BetDisplay) + ".";

    if (gameFinished) return;

    const int amountForAIToCall = player1BetDisplay - player2BetDisplay;
    const auto visibleBoard = makeVisibleBoard(communityCards, cardsToShow);
    const double winChance = ai.evaluateHand(player2Hand, visibleBoard); 

    const float current_fold_threshold = computeCurrentFoldThreshold(amountForAIToCall, cardsToShow, player2score);

    int aiActionAmount = 0; 
    if (amountForAIToCall > 0) {
        if (winChance < current_fold_threshold && player2score > amountForAIToCall) {
            gameFinished = true;
            winner = 0;
            winnerText += " " + player2Name + " folds. " + player1Name + " wins!";
            player1score += pot; 
            pot = 0;
        } else if (winChance < AI_RAISE_THRESHOLD || amountForAIToCall >= player2score) { 
            aiActionAmount = std::min(amountForAIToCall, player2score);
            player2score -= aiActionAmount;
            pot += aiActionAmount;
            player2BetDisplay += aiActionAmount;
            winnerText += " " + player2Name + " calls " + std::to_string(aiActionAmount) + ".";
            if (player2score == 0) allInPhase = true;
            if (!allInPhase && !gameFinished)
                advanceGamePhase(cardsToShow, riverBettingPhase, finalStakePhase, player1BetDisplay, player2BetDisplay);
        } else { 
            const int aiCallPart = std::min(amountForAIToCall, player2score);
            const int aiCanRaiseMax = player2score - aiCallPart;
            if (aiCanRaiseMax > 0) {
                int aiRaisePart = std::min({player1BetDisplay, pot / 2, aiCanRaiseMax}); 
                if (aiRaisePart <= 0) aiRaisePart = std::min(50, aiCanRaiseMax); // Ensure some raise if possible
                aiActionAmount = aiCallPart + aiRaisePart;
                player2score -= aiActionAmount;
                pot += aiActionAmount;
                player2BetDisplay += aiActionAmount;
                winnerText += " " + player2Name + " raises to " + std::to_string(player2BetDisplay) + ".";
                if (player2score == 0) allInPhase = true;
            } else { 
                aiActionAmount = std::min(amountForAIToCall, player2score);
                player2score -= aiActionAmount;
                pot += aiActionAmount;
                player2BetDisplay += aiActionAmount;
                winnerText += " " + player2Name + " calls " + std::to_string(aiActionAmount) + ".";
                if (player2score == 0) allInPhase = true;
                if (!allInPhase && !gameFinished)
                    advanceGamePhase(cardsToShow, riverBettingPhase, finalStakePhase, player1BetDisplay, player2BetDisplay);
            }
        }
    } else { // amountForAIToCall <= 0
        if (player1BetDisplay == player2BetDisplay) {
             winnerText += " Stakes are even.";
             if (!allInPhase && !gameFinished)
                advanceGamePhase(cardsToShow, riverBettingPhase, finalStakePhase, player1BetDisplay, player2BetDisplay);
        }
    }

    enterAllInIfNeeded(gameFinished, player1score, player2score, allInPhase, cardsToShow, riverBettingPhase, finalStakePhase);
}

void handlePlayerWaitAction(playerAI& ai, 
                            size_t& cardsToShow, bool& riverBettingPhase, bool& finalStakePhase,
                            int& player1score, int& player2score,
                            int& player1BetDisplay, int& player2BetDisplay, int& pot,
                            bool& allInPhase,
                            const Hand& player2Hand, const std::vector<Card>& communityCards,
                            std::string& winnerText, bool& gameFinished, int& winner,
                            const std::string& player1Name, const std::string& player2Name) {
    if (player1BetDisplay < player2BetDisplay) { 
        const int amountToCall = player2BetDisplay - player1BetDisplay;
        const int p1ActualCall = std::min(amountToCall, player1score);
        player1score -= p1ActualCall;
        pot += p1ActualCall;
        player1BetDisplay += p1ActualCall;
        winnerText = "You call " + std::to_string(p1ActualCall) + ".";
        if (player1score == 0) allInPhase = true;
        if (!allInPhase && !gameFinished)
            advanceGamePhase(cardsToShow, riverBettingPhase, finalStakePhase, player1BetDisplay, player2BetDisplay);
    } else { 
        winnerText = "You check.";
        const auto visibleBoard = makeVisibleBoard(communityCards, cardsToShow);
        const double winChance = ai.evaluateHand(player2Hand, visibleBoard);

        if (winChance > AI_RAISE_THRESHOLD && player2score > 0) {
            int aiBetAmount = std::min({pot / 2, player2score / 2, player2score}); 
            if (aiBetAmount <= 0) aiBetAmount = std::min(50, player2score);
            if (aiBetAmount > 0) {
                player2score -= aiBetAmount;
                pot += aiBetAmount;
                player2BetDisplay += aiBetAmount;
                winnerText += " " + player2Name + " bets " + std::to_string(aiBetAmount) + ".";
                if (player2score == 0) allInPhase = true;
            } else { 
                 winnerText += " " + player2Name + " checks.";
                 if (!allInPhase && !gameFinished)
                    advanceGamePhase(cardsToShow, riverBettingPhase, finalStakePhase, player1BetDisplay, player2BetDisplay);
            }
        } else {
            winnerText += " " + player2Name + " checks.";
            if (!allInPhase && !gameFinished)
                advanceGamePhase(cardsToShow, riverBettingPhase, finalStakePhase, player1BetDisplay, player2BetDisplay);
        }
    }

    enterAllInIfNeeded(gameFinished, player1score, player2score, allInPhase, cardsToShow, riverBettingPhase, finalStakePhase);
}

void handlePlayerPassAction(bool& gameFinished, int& winner, std::string& winnerText, int& pot, int& player2score, int& player1score, const std::string& player2Name) {
    gameFinished = true;
    winner = 1;
    winnerText = "You passed. " + player2Name + " wins the pot of " + std::to_string(pot) + "!";
    player2score += pot;
    pot = 0;
}

void updateAIInfo(playerAI& ai, const Hand& player2Hand, const std::vector<Card>& communityCards, size_t cardsToShow,
                  double& lastP2WinPercentage, int& lastCardsToShowState) {
    if (static_cast<int>(cardsToShow) != lastCardsToShowState) {
        const auto visibleBoard = makeVisibleBoard(communityCards, cardsToShow);
        lastP2WinPercentage = ai.evaluateHand(player2Hand, visibleBoard) * 100.0;
        lastCardsToShowState = static_cast<int>(cardsToShow);
    }
}

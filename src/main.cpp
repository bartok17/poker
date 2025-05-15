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

float AI_FOLD_THRESHOLD = 0.25; // AI folds 
float AI_RAISE_THRESHOLD = 0.55; // AI raises 
float AI_CALL_THRESHOLD = 0.55; // AI calls 

// Forward declarations for helper functions
struct Button;
Button createButton(const std::string& label, sf::Font& font, sf::Vector2f size, sf::Vector2f pos, sf::Color color, sf::Color textColor, unsigned int textSize);
void drawButton(sf::RenderWindow& window, const Button& btn);
bool isButtonClicked(const Button& btn, sf::Vector2i mouse);

void drawHand(const Hand& hand, sf::RenderWindow& window, sf::Font& font, float y, bool showCards);
void drawCommunityCards(const std::vector<Card>& communityCards, size_t cardsToShow, sf::RenderWindow& window, sf::Font& font);
void drawStakes(int player1BetDisplay, int player2BetDisplay, int pendingStake, sf::RenderWindow& window, sf::Font& font);

// Game Logic Helper Functions
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

void handlePlayerBetAction(playerAI& ai, // Pass AI object
                           int& pendingStake, int& player1score, int& player2score,
                           int& player1BetDisplay, int& player2BetDisplay, int& pot,
                           bool& allInPhase, size_t& cardsToShow, bool& riverBettingPhase, bool& finalStakePhase,
                           const Hand& player2Hand, const std::vector<Card>& communityCards, // For AI
                           std::string& winnerText, bool& gameFinished, int& winner); // For AI folding etc.

void handlePlayerWaitAction(playerAI& ai, // Pass AI object
                            size_t& cardsToShow, bool& riverBettingPhase, bool& finalStakePhase,
                            int& player1score, int& player2score,
                            int& player1BetDisplay, int& player2BetDisplay, int& pot,
                            bool& allInPhase,
                            const Hand& player2Hand, const std::vector<Card>& communityCards, // For AI
                            std::string& winnerText, bool& gameFinished, int& winner); // For AI folding etc.

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


// Button helper structure and functions
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
    // Center text
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
    // Game state variables
    size_t cardsToShow = 0;
    bool allInPhase = false;
    bool riverBettingPhase = false;
    bool finalStakePhase = false;
    bool gameFinished = false;

    Deck deck;
    Hand player1Hand;
    Hand player2Hand;
    std::vector<Card> communityCards;

    int player1score = 2000;
    int player2score = 2000;
    int player1BetDisplay = 0; // Stores the amount P1 bet in the last action for display
    int player2BetDisplay = 0; // Stores the amount P2 bet/called in the last action for display
    int pot = 0;
    int pendingStake = 0;
    
    std::string winnerText;
    int winner = -1; // 0: player1, 1: player2, 2: draw
    bool player1Out = false;
    bool player2Out = false;

    playerAI ai;
    double lastP2WinPercentage = 0.0;
    int lastCardsToShowState = -1; // Used to track changes in cardsToShow for AI update

    // Initial round setup
    startNewRound(deck, player1Hand, player2Hand, communityCards, cardsToShow,
                  player1score, player2score, player1BetDisplay, player2BetDisplay, pot,
                  finalStakePhase, gameFinished, riverBettingPhase, winnerText, winner, allInPhase);

    sf::RenderWindow window(sf::VideoMode(1920, 1080), "Poker Table");
    window.setFramerateLimit(60);
    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) { // Try local directory first
        if (!font.loadFromFile("C:/Windows/Fonts/arial.ttf") && // Windows fallback
            !font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf") && // Linux fallback
            !font.loadFromFile("/System/Library/Fonts/Supplemental/Arial.ttf")) { // macOS fallback
            std::cerr << "Could not load font!" << std::endl;
            return 1;
        }
    }

    // Button setup
    Button submitButton   = createButton("Submit", font, {160, 60}, {300, 980}, sf::Color(100, 200, 100), sf::Color::Black);
    Button add10Button    = createButton("+10", font, {80, 60}, {500, 980}, sf::Color(200, 200, 255), sf::Color::Black);
    Button add50Button    = createButton("+50", font, {80, 60}, {600, 980}, sf::Color(200, 200, 255), sf::Color::Black);
    Button add100Button   = createButton("+100", font, {80, 60}, {700, 980}, sf::Color(200, 200, 255), sf::Color::Black);
    Button waitButton     = createButton("Wait", font, {80, 60}, {800, 980}, sf::Color(200, 255, 200), sf::Color::Black);
    Button passButton     = createButton("Pass", font, {80, 60}, {900, 980}, sf::Color(255, 200, 200), sf::Color::Black);
    Button resetButton    = createButton("Reset Bet", font, {120, 60}, {1000, 980}, sf::Color(220, 220, 220), sf::Color::Black);
    Button nextRoundButton= createButton("Next Round", font, {200, 60}, {1200, 980}, sf::Color(255, 255, 100), sf::Color::Black);


    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::MouseButtonPressed) {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);

                if (!gameFinished && !player1Out && !player2Out) {
                    if (isButtonClicked(add10Button, mousePos)) {
                        pendingStake = std::min(pendingStake + 10, player1score);
                    } else if (isButtonClicked(add50Button, mousePos)) {
                        pendingStake = std::min(pendingStake + 50, player1score);
                    } else if (isButtonClicked(add100Button, mousePos)) {
                        pendingStake = std::min(pendingStake + 100, player1score);
                    } else if (isButtonClicked(waitButton, mousePos)) {
                        handlePlayerWaitAction(ai, cardsToShow, riverBettingPhase, finalStakePhase,
                                               player1score, player2score, player1BetDisplay, player2BetDisplay, pot,
                                               allInPhase, player2Hand, communityCards,
                                               winnerText, gameFinished, winner);
                    } else if (isButtonClicked(resetButton, mousePos)) {
                        pendingStake = 0;
                    } else if (isButtonClicked(passButton, mousePos)) {
                        handlePlayerPassAction(gameFinished, winner, winnerText, pot, player2score, player1score);
                         if (player1score <= 0) player1Out = true; // Check if passing made player lose
                         if (player2score <= 0) player2Out = true;
                    } else if (isButtonClicked(submitButton, mousePos)) {
                        if (pendingStake > 0) {
                            handlePlayerBetAction(ai, pendingStake, player1score, player2score,
                                                  player1BetDisplay, player2BetDisplay, pot,
                                                  allInPhase, cardsToShow, riverBettingPhase, finalStakePhase,
                                                  player2Hand, communityCards,
                                                  winnerText, gameFinished, winner);
                        }
                    }
                } else if (gameFinished && isButtonClicked(nextRoundButton, mousePos) && !player1Out && !player2Out) {
                    startNewRound(deck, player1Hand, player2Hand, communityCards, cardsToShow,
                                  player1score, player2score, player1BetDisplay, player2BetDisplay, pot,
                                  finalStakePhase, gameFinished, riverBettingPhase, winnerText, winner, allInPhase);
                    lastCardsToShowState = -1; // Reset for AI update
                }
            }
        }

        // Game State Updates (post-event processing)
        if (finalStakePhase && !gameFinished) {
            cardsToShow = 5; // Ensure all cards are revealed
            gameFinished = true; // Mark round as finished
            determineAndSetWinner(player1Hand, player2Hand, communityCards, pot, player1score, player2score,
                                  winnerText, winner, player1Out, player2Out);
        }
        
        // Check for overall game over
        if ((player1Out || player2Out) && gameFinished) {
            // gameFinished is already true from round end or pass
            if (player1Out && player2Out) winnerText = "Both players are out of money! Game over.";
            else if (player1Out) winnerText = "Player 1 is out of money! Player 2 wins the game!";
            else if (player2Out) winnerText = "Player 2 is out of money! Player 1 wins the game!";
        }

        updateAIInfo(ai, player2Hand, communityCards, cardsToShow, lastP2WinPercentage, lastCardsToShowState);

        // Drawing
        std::vector<Button*> activeButtons;
        if (!gameFinished && !player1Out && !player2Out) {
            activeButtons = {&submitButton, &add10Button, &add50Button, &add100Button, &waitButton, &passButton, &resetButton};
        } else if (gameFinished && !player1Out && !player2Out) {
            activeButtons = {&nextRoundButton};
        }

        drawGameElements(window, font, player1Hand, player2Hand, communityCards, cardsToShow,
                         player1BetDisplay, player2BetDisplay, pendingStake, pot, player1score, player2score,
                         gameFinished, winnerText, activeButtons, lastP2WinPercentage);
    }

    return 0;
}

// Game Logic Helper Function Implementations

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
    for (int i = 0; i < 5; ++i) {
        communityCards.push_back(deck.draw());
    }

    cardsToShow = 0;
    int blindAmount = 50; // Example blind
    
    // Reset street bets
    player1BetDisplay = 0;
    player2BetDisplay = 0;

    // Players post blinds - these are their first bets for the pre-flop street
    int p1BlindPaid = std::min(blindAmount, player1score);
    player1score -= p1BlindPaid;
    player1BetDisplay = p1BlindPaid;

    int p2BlindPaid = std::min(blindAmount, player2score);
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
                      int& player1BetDisplay, int& player2BetDisplay) { // Added bet displays
    // Reset street bets for the new street
    player1BetDisplay = 0;
    player2BetDisplay = 0;

    if (cardsToShow == 0) cardsToShow = 3;         // Flop
    else if (cardsToShow == 3) cardsToShow = 4;    // Turn
    else if (cardsToShow == 4) {                   // River
        cardsToShow = 5;
        riverBettingPhase = true;
    } else if (riverBettingPhase) {                // After river betting
        finalStakePhase = true;
        riverBettingPhase = false;
    }
}

void determineAndSetWinner(Hand& player1Hand, Hand& player2Hand, const std::vector<Card>& communityCards, 
                           int& pot, int& player1score, int& player2score,
                           std::string& winnerText, int& winner, bool& player1Out, bool& player2Out) {
    Hand fullHand1 = player1Hand.combineHands(Hand{communityCards});
    Hand fullHand2 = player2Hand.combineHands(Hand{communityCards});
    int cmp = Comparer::compareHands(fullHand1, fullHand2);

    if (cmp == 0) { // Player 1 wins
        winnerText = "Player 1 wins the pot of " + std::to_string(pot) + "!";
        winner = 0;
        player1score += pot;
    } else if (cmp == 1) { // Player 2 wins
        winnerText = "Player 2 wins the pot of " + std::to_string(pot) + "!";
        winner = 1;
        player2score += pot;
    } else { // Draw
        winnerText = "It's a draw! Pot: " + std::to_string(pot);
        winner = 2;
        player1score += pot / 2;
        player2score += pot - (pot / 2); // Handles odd pots correctly for P2
    }
    pot = 0; // Pot is awarded

    if (player1score <= 0) player1Out = true;
    if (player2score <= 0) player2Out = true;
}

void handlePlayerBetAction(playerAI& ai, // Pass AI object
                           int& pendingStake, int& player1score, int& player2score,
                           int& player1BetDisplay, int& player2BetDisplay, int& pot,
                           bool& allInPhase, size_t& cardsToShow, bool& riverBettingPhase, bool& finalStakePhase,
                           const Hand& player2Hand, const std::vector<Card>& communityCards, // For AI
                           std::string& winnerText, bool& gameFinished, int& winner) { // For AI folding etc.
    
    int p1TotalBetForStreet = pendingStake; // pendingStake is the total P1 wants to have bet this street
    int p1AdditionalBet = p1TotalBetForStreet - player1BetDisplay;

    // Player 1 must at least call P2's current bet, or be all-in
    if (p1TotalBetForStreet < player2BetDisplay && p1TotalBetForStreet < (player1score + player1BetDisplay)) {
        // Invalid bet by P1 (trying to bet less than call, not all-in)
        // For simplicity, we can just return or cap P1's bet.
        // Or, assume UI prevents this. Let's cap it for safety.
        p1TotalBetForStreet = player2BetDisplay; 
        p1AdditionalBet = p1TotalBetForStreet - player1BetDisplay;
    }
    
    p1AdditionalBet = std::min(p1AdditionalBet, player1score); // Cannot bet more than current score
    if (p1AdditionalBet < 0) p1AdditionalBet = 0;


    player1score -= p1AdditionalBet;
    pot += p1AdditionalBet;
    player1BetDisplay += p1AdditionalBet;

    pendingStake = 0; // Reset UI pending stake

    if (player1score == 0) allInPhase = true;
    winnerText = "You bet to " + std::to_string(player1BetDisplay) + ".";

    // --- AI's Response ---
    if (gameFinished) return; // If P1's bet made them win (e.g. P2 already folded - though not possible here)

    int amountForAIToCall = player1BetDisplay - player2BetDisplay;
    std::vector<Card> visibleBoard;
    if (cardsToShow <= communityCards.size()) { // Ensure cardsToShow is valid
         visibleBoard.assign(communityCards.begin(), communityCards.begin() + std::min(cardsToShow, communityCards.size()));
    }
    double winChance = ai.evaluateHand(player2Hand, visibleBoard); // Default 1000 iterations

    int aiActionAmount = 0; // Money AI adds to pot this action

    if (amountForAIToCall > 0) { // AI needs to respond to P1's bet/raise
        if (winChance < AI_FOLD_THRESHOLD && player2score > amountForAIToCall) { // AI Folds (if not forced all-in to call)
            gameFinished = true;
            winner = 0; // Player 1 wins
            winnerText += " Player 2 folds. You win!";
            player1score += pot; 
            pot = 0;
        } else if (winChance < AI_RAISE_THRESHOLD || amountForAIToCall >= player2score) { // AI Calls (or calls all-in)
            aiActionAmount = std::min(amountForAIToCall, player2score);
            player2score -= aiActionAmount;
            pot += aiActionAmount;
            player2BetDisplay += aiActionAmount;
            winnerText += " Player 2 calls " + std::to_string(aiActionAmount) + ".";
            if (player2score == 0) allInPhase = true;
            // Stakes are even, betting round for street ends
            if (!allInPhase && !gameFinished) {
                advanceGamePhase(cardsToShow, riverBettingPhase, finalStakePhase, player1BetDisplay, player2BetDisplay);
            }
        } else { // AI Raises
            int aiCallPart = std::min(amountForAIToCall, player2score);
            int aiCanRaiseMax = player2score - aiCallPart;

            if (aiCanRaiseMax > 0) {
                int aiRaisePart = std::min({player1BetDisplay, pot / 2, aiCanRaiseMax}); // Example raise sizing
                if (aiRaisePart <= 0) aiRaisePart = std::min(50, aiCanRaiseMax); // Min raise if others are 0
                
                aiActionAmount = aiCallPart + aiRaisePart;

                player2score -= aiActionAmount;
                pot += aiActionAmount;
                player2BetDisplay += aiActionAmount;
                winnerText += " Player 2 raises to " + std::to_string(player2BetDisplay) + ".";
                if (player2score == 0) allInPhase = true;
                // P1 would need to act again. Simplified: street ends. P1 needs to call this next.
                // Or, for true turn-based, set currentPlayer = P1.
                // For now, we advance. This implies P1 must meet the raise to continue.
                 if (!allInPhase && !gameFinished) {
                    // advanceGamePhase(cardsToShow, riverBettingPhase, finalStakePhase, player1BetDisplay, player2BetDisplay);
                    // Actually, if AI raises, the betting round is NOT over. P1 must respond.
                    // This simplified model will just advance. A full model needs a currentPlayer loop.
                    // Let's assume for now the game will prompt P1 to call/fold to AI's raise implicitly.
                 }
            } else { // Cannot raise, just call
                aiActionAmount = std::min(amountForAIToCall, player2score);
                player2score -= aiActionAmount;
                pot += aiActionAmount;
                player2BetDisplay += aiActionAmount;
                winnerText += " Player 2 calls " + std::to_string(aiActionAmount) + ".";
                if (player2score == 0) allInPhase = true;
                if (!allInPhase && !gameFinished) {
                    advanceGamePhase(cardsToShow, riverBettingPhase, finalStakePhase, player1BetDisplay, player2BetDisplay);
                }
            }
        }
    } else { // P1 opened betting (amountForAIToCall was 0 or less, meaning P1's bet is the first or matches AI's previous check/bet)
        // This case means P1 made an initial bet on an open board or re-raised. AI responds.
        // Logic is similar to AI responding to a bet.
        // If P1 just matched AI's previous bet (player1BetDisplay == player2BetDisplay), this means P1 called. Street ends.
        if (player1BetDisplay == player2BetDisplay) { // P1 called AI's previous bet.
             winnerText += " Stakes are even.";
             if (!allInPhase && !gameFinished) {
                advanceGamePhase(cardsToShow, riverBettingPhase, finalStakePhase, player1BetDisplay, player2BetDisplay);
            }
        } else { // P1 made a bet/raise, AI needs to respond (amountForAIToCall logic above handles this)
            // This path might be redundant if amountForAIToCall correctly captures P1's bet.
            // The logic above for amountForAIToCall > 0 should cover AI's response to P1's bet.
        }
    }


    if ((player1score == 0 || player2score == 0) && !gameFinished) {
        allInPhase = true;
    }
    if (allInPhase && !gameFinished) {
        cardsToShow = 5;
        finalStakePhase = true;
        riverBettingPhase = false;
    }
}

void handlePlayerWaitAction(playerAI& ai, // Pass AI object
                            size_t& cardsToShow, bool& riverBettingPhase, bool& finalStakePhase,
                            int& player1score, int& player2score,
                            int& player1BetDisplay, int& player2BetDisplay, int& pot,
                            bool& allInPhase,
                            const Hand& player2Hand, const std::vector<Card>& communityCards, // For AI
                            std::string& winnerText, bool& gameFinished, int& winner) { // For AI folding etc.

    if (player1BetDisplay < player2BetDisplay) { // P1 must Call P2's bet
        int amountToCall = player2BetDisplay - player1BetDisplay;
        int p1ActualCall = std::min(amountToCall, player1score);

        player1score -= p1ActualCall;
        pot += p1ActualCall;
        player1BetDisplay += p1ActualCall;
        winnerText = "You call " + std::to_string(p1ActualCall) + ".";
        if (player1score == 0) allInPhase = true;
        // Stakes are even. Street ends.
        if (!allInPhase && !gameFinished) {
            advanceGamePhase(cardsToShow, riverBettingPhase, finalStakePhase, player1BetDisplay, player2BetDisplay);
        }
    } else { // P1 Checks (player1BetDisplay == player2BetDisplay, meaning no outstanding bet from P2)
        winnerText = "You check.";
        // AI's turn to check or bet
        std::vector<Card> visibleBoard;
        if (cardsToShow <= communityCards.size()) {
            visibleBoard.assign(communityCards.begin(), communityCards.begin() + std::min(cardsToShow, communityCards.size()));
        }
        double winChance = ai.evaluateHand(player2Hand, visibleBoard);
        
        if (winChance > AI_RAISE_THRESHOLD && player2score > 0) { // AI Bets
            int aiBetAmount = std::min({pot / 2, player2score / 2, player2score}); // Example bet sizing
            if (aiBetAmount <= 0) aiBetAmount = std::min(50, player2score); // Min bet if pot/2 is 0

            if (aiBetAmount > 0) {
                player2score -= aiBetAmount;
                pot += aiBetAmount;
                player2BetDisplay += aiBetAmount; // AI's street bet increases
                winnerText += " Player 2 bets " + std::to_string(aiBetAmount) + ".";
                if (player2score == 0) allInPhase = true;
                // P1 needs to respond. Simplified: street ends.
                 if (!allInPhase && !gameFinished) {
                    // advanceGamePhase(cardsToShow, riverBettingPhase, finalStakePhase, player1BetDisplay, player2BetDisplay);
                    // Betting round not over. P1 must respond.
                 }
            } else { // AI cannot make a valid bet, so checks
                 winnerText += " Player 2 checks.";
                 if (!allInPhase && !gameFinished) { // Both checked
                    advanceGamePhase(cardsToShow, riverBettingPhase, finalStakePhase, player1BetDisplay, player2BetDisplay);
                 }
            }
        } else { // AI Checks
            winnerText += " Player 2 checks.";
            // Both P1 and P2 checked. Street ends.
            if (!allInPhase && !gameFinished) {
                advanceGamePhase(cardsToShow, riverBettingPhase, finalStakePhase, player1BetDisplay, player2BetDisplay);
            }
        }
    }

    if ((player1score == 0 || player2score == 0) && !gameFinished) {
        allInPhase = true;
    }
    if (allInPhase && !gameFinished) {
        cardsToShow = 5;
        finalStakePhase = true;
        riverBettingPhase = false;
    }
}

void handlePlayerPassAction(bool& gameFinished, int& winner, std::string& winnerText, int& pot, int& player2score, int& player1score) {
    gameFinished = true; // Round ends
    winner = 1; // Player 2 wins by default
    winnerText = "You passed. Player 2 wins the pot of " + std::to_string(pot) + "!";
    player2score += pot; // Player 2 gets the current pot
    pot = 0;
}

void updateAIInfo(playerAI& ai, const Hand& player2Hand, const std::vector<Card>& communityCards, size_t cardsToShow,
                  double& lastP2WinPercentage, int& lastCardsToShowState) {
    if (static_cast<int>(cardsToShow) != lastCardsToShowState) {
        std::vector<Card> visibleBoard;
        if (cardsToShow > 0 && cardsToShow <= communityCards.size()) {
            visibleBoard.assign(communityCards.begin(), communityCards.begin() + cardsToShow);
        }
        lastP2WinPercentage = ai.evaluateHand(player2Hand, visibleBoard) * 100.0;
        lastCardsToShowState = static_cast<int>(cardsToShow);
    }
}

// Drawing Function Implementations
void drawHand(const Hand& hand, sf::RenderWindow& window, sf::Font& font, float yPosCategory, bool showCards) {
    const auto& cards = hand.getCards();
    float handStartX = 800; // Centered more
    float cardOffsetY = (yPosCategory == 900) ? 800 : 200; // Player 1 y : Player 2 y

    for (size_t i = 0; i < cards.size(); ++i) { // Draw all cards in hand (usually 2)
        float x = handStartX + i * 120;
        Card card = cards[i]; // Make a copy to modify faceUp status for drawing
        card.setFaceUp(showCards);
        card.draw(window, x, cardOffsetY);
    }
}

void drawCommunityCards(const std::vector<Card>& communityCards, size_t cardsToShow, sf::RenderWindow& window, sf::Font& font) {
    float communityY = 500;
    float communityStartX = (1920 - (5 * 110 - 10)) / 2.0f; // Center community cards (5 cards, 100 width + 10 spacing)
    for (size_t i = 0; i < communityCards.size(); ++i) {
        float x = communityStartX + i * 120; // Card width 100 + spacing 20
        Card card = communityCards[i]; // Make a copy
        card.setFaceUp(i < cardsToShow);
        card.draw(window, x, communityY);
    }
}

void drawStakes(int player1BetDisplay, int player2BetDisplay, int pendingStake, sf::RenderWindow& window, sf::Font& font) {
    sf::Text p1BetText("P1 Bet: " + std::to_string(player1BetDisplay), font, 20);
    p1BetText.setFillColor(sf::Color::Yellow);
    p1BetText.setPosition(50, 1000);
    window.draw(p1BetText);

    sf::Text p2BetText("P2 Bet: " + std::to_string(player2BetDisplay), font, 20);
    p2BetText.setFillColor(sf::Color::Yellow);
    p2BetText.setPosition(50, 40);
    window.draw(p2BetText);

    sf::Text pendingText("Your Bet: " + std::to_string(pendingStake), font, 30);
    pendingText.setFillColor(sf::Color::White);
    pendingText.setPosition(50, 940);
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
    window.clear(sf::Color(0, 100, 0)); // Dark green table color

    // AI win percentage
    sf::Text p2winText("Opponent Win%: " + std::to_string(static_cast<int>(lastP2WinPercentage)) + "%", font, 28);
    p2winText.setFillColor(sf::Color::Magenta);
    p2winText.setPosition(1200, 40);
    window.draw(p2winText);

    // Hands
    drawHand(player1Hand, window, font, 900, true); // Player 1's cards always visible
    drawHand(player2Hand, window, font, 100, gameFinished); // Player 2's cards visible when round/game is finished

    // Community Cards
    drawCommunityCards(communityCards, cardsToShow, window, font);

    // Stakes and Pot
    drawStakes(player1BetDisplay, player2BetDisplay, pendingStake, window, font);
    sf::Text potText("Pot: " + std::to_string(pot), font, 28);
    potText.setFillColor(sf::Color::Cyan);
    potText.setPosition(900, 400); // Centered above community cards
    window.draw(potText);

    // Scores
    sf::Text scoreText1("Player 1 Score: " + std::to_string(player1score), font, 20);
    scoreText1.setFillColor(sf::Color::White);
    scoreText1.setPosition(1500, 1000);
    window.draw(scoreText1);

    sf::Text scoreText2("Player 2 Score: " + std::to_string(player2score), font, 20);
    scoreText2.setFillColor(sf::Color::White);
    scoreText2.setPosition(1500, 40);
    window.draw(scoreText2);

    // Winner/Game Over Text
    if (gameFinished) {
        sf::Text winAnnounceText(winnerText, font, 36);
        winAnnounceText.setFillColor(sf::Color::White);
        sf::FloatRect textRect = winAnnounceText.getLocalBounds();
        winAnnounceText.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
        winAnnounceText.setPosition(window.getSize().x / 2.0f, window.getSize().y / 2.0f + 200.f); // Center screen, lower part
        window.draw(winAnnounceText);
    }

    // Buttons
    for (const auto& btnPtr : activeButtons) {
        if (btnPtr) drawButton(window, *btnPtr);
    }

    window.display();
}

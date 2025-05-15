#include "playerAI.h"
#include "hand.h"
#include "card.h"
#include "comparer.h"
#include "deck.h"
#include <random>
#include <algorithm>
#include <thread>
 
double playerAI::evaluateHand(const Hand& hand, const std::vector<Card>& board) {
    // Monte Carlo simulation to estimate win percentage (multithreaded)
    int wins = 0;
    int iterations = 100000;
    int numThreads = std::thread::hardware_concurrency();
    if (numThreads == 0) numThreads = 16; // fallback

    std::vector<std::thread> threads;
    std::vector<int> threadWins(numThreads, 0);

    auto worker = [&](int tid) {
        int localWins = 0;
        for (int i = tid; i < iterations; i += numThreads) {
            if (simulateWin(hand, board)) ++localWins;
        }
        threadWins[tid] = localWins;
    };

    for (int t = 0; t < numThreads; ++t) {
        threads.emplace_back(worker, t);
    }
    for (auto& th : threads) th.join();

    for (int w : threadWins) wins += w;
    return static_cast<double>(wins) / iterations;

}
bool playerAI::simulateWin(Hand myHand, std::vector<Card> board) {
    //Gather all used cards 
    std::vector<Card> used;
    for (const auto& c : myHand.getCards()) used.push_back(c);
    for (const auto& c : board) used.push_back(c);

    //Build deck of remaining cards
    std::vector<Card> deck;
    for (int suit = 0; suit < 4; ++suit) {
        for (int rank = 2; rank <= 14; ++rank) {
            Card candidate(static_cast<Card::Rank>(rank), static_cast<Card::Suit>(suit));
            // Check if already used
            bool found = false;
            for (const auto& u : used) {
                if (u.getRank() == candidate.getRank() && u.getSuit() == candidate.getSuit()) {
                    found = true;
                    break;
                }
            }
            if (!found) deck.push_back(candidate);
        }
    }

    //Shuffle deck
    static std::random_device rd;
    static std::mt19937 rng(rd());
    std::shuffle(deck.begin(), deck.end(), rng);

    //Fill board up to 5 cards
    while (board.size() < 5 && !deck.empty()) {
        board.push_back(deck.back());
        deck.pop_back();
    }

    //Draw 2 random enemy cards
    std::vector<Card> enemyHand;
    for (int i = 0; i < 2 && !deck.empty(); ++i) {
        enemyHand.push_back(deck.back());
        deck.pop_back();
    }

    //Build full hands
    Hand myFull = myHand.combineHands(Hand{board});
    Hand enemyFull = Hand(enemyHand).combineHands(Hand{board});

    //Compare
    int cmp = Comparer::compareHands(myFull, enemyFull);
    return cmp == 0; // true if myHand wins, false if loses or ties for simplicity
}
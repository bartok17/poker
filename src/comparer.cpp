#include "comparer.h"
#include <algorithm>

int Comparer::getHandType(const Hand& hand)
{
    int rankCount[15] = {0}; // 2-14 (Ace)
    int suitCount[4] = {0}; // Hearts, Diamonds, Clubs, Spades
    for (const auto& card : hand.getCards()) {
        rankCount[static_cast<int>(card.getRank())]++;
        suitCount[static_cast<int>(card.getSuit())]++;
    }

    int maxType = 0;

    // Check for Flush and collect suited ranks
    int flushSuit = -1;
    for (int i = 0; i < 4; ++i) {
        if (suitCount[i] >= 5) {
            flushSuit = i;
            break;
        }
    }

    // Collect ranks for flush suit if exists
    std::vector<int> flushRanks;
    if (flushSuit != -1) {
        for (const auto& card : hand.getCards()) {
            if (static_cast<int>(card.getSuit()) == flushSuit) {
                flushRanks.push_back(static_cast<int>(card.getRank()));
            }
        }
    }

    // Check for Straight Flush and Royal Flush
    if (!flushRanks.empty()) {
        bool hasRank[15] = {false};
        for (int r : flushRanks) hasRank[r] = true;
        // Royal Flush: 10, J, Q, K, A
        if (hasRank[10] && hasRank[11] && hasRank[12] && hasRank[13] && hasRank[14]) {
            maxType = std::max(maxType, 9);
        }
        // Straight Flush
        for (int start = 10; start >= 2; --start) {
            bool straightFlush = true;
            for (int j = 0; j < 5; ++j) {
                if (!hasRank[start + j]) {
                    straightFlush = false;
                    break;
                }
            }
            if (straightFlush) maxType = std::max(maxType, 8);
        }
        // Ace-low straight flush
        if (hasRank[14] && hasRank[2] && hasRank[3] && hasRank[4] && hasRank[5]) {
            maxType = std::max(maxType, 8);
        }
    }

    // Four of a Kind, Full House, Three of a Kind, Two Pair, One Pair
    int pairs = 0, threes = 0, fours = 0;
    for (int i = 2; i <= 14; ++i) {
        if (rankCount[i] == 4) fours++;
        else if (rankCount[i] == 3) threes++;
        else if (rankCount[i] == 2) pairs++;
    }
    if (fours) maxType = std::max(maxType, 7);
    if (threes && pairs) maxType = std::max(maxType, 6); // Full House
    if (threes) maxType = std::max(maxType, 3);
    if (pairs >= 2) maxType = std::max(maxType, 2);
    if (pairs == 1) maxType = std::max(maxType, 1);

    // Flush
    if (flushSuit != -1) maxType = std::max(maxType, 5);

    // Straight
    for (int i = 10; i >= 2; --i) {
        bool straight = true;
        for (int j = 0; j < 5; ++j) {
            if (rankCount[i + j] == 0) {
                straight = false;
                break;
            }
        }
        if (straight) maxType = std::max(maxType, 4);
    }
    // Ace-low straight
    if (rankCount[14] && rankCount[2] && rankCount[3] && rankCount[4] && rankCount[5]) {
        maxType = std::max(maxType, 4);
    }

    // High Card is 0, already default

    return maxType;
}

int Comparer::compareHands(const Hand& hand1, const Hand& hand2)
{
    int type1 = getHandType(hand1);
    int type2 = getHandType(hand2);
    if (type1 != type2) {
        return type1 > type2 ? 0 : 1; // 1 if hand1 wins, 0 if hand2 wins
    }
    // If types are the same, compare high cards
    std::vector<Card> sortedHand1 = hand1.getSortedCards();
    std::vector<Card> sortedHand2 = hand2.getSortedCards();
    for (int i = sortedHand1.size() - 1; i >= 0; --i) {
        if (sortedHand1[i].getRank() != sortedHand2[i].getRank()) {
            return sortedHand1[i].getRank() > sortedHand2[i].getRank() ? 0 : 1;
        }
    }
    return -1; // Hands are equal
}

std::vector<int> Comparer::getWinners(const Hand hands[], int numHands)
{
    std::vector<int> winnerIndices = {0};
    for (int i = 1; i < numHands; ++i) {
        int result = compareHands(hands[winnerIndices[0]], hands[i]);
        if (result == 0) {
            // hand i wins, clear previous winners
            winnerIndices.clear();
            winnerIndices.push_back(i);
        } else if (result == -1) {
            // hands are equal, add to winners
            winnerIndices.push_back(i);
        }
        // else: current winners remain
    }
    return winnerIndices;
}
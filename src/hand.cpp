#include "hand.h"
#include "card.h"
#include <vector>
#include <algorithm>
#include <iostream>

Hand::Hand(std::vector<Card> cards) {
    this->cards = cards;
}
void Hand::addCard(const Card& card) {
    cards.push_back(card);
}

void Hand::clear() {
    cards.clear();
}

std::string Hand::toString() const {
    std::string result;
    for (const auto& card : cards) {
        if (!result.empty()) {
            result += " ";
        }
        result += card.toString();
    }
    return result;
}
Hand Hand::combineHands(const Hand& other) {
    Hand combined = *this;
    combined.cards.insert(combined.cards.end(), other.cards.begin(), other.cards.end());
    return combined; // return by value, not by reference
}
const std::vector<Card>& Hand::getCards() const {
    return cards;
}

std::vector<Card> Hand::getCards(std::vector<Card> addition) const {
    std::vector<Card> combined = cards;
    combined.insert(combined.end(), addition.begin(), addition.end());
    return combined;
}

std::vector<Card> Hand::getSortedCards() const {
    std::vector<Card> sorted = cards;
    std::sort(sorted.begin(), sorted.end(), [](const Card& a, const Card& b) {
        return a.getRank() < b.getRank();
    });
    return sorted;
}

void Hand::print() const {
    for (const auto& card : cards) {
        std::cout << card.toString() << " ";
    }
    std::cout << std::endl;
}


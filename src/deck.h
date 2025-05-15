#pragma once
#include <vector>
#include "card.h"

class Deck {
public:
    Deck();

    void reset(); // Fill and shuffle the deck
    void shuffle();
    bool isEmpty() const;
    Card draw(); // Draw the top card

    size_t size() const;

private:
    std::vector<Card> cards_;
    size_t currentIndex_;
};
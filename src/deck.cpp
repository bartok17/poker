#include "deck.h"
#include <algorithm>
#include <random>
#include <chrono>

Deck::Deck() {
    reset();
}

void Deck::reset() {
    cards_.clear();
    for (int suit = 0; suit < 4; ++suit) {
        for (int rank = 2; rank <= 14; ++rank) {
            cards_.emplace_back(static_cast<Card::Rank>(rank), static_cast<Card::Suit>(suit));
        }
    }
    shuffle();
    currentIndex_ = 0;
}

void Deck::shuffle() {
    auto rng = std::default_random_engine(
        static_cast<unsigned>(std::chrono::system_clock::now().time_since_epoch().count())
    );
    std::shuffle(cards_.begin(), cards_.end(), rng);
    currentIndex_ = 0;
}

bool Deck::isEmpty() const {
    return currentIndex_ >= cards_.size();
}

Card Deck::draw() {
    if (isEmpty()) throw std::out_of_range("Deck is empty");
    return cards_[currentIndex_++];
}

size_t Deck::size() const {
    return cards_.size() - currentIndex_;
}
#ifndef HAND_H
#define HAND_H

#include <vector>
#include <string>

#include "card.h"

class Hand {
public:
    void addCard(const Card& card);

   
    Hand() = default;
    Hand(std::vector<Card> cards);
    void clear();

    std::string toString() const;

    Hand combineHands(const Hand& other); // return by value

    const std::vector<Card>& getCards() const;
    std::vector<Card> getCards(std::vector<Card> addition) const;

    std::vector<Card> getSortedCards() const;
    void print() const;

protected:
    std::vector<Card> cards;
};

#endif // HAND_H
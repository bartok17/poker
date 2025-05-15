#ifndef COMPARER_H
#define COMPARER_H
#include <vector>
#include <string>
#include "hand.h"
#include "card.h"

class Comparer {
public:
    static int getHandType(const Hand& hand);
    static int compareHands(const Hand& hand1, const Hand& hand2);
    static std::vector<int> getWinners(const Hand hands[], int numHands);
};

#endif
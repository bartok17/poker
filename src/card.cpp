#include "card.h"


#include <map>
#include <iostream>

Card::Card(Rank rank, Suit suit) : rank_(rank), suit_(suit) {}

Card::Rank Card::getRank() const {
    return rank_;
}

Card::Suit Card::getSuit() const {
    return suit_;
}

bool Card::isFaceUp() const {
    return isFaceUp_;
}

std::string Card::toString() const {
    static const char* suitNames[] = {"Hearts", "Diamonds", "Clubs", "Spades"};
    static const char* rankNames[] = {
        "", "", "2", "3", "4", "5", "6", "7", "8", "9", "10",
        "J", "Q", "K", "A"
    };
    return std::string(rankNames[static_cast<int>(rank_)]) + " of " + suitNames[static_cast<int>(suit_)];
}

std::string Card::toPokerStoveString() const {
    static const char* ranks = "23456789TJQKA";
    static const char* suits = "cdhs"; // clubs, diamonds, hearts, spades
    return std::string(1, ranks[static_cast<int>(rank_) - 2]) +
           std::string(1, suits[static_cast<int>(suit_)]);
}

void Card::print() const {
    std::cout << toString() << std::endl;
}

void Card::setFaceUp(bool faceUp) {
    isFaceUp_ = faceUp;
}

void Card::draw(sf::RenderWindow& window, float x, float y) const {
    // Static cache for textures
    static std::map<std::string, sf::Texture> textureCache;

    std::string filename;
    if (!isFaceUp_) {
        filename = "src/PNG-cards/revers.png";
    } else {
        // Map to filename
        static const char* suitNames[] = {"hearts", "diamonds", "clubs", "spades"};
        static const char* rankNames[] = {
            "", "", "2", "3", "4", "5", "6", "7", "8", "9", "10",
            "jack", "queen", "king", "ace"
        };
        filename = "src/PNG-cards/";
        filename += rankNames[static_cast<int>(rank_)];
        filename += "_of_";
        filename += suitNames[static_cast<int>(suit_)];
        filename += ".png";
    }

    // Load texture if not cached
    if (textureCache.find(filename) == textureCache.end()) {
        sf::Texture tex;
        if (!tex.loadFromFile(filename)) {
            // fallback to a rectangle if texture loading fails
            sf::RectangleShape rect(sf::Vector2f(80, 120));
            rect.setPosition(x, y);
            rect.setFillColor(sf::Color::White);
            window.draw(rect);
            return;
        }
        textureCache[filename] = tex;
    }

    sf::Sprite sprite;
    sprite.setTexture(textureCache[filename]);
    sprite.setPosition(x, y);
    sprite.setScale(0.18f, 0.18f);
    window.draw(sprite);
}

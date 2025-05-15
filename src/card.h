#pragma once
#include <string>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <iostream>

class Card {
public:
    enum class Suit { Hearts, Diamonds, Clubs, Spades };
    enum class Rank {
        Two = 2, Three, Four, Five, Six, Seven, Eight, Nine, Ten,
        Jack, Queen, King, Ace
    };

    Card(Rank rank, Suit suit);

    Card(const Card&) = default;
    Card& operator=(const Card&) = default;
    Card(Card&&) = default;
    Card& operator=(Card&&) = default;

    Rank getRank() const;
    Suit getSuit() const;
    bool isFaceUp() const;
    std::string toString() const;
    std::string toPokerStoveString() const;
    void print() const;
    void draw(sf::RenderWindow& window, float x, float y) const;
    void setFaceUp(bool faceUp);

private:
    Rank rank_;
    Suit suit_;
    bool isFaceUp_ = true;
};
#ifndef _MANA_H_
#define _MANA_H_

#include <set>

enum Color {
    WHITE,
    BLUE,
    BLACK,
    RED,
    GREEN,
    COLORLESS
};

class Mana {
public:
    Mana();
    Mana(std::multiset<Color> manaString);
    Mana(unsigned int generic, std::multiset<Color> manaString);

    void add(Mana& other);
    bool contains(Mana& other);
    bool subtract(Mana& other);
    unsigned int cmc();

    Mana& operator+=(Mana other);
    Mana& operator-=(Mana other);

private:
    friend std::ostream& operator<<(std::ostream& os, Mana& mana);
    unsigned int generic;
    std::multiset<Color> manaString;
};
#endif

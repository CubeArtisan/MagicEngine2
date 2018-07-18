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

    void add(const Mana& other);
    bool contains(const  Mana& other) const;
    bool subtract(const Mana& other);
	void clear();
    size_t cmc() const;

    Mana& operator+=(const Mana& other);
    Mana& operator-=(const Mana& other);

private:
    friend std::ostream& operator<<(std::ostream& os, const Mana& mana);
    unsigned int generic;
    std::multiset<Color> manaString;
};
#endif

#ifndef _MANA_H_
#define _MANA_H_

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
    std::vector<Color> colors;
    unsigned int generic;

    unsigned int cmc();
};
#endif

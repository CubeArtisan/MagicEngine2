#include <algorithm>

#include "mana.h"

Mana::Mana()
{}

Mana::Mana(unsigned int generic, std::multiset<Color> manaString)
: generic(generic), manaString(manaString)
{
}

void Mana::add(Mana& other) {
    this->manaString.insert(other.manaString.begin(), other.manaString.end());
    this->generic += other.generic;
}

bool Mana::contains(Mana& other) {
    std::multiset<Color> ourString = this->manaString;
    for(Color mana : other.manaString) {
        auto found = ourString.find(mana);
        if(found == ourString.end()) return false;

        ourString.erase(found);
    }
    return ourString.size() >= other.generic;
}

bool Mana::subtract(Mana& other) {
    std::multiset<Color> ourString = this->manaString;
    for(Color mana : other.manaString) {
        bool good = false;
        for(auto iter=ourString.begin(); iter != ourString.end(); iter++) {
            if(mana == *iter){
                ourString.erase(iter);
                good = true;
                break;
            }
        }
        if(!good) return false;
    }
    if(ourString.size() < other.generic) return false;
    
    for(unsigned int i=0; i < other.generic; i++){
        ourString.erase(ourString.end()--);
    }

    this->manaString = ourString;
    return true;
}

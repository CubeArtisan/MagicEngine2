#include <algorithm>
#include <iostream>
#include <map>

#include "mana.h"

Mana::Mana()
    : generic(0)
{}

Mana::Mana(std::multiset<Color> manaString)
: generic(0), manaString(manaString)
{
}

Mana::Mana(unsigned int generic, std::multiset<Color> manaString)
: generic(generic), manaString(manaString)
{
}

std::ostream& operator<<(std::ostream& os, Mana& mana)
{
    os << mana.generic;
    std::map<Color, char> mapping { {WHITE, 'W'}, {BLUE, 'U'}, {BLACK, 'B'}, {RED, 'R'}, {GREEN, 'G'}, {COLORLESS, 'C'}};
    for(Color color : mana.manaString) {
        os << ", " << mapping[color];
    }
    return os;
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
    int generic = (int)other.generic - (int)this->generic;
    generic = std::max(generic, 0);
    return ourString.size() >= other.generic - this->generic;
}

bool Mana::subtract(Mana& other) {
    std::multiset<Color> ourString = this->manaString;
    for(Color mana : other.manaString) {
        auto found = ourString.find(mana);
        if(found == ourString.end()) return false;

        ourString.erase(found);
    }
    
    int generic = (int)other.generic - (int)this->generic;
    if((unsigned int)generic > ourString.size()) return false;
    generic = std::max(generic, 0);
    int ourGeneric = std::max(-generic, 0);
    if(generic > 0) {
        auto end = ourString.begin();
        std::advance(end, generic);
        ourString.erase(ourString.begin(), end);
    }
    
    this->generic = ourGeneric;
    this->manaString = ourString;
    
    return true;
}

void Mana::clear() {
	this->manaString.clear();
	this->generic = 0;
}

size_t Mana::cmc() {
	return this->generic + this->manaString.size();
}

Mana& Mana::operator+=(Mana other){
    this->add(other);
    return *this;
}

Mana& Mana::operator-=(Mana other){
    this->subtract(other);
    return *this;
}

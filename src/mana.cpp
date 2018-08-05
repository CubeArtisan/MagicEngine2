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

std::ostream& operator<<(std::ostream& os, const Mana& mana)
{
    os << mana.generic;
    std::map<Color, char> mapping { {WHITE, 'W'}, {BLUE, 'U'}, {BLACK, 'B'}, {RED, 'R'}, {GREEN, 'G'}, {COLORLESS, 'C'}};
    for(Color color : mana.manaString) {
        os << ", " << mapping[color];
    }
    return os;
}

void Mana::add(const Mana& other) {
    this->manaString.insert(other.manaString.begin(), other.manaString.end());
    this->generic += other.generic;
}

bool Mana::contains(const Mana& other) const {
    std::multiset<Color> ourString = this->manaString;
    for(Color mana : other.manaString) {
        auto found = ourString.find(mana);
        if(found == ourString.end()) return false;

        ourString.erase(found);
    }
    int generic2 = (int)other.generic - (int)this->generic;
    generic2 = std::max(generic2, 0);
    return ourString.size() >= generic2;
}

bool Mana::subtract(const Mana& other) {
    std::multiset<Color> ourString = this->manaString;
    for(Color mana : other.manaString) {
        auto found = ourString.find(mana);
		if (found == ourString.end()) return false;
        ourString.erase(found);
    }
    
    int generic2 = (int)other.generic - (int)this->generic;
    if((unsigned int)generic2 > ourString.size()) return false;
    int ourGeneric = std::max(-generic2, 0);
	generic2 = std::max(generic2, 0);
	if(generic2 > 0) {
        auto end = ourString.begin();
        std::advance(end, generic2);
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

size_t Mana::cmc() const{
	return this->generic + this->manaString.size();
}

Mana& Mana::operator+=(const Mana& other){
    this->add(other);
    return *this;
}

Mana& Mana::operator-=(const Mana& other){
    this->subtract(other);
    return *this;
}

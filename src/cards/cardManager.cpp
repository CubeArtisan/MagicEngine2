#include <memory>

#include "../card.h"
#include "cardManager.h"

#include "a.h"
#include "b.h"
#include "c.h"
#include "d.h"
#include "e.h"
#include "f.h"
#include "g.h"
#include "h.h"
#include "i.h"
#include "j.h"
#include "k.h"
#include "l.h"
#include "m.h"
#include "n.h"
#include "o.h"
#include "p.h"
#include "q.h"
#include "r.h"
#include "s.h"
#include "t.h"
#include "u.h"
#include "v.h"
#include "w.h"
#include "x.h"
#include "y.h"
#include "z.h"

CardManager::CardManager(){
	AManager().getCards(this->cards, this->mvids);
    DManager().getCards(this->cards, this->mvids);
	FManager().getCards(this->cards, this->mvids);
    IManager().getCards(this->cards, this->mvids);
	MManager().getCards(this->cards, this->mvids);
	PManager().getCards(this->cards, this->mvids);
	SManager().getCards(this->cards, this->mvids);
}

Card CardManager::getCard(int mvid){
    std::string name = this->mvids.at(mvid);
    return getCard(name);
}
Card CardManager::getCard(std::string name) {
    return this->cards.at(name);
}

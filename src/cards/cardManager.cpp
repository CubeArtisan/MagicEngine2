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

#include "cards/cardManager.h"

CardManager::CardManager(){
    DManager().getCards(this->cards, this->mvids);
    IManager().getCards(this->cards, this->mvids);
}

std::shared_ptr<Card> CardManager::getCard(int mvid){
    std::string name = this->mvids[mvid];
    return getCard(name);
}
std::shared_ptr<Card> CardManager::getCard(std::string name) {
    return this->cards[name];
}

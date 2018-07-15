#include "cards/a.h"
#include "cards/b.h"
#include "cards/c.h"
#include "cards/d.h"
#include "cards/e.h"
#include "cards/f.h"
#include "cards/g.h"
#include "cards/h.h"
#include "cards/i.h"
#include "cards/j.h"
#include "cards/k.h"
#include "cards/l.h"
#include "cards/m.h"
#include "cards/n.h"
#include "cards/o.h"
#include "cards/p.h"
#include "cards/q.h"
#include "cards/r.h"
#include "cards/s.h"
#include "cards/t.h"
#include "cards/u.h"
#include "cards/v.h"
#include "cards/w.h"
#include "cards/x.h"
#include "cards/y.h"
#include "cards/z.h"

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

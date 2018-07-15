#ifndef _CARDMANAGER_H_
#define _CARDMANAGER_H_

#include <memory>
#include <string>

#include "card.h"

class CardManager {
public:
    std::shared_ptr<Card> getCard(int mvid);
    std::shared_ptr<Card> getCard(std::string name);

    CardManager();

private:
    std::map<std::string, std::shared_ptr<Card>> cards;
    std::map<int, std::string> mvids;
};

struct LetterManager {
public:
    virtual void getCards(std::map<std::string, std::shared_ptr<Card>>, std::map<int, std::string>) = 0;
};

#endif

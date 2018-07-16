#ifndef _CARDMANAGER_H_
#define _CARDMANAGER_H_

#include <memory>
#include <string>

#include "../card.h"

class CardManager {
public:
    Card getCard(int mvid);
    Card getCard(std::string name);

    CardManager();

private:
    std::map<std::string, Card> cards;
    std::map<int, std::string> mvids;
};

struct LetterManager {
    virtual void getCards(std::map<std::string, Card>& cards, std::map<int, std::string>& mvids) = 0;
};

#endif

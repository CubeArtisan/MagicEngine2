#include "card.h"
#include "ability.h"

#include "cards/cardManager.h"

class IManager : public LetterManager {
public:
    void getCards(std::map<std::string, std::shared_ptr<Card>>, std::map<int, std::string>)
    {}
};

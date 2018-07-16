#include "../ability.h"
#include "../card.h"
#include "../environment.h"

#include "cardManager.h"

Card Island = Card(std::set<CardSuperType>{BASIC}, std::set<CardType>{LAND}, std::set<CardSubType>{ISLAND}, 0, 0, 0,
	"Island", 0, std::set<Color>{}, std::vector<std::shared_ptr<ActivatedAbility>>{
	std::shared_ptr<ActivatedAbility>(new ManaAbility(Mana(std::multiset<Color>{BLUE}),
		std::vector<std::shared_ptr<Cost>>{std::shared_ptr<Cost>(new TapCost())}))},
                   std::vector<std::function<Changeset&(Changeset&, const Environment&)>>{},
                   std::vector<std::shared_ptr<Cost>>{std::shared_ptr<Cost>(new LandPlayCost())}, std::vector<std::shared_ptr<Cost>>{});

class IManager : public LetterManager {
public:
    void getCards(std::map<std::string, Card>& cards, std::map<int, std::string>&)
    {
        cards[Island.name] = Island;
    }
};

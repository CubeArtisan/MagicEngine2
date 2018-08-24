#include "../ability.h"
#include "../card.h"
#include "../environment.h"

#include "cardManager.h"

Card Mountain = newCard("Mountain", 0, std::set<CardSuperType>{BASIC}, std::set<CardType>{LAND}, std::set<CardSubType>{MOUNTAIN}, 0, 0, 0,
					std::set<Color>{}, std::shared_ptr<TargetingRestriction>(new NoTargets()),
						{ LandPlayCost() }, {},
					{},
					std::vector<std::shared_ptr<ActivatedAbility>>{
						std::shared_ptr<ActivatedAbility>(new ManaAbility(Mana(std::multiset<Color>{RED}), {TapCost()}))});

class MManager : public LetterManager {
public:
	void getCards(std::map<std::string, Card>& cards, std::map<int, std::string>&)
	{
		cards.insert(std::make_pair(Mountain.name, Mountain));
	}
};

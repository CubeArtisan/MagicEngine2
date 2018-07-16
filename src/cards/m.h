#include "../ability.h"
#include "../card.h"
#include "../environment.h"

#include "cardManager.h"

Card Mountain = Card(std::set<CardSuperType>{BASIC}, std::set<CardType>{LAND}, std::set<CardSubType>{MOUNTAIN}, 0, 0, 0,
					"Mountain", 0, std::set<Color>{}, std::vector<std::shared_ptr<ActivatedAbility>>{
					std::shared_ptr<ActivatedAbility>(new ManaAbility(Mana(std::multiset<Color>{RED}),
						std::vector<std::shared_ptr<Cost>>{std::shared_ptr<Cost>(new TapCost())}))},
					std::shared_ptr<TargetingRestriction>(new NoTargets()),
						std::vector<std::function<Changeset&(Changeset&, const Environment&, xg::Guid)>>{},
						std::vector<std::shared_ptr<Cost>>{std::shared_ptr<Cost>(new LandPlayCost())}, std::vector<std::shared_ptr<Cost>>{});

class MManager : public LetterManager {
public:
	void getCards(std::map<std::string, Card>& cards, std::map<int, std::string>&)
	{
		cards[Mountain.name] = Mountain;
	}
};

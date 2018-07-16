#include <map>

#include "../ability.h"
#include "../card.h"
#include "../environment.h"

#include "cardManager.h"

Card LightningBolt = Card(std::set<CardSuperType>{}, std::set<CardType>{INSTANT}, std::set<CardSubType>{}, 0, 0, 0,
						  "Lightning Bolt", 0, std::set<Color>{RED}, std::vector<std::shared_ptr<ActivatedAbility>>{},
						  std::shared_ptr<TargetingRestriction>(new AnyTarget()),
						  std::vector<std::function<Changeset&(Changeset&, const Environment&, xg::Guid)>>
							{ [](Changeset& change, const Environment& env, xg::Guid source) -> 
								Changeset& { change.damage.push_back(DamageToTarget{ env.targets.at(source)[0], 3 }); return change; } },
	std::vector<std::shared_ptr<Cost>>{std::shared_ptr<Cost>(new ManaCost(Mana(std::multiset<Color>{RED})))}, std::vector<std::shared_ptr<Cost>>{});

class LManager : public LetterManager {
public:
	void getCards(std::map<std::string, Card>& cards, std::map<int, std::string>&)
	{
		cards[LightningBolt.name] = LightningBolt;
	}
};

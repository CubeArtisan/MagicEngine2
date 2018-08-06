#include <map>

#include "../ability.h"
#include "../card.h"
#include "../environment.h"

#include "cardManager.h"

Card LightningBolt = newCard("Lightning Bolt", 0, std::set<CardSuperType>{}, std::set<CardType>{INSTANT}, std::set<CardSubType>{}, 0, 0, 0,
						     std::set<Color>{RED}, std::shared_ptr<TargetingRestriction>(new AnyTarget()),
							{ std::shared_ptr<Cost>(new ManaCost(Mana(std::multiset<Color>{RED}))) }, std::vector<std::shared_ptr<Cost>>{},
						 	 std::vector<std::function<std::optional<Changeset>(xg::Guid, const Environment&)>>
						 		 { [](xg::Guid source, const Environment& env) ->
								std::optional<Changeset> { Changeset change;
											if (env.gameObjects.find(env.targets.at(source)[0]) != env.gameObjects.end())
												change.damage.push_back(DamageToTarget{ env.targets.at(source)[0], 3, source });
											return change; } });

class LManager : public LetterManager {
public:
	void getCards(std::map<std::string, Card>& cards, std::map<int, std::string>&)
	{
		cards.insert(std::make_pair(LightningBolt.name, LightningBolt));
	}
};

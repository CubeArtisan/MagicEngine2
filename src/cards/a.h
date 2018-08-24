#include "cardManager.h"
#include "../environment.h"
#include "../targeting.h"

Card AncestralRecall = newCard("Ancestral Recall", 1, std::set<CardSuperType>{}, std::set<CardType>{SORCERY}, std::set<CardSubType>{}, 0, 0, 0,
							   std::set<Color>{BLUE}, std::shared_ptr<TargetingRestriction>(new PlayerTarget()),
								{ManaCost(Mana(std::multiset<Color>{BLUE})) },
								{}, std::vector<std::function<std::optional<Changeset>(xg::Guid, const Environment&)>>{
								   [](xg::Guid source, const Environment& env) -> std::optional<Changeset> {
									   return Changeset::drawCards(env.targets.at(source)[0], 3, env);
								   }});

class AManager : public LetterManager {
public:
	void getCards(std::map<std::string, Card>& cards, std::map<int, std::string>&)
	{
		cards.insert(std::make_pair(AncestralRecall.name, AncestralRecall));
	}
};

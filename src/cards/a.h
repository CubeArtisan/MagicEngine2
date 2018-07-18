#include "cardManager.h"
#include "../environment.h"
#include "../targeting.h"

Card AncestralRecall = newCard("Ancestral Recall", 1, std::set<CardSuperType>{}, std::set<CardType>{SORCERY}, std::set<CardSubType>{}, 0, 0, 0,
							   std::set<Color>{BLUE}, std::shared_ptr<TargetingRestriction>(new TargetPlayer()),
							   Mana(std::multiset<Color>{BLUE}), std::vector<std::shared_ptr<Cost>>{}, std::vector<std::function<Changeset&(Changeset&, const Environment&, xg::Guid)>>{
								   [](Changeset& changeset, const Environment& env, xg::Guid source) -> Changeset& {
									   return changeset += Changeset::drawCards(env.targets.at(source)[0], 3, env);
								   }});

class AManager : public LetterManager {
public:
	void getCards(std::map<std::string, Card>& cards, std::map<int, std::string>&)
	{
		cards.insert(std::make_pair(AncestralRecall.name, AncestralRecall));
	}
};

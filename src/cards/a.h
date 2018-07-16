#include "cardManager.h"
#include "../environment.h"
#include "../targeting.h"

Card AncestralRecall = Card(std::set<CardSuperType>{}, std::set<CardType>{SORCERY}, std::set<CardSubType>{}, 0, 0, 0,
	"Ancestral Recall", 1, std::set<Color>{BLUE}, std::vector<std::shared_ptr<ActivatedAbility>>{},
	std::shared_ptr<TargetingRestriction>(new TargetPlayer()),
	std::vector<std::function<Changeset&(Changeset&, const Environment&, xg::Guid)>>{
		[](Changeset& changeset, const Environment& env, xg::Guid source) -> Changeset& {
			return changeset += Changeset::drawCards(env.targets.at(source)[0], 3, env);
		}},
	std::vector<std::shared_ptr<Cost>>{std::shared_ptr<Cost>(new ManaCost(Mana(std::multiset<Color>{BLUE})))},
	std::vector<std::shared_ptr<Cost>>{});

class AManager : public LetterManager {
public:
	void getCards(std::map<std::string, Card>& cards, std::map<int, std::string>&)
	{
		cards[AncestralRecall.name] = AncestralRecall;
	}
};

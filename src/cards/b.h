#include "cardManager.h"
#include "../environment.h"
#include "../targeting.h"

Card BronzeSable = Card(std::set<CardSuperType>{}, std::set<CardType>{CREATURE, ARTIFACT}, std::set<CardSubType>{SABLE}, 2, 1, 0,
						"Bronze Sable", 1, std::set<Color>{}, std::vector<std::shared_ptr<ActivatedAbility>>{},
						std::shared_ptr<TargetingRestriction>(new NoTargets()),
						std::vector<std::function<Changeset&(Changeset&, const Environment&, xg::Guid)>>{},
						std::vector<std::shared_ptr<Cost>>{std::shared_ptr<Cost>(new ManaCost(Mana(2, std::multiset<Color>{})))},
						std::vector<std::shared_ptr<Cost>>{});

class BManager : public LetterManager {
public:
	void getCards(std::map<std::string, Card>& cards, std::map<int, std::string>&)
	{
		cards[BronzeSable.name] = BronzeSable;
	}
};

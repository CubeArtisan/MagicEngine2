#include "cardManager.h"
#include "../environment.h"
#include "../targeting.h"

Card BronzeSable = newCard("Bronze Sable", 1, std::set<CardSuperType>{}, std::set<CardType>{CREATURE, ARTIFACT}, std::set<CardSubType>{SABLE},
						   2, 1, 0, std::set<Color>{}, std::shared_ptr<TargetingRestriction>(new NoTargets()),
						   Mana(2, std::multiset<Color>{}));

class BManager : public LetterManager {
public:
	void getCards(std::map<std::string, Card>& cards, std::map<int, std::string>&)
	{
		cards[BronzeSable.name] = BronzeSable;
	}
};

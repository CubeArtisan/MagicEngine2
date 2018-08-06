#include "cardManager.h"
#include "../environment.h"
#include "../targeting.h"
#include "../propositions/raid.h"

Card Opt = newCard("Opt", 1, {}, { INSTANT }, {},
	0, 0, 0, { BLUE }, std::shared_ptr<TargetingRestriction>(new NoTargets()),
	std::vector<std::shared_ptr<Cost>> { std::shared_ptr<Cost>(new ManaCost(Mana({ BLUE }))) }, {},
	{ [](xg::Guid source, const Environment& env) -> std::optional<Changeset> { return Changeset::scryCards(env.getController(source), 1, env); },
	[](xg::Guid source, const Environment& env) -> std::optional<Changeset> { return Changeset::drawCards(env.getController(source), 1, env); } });

class OManager : public LetterManager {
public:
	void getCards(std::map<std::string, Card>& cards, std::map<int, std::string>&)
	{
		insertCard(cards, Opt);
	}
};

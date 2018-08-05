#include "cardManager.h"
#include "../environment.h"
#include "../targeting.h"
#include "../propositions/raid.h"

Card ChartACourse = newCard("Chart a Course", 1, {}, { SORCERY }, {},
	2, 1, 0, { BLUE }, std::shared_ptr<TargetingRestriction>(new NoTargets()),
	Mana(1, { BLUE }), {}, { [](Changeset& changes, const Environment& env, xg::Guid source) -> Changeset& { changes += Changeset::drawCards(env.getController(source), 2, env); return changes; },
							 [](Changeset& changes, const Environment& env, xg::Guid source) -> Changeset& {
								if (RaidProposition(env.getController(source))(env))
									changes += Changeset::discardCards(env.getController(source), 1, env);
								return changes; } });

class CManager : public LetterManager {
public:
	void getCards(std::map<std::string, Card>& cards, std::map<int, std::string>&)
	{
		insertCard(cards, ChartACourse);
	}
};

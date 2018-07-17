#include "cardManager.h"
#include "../environment.h"
#include "../targeting.h"

Card Divination = Card(std::set<CardSuperType>{}, std::set<CardType>{SORCERY}, std::set<CardSubType>{}, 0, 0, 0,
                       "Divination", 3, std::set<Color>{BLUE}, std::vector<std::shared_ptr<ActivatedAbility>>{},
                       std::shared_ptr<TargetingRestriction>(new NoTargets()),
					   std::vector<std::function<Changeset&(Changeset&, const Environment&, xg::Guid)>>{
							[](Changeset& changeset, const Environment& env, xg::Guid) -> Changeset& {
								return changeset += Changeset::drawCards(env.players[0]->id, 2, env);
							}},
                       std::vector<std::shared_ptr<Cost>>{std::shared_ptr<Cost>(new ManaCost(Mana(2, std::multiset<Color>{BLUE})))},
                       std::vector<std::shared_ptr<Cost>>{});

class DManager : public LetterManager {
public:
    void getCards(std::map<std::string, Card>& cards, std::map<int, std::string>&)
    {
        cards[Divination.name] = Divination;
    }
};

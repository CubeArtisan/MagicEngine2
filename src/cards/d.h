#include "cardManager.h"
#include "../environment.h"
#include "../targeting.h"

Card Divination = newCard("Divination", 3, std::set<CardSuperType>{}, std::set<CardType>{SORCERY}, std::set<CardSubType>{}, 0, 0, 0,
                          std::set<Color>{BLUE}, std::shared_ptr<TargetingRestriction>(new NoTargets()),  
	std::vector<std::shared_ptr<Cost>>{std::shared_ptr<Cost>(new ManaCost(Mana(2, std::multiset<Color>{BLUE}))) }, std::vector<std::shared_ptr<Cost>>{},
						  std::vector<std::function<std::optional<Changeset>(xg::Guid, const Environment&)>>{
						  [](xg::Guid source, const Environment& env) -> std::optional<Changeset> {
						  	return Changeset::drawCards(env.getController(source), 2, env);
						  }});

class DManager : public LetterManager {
public:
    void getCards(std::map<std::string, Card>& cards, std::map<int, std::string>&)
    {
        cards.insert(std::make_pair(Divination.name, Divination));
    }
};

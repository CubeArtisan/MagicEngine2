#ifndef _CARDMANAGER_H_
#define _CARDMANAGER_H_

#include <memory>
#include <string>

#include "../card.h"

class CardManager {
public:
    Card getCard(int mvid);
    Card getCard(std::string name);

    CardManager();

private:
	// CodeReview: Double faced cards likely require a subclass so this map will not work
	// Also split cards
    std::map<std::string, Card> cards;
    std::map<int, std::string> mvids;
};

struct LetterManager {
    virtual void getCards(std::map<std::string, Card>& cards, std::map<int, std::string>& mvids) = 0;
};

Card newCard(std::string name, unsigned int cmc, std::set<CardSuperType> superTypes, std::set<CardType> types,
			 std::set<CardSubType> subTypes, int power, int toughness, int loyalty, std::set<Color> colors,
			 std::shared_ptr<TargetingRestriction> targeting,
			 Mana cost, std::vector<std::shared_ptr<Cost>> additionalCosts = {},
			 std::vector<std::function<Changeset&(Changeset&, const Environment&, xg::Guid)>> applyAbilities = {},
			 std::vector<std::shared_ptr<ActivatedAbility>> activatedAbilities = {},
	std::vector<std::shared_ptr<EventHandler>> replacementEffects = {}, std::vector<std::shared_ptr<TriggerHandler>> triggerEffects = {},
	std::vector<std::shared_ptr<StateQueryHandler>> staticEffects = {}, std::vector<size_t> thisOnlyReplacementIndexes = {});

Card newCard(std::string name, unsigned int cmc, std::set<CardSuperType> superTypes, std::set<CardType> types,
			 std::set<CardSubType> subTypes, int power, int toughness, int loyalty, std::set<Color> colors,
			 std::shared_ptr<TargetingRestriction> targeting,
			 std::vector<std::shared_ptr<Cost>> costs, std::vector<std::shared_ptr<Cost>> additionalCosts = {},
			 std::vector<std::function<Changeset&(Changeset&, const Environment&, xg::Guid)>> applyAbilities = {},
			 std::vector<std::shared_ptr<ActivatedAbility>> activatedAbilities = {},
			 std::vector<std::shared_ptr<EventHandler>> replacementEffects = {}, std::vector<std::shared_ptr<TriggerHandler>> triggerEffects = {},
			 std::vector<std::shared_ptr<StateQueryHandler>> staticEffects = {}, std::vector<size_t> thisOnlyReplacementIndexes = {});

void insertCard(std::map<std::string, Card>& cards, Card card);
#endif

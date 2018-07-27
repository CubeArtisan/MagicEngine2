#ifndef _TOKENMANAGER_H_
#define _TOKENMANAGER_H_

#include <memory>
#include <string>

#include "../card.h"

class TokenManager {
public:
    Token getToken(int mvid);
    Token getToken(std::string name);

    TokenManager();

private:
	// CodeReview: Double faced cards likely require a subclass so this map will not work
	// Also split cards
    std::map<std::string, Token> tokens;
	// Do tokens have mvids?
    std::map<int, std::string> mvids;
};

struct LetterTokenManager {
    virtual void getTokens(std::map<std::string, Token>& tokens, std::map<int, std::string>& mvids) = 0;
};

Token newToken(std::string name, unsigned int cmc, std::set<CardSuperType> superTypes, std::set<CardType> types,
			   std::set<CardSubType> subTypes, int power, int toughness, int loyalty, std::set<Color> colors,
			   std::shared_ptr<TargetingRestriction> targeting,
			   std::vector<std::function<Changeset&(Changeset&, const Environment&, xg::Guid)>> applyAbilities = {},
			   std::vector<std::shared_ptr<ActivatedAbility>> activatedAbilities = {},
			   std::vector<std::shared_ptr<EventHandler>> replacementEffects = {}, std::vector<std::shared_ptr<TriggerHandler>> triggerEffects = {},
			   std::vector<std::shared_ptr<StaticEffectHandler>> staticEffects = {}, std::vector<size_t> thisOnlyReplacementIndexes = {});

void insertToken(std::map<std::string, Token>& tokens, const Token& token);

extern TokenManager tokenManager;
#endif

#include "tokenManager.h"
#include "../environment.h"
#include "../targeting.h"



class GTokenManager : public LetterTokenManager {
public:
	void getTokens(std::map<std::string, Token>& tokens, std::map<int, std::string>&)
	{
		Token Goblin = newToken("Goblin", 0, {}, { CREATURE }, { GOBLIN }, 1, 1, 0,
			{ RED }, std::shared_ptr<TargetingRestriction>(new NoTargets()));
		insertToken(tokens, Goblin);
	}
};

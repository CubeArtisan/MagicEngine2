#include "tokenManager.h"
#include "../environment.h"
#include "../targeting.h"


class STokenManager : public LetterTokenManager {
public:
	void getTokens(std::map<std::string, Token>& tokens, std::map<int, std::string>&)
	{
		Token Saproling = newToken("Saproling", 0, {}, { CREATURE }, { SAPROLING }, 1, 1, 0,
			{ GREEN }, std::shared_ptr<TargetingRestriction>(new NoTargets()));
		insertToken(tokens, Saproling);
	}
};

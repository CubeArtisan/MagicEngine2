#include <variant>

#include "changeset.h"
#include "environment.h"

class Runner {
public:
    void runGame();
    Runner(std::vector<std::vector<Card>>& libraries, std::vector<Player> players);

private:
	std::vector<std::vector<Card>> libraries;
	std::vector<Player> players;
    Environment env;

	std::variant<std::monostate, Changeset> checkStateBasedActions();
	std::variant<Changeset, PassPriority> executeStep();
	bool applyReplacementEffects(Changeset& changeset, std::set<xg::Guid> applied = {});
	void applyChangeset(Changeset& changeset, bool replacementEffects=true);
};

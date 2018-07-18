#include <variant>

#include "changeset.h"
#include "environment.h"

class Runner {
public:
    void runGame();

    Runner(std::vector<std::vector<Card>>& libraries, std::vector<Player> players);

private:
	const std::vector<std::vector<Card>> libraries;
	const std::vector<Player> players;
    Environment env;

	std::variant<std::monostate, Changeset> checkStateBasedActions() const;
	std::variant<Changeset, PassPriority> executeStep() const;
	void applyMoveRules(Changeset& changeset);
	bool applyReplacementEffects(Changeset& changeset, std::set<xg::Guid> applied = {});
	void applyChangeset(Changeset& changeset, bool replacementEffects=true);
};

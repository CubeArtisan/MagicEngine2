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

	std::optional<Changeset> checkStateBasedActions() const;
	std::optional<Changeset> executeStep() const;
	void applyMoveRules(Changeset& changeset);
	bool applyReplacementEffects(Changeset& changeset, std::set<xg::Guid> applied = {});
	void applyChangeset(Changeset& changeset, bool replacementEffects=true);
	void allowActivateManaAbility(xg::Guid player) const;
};

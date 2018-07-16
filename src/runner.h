#include <variant>

#include "changeset.h"
#include "environment.h"

class Runner {
public:
    std::variant<Changeset, PassPriority> executeStep();
	std::variant<std::monostate, Changeset> checkStateBasedActions();
    void runGame();
    void applyChangeset(Changeset& changeset);
    Runner(std::vector<std::vector<Card>>& libraries, std::vector<Player> players);

private:
    Environment env;
};

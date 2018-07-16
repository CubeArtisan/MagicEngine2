#include <variant>

#include "changeset.h"
#include "environment.h"

class Runner {
public:
    std::variant<Changeset, PassPriority> executeStep();
    void runGame();
    void applyChangeset(Changeset& changeset);
    Runner(std::vector<std::vector<std::shared_ptr<Card>>> libraries, std::vector<Player> players);

private:
    Environment env;
};

#include <variant>

#include "changeset.h"
#include "environment.h"

class Runner {
public:
    std::variant<Changeset, PassPriority> executeStep();
    void runGame();
    void applyChangeset(Changeset& changeset);
    void startGame(std::vector<std::vector<std::shared_ptr<Card>>> libraries, std::vector<std::shared_ptr<Strategy>> strategies);

private:
    Environment env;
};

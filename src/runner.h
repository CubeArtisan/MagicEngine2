#include <variant>

#include "changeset.h"
#include "environment.h"

class Runner {
public:
    std::variant<Changeset, PassPriority> executeStep();
    void runGame();
    void applyChangeset(Changeset& changeset);

private:
    Environment env;
};

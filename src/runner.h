#include "changeset.h"
#include "environment.h"

class Runner {
public:
    std::vector<Changeset> executeStep();

private:
    Environment env;
};

#include "changeset.h"
#include "environment.h"

class Runner {
public:
    Changeset executeStep();

private:
    Environment env;
};

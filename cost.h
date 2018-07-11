#ifndef _COST_H_
#define _COST_H_

#include "mana.h"

class Cost {
public:
    virtual bool canPay(Environment& env);
    virtual Changeset payCost(Environment& env);
};

class ManaCost : public Cost {
public:
    virtual bool canPay(Environement& env);
    virtual Changeset payCost(Environment& env);

private:
    Mana mana;
}
#endif

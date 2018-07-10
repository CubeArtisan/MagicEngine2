#ifndef _COST_H_
#define _COST_H_

class Cost {
public:
    virtual bool canPay(Environment& env);
    virtual Changeset payCost(Environment& env);
};

#endif

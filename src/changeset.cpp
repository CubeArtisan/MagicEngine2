#include "changeset.h"
#include "environment.h"

Targetable::Targetable()
    : id(xg::newGuid())
{}

Changeset Changeset::drawCards(xg::Guid player, unsigned int amount, Environment& env){
    Changeset result = Changeset();
    Zone<std::variant<Card, Token>> libraryZone = env.libraries[player];
    std::vector<std::variant<Card, Token>> library = libraryZone.objects;
    Zone<std::variant<Card, Token>> handZone = env.hands[player];
    
    if(amount > library.size()){
        result.millOut = true;
        amount = library.size();
    }
    auto card = library.begin() + library.size() - amount;
    for(; card != library.end(); card++) {
        Targetable& c = getBaseClass<Targetable>(*card);
        result.moves.push_back(ObjectMovement{c.id, libraryZone.id, handZone.id});
    }
    return result;
}

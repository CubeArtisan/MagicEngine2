#include "changeset.h"
#include "player.h"
#include "runner.h"
#include "gameAction.h"

std::vector<Changeset> Runner::executeStep()
{
    Player& active = (Player&)this->env.gameObjects[this->env.currentPlayer];

    GameAction action = active.strategy.chooseGameAction(active, env);

    

    if(CastSpell* pCastSpell = std::get_if<CastSpell>(&action)){
        std::vector<Changeset> results;
        Changeset castSpell;
        Targetable hand = this->env.hands[active.id];
        castSpell.moves.push_back(ObjectMovement{pCastSpell->spell, hand.id, this->env.stack.id});
        results.push_back(castSpell);
        // Assign targets
        results.push_back(pCastSpell->cost.payCost(active, env));
        for(auto& c : pCastSpell->additionalCost) {
            results.push_back(c.payCost(active, env));
        }
        // Use the chosen X value
        return results;
    }

    if(PlayLand* pPlayLand = std::get_if<PlayLand>(&action)){
        std::vector<Changeset> results;
        Changeset castSpell;
        Targetable hand = this->env.hands[active.id];
        castSpell.moves.push_back(ObjectMovement{pPlayLand->land, hand.id, this->env.stack.id});
        // Use land play for the turn
    }
}

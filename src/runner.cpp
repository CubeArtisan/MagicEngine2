#include "changeset.h"
#include "player.h"
#include "runner.h"
#include "gameAction.h"

Changeset Runner::executeStep()
{
    Player& active = (Player&)this->env.gameObjects[this->env.currentPlayer];

    GameAction action = active.strategy.chooseGameAction(active, env);

    

    if(CastSpell* pCastSpell = std::get_if<CastSpell>(&action)){
        Changeset castSpell;
        Targetable hand = this->env.hands[active.id];
        castSpell.moves.push_back(ObjectMovement{pCastSpell->spell, hand.id, this->env.stack.id});
        // CodeReview: Assign targets
        castSpell += pCastSpell->cost.payCost(active, env);
        for(Cost& c : pCastSpell->additionalCosts) {
            castSpell += c.payCost(active, env);
        }
        // CodeReview: Use the chosen X value
        return castSpell;
    }

    if(PlayLand* pPlayLand = std::get_if<PlayLand>(&action)){
        std::vector<Changeset> results;
        Changeset playLand;
        Targetable hand = this->env.hands[active.id];
        playLand.moves.push_back(ObjectMovement{pPlayLand->land, hand.id, this->env.stack.id});
        // CodeReview: Use land play for the turn
        return playLand;
    }

    if(ActivateAnAbility* pActivateAnAbility = std::get_if<ActivateAnAbility>(&action)){
        Changeset activateAbility;
        ActivatedAbility result = pActivateAnAbility->ability;
        result.source = pActivateAnAbility->source;
        activateAbility.create.push_back(ObjectCreation{this->env.stack.id, result});
        // CodeReview: Assign targets
        activateAbility += pActivateAnAbility->cost.payCost(active, env);
        // CodeReview: Use the chosen X value
        return activateAbility;
    }

    if(std::get_if<PassPriority>(&action)){
        // CodeReview: resolve stack or pass
    }

    return Changeset();
}

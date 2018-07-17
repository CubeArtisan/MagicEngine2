#include <random>
#include <vector>

#include "card.h"
#include "environment.h"
#include "gameAction.h"
#include "player.h"
#include "strategy.h"
#include "targeting.h"

template<typename Iter, typename RandomGenerator>
Iter select_randomly(Iter start, Iter end, RandomGenerator& g) {
    std::uniform_int_distribution<> dis(0, (int)std::distance(start, end) - 1);
    std::advance(start, dis(g));
    return start;
}

template<typename Iter>
Iter select_randomly(Iter start, Iter end) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    return select_randomly(start, end, gen);
}

template<template<class, class> class Container, typename T, typename Alloc>
T select_randomly(Container<T, Alloc> cont) {
    return *select_randomly(cont.begin(), cont.end());
}

std::variant<std::shared_ptr<Card>, std::shared_ptr<Token>,
             std::shared_ptr<Emblem>> upShiftVariant(std::variant<std::shared_ptr<Card>,
                                                                         std::shared_ptr<Token>> variant) {
    if(std::shared_ptr<Card>* pCard = std::get_if<std::shared_ptr<Card>>(&variant)) {
        return *pCard;
    }
    if(std::shared_ptr<Token>* pToken = std::get_if<std::shared_ptr<Token>>(&variant)) {
        return *pToken;
    }
#ifdef DEBUG
    std::cerr << "Variant was in a malformed state" << std::endl;
#endif
    throw "Variant was in a malformed state";
}


GameAction RandomStrategy::chooseGameAction(Player& player, Environment& env) 
{
	if (env.currentPhase != PRECOMBATMAIN) return PassPriority();
    std::vector<GameAction> possibilities;
    for(auto& cardWrapper : env.hands[player.id]->objects) {
        if(std::shared_ptr<Card>* pCard = std::get_if<std::shared_ptr<Card>>(&cardWrapper)) {
            std::shared_ptr<Card> card = *pCard;
            if(std::shared_ptr<Cost> pCost = card->canPlay(player, env)) {
				std::set<CardType> types = env.getTypes(card);
                if(types.find(LAND) != types.end()) {
                    possibilities.push_back(PlayLand{card->id});
                }
                else {
					std::vector<xg::Guid> targets;
					if (card->targeting->maxTargets > 0) {
						for (int i = 0; i < card->targeting->maxTargets; i++) {
							for (const auto& object : env.gameObjects) {
								if (object.first != object.second->id) continue;
								targets.push_back(object.first);
								if (card->targeting->validFirstN(targets, env)) {
									break;
								}
								targets.pop_back();
							}
								
						}
					}
                    possibilities.push_back(CastSpell{card->id, targets, *pCost,
                                            std::vector<std::shared_ptr<Cost>>(), 0});
                }
            }
        }
    }

    for(auto& cardWrapper : env.battlefield->objects){
        std::shared_ptr<CardToken> card = getBaseClassPtr<CardToken>(cardWrapper);
        for(std::shared_ptr<ActivatedAbility> pAbility : env.getActivatedAbilities(card)) {
			pAbility->source = cardWrapper;
            if(std::shared_ptr<Cost> pCost = pAbility->canPlay(player, env)) {
                possibilities.push_back(ActivateAnAbility{cardWrapper, pAbility, std::vector<xg::Guid>(), *pCost, 0});
            }
        }
    }

    if(possibilities.empty()) possibilities.push_back(PassPriority());

    return select_randomly(possibilities);
}

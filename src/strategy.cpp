#include <numeric>
#include <random>
#include <vector>

#include "card.h"
#include "changeset.h"
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

std::variant<std::shared_ptr<const Card>, std::shared_ptr<const Token>,
             std::shared_ptr<const Emblem>> upShiftVariant(std::variant<std::shared_ptr<const Card>,
                                                                        std::shared_ptr<const Token>> variant) {
    if(std::shared_ptr<const Card>* pCard = std::get_if<std::shared_ptr<const Card>>(&variant)) {
        return *pCard;
    }
    if(std::shared_ptr<const Token>* pToken = std::get_if<std::shared_ptr<const Token>>(&variant)) {
        return *pToken;
    }
#ifdef DEBUG
    std::cerr << "Variant was in a malformed state" << std::endl;
#endif
    throw "Variant was in a malformed state";
}


GameAction RandomStrategy::chooseGameAction(const Player& player, const Environment& env) 
{
	if (env.currentPhase != PRECOMBATMAIN) return PassPriority();
    std::vector<GameAction> possibilities;
    for(auto& cardWrapper : env.hands.at(player.id)->objects) {
        if(const std::shared_ptr<const Card>* pCard = std::get_if<std::shared_ptr<const Card>>(&cardWrapper)) {
            std::shared_ptr<const Card> card = *pCard;
            if(std::shared_ptr<const Cost> pCost = card->canPlay(player, env)) {
				std::shared_ptr<const std::set<CardType>> types = env.getTypes(card);
                if(types->find(LAND) != types->end()) {
                    possibilities.push_back(PlayLand{card->id});
                }
                else {
					std::vector<xg::Guid> targets = this->chooseTargets(card, player, env);
					if (targets.size() < card->targeting->minTargets) continue;
                    possibilities.push_back(CastSpell{card->id, targets, *pCost,
                                            std::vector<std::shared_ptr<Cost>>(), 0});
                }
            }
        }
    }

    for(auto& cardWrapper : env.battlefield->objects){
        std::shared_ptr<const CardToken> card = getBaseClassPtr<const CardToken>(cardWrapper);
		if (player.id != card->owner) continue;
		for(std::shared_ptr<const ActivatedAbility> pAbility : *env.getActivatedAbilities(card)) {
			std::shared_ptr<ActivatedAbility> ability = std::dynamic_pointer_cast<ActivatedAbility>(pAbility->clone());
			ability->source = cardWrapper;
            if(std::shared_ptr<const Cost> pCost = ability->canPlay(player, env)) {
                possibilities.push_back(ActivateAnAbility{cardWrapper, ability, std::vector<xg::Guid>(), *pCost, 0});
            }
        }
    }

    if(possibilities.empty()) possibilities.push_back(PassPriority());

    return select_randomly(possibilities);
}

std::vector<xg::Guid> RandomStrategy::chooseTargets(const std::shared_ptr<const HasEffect> effect, const Player& player, const Environment& env) {
	std::vector<xg::Guid> targets;
	if (effect->targeting->maxTargets > 0) {
		for (int i = 0; i < effect->targeting->maxTargets; i++) {
			for (const auto& object : env.gameObjects) {
				targets.push_back(object.first);
				if (effect->targeting->validFirstN(targets, env)) {
					break;
				}
				targets.pop_back();
			}
		}
	}
	// CodeReview: Handle this case
	if (targets.size() < effect->targeting->minTargets) {
		throw "Can't choose targets";
	}

	return targets;
}

std::vector<xg::Guid> RandomStrategy::chooseDiscards(size_t amount, const Player& player, const Environment& env) {
	auto handObjects = env.hands.at(player.id)->objects;
	std::vector<size_t> indices(handObjects.size());
	std::iota(indices.begin(), indices.end(), 0);
	std::shuffle(indices.begin(), indices.end(), std::mt19937{ std::random_device{}() });
	std::vector<xg::Guid> result;
	result.reserve(amount);
	for (int i = 0; i < amount; i++) result.push_back(getBaseClassPtr<const Targetable>(handObjects.at(indices[i]))->id);
	return result;
}


std::optional<std::pair<std::shared_ptr<CardToken>, xg::Guid>> RandomStrategy::chooseAttacker(std::vector<std::shared_ptr<CardToken>>& possibleAttackers,
																							  std::map<xg::Guid, std::set<xg::Guid>>& possibleAttacks,
																							  std::map<xg::Guid, std::multiset<xg::Guid>>& requiredAttacks,
																							  std::vector<std::pair<std::shared_ptr<CardToken>, xg::Guid>>& declaredAttacks) {
	std::vector<std::optional<std::pair<std::shared_ptr<CardToken>, xg::Guid>>> possibilities;
	for (auto& possibleAttacker : possibleAttackers) {
		for (auto& possibleDefender : possibleAttacks[possibleAttacker->id]) {
			possibilities.push_back(make_pair(possibleAttacker, possibleDefender));
		}
	}
	possibilities.push_back(std::nullopt);
	return select_randomly(possibilities);
}

std::optional<std::pair<std::shared_ptr<CardToken>, xg::Guid>> RandomStrategy::chooseBlocker(std::vector<std::shared_ptr<CardToken>>& possibleBlockers,
																							 std::map<xg::Guid, std::set<xg::Guid>>& possibleBlocks,
																							 std::map<xg::Guid, std::multiset<xg::Guid>>& requiredBlocks,
																							 std::vector<std::pair<std::shared_ptr<CardToken>, xg::Guid>>& declaredBlocks) {
	std::vector<std::optional<std::pair<std::shared_ptr<CardToken>, xg::Guid>>> possibilities;
	for (auto& possibleBlocker : possibleBlockers) {
		for (auto& possibleAttacker : possibleBlocks[possibleBlocker->id]) {
			possibilities.push_back(make_pair(possibleBlocker, possibleAttacker));
		}
	}
	possibilities.push_back(std::nullopt);
	return select_randomly(possibilities);
}
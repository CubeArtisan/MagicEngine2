#ifndef _FLIERS_H_
#define _FLIERS_H_

#include "../guid.hpp"
#include "../strategy.h"
#include "../linq/util.h"

class FliersStrategy : public RandomStrategy {
	virtual GameAction chooseGameAction(const Player& player, const Environment& env) override {
		std::vector<GameAction> possibilities = this->generateGameOptions(player, env);
		if (env.currentPhase != POSTCOMBATMAIN) return PassPriority();
		std::sort(possibilities.begin(), possibilities.end(), [=](const GameAction& action, const GameAction& action2) {return this->givePriority(player, env, action) < this->givePriority(player, env, action2); });
		if (this->givePriority(player, env, possibilities.front()) > 100) return PassPriority();
		else return possibilities.front();
	}

	virtual size_t givePriority(const Player& player, const Environment& env, const GameAction& action) {
		if (action.index() == index_of_v<CastSpell, GameAction>) {
			const CastSpell& castSpell = std::get<CastSpell>(action);
			std::shared_ptr<CardToken> spell = std::dynamic_pointer_cast<CardToken>(env.gameObjects.at(castSpell.spell));
			size_t islandCount = 0, lordCount = 0, creatures = 0;
			for (const auto& object : env.battlefield->objects) {
				std::shared_ptr<const CardToken> card = getBaseClassPtr<const CardToken>(object);
				std::shared_ptr<const std::set<CardType>> types = env.getTypes(card);
				std::shared_ptr<const std::set<CardSubType>> subtypes = env.getSubTypes(card);
				std::shared_ptr<const std::set<CardSuperType>> supertypes = env.getSuperTypes(card);
				xg::Guid controller = env.getController(card);
				if (controller != player.id) continue;
				if (subtypes->find(ISLAND) != subtypes->end() && supertypes->find(BASIC) != supertypes->end()) islandCount++;
				if (card->name == "Favorable Winds") lordCount++;
				if (types->find(CREATURE) != types->end()) creatures++;
			}
			if (spell->baseTypes->find(CREATURE) != spell->baseTypes->end()) {
				if (env.currentPhase == PRECOMBATMAIN) return 999;
				if (spell->name == "Tempest Djinn") return 20 - islandCount - lordCount;
				else return (size_t)20 - spell->basePower - lordCount;
			}
			else if (spell->name == "Favorable Winds") {
				return 20 - creatures;
			}
			else if (spell->name == "Curious Obsession") {
				if (creatures == 0) return 999;
				else if (env.currentPhase == PRECOMBATMAIN) return 17;
				else return 19;
			}
			else return env.currentPhase == PRECOMBATMAIN ? 999 : 30;
		}
		else if (action.index() == index_of_v<PlayLand, GameAction>) {
			return 0;
		}
		else if (action.index() == index_of_v<ActivateAnAbility, GameAction>) {
			const ActivateAnAbility& ability = std::get<ActivateAnAbility>(action);
			if (std::dynamic_pointer_cast<ManaAbility>(ability.ability)) {
				return 1;
			}
			else {
				return 100;
			}
		}
		else {
			return 100;
		}
	}
	/*
	virtual std::vector<xg::Guid> chooseTargets(std::shared_ptr<const HasEffect> effect, const Player& player, const Environment& env) = 0;
	virtual std::vector<xg::Guid> chooseDiscards(size_t amount, const Player& player, const Environment& env) = 0;
	virtual std::optional<std::pair<std::shared_ptr<CardToken>, xg::Guid>> chooseAttacker(std::vector<std::shared_ptr<CardToken>>& possibleAttackers,
		std::map<xg::Guid, std::set<xg::Guid>>& possibleAttacks,
		std::map<xg::Guid, std::multiset<xg::Guid>>& requiredAttacks,
		std::vector<std::pair<std::shared_ptr<CardToken>, xg::Guid>>& declaredAttacks) = 0;
	virtual std::optional<std::pair<std::shared_ptr<CardToken>, xg::Guid>> chooseBlocker(std::vector<std::shared_ptr<CardToken>>& possibleBlockers,
		std::map<xg::Guid, std::set<xg::Guid>>& possibleBlocks,
		std::map<xg::Guid, std::multiset<xg::Guid>>& requiredBlocks,
		std::vector<std::pair<std::shared_ptr<CardToken>, xg::Guid>>& declaredBlocks) = 0;
	virtual std::vector<xg::Guid> chooseBlockingOrder(std::shared_ptr<CardToken> attacker, std::vector<std::shared_ptr<CardToken>> blockers,
		const Environment& env) = 0;
	virtual int chooseDamageAmount(std::shared_ptr<CardToken> attacker, xg::Guid blocker, int minDamage, int maxDamage, const Environment& env) = 0;
	virtual std::optional<Changeset> chooseUpToOne(const std::vector<Changeset>& changes, const Player& player, const Environment& env) = 0;
	virtual Changeset chooseOne(const std::vector<Changeset>& changes, const Player& player, const Environment& env) = 0;
	*/
};

#endif
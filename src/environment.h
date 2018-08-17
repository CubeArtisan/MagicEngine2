#ifndef _ENVIRONMENT_H_
#define _ENVIRONMENT_H_

#include <functional>
#include <map>
#include <memory>
#include <set>
#include <typeinfo>
#include <typeindex>
#include <unordered_map>
#include <variant>
#include <vector>

#include "guid.hpp"

#include "changeset.h"
#include "combat.h"
#include "player.h"
#include "card.h"
#include "ability.h"

struct ZoneInterface : public clone_inherit<abstract_method<ZoneInterface>>{
    virtual xg::Guid addObject(const std::shared_ptr<const Targetable>& object, int index = 0) = 0;
    virtual std::shared_ptr<const Targetable> removeObject(xg::Guid object) = 0;
	virtual std::shared_ptr<const Targetable> findObject(xg::Guid object) const = 0;
	virtual std::vector<std::shared_ptr<const Targetable>> getObjects() const = 0;
	const ZoneType type;

	ZoneInterface(ZoneType type)
		: type(type)
	{}
};

template<typename... Args>
struct Zone : public clone_inherit<Zone<Args...>, ZoneInterface, Targetable> {
    std::vector<std::variant<std::shared_ptr<const Args>...>> objects;
    
    xg::Guid addObject(const std::shared_ptr<const Targetable>& object, int index = 0) override {
        this->addObjectInternal<Args...>(object, index);
		return object->id;
	}

    std::shared_ptr<const Targetable> removeObject(xg::Guid object) override {
        for(auto iter=objects.rbegin(); iter != objects.rend(); iter++) {
            std::shared_ptr<const Targetable> val = getBaseClassPtr<const Targetable>(*iter);
            if(val->id == object){
                std::advance(iter, 1);
                this->objects.erase(iter.base());
                return val;
            }
        }
#ifdef DEBUG
        std::cerr << "Failed to find object for removal" << std::endl;
#endif
        throw "Failed to find object for removal";
    }
	std::shared_ptr<const Targetable> findObject(xg::Guid object) const override {
		for (auto iter = objects.rbegin(); iter != objects.rend(); iter++) {
			std::shared_ptr<const Targetable> val = getBaseClassPtr<const Targetable>(*iter);
			if (val->id == object) {
				return val;
			}
		}
		return std::shared_ptr<Targetable>();
	}

	std::vector<std::shared_ptr<const Targetable>> getObjects() const override {
		std::vector<std::shared_ptr<const Targetable>> result;
		result.reserve(objects.size());
		for (auto& object : objects) {
			result.push_back(getBaseClassPtr<const Targetable>(object));
		}
		return result;
	}

	using clone_inherit<Zone<Args...>, ZoneInterface, Targetable>::clone_inherit;

private:
    template<typename T, typename... Extra>
    void addObjectInternal(const std::shared_ptr<const Targetable>& object, int index){
        if(std::shared_ptr<const T> result = std::dynamic_pointer_cast<const T>(object)){
			if (index >= 0) this->objects.insert(this->objects.begin() + (this->objects.size() - index), result);
			else this->objects.insert(this->objects.begin() + (1 - index), result);
        }
        else {
            if constexpr(sizeof...(Extra) == 0) {
#ifdef DEBUG
                std::cerr << "Could not convert to internal types" << std::endl;
#endif
                throw "Could not convert to internal types";
            }
            else {
                return addObjectInternal<Extra...>(object, index);
            }
        }
    }
};

template<typename... Args>
using PrivateZones = std::map<xg::Guid, std::shared_ptr<Zone<Args...>>>;

struct Environment {
    std::unordered_map<xg::Guid, std::shared_ptr<Targetable>> gameObjects;

    PrivateZones<Card, Token> hands;
    PrivateZones<Card, Token> libraries;
    PrivateZones<Card, Token> graveyards;
    std::shared_ptr<Zone<Card, Token>> battlefield;
    std::shared_ptr<Zone<Card, Token, Ability>> stack;
    std::shared_ptr<Zone<Card, Token>> exile;
    std::shared_ptr<Zone<Card, Emblem>> command;
	// CodeReview: Create Sideboard?

	// CodeReview: Suspend, skullbriar, lighnting storm all have counters off battlefield
	std::map<xg::Guid, std::map<PermanentCounterType, unsigned int>> permanentCounters;
	// Should be unsigned but that causes issues with comparisons
	std::map<xg::Guid, int> damage;

	std::vector<std::shared_ptr<Player>> players;
    std::map<xg::Guid, Mana> manaPools;
    // CodeReview: Does this maintain across undoing turn changes
    std::map<xg::Guid, unsigned int> landPlays;
    std::map<xg::Guid, std::map<PlayerCounterType, unsigned int>> playerCounters;
    std::map<xg::Guid, int> lifeTotals;

	std::map<xg::Guid, std::vector<xg::Guid>> targets;

    std::vector<std::shared_ptr<TriggerHandler>> triggerHandlers;
    std::vector<std::shared_ptr<EventHandler>> replacementEffects;
    std::vector<std::shared_ptr<StaticEffectHandler>> stateQueryHandlers;
	std::vector<QueueTrigger> triggers;

    std::vector<Changeset> changes;

    StepOrPhase currentPhase;
    unsigned int currentPlayer;
    unsigned int turnPlayer;

	std::vector<std::pair<std::shared_ptr<CardToken>, xg::Guid>> declaredAttacks;
	std::vector<std::pair<std::shared_ptr<CardToken>, xg::Guid>> declaredBlocks;
	std::set<xg::Guid> blocked;
	std::map<xg::Guid, std::vector<xg::Guid>> blockingOrder;

    Environment(const std::vector<Player>& players, const std::vector<std::vector<Card>>& libraries);

	int getPower(xg::Guid target)  const;
	int getPower(std::shared_ptr<const CardToken> target) const;
	int getToughness(xg::Guid target)  const;
	int getToughness(std::shared_ptr<const CardToken> target)  const;
	bool goodTiming(xg::Guid target) const;
	bool goodTiming(std::shared_ptr<const HasCost> target) const;
	std::shared_ptr<const std::set<CardSuperType>> getSuperTypes(xg::Guid target)  const;
	std::shared_ptr<const std::set<CardSuperType>> getSuperTypes(std::shared_ptr<const CardToken> target)  const;
	std::shared_ptr<const std::set<CardType>> getTypes(xg::Guid target) const;
	std::shared_ptr<const std::set<CardType>> getTypes(std::shared_ptr<const CardToken> target) const;
	std::shared_ptr<const std::set<CardSubType>> getSubTypes(xg::Guid target)  const;
	std::shared_ptr<const std::set<CardSubType>> getSubTypes(std::shared_ptr<const CardToken> target)  const;
	std::set<Color> getColors(xg::Guid target)  const;
	std::set<Color> getColors(std::shared_ptr<const CardToken> target)  const;
	xg::Guid getController(xg::Guid target) const;
	xg::Guid getController(std::shared_ptr<const Targetable> target) const;
	std::shared_ptr<const std::vector<std::shared_ptr<const ActivatedAbility>>> getActivatedAbilities(xg::Guid target) const;
	std::shared_ptr<const std::vector<std::shared_ptr<const ActivatedAbility>>> getActivatedAbilities(std::shared_ptr<const CardToken> target) const;
	unsigned int getLandPlays(xg::Guid player) const;
	std::vector<std::shared_ptr<EventHandler>> getReplacementEffects(std::shared_ptr<const HasAbilities> target, ZoneType destinationZone, std::optional<ZoneType> sourceZone = std::nullopt) const;
	std::vector<std::shared_ptr<EventHandler>> getActiveReplacementEffects() const;
	std::vector<std::shared_ptr<TriggerHandler>> getTriggerEffects(std::shared_ptr<const HasAbilities> target, ZoneType destinationZone, std::optional<ZoneType> sourceZone = std::nullopt) const;
	std::vector<std::shared_ptr<TriggerHandler>> getActiveTriggerEffects() const;
	std::vector<std::shared_ptr<StaticEffectHandler>> getStaticEffects(std::shared_ptr<const HasAbilities> target, ZoneType destinationZone, std::optional<ZoneType> sourceZone = std::nullopt) const;
	std::vector<std::shared_ptr<StaticEffectHandler>> getActiveStaticEffects() const;
	std::vector<std::shared_ptr<EventHandler>> getSelfReplacementEffects(std::shared_ptr<const HasAbilities> target, ZoneType destinationZone, std::optional<ZoneType> sourceZone = std::nullopt) const;
	bool canAttack(xg::Guid target) const;
	bool canAttack(std::shared_ptr<const CardToken> target) const;
	bool canBlock(xg::Guid target) const;
	bool canBlock(std::shared_ptr<const CardToken> target) const;
	int getLethalDamage(xg::Guid target, xg::Guid blocker) const;
	int getLethalDamage(std::shared_ptr<const CardToken> attacker, xg::Guid blocker) const;
	int getLethalDamage(std::shared_ptr<const CardToken> attacker, std::shared_ptr<const CardToken> blocker) const;
	std::vector<AttackRestrictionValue> getAttackRestrictions(xg::Guid attacker) const;
	std::vector<AttackRestrictionValue> getAttackRestrictions(std::shared_ptr<const CardToken> attacker) const;
	std::vector<AttackRequirementValue> getAttackRequirements(xg::Guid attacker) const;
	std::vector<AttackRequirementValue> getAttackRequirements(std::shared_ptr<const CardToken> attacker) const;
	std::vector<BlockRestrictionValue> getBlockRestrictions(xg::Guid blocker) const;
	std::vector<BlockRestrictionValue> getBlockRestrictions(std::shared_ptr<const CardToken> blocker) const;
	std::vector<BlockRequirementValue> getBlockRequirements(xg::Guid blocker) const;
	std::vector<BlockRequirementValue> getBlockRequirements(std::shared_ptr<const CardToken> blocker) const;

private:
	StaticEffectQuery& executeStateQuery(StaticEffectQuery&& query) const;

	void createRulesEffects();
};

#endif

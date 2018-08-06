#ifndef _STRATEGY_H_
#define _STRATEGY_H_

#include "gameAction.h"

struct Player;
struct Environment;
struct HasEffect;

class Strategy {
public:
    virtual GameAction chooseGameAction(const Player& player, const Environment& env) = 0;
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
};

class RandomStrategy : public Strategy {
public:
    virtual GameAction chooseGameAction(const Player& player, const Environment& env);
	virtual std::vector<xg::Guid> chooseTargets(std::shared_ptr<const HasEffect> effect, const Player& player, const Environment& env);
	virtual std::vector<xg::Guid> chooseDiscards(size_t amount, const Player& player, const Environment& env);
	virtual std::optional<std::pair<std::shared_ptr<CardToken>, xg::Guid>> chooseAttacker(std::vector<std::shared_ptr<CardToken>>& possibleAttackers,
																						  std::map<xg::Guid, std::set<xg::Guid>>& possibleAttacks,
																						  std::map<xg::Guid, std::multiset<xg::Guid>>& requiredAttacks,
																						  std::vector<std::pair<std::shared_ptr<CardToken>, xg::Guid>>& declaredAttacks);
	virtual std::optional<std::pair<std::shared_ptr<CardToken>, xg::Guid>> chooseBlocker(std::vector<std::shared_ptr<CardToken>>& possibleBlockers,
																						 std::map<xg::Guid, std::set<xg::Guid>>& possibleBlocks,
																						 std::map<xg::Guid, std::multiset<xg::Guid>>& requiredBlocks,
																						 std::vector<std::pair<std::shared_ptr<CardToken>, xg::Guid>>& declaredBlocks);
	virtual std::vector<xg::Guid> chooseBlockingOrder(std::shared_ptr<CardToken> attacker, std::vector<std::shared_ptr<CardToken>> blockers,
													  const Environment& env);
	virtual int chooseDamageAmount(std::shared_ptr<CardToken> attacker, xg::Guid blocker, int minDamage, int maxDamage, const Environment& env);
	virtual std::optional<Changeset> chooseUpToOne(const std::vector<Changeset>& changes, const Player& player, const Environment& env);
	virtual Changeset chooseOne(const std::vector<Changeset>& changes, const Player& player, const Environment& env);
};

#endif

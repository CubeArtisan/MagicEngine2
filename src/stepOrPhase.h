#ifndef _STEPORPHASE_H_
#define _STEPORPHASE_H_

#include "enum.h"
#include "environment.h"
#include "runner.h"

class StepOrPhase {
public:
	virtual bool applyEnter(Environment&, Runner&) const { return true; }
	virtual bool applyLeave(Environment&, Runner&) const { return true; }

	const StepOrPhaseId id;

	static std::shared_ptr<const StepOrPhase> getStepOrPhase(StepOrPhaseId desiredId) {
		return initializer.steps.at(desiredId);
	}

protected:
	StepOrPhase(StepOrPhaseId id)
		: id(id)
	{}

	struct StaticInitializer {
	public:
		const std::map<StepOrPhaseId, std::shared_ptr<const StepOrPhase>> steps;
		friend StepOrPhase;
	private:
		StaticInitializer();
	};

private:

	inline const static StaticInitializer initializer{};
};

class UntapStep : public StepOrPhase {
	bool applyEnter(Environment& env, Runner& runner) const;

	friend StepOrPhase::StaticInitializer;
private:
	UntapStep()
		: StepOrPhase(UNTAP)
	{}
};

class UpkeepStep : public StepOrPhase {
	friend StepOrPhase::StaticInitializer;
private:
	UpkeepStep()
		: StepOrPhase(UPKEEP)
	{}
};

class DrawStep : public StepOrPhase {
	bool applyEnter(Environment& env, Runner& runner) const;

	friend StepOrPhase::StaticInitializer;
private:
	DrawStep()
		: StepOrPhase(DRAW)
	{}
};

class PreCombatMainPhase : public StepOrPhase {
	bool applyEnter(Environment& env, Runner& runner) const;

	friend StepOrPhase::StaticInitializer;
private:
	PreCombatMainPhase()
		: StepOrPhase(PRECOMBATMAIN)
	{}
};

class BeginCombatStep : public StepOrPhase {
	bool applyEnter(Environment& env, Runner& runner) const;

	friend StepOrPhase::StaticInitializer;
private:
	BeginCombatStep()
		: StepOrPhase(BEGINCOMBAT)
	{}
};

// CodeReview: Combine code for restriction/requirement checking into one function
class DeclareAttackersStep : public StepOrPhase {
	bool applyEnter(Environment& env, Runner& runner) const;

	friend StepOrPhase::StaticInitializer;
private:
	DeclareAttackersStep()
		: StepOrPhase(DECLAREATTACKERS)
	{}
};

class DeclareBlockersStep :public StepOrPhase {
	bool applyEnter(Environment& env, Runner& runner) const;

	friend StepOrPhase::StaticInitializer;
private:
	DeclareBlockersStep()
		: StepOrPhase(DECLAREBLOCKERS)
	{}
};

class DamageStep : public StepOrPhase {
protected:
	bool assignDamage(Environment& env, Runner& runner, const std::vector<std::shared_ptr<CardToken>>& attackers, const std::vector<std::shared_ptr<CardToken>>& blockers) const;
	bool chooseDamageAssignment(Environment& env, const std::shared_ptr<CardToken>& aggressor, Changeset& damageEvent) const;

	DamageStep(StepOrPhaseId id)
		: StepOrPhase(id)
	{}
};

class FirstStrikeDamageStep : public DamageStep {
	bool applyEnter(Environment& env, Runner& runner) const;

	friend StepOrPhase::StaticInitializer;
private:
	FirstStrikeDamageStep()
		: DamageStep(FIRSTSTRIKEDAMAGE)
	{}
};

class CombatDamageStep :public DamageStep {
	bool applyEnter(Environment& env, Runner& runner) const;

	friend StepOrPhase::StaticInitializer;
private:
	CombatDamageStep()
		: DamageStep(COMBATDAMAGE)
	{}
};

class EndCombatStep :public StepOrPhase {
	bool applyLeave(Environment& env, Runner& runner) const;

	friend StepOrPhase::StaticInitializer;
private:
	EndCombatStep()
		: StepOrPhase(ENDCOMBAT)
	{}
};

class PostCombatMainPhase :public StepOrPhase {
	friend StepOrPhase::StaticInitializer;
private:
	PostCombatMainPhase()
		: StepOrPhase(POSTCOMBATMAIN)
	{}
};

class EndStep :public StepOrPhase {
	friend StepOrPhase::StaticInitializer;
private:
	EndStep()
		: StepOrPhase(END)
	{}
};

class CleanupStep :public StepOrPhase {
	bool applyEnter(Environment& env, Runner& runner) const;
	bool applyLeave(Environment& env, Runner& runner) const;

	friend StepOrPhase::StaticInitializer;
private:
	CleanupStep()
		: StepOrPhase(DECLAREBLOCKERS)
	{}
};
#endif

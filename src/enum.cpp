#include <map>
#include <string>
#include "enum.h"

std::ostream& operator<<(std::ostream& os, const StepOrPhase& step) {
	static std::map<StepOrPhase, std::string> StepOrPhaseMapping{
		{ UNTAP, "Untap" },{ UPKEEP, "Upkeep" },{ DRAW, "Draw" },{ PRECOMBATMAIN, "Precombat Main" },
		{ BEGINCOMBAT, "Begin Combat" },{ DECLAREATTACKERS, "Declare Attackers" },{ DECLAREBLOCKERS, "Declare Blockers" },
		{ FIRSTSTRIKEDAMAGE, "First Strike Damage" },{ COMBATDAMAGE, "Combat Damage" },{ ENDCOMBAT, "End Combat" },
		{ POSTCOMBATMAIN, "Postcombat Main" },{ END, "End" },{ CLEANUP, "Cleanup" } };
	return os << StepOrPhaseMapping[step];
}
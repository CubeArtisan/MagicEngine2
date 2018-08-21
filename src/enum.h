#ifndef _ENUM_H_
#define _ENUM_H_
#include <map>
#include <string>

enum StepOrPhase : unsigned int {
    UNTAP = 0,
    UPKEEP,
    DRAW,
    PRECOMBATMAIN,
    BEGINCOMBAT,
    DECLAREATTACKERS,
    DECLAREBLOCKERS,
    FIRSTSTRIKEDAMAGE,
    COMBATDAMAGE,
    ENDCOMBAT,
    POSTCOMBATMAIN,
    END,
    CLEANUP,
    PHASECOUNT
};

std::ostream& operator<<(std::ostream& os, const StepOrPhase& step);

enum PlayerCounterType : unsigned int {
    POISONCOUNTER,
    EXPERIENCECOUNTER,
    ENERGYCOUNTER
};

enum PermanentCounterType : unsigned int {
    PLUSONEPLUSONECOUNTER,
	MINUSONEMINUSONECOUNTER,
	LOYALTY
};

enum CardSuperType : unsigned int {
    LEGENDARY,
    BASIC,
    SNOW,
    WORLD
};

enum CardType : unsigned int {
    PERMANENTBEGIN,
    CREATURE,
    LAND,
    PLANESWALKER,
    ARTIFACT,
    ENCHANTMENT,
    PERMANENTEND,
    TRIBAL,
    INSTANT,
    SORCERY
};

enum CardSubType : unsigned int {
    FIRSTCREATURETYPE,
    DJINN,
	DRAKE,
	FUNGUS,
	GOBLIN,
    HUMAN,
	PIRATE,
	SABLE,
	SAPROLING,
	SHAPESHIFTER,
	SIREN,
	THOPTER,
	WIZARD,
    LASTCREATURETYPE,

    FIRSTLANDTYPE,
    PLAINS,
    ISLAND,
    SWAMP,
    MOUNTAIN,
    FOREST,
    DESERT,
    LASTLANDTYPE,
    
    FIRSTARTIFACTTYPE,
    VEHICLE,
    LASTARTIFACTTYPE,
    
    FIRSTENCHANTMENTTYPE,
    AURA,
    SAGA,
    LASTENCHANTMENTTYPE,
    
    FIRSTSORCERYTYPE,
    ARCANE,
    TRAP,
    LASTSORCERYTYPE,
    
    FIRSTTRIBALTYPE = FIRSTCREATURETYPE,
    LASTTRIBALTYPE = LASTCREATURETYPE,
    
    FIRSTINSTANTTYPE = FIRSTSORCERYTYPE,
    LASTINSTANTTYPE = LASTSORCERYTYPE
};

enum ZoneType {
	HAND,
	GRAVEYARD,
	LIBRARY,
	BATTLEFIELD,
	STACK,
	EXILE,
	COMMAND
};

enum MovementType {
	CAST,
	COUNTER,
	DEFAULTMOVEMENTTYPE,
	DESTROY,
	DISCARD,
	DRAWCARD,
	SACRIFICE
};
std::ostream& operator<<(std::ostream& os, const ZoneType& step);

#endif

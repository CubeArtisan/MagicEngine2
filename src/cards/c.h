#include "cardManager.h"

#include "../environment.h"
#include "../targeting.h"

#include "../propositions/proposition.h"
#include "../propositions/raid.h"
#include "../staticeffects/auraEffect.h"
#include "../triggeredeffects/lambdaTrigger.h"
#include "../triggeredeffects/combatDamageTrigger.h"

Card ChartACourse = newCard("Chart a Course", 1, {}, { SORCERY }, {},
	2, 1, 0, { BLUE }, std::shared_ptr<TargetingRestriction>(new NoTargets()),
	{ ManaCost(Mana(1, { BLUE })) }, {}, { [](xg::Guid source, const Environment& env) -> std::optional<Changeset> { return Changeset::drawCards(env.getController(source), 2, env); },
							 [](xg::Guid source, const Environment& env) -> std::optional<Changeset> {
								Changeset changes;
								if (RaidProposition(env.getController(source))(env))
									changes += Changeset::discardCards(env.getController(source), 1, env);
								return changes; } });

Card CuriousObsession = newCard("Curious Obsession", 1, {}, { ENCHANTMENT }, { AURA }, 0, 0, 0, { BLUE }, std::shared_ptr<TargetingRestriction>(new CreatureTarget()), { ManaCost(Mana({ BLUE })) },
	{}, {}, {}, {}, { std::make_shared<LambdaTriggerHandler>(AndPropositionImpl(LambdaProposition([](const Changeset& c, const Environment&)
		{ return c.phaseChange.changed && c.phaseChange.starting == POSTCOMBATMAIN; }), NotProposition(RaidProposition())),
		[](const TriggerInfo& info) { std::vector<QueueTrigger> res; Changeset cause;
									  cause.phaseChange = info.change.phaseChange;
									  res.push_back(QueueTrigger{ info.player, info.source, cause,
											std::make_shared<Ability>(LambdaEffects([](xg::Guid source, const Environment& env) -> std::optional<Changeset>
												{ Changeset res;
												  std::shared_ptr<Ability> ability = std::dynamic_pointer_cast<Ability>(env.gameObjects.at(source));
												  xg::Guid owner = ability->owner;
												  xg::Guid trueSource = getBaseClassPtr<const Targetable>(ability->source)->id;
												  res.moves.push_back(ObjectMovement{trueSource, env.battlefield->id, env.graveyards.at(owner)->id, 0, SACRIFICE});
												  return res; })) });
									  return res; }) }, { std::shared_ptr<StaticEffectHandler>(new AuraEffect(1, 1, {}, { std::make_shared<CombatDamageTrigger>(toPlayerProp, [](DamageToTarget)
															{ return std::make_shared<Ability>(LambdaEffects([](xg::Guid source, const Environment& env)
									  { return Changeset::drawCards(env.getController(source), 1, env); })); })})) });

class CManager : public LetterManager {
public:
	void getCards(std::map<std::string, Card>& cards, std::map<int, std::string>&)
	{
		insertCard(cards, ChartACourse);
		insertCard(cards, CuriousObsession);
	}
};

#pragma once
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpath/json_query.hpp>
#include <jsoncons/json_type_traits_macros.hpp>
#include <jsoncons_ext/csv/csv.hpp>
#include <fstream>
#include "Class.h"
#include "Game.h"

// ACKS Conditions were rather loose in the core rules but specified and defined in AXIOMS 6 (What's Your Condition)
// As well as granting buffs and penalties to various things, they also specify a range of capabilities that are allowed or disallowed by the condition
// As an example, a Prone creature has highly limited movement and cannot attack or perform attack-equivalent actions
// When a condition starts or ends, we check all the existing conditions to determine the capability range of the creature
// As a note - conditions apply equally to Mobs and Characters
//
// Conditions now also includes Mortal Wound and Tampering With Mortality results, as part of the refactor of Tags.

class ACKSClass;

class Condition
{
	std::string Name_;
	std::string CanTakeActions_;
	std::string CanFight_;
	std::string CanCastSpells_;
	std::string CanBeBackstabbed_;
	std::string Recovery_;

	std::vector<std::string> Includes_;

	double MoveRate_;
	int MoveLimit_;
	int ToHitMeBonus_;
	int ToHitOthersBonus_;
	int ArmorClassBonus_;
	int SurpriseBonus_;

public:
	Condition(const std::string Name, const std::vector<std::string> Includes, const std::string CanTakeActions, const std::string CanFight, const std::string CanCastSpells, const double MoveRate, const int MoveLimit,
			  const int ToHitMeBonus, const int ToHitOthersBonus, const int ArmorClassBonus, const int SurpriseBonus, const std::string CanBeBackstabbed, const std::string Recovery)
		: Name_(Name), Includes_(Includes), CanTakeActions_(CanTakeActions), CanFight_(CanFight), CanCastSpells_(CanCastSpells), CanBeBackstabbed_(CanBeBackstabbed), Recovery_(Recovery),
			MoveRate_(MoveRate), MoveLimit_(MoveLimit), ToHitMeBonus_(ToHitMeBonus), ToHitOthersBonus_(ToHitOthersBonus), ArmorClassBonus_(ArmorClassBonus), SurpriseBonus_(SurpriseBonus)
	{}
	
	const std::string Name() {
		return Name_;
	}
	const std::string CanTakeActions() {
		return CanTakeActions_;
	}
	const std::string CanFight() {
		return CanFight_;
	}
	const std::string CanCastSpells() {
		return CanCastSpells_;
	}
	const std::string CanBeBackstabbed() {
		return CanBeBackstabbed_;
	}
	const std::string Recovery() {
		return Recovery_;
	}

	const std::vector<std::string> Includes() {
		return Includes_;
	}

	const double MoveRate() { return MoveRate_; }
	const int MoveLimit() { return MoveLimit_; }
	const int ToHitMeBonus() { return ToHitMeBonus_; }
	const int ToHitOthersBonus() { return ToHitOthersBonus_; }
	const int ArmorClassBonus() { return ArmorClassBonus_; }
	const int SurpriseBonus() { return SurpriseBonus_; }
};

class ConditionData
{
	std::vector<Condition> Conditions_;

public:
	ConditionData(const std::vector<Condition>& Conditions)
		: Conditions_(Conditions)
	{}

	const std::vector<Condition>& Conditions() const { return Conditions_; }
	std::vector<Condition>& Conditions_NC() { return Conditions_; }
};

JSONCONS_ALL_GETTER_CTOR_TRAITS_DECL(Condition, Name, Includes, CanTakeActions, CanFight, CanCastSpells, MoveRate, MoveLimit, ToHitMeBonus, ToHitOthersBonus, ArmorClassBonus, SurpriseBonus, CanBeBackstabbed, Recovery)
JSONCONS_ALL_GETTER_CTOR_TRAITS_DECL(ConditionData, Conditions)

class SpecialPenalty
{
	std::string Name_;
	int Value_;
	
public:
	SpecialPenalty(const std::string Name, const int Value)
		: Name_(Name), Value_(Value)
	{}
	
	const std::string Name() {
		return Name_;
	}
	const int Value() {
		return Value_;
	}
};

JSONCONS_ALL_GETTER_CTOR_TRAITS_DECL(SpecialPenalty, Name, Value)

class MortalEffect
{
	std::string Code_;
	std::string PlayerText_;
	std::string MonsterText_;
	std::string DoubleTo_;

	std::vector<SpecialPenalty> SpecialPenalties_;

public:
	MortalEffect(const std::string Code, const std::string PlayerText, const std::string MonsterText, const std::string DoubleTo, const std::vector<SpecialPenalty> SpecialPenalties)
		: Code_(Code), PlayerText_(PlayerText), MonsterText_(MonsterText), DoubleTo_(DoubleTo), SpecialPenalties_(SpecialPenalties)
	{}

	const std::string Code() {
		return Code_;
	}

	const std::string PlayerText() {
		return PlayerText_;
	}

	const std::string MonsterText() {
		return MonsterText_;
	}

	const std::string DoubleTo() {
		return DoubleTo_;
	}

	const std::vector<SpecialPenalty> SpecialPenalties() {
		return SpecialPenalties_;
	}
};

JSONCONS_ALL_GETTER_CTOR_TRAITS_DECL(MortalEffect, Code, PlayerText, MonsterText, DoubleTo, SpecialPenalties)

class MortalWoundData
{
	std::vector<MortalEffect> MortalEffects_;
public:
	MortalWoundData(const std::vector<MortalEffect>& MortalEffects)
		: MortalEffects_(MortalEffects)
	{}

	const std::vector<MortalEffect>& MortalEffects() const { return MortalEffects_; }
	std::vector<MortalEffect>& MortalEffects_NC() { return MortalEffects_; }
};

JSONCONS_ALL_GETTER_CTOR_TRAITS_DECL(MortalWoundData, MortalEffects)

struct MortalRollResult
{
	std::string status;
	std::string recovery;
	int bedRest;
	MortalEffect* effect;
	int mortalIndex;
};

class MortalRollStore
{
	std::vector<int> min;
	std::vector<int> max;

	std::vector<std::string> status;
	std::vector<std::string> recovery;
	std::vector<int> bedRest;
	std::vector<std::vector<std::string>> results;
	
public:

	MortalRollStore()
	{

	}

	void LoadMortalRollStore();

	MortalRollResult* GetRoll(int severity, int d6);
};

class MortalWoundManager
{
	std::vector<std::string> Codes;
	std::vector<std::string> PlayerTexts;
	std::vector<std::string> MonsterTexts;
	std::vector<int> DoubleTo;

	std::vector<std::map<std::string, int>> SpecialPenalties;

	std::map<std::string, int> CodeLookup;
	
	MortalWoundData mwd;
	MortalRollStore mortalstore;

public:
	MortalWoundManager(MortalWoundData& rr) : mwd(rr)
	{}

	static MortalWoundManager* LoadMortalWoundData();

	int GetConditionIndex(std::string s) { return CodeLookup[s]; }

	std::string GetCodeFromIndex(int index) { return Codes[index]; }

	MortalEffect* GetMortalEffectFromIndex(int index) { return &mwd.MortalEffects_NC()[index]; }
	MortalEffect* GetMortalEffectFromCode(std::string s) { return &mwd.MortalEffects_NC()[CodeLookup[s]]; }

	MortalRollResult* RollMortalWound(int severity, int d6)
	{
		return mortalstore.GetRoll(severity, d6);
	}

	void DebugLog(std::string message);
};

class ConditionManager
{
	std::vector<std::string> Names;

	std::vector<bool> CanTakeActions;
	std::vector<bool> CanAttack;
	std::vector<bool> CanCastSpells;
	std::vector<bool> CanBeBackstabbed;

	std::vector<double> MoveRate;
	std::vector<int> MoveLimit;
	std::vector<int> ToHitMeBonus;
	std::vector<int> ToHitOthersBonus;
	std::vector<int> ArmorClassBonus;
	std::vector<int> SurpriseBonus;

	std::vector<std::string> Recovery;

	std::map<std::string, int> NameLookup; // reverse name to index
	std::vector<std::vector<int>> Includes;
	
	// loaded data from JSONCons
	ConditionData cd;
	
public:
	
	ConditionManager(ConditionData& rr) : cd(rr)
	{
	}

	static ConditionManager* LoadConditions();

	int GetConditionIndex(std::string s) { return NameLookup[s]; }

	std::string GetNameFromIndex(int index) { return Names[index]; }

	// example inclusion lookup:
	// Unconscious includes Helpless, Blinded and Deafened
	// Held includes Helpless
	// We are checking to see if someone is Helpless, but the character will not automatically get the Helpless condition added, so we need a secondary lookup
	// the Inclusions vec contains a list of every included condition, so we check the includes for every condition we know we have.
	bool HasCondition(std::vector<int>& conditions, int findVal);

	std::string GetRecovery(int index) { return Recovery[index]; }

	void DebugLog(std::string message);
};

#pragma once
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpath/json_query.hpp>
#include <jsoncons/json_type_traits_macros.hpp>
#include <jsoncons_ext/csv/csv.hpp>
#include <fstream>
#include "Character.h"

// ACKS Classes are rather fungible, being based upon a generation system taken from the Player's Handbook.
// I've gone down the route of creating them distinctly for this RL. However, I've attempted to keep the data structures relatively flexible
// So we can feasibly add new classes based on the PH class generation system later.
// 
// Many of our statistics are based upon Open Game Content. To ensure we keep with the terms of the OGC license, we need to store these statistics in a
// user-accessible format. We do this using JSON files (for more complex structures) and CSV files (for straightforward tables of data such as market stats),
// and we load them using jsoncons.

extern std::string saveTypes[];

class LevelledChartColumn
{
	std::string Name_;
	int Column_;

public:
	LevelledChartColumn(const std::string& Name, const int Column) : Name_(Name), Column_(Column)
	{
		
	}

	const std::string& Name() const { return Name_; }
	const int Column() const { return Column_; }
};
JSONCONS_ALL_GETTER_CTOR_TRAITS_DECL(LevelledChartColumn, Name, Column)

class LevelledAbility
{
	std::string Name_;
	int Level_;
	std::string Type_;
	int Value_;

public:
	LevelledAbility(const std::string& Name, const int Level, const std::string& Type, const int Value)
		: Name_(Name), Level_(Level), Type_(Type), Value_(Value)
	{

	}

	const std::string& Name() const { return Name_; }
	const std::string& Type() const { return Type_; }
	const int Level() const { return Level_; }
	const int Value() const { return Value_; }
};
JSONCONS_ALL_GETTER_CTOR_TRAITS_DECL(LevelledAbility, Name, Level, Type, Value)

// Changed the naming so I'm not typing "class Class" :) 
class ACKSClass
{
	std::string Name_;
	std::string LevelChart_; // filename of level chart csv
	std::string AttackProgression_;
	std::string SaveProgression_;
	std::string SpellProgression_;

	std::vector<std::string> PrimeRequisites_;

	int HitDie_;

	std::vector<std::string> ArmourProficiencies_;
	std::vector<std::string> WeaponProficiencies_;
	std::vector<std::string> FightingStyles_;

	std::vector<LevelledChartColumn> LevelledChartColumns_;
	std::vector<LevelledAbility> LevelledAbilities_;
public:
	ACKSClass(const std::string& Name, const std::string& LevelChart, const std::string& AttackProgression, const std::string& SaveProgression, const std::string& SpellProgression,
		const int HitDie, const std::vector<std::string>& PrimeRequisites, const std::vector<std::string>& ArmourProficiencies, const std::vector<std::string>& WeaponProficiencies, 
		const std::vector<std::string>& FightingStyles, const std::vector<LevelledChartColumn>& LevelledChartColumns,
		std::vector<LevelledAbility>& LevelledAbilities) :
	Name_(Name), LevelChart_(LevelChart), AttackProgression_(AttackProgression), SaveProgression_(SaveProgression), SpellProgression_(SpellProgression), HitDie_(HitDie),
	PrimeRequisites_(PrimeRequisites), ArmourProficiencies_(ArmourProficiencies), WeaponProficiencies_(WeaponProficiencies), FightingStyles_(FightingStyles),
	LevelledChartColumns_(LevelledChartColumns), LevelledAbilities_(LevelledAbilities)
	{}

	const std::string Name() const { return Name_; }
	const std::string LevelChart() const { return LevelChart_; }
	const std::string AttackProgression() const { return AttackProgression_; }
	const std::string SaveProgression() const { return SaveProgression_; }
	const std::string SpellProgression() const { return SpellProgression_; }

	const int HitDie() const { return HitDie_;  }

	const std::vector<std::string> PrimeRequisites() const { return PrimeRequisites_; }

	const std::vector<std::string> ArmourProficiencies() const { return ArmourProficiencies_; }
	const std::vector<std::string> WeaponProficiencies() const { return WeaponProficiencies_; }
	const std::vector<std::string> FightingStyles() const { return FightingStyles_; }

	const std::vector<LevelledChartColumn> LevelledChartColumns() const { return LevelledChartColumns_;  }
	const std::vector<LevelledAbility> LevelledAbilities() const { return LevelledAbilities_; }

	std::vector<int> LevelXPValues;
	std::vector<std::string> LevelTitles;
	std::vector<int> LevelHitDie;
	std::vector<int> LevelHitBonus;

	std::vector<int> PrimeReqIdx;
	
	std::map<std::string, std::vector<int>> LevelTagBonuses; // compiled list of tags per level
};
JSONCONS_ALL_GETTER_CTOR_TRAITS_DECL(ACKSClass, Name, LevelChart, AttackProgression, SaveProgression, SpellProgression, HitDie, PrimeRequisites, ArmourProficiencies, WeaponProficiencies, FightingStyles, LevelledChartColumns, LevelledAbilities)

class ClassSet
{
	std::vector<ACKSClass> Classes_;

public:
	ClassSet(const std::vector<ACKSClass>& Classes) : Classes_(Classes)
	{}

	const std::vector<ACKSClass>& Classes() const { return Classes_; }
};
JSONCONS_ALL_GETTER_CTOR_TRAITS_DECL(ClassSet, Classes)

class AdvancementStore
{
	
public:
	
	std::map<std::string, std::map<std::string, std::vector<int>>> SaveLookup; // SaveType, then Class, then level
	std::map<std::string, std::vector<int>> AttackBonusLookup; // Class, then Level

	AdvancementStore()
	{
		
	}

	void LoadAdvancementSets();
};

class ClassManager
{
	ClassSet classes;

	std::map<const std::string, int> classLookup;

	AdvancementStore* advancementStore;

public:
	ClassManager(ClassSet& _classes) : classes(_classes)
	{
	}

	static ClassManager* LoadClasses();

	ClassSet& Classes() { return classes;  }

	int GetClassIndex(std::string className);

	AdvancementStore* GetAdvancementStore()
	{
		return advancementStore;
	}
};
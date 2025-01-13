#pragma once
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpath/json_query.hpp>
#include <jsoncons/json_type_traits_macros.hpp>
#include <jsoncons_ext/csv/csv.hpp>
#include <fstream>
#include "ItemTemplate.h"
#include "OutputLog.h"

// "Mobs" refers to creatures (in creatures.json) and to "mob" used as the generic group noun for creature groups (in mobs.json).
// Creature groups will be in here but are currently unimplemented

enum MobBehaviour
{
	MOB_BEHAVIOUR_UNSET = -1,
	MOB_BEHAVIOUR_IDLE,
	MOB_BEHAVIOUR_WANDER,
	MOB_BEHAVIOUR_FOLLOW,
	MOB_BEHAVIOUR_SEEK_ENEMY,
	MOB_BEHAVIOUR_SEEK_PLAYER,
	MOB_BEHAVIOUR_SEEK_ITEM,
	MOB_BEHAVIOUR_RANGE_AT,
	MOB_BEHAVIOUR_FLEE,
	MOB_BEHAVIOUR_UNCONSCIOUS,
	MOB_BEHAVIOUR_MAX
};

const std::string MobBehaviourNames[] = {"Idle", "Wander", "Follow", "SeekEnemy", "SeekPlayer", "SeekItem", "Range", "Flee", "Unconscious"};

// Attacks are described followed by a set of attack chain options taken from these attack types
// Special attack effects (petrify etc) will go here later
class AttackType
{
	std::string Name_;
	int DamageDie_;
	int DamageBonus_;

public:
	AttackType(const std::string& Name, const int DamageDie, const int DamageBonus ) : Name_(Name), DamageDie_(DamageDie), DamageBonus_(DamageBonus)
	{
		
	}
	AttackType() {}

	const std::string& Name() const { return Name_; }
	const int DamageDie() const { return DamageDie_; }
	const int DamageBonus() const { return DamageBonus_; }
};
JSONCONS_ALL_GETTER_CTOR_TRAITS_DECL(AttackType, Name, DamageDie, DamageBonus)

// Some creatures (especially humanoids and beastmen) have equipment rather than natural weapons/armour etc
class EquipmentType
{
	std::vector<std::string> Always_;
	std::vector<std::vector<std::string>> OneOf_;

public:
	EquipmentType() {}
	EquipmentType(const std::vector<std::string>& Always, const std::vector<std::vector<std::string>>& OneOf) : Always_(Always), OneOf_(OneOf)
	{
		
	}

	const std::vector<std::string> Always() const { return Always_; }
	const std::vector<std::vector<std::string>> OneOf() const { return OneOf_; }
};
JSONCONS_ALL_GETTER_CTOR_TRAITS_DECL(EquipmentType, Always, OneOf)


class CreatureTemplate
{
	std::string Name_;
	std::string Visual_;
	std::string SaveAs_;
	std::string Alignment_;
	
	int HitDie_;
	int HitDieModifier_;
	int ArmourClass_;
	int Morale_;
	int Movement_;
	int XP_;

	double LairProbability_;

	std::vector<AttackType> Attacks_;
	std::vector<std::vector<std::string>> AttackSequences_;

	std::vector<std::string> Abilities_;
	std::vector<std::string> Behaviours_;

	EquipmentType EquipmentSelection_;
	
public:
	CreatureTemplate(const std::string& Name, const std::string& Visual, const std::string& SaveAs, const std::string& Alignment, 
		const int HitDie, const int HitDieModifier, const int ArmourClass, const int Morale, const int Movement, const int XP, const double LairProbability,
		const std::vector<AttackType>& Attacks, const std::vector<std::string>& Abilities, const std::vector<std::string>& Behaviours, const std::vector<std::vector<std::string>>& AttackSequences, const EquipmentType& EquipmentSelection) :
	Name_(Name), Visual_(Visual), SaveAs_(SaveAs), Alignment_(Alignment), HitDie_(HitDie), HitDieModifier_(HitDieModifier), ArmourClass_(ArmourClass), Morale_(Morale),
	Movement_(Movement), XP_(XP), LairProbability_(LairProbability), Attacks_(Attacks), Abilities_(Abilities), Behaviours_(Behaviours), AttackSequences_(AttackSequences), EquipmentSelection_(EquipmentSelection)
	{}

	const std::string Name() const { return Name_; }
	const std::string Visual() const { return Visual_; }
	const std::string SaveAs() const { return SaveAs_; }
	const std::string Alignment() const { return Alignment_; }

	const int HitDie() const { return HitDie_;  }
	const int HitDieModifier() const { return HitDieModifier_; }
	const int ArmourClass() const { return ArmourClass_; }
	const int Morale() const { return Morale_; }
	const int Movement() const { return Movement_; }
	const int XP() const { return XP_; }

	const double LairProbability() const { return LairProbability_; }

	const std::vector<AttackType> Attacks() const { return Attacks_; }
	const std::vector<std::string> Abilities() const { return Abilities_; }
	const std::vector<std::string> Behaviours() const { return Behaviours_; }
	const std::vector<std::vector<std::string>> AttackSequences() const { return AttackSequences_; }
	const EquipmentType& EquipmentSelection() const { return EquipmentSelection_; }
};
JSONCONS_ALL_GETTER_CTOR_TRAITS_DECL(CreatureTemplate, Name, Visual, SaveAs, Alignment, HitDie, HitDieModifier, ArmourClass, Morale, Movement, XP, LairProbability, Attacks, Abilities, Behaviours, AttackSequences, EquipmentSelection)

class CreatureSet
{
	std::vector<CreatureTemplate> CreatureTemplates_;

public:
	CreatureSet(const std::vector<CreatureTemplate>& CreatureTemplates) : CreatureTemplates_(CreatureTemplates)
	{}

	const std::vector<CreatureTemplate>& CreatureTemplates() const { return CreatureTemplates_; }
};
JSONCONS_ALL_GETTER_CTOR_TRAITS_DECL(CreatureSet, CreatureTemplates)

class Creature
{
	std::string Name_;		// the name of this particular monster (in this version and for most monsters generally, this is just a copy of the monster name)
	std::string Visual_;
	std::string SaveAs_;
	std::string Alignment_;

	int HitDie_;			// number of HD in this case, not the die type (that's always a d8)
	int HitPoints_;
	int ArmourClass_;
	int Morale_;
	int Movement_;
	int XP_;

	int CreatureType_;	// use this to reference things like the creature name

	std::map<std::string, AttackType> Attacks_;	// we feed this out into a lookup table
	std::vector<std::vector<std::string>> AttackSequences_;

	std::vector<std::string> Abilities_;
	std::vector<int> Behaviours_;

	std::vector<int> ItemsEquipped; // indexed as per character equipment, not including armour
	// Why do we not track armour? Because Monsters don't (usually) have stats.
	// As a result, the AC is listed directly - including AC bonuses from stats and any extras from the creature's nature.
	std::vector<int> Held; // just a vector list of items. This is what is dropped when the creature is killed.

	std::vector<int> Conditions; // WHAT DO WE GOT YO
	
	bool hostile;

public:
	Creature() { }

	std::string GetName() { return Name_; }
	std::string GetVisual() { return Visual_; }
	std::string GetSaveAs() { return SaveAs_; }
	std::string GetAlignment() { return Alignment_; }

	void SetName(std::string in) { Name_ = in; }
	void SetVisual(std::string in) { Visual_ = in; }
	void SetSaveAs(std::string in) { SaveAs_ = in; }
	void SetAlignment(std::string in) { Alignment_ = in; }

	int GetHitDie() { return HitDie_; }
	int GetHitPoints() { return HitPoints_; }
	int GetArmourClass() { return ArmourClass_; }
	int GetMorale() { return Morale_; }
	int GetMovement() { return Movement_; }
	int GetXP() { return XP_; }
	int GetCreatureType() { return CreatureType_; }

	void SetHitDie(int in) { HitDie_ = in; }
	void SetHitPoints(int in) { HitPoints_ = in; }
	void SetArmourClass(int in) { ArmourClass_ = in; }
	void SetMorale(int in) { Morale_ = in; }
	void SetMovement(int in) { Movement_ = in; }
	void SetXP(int in) { XP_ = in; }

	AttackType GetAttack(std::string in) { return Attacks_[in]; }
	std::vector<std::string>& GetAbilities() { return Abilities_; }
	std::vector<int>& GetBehaviours() { return Behaviours_; }
	std::vector<std::vector<std::string>>& GetAttackSequences() { return AttackSequences_; }

	int GetEquippedInSlot(int slot) { return ItemsEquipped[slot]; }

	void AddAttack(std::string name, int damageDie, int damageBonus);

	//static Creature GenerateCreature(const CreatureTemplate& ct);
	static Creature GenerateCreature(int templateIndex);

	bool IsHostile() { return hostile; }
	void SetHostile(bool isHostile) { hostile = isHostile; }

	int GetCleaveCount() { return 0; } // for testing

	int SetCondition(int condition);
	int SetCondition(std::string condition);
	int RemoveCondition(int condition);
	int RemoveCondition(std::string condition);

	bool HasCondition(int condition) { return std::find(Conditions.begin(), Conditions.end(), condition) != Conditions.end(); }
	bool HasCondition(std::string condition);

	std::vector<int> GetConditions() { return Conditions; }

	bool HasAbility(std::string ability)
	{
		return std::find(Abilities_.begin(), Abilities_.end(), ability) != Abilities_.end();
	}

	bool IsBlocking();
};


class MobManager
{
	CreatureSet creatureTemplateSet;
	std::map<const std::string, int> creatureNameLookup;
	std::vector<Creature> Monsters_;
	std::vector<int> mapIDs;

	std::vector<int> targetID;
	std::vector<int> targetManager;

	std::vector<TCODPath*> paths;

	std::vector<int> mobXPos;
	std::vector<int> mobYPos;
	
	std::vector<int> currentBehaviour; // -1 means no behaviour currently set

	int nextMonsterIndex = 1; // starting at 1 as maps init to 0

	
public:
	MobManager(CreatureSet& templates) : creatureTemplateSet(templates)
	{
		// fill in the lookup tables
		// we skip -1, as thats MOB_BEHAVIOUR_UNSET
		for(int i=0;i<MOB_BEHAVIOUR_MAX;i++)
		{
			behaviourLookup[MobBehaviourNames[i]] = i;
		}
		Creature c;
		EmptyCreature(c);
	}

	void EmptyCreature(Creature c);

	CreatureSet& CreatureTemplates() { return creatureTemplateSet; }

	int GetTemplateIndex(std::string templateName);

	Creature& GetMonster(int monsterID) { return Monsters_[monsterID]; }

	double MoveTo(int entityID, int new_x, int new_y, int currentTime);

	void SpawnOnMap(int entityID, int mapID, int new_x, int new_y);

	void SetMobX(int entityID, int x);
	void SetMobY(int entityID, int y);

	int GetMobX(int entityID);
	int GetMobY(int entityID);
	
	// factory
	static MobManager* LoadMobData();

	// utility generators
	int GenerateMonster(std::string templateName, int mapID, int x, int y, bool hostile = true) { return GenerateMonster(GetTemplateIndex(templateName),mapID,x,y,hostile); }
	int GenerateMonster(int templateIndex, int mapID, int x, int y, bool hostile = true);

	std::map<std::string, int> behaviourLookup;

	int SelectBehaviour(int entityID);
	void SetBehaviour(int entityID, int behaviourType);
	void SetBehaviour(int entityID, std::string behaviour) { SetBehaviour(entityID, behaviourLookup[behaviour]); }

	// accessors
	std::vector<int> GetAllMonstersOnMap(int mapID, bool hostileOnly = false, bool liveOnly = true);
	std::vector<int> GetAllEnemyMonstersOnMap(int mapID, bool partyId, bool liveOnly);
	std::vector<int> GetAllMonstersInRange(int mapID, int centerX, int centerY, int range, bool hostileOnly = false, bool liveOnly = true);

	// dump
	void DumpMob(int mobID);
	std::string DumpConditions(int mobID);
	
	void DebugLog(std::string message);

	// handlers
	bool TurnHandler(int entityID, double time);
	bool TargetHandler(int entityID, int returnCode); // disambiguation: targeting system in the UI, not our pathing target
	bool TimeHandler(int rounds, int turns, int hours, int days, int weeks, int months);
};
#pragma once
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpath/json_query.hpp>
#include <jsoncons/json_type_traits_macros.hpp>
#include <jsoncons_ext/csv/csv.hpp>
#include <fstream>
#include "Class.h"
#include "Game.h"
#include "Conditions.h"

// Characters in ACKS are defined by a wide variety of values, but a few of them are absolutely universal.
// The universal ones include Hit Points, Hit Dice (which is determined in a variety of ways - level for levelled PCs/NPCs, or HD for monsters),
// AC (determined by equipment for PCs/NPCs, or just set for monsters), attack rolls and saving throws (based on class), and experience/level
//
// Many of our statistics are based upon Open Game Content. To ensure we keep with the terms of the OGC license, we need to store these statistics in a
// user-accessible format. We do this using JSON files (for more complex structures) and CSV files (for straightforward tables of data such as market stats),
// and we load them using jsoncons.

class ACKSClass;
class MortalEffect;

class Statistic
{
	std::string name_;
	std::vector<std::string> bonusTo_;
public:
	Statistic(const std::string& name,
		const std::vector<std::string>& bonusTo)
		: name_(name), bonusTo_(bonusTo)
	{
	}

	const std::string& name() const { return name_; }
	const std::vector<std::string>& bonusTo() const { return bonusTo_; }
};

class CharacteristicData
{
	std::vector<Statistic> statistics_;

public:
	CharacteristicData(const std::vector<Statistic>& statistics)
		: statistics_(statistics)
	{

	}

	const std::vector<Statistic>& statistics() const { return statistics_; }
	std::vector<Statistic>& statistics_NC() { return statistics_; }
};

JSONCONS_ALL_GETTER_CTOR_TRAITS_DECL(Statistic, name, bonusTo)
JSONCONS_ALL_GETTER_CTOR_TRAITS_DECL(CharacteristicData, statistics)

// *****************************************************************************
// CHARACTER INFORMATION
// This section is the part where we store the OGL info loaded from the CSV and JSON files.
// All characters are compared to this part.
// *****************************************************************************

// Characters don't work quite the same way as mobs when it comes to behaviour.
// PCs will need quite complicated behaviours to manage class elements, while Henchmen should mostly be defined by their Role.
// Both of these will be defined in the Party manager, so we will reference back to that to determine which behaviour we're going to enact
enum CharBehaviour
{
	CHAR_BEHAVIOUR_UNSET = -1,
	CHAR_BEHAVIOUR_IDLE,
	CHAR_BEHAVIOUR_WANDER,
	CHAR_BEHAVIOUR_FOLLOW,
	CHAR_BEHAVIOUR_SEEK_ENEMY,
	CHAR_BEHAVIOUR_LOOT,
	CHAR_BEHAVIOUR_RANGE_AT,
	CHAR_BEHAVIOUR_FLEE,
	CHAR_BEHAVIOUR_UNCONSCIOUS,
	CHAR_BEHAVIOUR_MAX
};


const std::string CharBehaviourNames[] = { "Idle", "Wander", "Follow", "SeekEnemy", "SeekItem", "Range", "Flee", "Unconscious" };

// conditions are not a bitfield, you can have more than one at a time.
// These conditions are based on those in AXIOMS 6, although not all are relevant to the RL version
// We'll expand this as we add more spells etc
enum ConditionFlags
{
	CONDITION_SLUMBERING,
	CONDITION_CONCENTRATING,
	CONDITION_PRONE,
	CONDITION_UNCONSCIOUS,
	CONDITION_CHARGING,
	CONDITION_ENGAGED,
	CONDITION_STUNNED,
	CONDITION_VULNERABLE,
	CONDITION_HELPLESS,
	CONDITION_MAX
};

// capabilities are a bitfield, because you'll usually have a bunch of them
enum CapabilityFlags : unsigned long long
{
	CAPABILITY_VISION_NORMAL			= 1ULL << 0,			// normal sight. Turns off if unconscious/blind/whatever
	CAPABILITY_VISION_INFRA				= 1ULL << 1,			// darkvision.
	CAPABILITY_HEARING_NORMAL			= 1ULL << 2,			// normal hearing. Turns off if unconscious/deaf/whatever
	CAPABILITY_HEARING_ULTRA			= 1ULL << 3,			// ultrasound hearing/ 
	CAPABILITY_SMELL					= 1ULL << 4,			// normal smelling. Turns off if unconscious/whatever
	CAPABILITY_AWARENESS_NORMAL			= 1ULL << 5,			// normal "sensorium". If turned off, completely unaware of any events.
	CAPABILITY_COGNITION				= 1ULL << 6,			// normal temporal awareness. Turns off when time skips forward
	CAPABILITY_HALF_MOVEMENT			= 1ULL << 7,			// half move or action in lieu of move
	CAPABILITY_FULL_MOVEMENT			= 1ULL << 8,			// can move around
	CAPABILITY_FORCED_MARCH				= 1ULL << 9,
	CAPABILITY_DEFENCE					= 1ULL << 10,			// can defend self
	CAPABILITY_SPELLCASTING				= 1ULL << 11,			// includes scrolls and spell-like abilities
	CAPABILITY_ATTACK					= 1ULL << 12,			// attack or action in lieu of attack
	CAPABILITY_USE_ITEM					= 1ULL << 13,
	CAPABILITY_ACTION					= 1ULL << 14,			// can perform action in lieu of attack or move
	CAPABILITY_SPEECH					= 1ULL << 15,
	CAPABILITY_TURN_UNDEAD				= 1ULL << 16,
	CAPABILITY_CLIMB					= 1ULL << 17,
	CAPABILITY_OPENLOCK					= 1ULL << 18,
	CAPABILITY_DETECT_TRAP				= 1ULL << 19,
	CAPABILITY_REMOVE_TRAP				= 1ULL << 20,
	CAPABILITY_HIDE_IN_SHADOWS			= 1ULL << 21,
	CAPABILITY_MOVE_SILENTLY			= 1ULL << 22,
	CAPABILITY_PICK_POCKETS				= 1ULL << 23,
	CAPABILITY_BACKSTAB					= 1ULL << 24,
	CAPABILITY_USE_TWO_HANDED			= 1ULL << 25,
	CAPABILITY_USE_SHIELD				= 1ULL << 26,
	CAPABILITY_DUAL_WIELD				= 1ULL << 27,
	CAPABILITY_MAGIC_ARCANE				= 1ULL << 28,
	CAPABILITY_MAGIC_DIVINE				= 1ULL << 29,
	CAPABILITY_MAGIC_RESEARCH			= 1ULL << 30,
	CAPABILITY_MAGIC_RITUAL				= 1ULL << 31,
	CAPABILITY_USE_SCROLL_ARCANE		= 1ULL << 32,
	CAPABILITY_USE_SCROLL_DIVINE		= 1ULL << 33,
	CAPABILITY_MAGIC_CRAFT_SCROLL		= 1ULL << 34,
	CAPABILITY_MAGIC_CRAFT_POTION		= 1ULL << 35,
	CAPABILITY_MAGIC_CRAFT_RING			= 1ULL << 36,
	CAPABILITY_MAGIC_CRAFT_STAFF		= 1ULL << 37,
	CAPABILITY_MAGIC_CRAFT_WEAPON		= 1ULL << 38,
	CAPABILITY_MAGIC_CRAFT_ARMOUR		= 1ULL << 39,
	CAPABILITY_MAGIC_CRAFT_CONSTRUCT	= 1ULL << 40,
	CAPABILITY_MAGIC_CRAFT_CROSSBREED	= 1ULL << 41,
	CAPABILITY_MAGIC_CRAFT_UNDEAD		= 1ULL << 42,
	CAPABILITY_TREAT_WOUNDS				= 1ULL << 43,
	CAPABILITY_NATURAL_HEALING			= 1ULL << 44,
	CAPABILITY_MAX						= 45
};

const std::string CapabilityNames[] = {
	"Vision", "VisionInfra", "HearingNormal", "HearingUltra", "Smell", "Awareness", "Cognition", "HalfMove", "Move", "ForcedMarch", "Defence", "Magic", "Attack", "UseItem",
	"Action", "Speech", "TurnUndead", "Climb", "OpenLock", "DetectTrap", "RemoveTrap", "HideInShadows", "MoveSilently", "PickPockets", "Backstab", "UseTwoHanded", "UseShield",
	"DualWield", "Magic:Arcane", "Magic:Divine", "Magic:Research", "Magic:Ritual", "UseScroll:Arcane", "UseScroll:Divine", "MagicCraft:Scroll", "MagicCraft:Potion", "MagicCraft:Ring",
	"MagicCraft:Staff", "MagicCraft:Weapon", "MagicCraft:Armour", "MagicCraft:Construct", "MagicCraft:Crossbreed", "MagicCraft:Undead", "TreatWounds", "NaturalHealing"};

class CharacterManager
{
	std::map<std::string, int> CharacteristicTags;

	std::vector<int> AbilityBonuses;
	std::vector<float> PrimeReqMultiplier; // the multiplier on bonus XP for prime requisites
	std::map<std::string, int> StatisticLookup;
	
	/***/

	int nextCharacterIndex = 0;
	// each Character ID has a vector of Characteristics, numbered based on the loaded data
	std::vector<std::vector<int>> pcCharacteristics;
	std::vector<int> pcClass;
	std::vector<int> pcTotalHitPoints;
	std::vector<int> pcCurrentHitPoints;
	std::vector<int> pcLevel;
	std::vector<int> pcExperience;
	std::vector<int> pcCurrentArmourClass;

	std::vector<std::map<std::string, int>> pcCollectedTags;

	std::vector<std::string> pcName;

	// cached proficiencies (from class and proficiency picks)
	std::vector<std::vector<std::string>> pcWeaponProficiencies;
	std::vector<std::vector<std::string>> pcArmourProficiencies;
	std::vector<std::vector<std::string>> pcFightingStyles;

	std::vector<std::list<int>> pcInventory;
	std::vector<std::vector<int>> pcEquipped;

	std::vector<int> pcXPos;
	std::vector<int> pcYPos;

	std::vector<int> pcCurrentBehaviour; // -1 means no behaviour currently set

	std::vector<int> pcMapID;
	std::vector<int> pcRemainingCleaves;

	std::vector<std::set<std::string>> pcTravelModes;
	std::vector<std::string> pcDomainAction;

	std::vector<unsigned long long> pcCapabilityFlags;

	std::vector<std::vector<std::pair<int, int>>> pcConditions;
	std::vector<std::vector<MortalEffect*>> pcMortalWounds;

	// LOADED DATA from Jsons

	CharacteristicData cd;

	int EquipWeapon(int characterID, int itemID);
	int EquipShield(int characterID, int itemID);
	int EquipArmour(int characterID, int itemID);

	int GetItemIndex(int characterID, int inventoryID);

	void UpdateProficiencyCache(int characterID);
	void UpdateCapabilities(int characterID);

	int GetEncumbranceClass(int characterID);

public:
	CharacterManager(CharacteristicData& _cd) : cd(_cd)
	{
		for (int i = 0; i < CHAR_BEHAVIOUR_MAX; i++)
		{
			behaviourLookup[CharBehaviourNames[i]] = i;
		}

		for (unsigned long long i=0; i < CAPABILITY_MAX; i++)
		{
			CapabilityLookup[CapabilityNames[i]] = 1ULL << i;
		}
	}
	
	// factory
	static CharacterManager* LoadCharacteristics();

	// accessors
	std::string getCharacterName(int id) { return pcName[id]; }
	const ACKSClass* getCharacterClass(int id);

	int getCharacterTotalHitPoints(int id) { return pcTotalHitPoints[id]; }
	int getCharacterCurrentHitPoints(int id) { return pcCurrentHitPoints[id]; }
	int getCharacterCurrentArmourClass(int id) { return pcCurrentArmourClass[id]; }
	int getCharacterLevel(int id) { return pcLevel[id]; }
	int getCharacterExperience(int id) { return pcExperience[id]; }

	int getCharacterCharacteristic(int id, int characteristicIndex) { return pcCharacteristics[id][characteristicIndex]; }
	int getCharacterAbilityBonus(int id, int characteristicIndex) { return AbilityBonus(getCharacterCharacteristic(id, characteristicIndex)); }

	int getCharacterCharacteristic(int id, std::string characteristic) { return getCharacterCharacteristic(id, GetStatisticIndex(characteristic)); }
	int getCharacterAbilityBonus(int id, std::string characteristic) { return getCharacterAbilityBonus(id, GetStatisticIndex(characteristic)); }

	bool getCharacterCapabilityFlag(int id, CapabilityFlags capability) { return pcCapabilityFlags[id] & capability; }
	bool getCharacterCapabilityFlag(int id, std::string capabilityName)
	{
		unsigned long long c = CapabilityLookup[capabilityName];
		return getCharacterCapabilityFlag(id, (CapabilityFlags)c);
	}

	std::string getCharacterDomainAction(int id);
	void setCharacterDomainAction(int id, std::string action);

	bool getCharacterTravelModeSet(int id, std::string mode);
	void setCharacterTravelMode(int id, std::string mode);
	void unsetCharacterTravelMode(int id, std::string mode);
	void toggleCharacterTravelMode(int id, std::string mode);

	bool getCharacterHasCondition(int id, int condition);
	bool getCharacterHasCondition(int id, std::string condition);

	void setCharacterCurrentHitPoints(int id, int value) { pcCurrentHitPoints[id] = value; }

	// utilities
	int GetAbilityIndexForTag(std::string tag);
	int AbilityBonus(int characteristicValue);
	int GetStatisticIndex(std::string name)
	{
		return StatisticLookup[name];
	}

	void UpdateTagCache(int characterID);
	int getTagValue(int characterID, std::string tag);

	// Generators
	void BaseGenerate();
	int GenerateTestCharacter(std::string name, const std::string _class);
	int GenerateFighter(std::string name);
	int GenerateNormalMan(std::string name);
	
	unsigned long long GenerateBaseCapabilityFlags();

	// Removers
	void DeactivateCharacter(int characterID);

	// Item Handling
	int EquipItem(int characterID, int inventoryID);
	void UnequipItem(int characterID, int inventoryID);

	int GetItemInEquipSlot(int characterID, int slot)
	{
		return pcEquipped[characterID][slot];
	}

	int GetEquipSlotForInventoryItem(int characterID, int inventoryID);

	bool CanUseItem(int characterID, int itemID);
	bool CanUseStyle(int characterID, std::string style);

	int AddInventoryItem(int characterID, int itemID);				// returns inventory index
	int RemoveInventoryItem(int characterID, int inventoryID);		// returns item ID


	std::list<int> GetInventory(int characterID)
	{
		return pcInventory[characterID];
	}

	int UpdateCurrentAttackValue(int characterID, bool missile);
	int UpdateCurrentSaveValue(int characterID, std::string save);

	// update and return current value
	int GetCurrentSpeed(int characterID);
	double GetCurrentEncumbrance(int characterID);				// returns current load in stones
	std::string GetCurrentEncumbranceType(int characterID);
	int GetCurrentAC(int characterID);

	int GetCurrentDamageBonus(int characterID, bool missile);

	double MoveTo(int entityID, int new_x, int new_y, int currentTime);
	int GetPlayerX(int characterID) { return pcXPos[characterID]; }
	int GetPlayerY(int characterID) { return pcYPos[characterID]; }
	int GetPlayerMap(int characterID) { return pcMapID[characterID]; }
	void SetPlayerX(int characterID, int xpos) { pcXPos[characterID] = xpos; }
	void SetPlayerY(int characterID, int ypos) { pcYPos[characterID] = ypos; }
	void SetPlayerMap(int characterID, int map) { pcMapID[characterID] = map; }

	void SpawnOnMap(int entityID, int mapID, int spawn_x, int spawn_y);

	int GetCleaveCount(int characterID);

	int SetCondition(int id, int condition, int time);
	int RemoveCondition(int id, int condition);
	int ReduceCondition(int id, int condition, int timeToReduce);

	int SetCondition(int id, std::string condition,int time);
	int RemoveCondition(int id, std::string condition);
	int ReduceCondition(int id, std::string condition, int timeToReduce);

	std::vector<MortalEffect*>  GetMortalEffects(int id);
	void AddMortalEffect(int id, MortalEffect* effect);

	std::map<std::string, int> behaviourLookup;
	std::map<std::string, unsigned long long> CapabilityLookup;

	int SelectBehaviour(int entityID);
	void SetBehaviour(int entityID, int behaviourType);
	void SetBehaviour(int entityID, std::string behaviour) { SetBehaviour(entityID, behaviourLookup[behaviour]); }

	void DumpCharacter(int characterID);
	
	std::string DumpProficiencyCache(int characterID);
	std::string DumpTagCache(int characterID);
	std::string DumpConditions(int characterID);
	std::string DumpCapabilities(int characterID);

	void DebugLog(std::string message);

	std::vector<int> GetCharactersOnMap(int mapID);
	std::vector<int> GetTaggedCharactersOnMap(int mapID, std::string tag, bool value);
	std::vector<int> GetConditionCharactersOnMap(int mapID, std::string condition, bool value);

	// system handlers
	bool TurnHandler(int entityID, double time);
	bool TargetHandler(int entityID, int returnCode);
	bool TimeHandler(int rounds, int turns, int hours, int days, int weeks, int months);
};
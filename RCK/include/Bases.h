#pragma once
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpath/json_query.hpp>
#include <jsoncons/json_type_traits_macros.hpp>
#include <jsoncons_ext/csv/csv.hpp>
#include <fstream>
#include "Class.h"
#include "Character.h"
#include "Game.h"

class PurchasableSlot
{
	std::string Name_;
	int Cost_;

public:
	PurchasableSlot(const std::string& Name, const int Cost) : Name_(Name), Cost_(Cost)
	{

	}

	const std::string& Name() const { return Name_; }
	const int Cost() const { return Cost_; }
};
JSONCONS_ALL_GETTER_CTOR_TRAITS_DECL(PurchasableSlot, Name, Cost)

class BaseTag
{
	std::string Tag_;
	std::string Type_;
	std::string Indicator_;
	std::string MenuText_;
	std::vector<std::string> Requires_;
	std::vector<std::string> Excludes_;

public:
	BaseTag(const std::string& Tag, const std::string& Type, const std::string& Indicator, const std::string& MenuText, const std::vector<std::string>& Requires, const std::vector<std::string>& Excludes) :
	Tag_(Tag), Type_(Type), Indicator_(Indicator), MenuText_(MenuText), Requires_(Requires),Excludes_(Excludes)
	{
		
	}

	const std::string& Tag() const { return Tag_; }
	const std::string& Type() const { return Type_; }
	const std::string& Indicator() const { return Indicator_; }
	const std::string& MenuText() const { return MenuText_; }
	const std::vector<std::string>& Requires() const { return Requires_; }
	const std::vector<std::string>& Excludes() const { return Excludes_; }
};
JSONCONS_ALL_GETTER_CTOR_TRAITS_DECL(BaseTag, Tag, Type, Indicator, MenuText, Requires, Excludes)

class BaseType
{
	std::string Name_;
	std::string Buildable_;
	std::vector<std::string> Upkeep_;
	std::vector<std::string> Core_;
	std::vector<std::string> Options_;
	std::vector<PurchasableSlot> Purchasable_;

	
public:
	BaseType(const std::string& Name, const std::string& Buildable, const std::vector<std::string>& Upkeep, const std::vector<std::string>& Core, const std::vector<std::string>& Options,
		std::vector<PurchasableSlot>& Purchasable) :
		Name_(Name), Buildable_(Buildable), Upkeep_(Upkeep), Core_(Core), Options_(Options), Purchasable_(Purchasable)
	{}

	const std::string Name() const { return Name_; }
	const std::string Buildable() const { return Buildable_; }
	const std::vector<std::string> Upkeep() const { return Upkeep_; }
	const std::vector<std::string> Core() const { return Core_; }
	const std::vector<std::string> Options() const { return Options_; }

	const std::vector<PurchasableSlot> Purchasable() const { return Purchasable_; }
};
JSONCONS_ALL_GETTER_CTOR_TRAITS_DECL(BaseType, Name, Buildable, Upkeep, Core, Options, Purchasable)


class BaseInfoSet
{
	std::vector<BaseType> BaseTypes_;
	std::vector<BaseTag> Tags_;

public:
	BaseInfoSet(const std::vector<BaseType>& BaseTypes, const std::vector<BaseTag>& Tags) : BaseTypes_(BaseTypes), Tags_(Tags)
	{}

	const std::vector<BaseType>& BaseTypes() const { return BaseTypes_; }
	const std::vector<BaseTag>& Tags() const { return Tags_; }
};
JSONCONS_ALL_GETTER_CTOR_TRAITS_DECL(BaseInfoSet, BaseTypes, Tags)

// screens are in fact handled in the tags. Each one has 3 sections: Base, Party and Manipulator.

enum BaseMenuPanes
{
	PANE_BASE_CHARACTERS = 0,	   // base party 
	PANE_PARTY_CHARACTERS = 1,     // virtual "party" which will form active party on leaving
	PANE_CHARACTER_OPTIONS = 2,
	PANE_PARTY_INVENTORY = 3,	   // base party inventory
	PANE_BASE_INVENTORY = 4,       // base "store"
	PANE_PURCHASES = 5
};

class BaseManager
{
	// Party Base Mechanics
	
	// It is quite possible for a party to control more than one base of operations. Indeed at later levels this is entirely expected.
	// There are 3 particular kinds of Base:
	// 1) Camps (Temporary bases created by a Party)
	// 2) Settlement Bases (Towns and Cities, in which the Party has a base of operations such as an Inn or a house etc)
	// 3) Strongholds (Structures constructed or conquered by a Party outside an existing Settlement)
	// These differ largely by capability, and as a result are detailed in a JSON file.
	// Note that while some capabilities are limited by type (eg Camps cannot provide true Bed Rest, do not count as Return to Civilisation for XP purposes and do not allow Living Costs expenditure),
	// other capabilities must be unlocked in some fashion (usually by paying for it such as Storage or Workshops)
	// One final element is that Camps will eventually form the basis of the player construction system, which means we probably want some kind of map representation of the Camp besides the region map icon.

	// NB - This is not the same thing as Settlements. Settlements are a whole other issue and will be dealt with in SettlementManager - although a Settlement can be the site of a Base. The Settlement Interface will handle that transition.

	// Loaded data
	BaseInfoSet baseInfoSet;
	
	// Core Base info
	std::vector<int> baseType; // what type of Base is this? Indexes into the BaseInfoSet
	//std::vector<int> controllingFaction; // to be used when we have Factions
	
	// Base Party
	// Base Parties are created when the base is made and can be used to transfer characters and items
	// In future we'll have more complex controls for managing characters between parties (sub-groups etc) to make big camps easier to manage,
	// Why multiple parties? Because for example, wounded characters might be left in camp to recover (bed rest) with some henches to guard them
	// while other characters rove around the wilderness. Likewise a freshened party may leave treasure with the carts in the base while dungeoneering.
	// In all these cases, party events (eg daily encounter rolls in wilderness) will continue to happen to the base party.
	std::vector<int> basePartyID;
	// the Owner Party is the party which created this base. This is generally the player's Party, although this may change in future.
	std::vector<int> ownerPartyID;

	int nextBaseID;

	int shellGenerate(); // just creates all the internal structures for the next party, with nothing added in

	std::vector<int> baseXPos;
	std::vector<int> baseYPos;

	std::map<std::string, int> reverseBaseTypeDictionary;
	std::map<std::string, int> reverseBaseTagDictionary;

	int controlPane = 0;
	int menuPosition = 0;

	std::vector<std::vector<std::map<std::string, int>>> pcActiveTags;

	void getBasePartyCharacters(std::vector<int>& out, int baseID);
	void getVisitingPartyCharacters(std::vector<int>& out, int partyID);
	
	int getBasePartyCharacterCount(int baseID);
	int getVisitingCharacterCount(int partyID);

public:
	BaseManager(BaseInfoSet& _bis) : baseInfoSet(_bis)
	{}
	~BaseManager()
	{}

	// Manager Factory
	static BaseManager* LoadBaseData();

	int GenerateCampAtLocation(int partyID, int basePosX, int basePosY);

	int GetBaseAt(int x, int y);
	int GetBaseOwner(int baseID)
	{
		return ownerPartyID[baseID];
	}

	bool IsAPC(int entityID, int baseID);
	bool IsAHenchman(int entityID, int baseID);
	bool IsAnAnimal(int entityID, int baseID);

	// Accessors
	std::vector<int>& getPlayerCharacters(int baseID);
	std::vector<int>& getHenchmen(int baseID);
	std::vector<int>& getAnimals(int baseID);

	void AddPlayerCharacter(int characterID, int baseID);
	void AddHenchman(int characterID, int baseID);
	void AddAnimal(int mobID, int baseID);
	void RemoveCharacter(int entityID, int baseID);
	void RemoveAnimal(int entityID, int baseID);

	bool CharacterCanUseAction(int baseID, int characterID, int tag);
	std::vector<BaseTag> GetCharacterActionList(int baseID, int charID);
	int GetSelectedCharacter(int baseID);

	std::string GetBaseType(int baseID);
	
	int GetBaseX(int baseID) { return baseXPos[baseID]; }
	int GetBaseY(int baseID) { return baseYPos[baseID]; }
	void SetBaseX(int baseID, int xpos) { baseXPos[baseID] = xpos; }
	void SetBaseY(int baseID, int ypos) { baseYPos[baseID] = ypos; }

	bool ControlCommand(TCOD_key_t* key,int baseID);
	
	void RenderBaseMenu(int xpos, int ypos);
	void RenderBaseMenu(int baseID);
	
	void DumpBase(int partyID);

	void DebugLog(std::string message);

	bool TurnHandler(int entityID, double time);
	// no TargetHandler - there isn't a situation for that
	bool TimeHandler(int rounds, int turns, int hours, int days, int weeks, int months);
};
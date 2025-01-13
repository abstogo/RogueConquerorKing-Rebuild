#pragma once
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpath/json_query.hpp>
#include <jsoncons/json_type_traits_macros.hpp>
#include <jsoncons_ext/csv/csv.hpp>
#include <fstream>
#include "Class.h"
#include "Character.h"
#include "Game.h"

class PartyManager
{
	// How the Party works:
	// The party is the main unit of control - either directly or piecemeal.
	// In Settlement and Camp mode, the party operates as a whole (doing Domain mechanics)
	// In Region mode we move by party, which has a movement rate equal to its lowest member.
	// In Wilderness and Dungeon mode we move by current character.
	//
	// In the current version the Party operates primarily as a means of collecting the PCs and henches together, as we currently have no military game.
	// Later the Party will also manage the local mercenary set and any local specialist hirelings.
	// To begin with we only have 1 Party. Later we will have non-player Parties and subordinate Parties (eg you send a Hench and some military to fight goblins)
	// Note: There are currently no plans to have henches of henches in the game for simplicity, but subordinate Parties will handle some of that.

	// Characters
	
	std::vector<std::vector<int>> playerCharacters; // playable characters, referenced to CharacterManager
	std::vector<std::vector<int>> henchmen; // non-playable characters, referenced to CharacterManager
	std::vector<std::vector<int>> animals; // animal henches etc, referenced to MobManager
	// std::vector<std::vector<int>> mercenaries; // non-playable characters, only present in Region parties and Camps, referenced to TroopManager and gets instantiated into MobManager
	// std::vector<std::vector<int>> subordinates; // other parties controlled by this party, referenced to PartyManager

	// Inventory
	// The core problem with how to model the carrying ability of the party is that we don't want to try to model the carrying ability of each individual mule, hench, cart etc
	// as well as the PCs themselves. Not due to processing - it's actually pretty low-end - but because it would be incredibly complicated for the player to micromanage.
	// Instead, animals and "porter" henches are subsumed into a Party Inventory and items can be switched between that and the player inventories.
	// When the party enters a Settlement, items are sold directly from the Party Inventory.

	std::vector<std::vector<std::pair<int, int>>> partyInventory; // inventory is set up as pairs of IDs and counts (eg we have 25 flasks of military oil)
	std::vector<int> totalCarryCapacity; // in stone. Total of all porters and animals (which includes carts etc). We assume the non-porter Henches and the PCs do not carry items between dungeons.

	std::vector<int> totalSuppliesFood; // in mandays. 
	// std::vector<int> totalSuppliesWater; // add this later

	int nextPartyID; // at some point we should make this go back to inactive entries so it doesn't inflate forever

	int shellGenerate(); // just creates all the internal structures for the next party, with nothing added in

	std::vector<int> partyXPos;
	std::vector<int> partyYPos;

	std::vector<bool> active;

public:
	PartyManager();
	~PartyManager();

	// Manager Factory
	static PartyManager* LoadPartyData();

	int GenerateEmptyParty();
	// convenience function 1. Generates 1 Fighter and a Mule
	int GenerateBaseTestParty();
	// second convenience function for AI control testing. Generates 2 Fighters, 1 Level 0 Hench and a Mule
	int GenerateAITestParty();

	// Member Manipulators
	void AddPlayerCharacter(int partyID, int characterID);
	void GeneratePlayerCharacter(int partyID, std::string name, const std::string _class);

	void AddHenchman(int partyID, int characterID);
	void GenerateHenchman(int partyID, std::string name); // only generates Lvl 0 Normal Men. Generate more advanced Henches using CharacterManager and add them with AddHenchman.

	void AddAnimal(int partyID, int mobID);
	void GenerateAnimal(int partyID, std::string name);

	void RemoveCharacter(int partyID, int entityID);
	void RemoveAnimal(int partyID, int entityID);

	void MergeParty(int fromID, int toID); // transfer all characters and destroy original party

	void TransferCharacter(int sourcePartyID, int destinationPartyID, int characterID);
	void TransferAnimal(int sourcePartyID, int destinationPartyID, int mobID);
	void TransferParty(int sourcePartyID, int destinationPartyID);

	// Accessors
	std::vector<int>& getPlayerCharacters(int partyID) { return playerCharacters[partyID]; }
	std::vector<int>& getHenchmen(int partyID) { return henchmen[partyID]; }
	std::vector<int>& getAnimals(int partyID) { return animals[partyID]; }

	bool IsInParty(int partyID, int managerType, int entityID);
	bool IsAPC(int partyID, int entityID);
	bool IsAHenchman(int partyID, int entityID);
	bool IsAnAnimal(int partyID, int entityID);
	
	int getTotalSuppliesFood(int partyID) { return totalSuppliesFood[partyID]; }
	int getTotalCarryCapacity(int partyID) { return totalCarryCapacity[partyID]; }

	std::vector<std::pair<int, int>>& getPartyInventory(int partyID) { return partyInventory[partyID]; }
	void SetPartyInventory(int partyID, std::vector<std::pair<int, int>>& newInventory) { partyInventory[partyID] = newInventory; }

	int getNextPlayerCharacter(int partyID, int currentPC = -1); // if currentPC is -1, get the first one. Does not return incapable PCs; returns -1 if no incapable PCs left (which generally means game over)

	int GetPartyX(int partyID) { return partyXPos[partyID]; }
	int GetPartyY(int partyID) { return partyYPos[partyID]; }
	void SetPartyX(int partyID, int xpos) { partyXPos[partyID] = xpos; }
	void SetPartyY(int partyID, int ypos) { partyYPos[partyID] = ypos; }

	int GetPartyAt(int x, int y);

	void DumpParty(int partyID);

	
	
	void DebugLog(std::string message);

	bool TurnHandler(int entityID, double time);
	// no TargetHandler - there isn't a situation for that
	bool TimeHandler(int rounds, int turns, int hours, int days, int weeks, int months);
};
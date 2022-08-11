#pragma once
#include "Class.h"
#include "Character.h"
#include "Maps.h"
#include "GameTime.h"
#include "Mobs.h"
#include "Conditions.h"
#include "ItemTemplate.h"
#include "Party.h"
#include "Bases.h"

/*
 * The Game class exists to contain the various managers etc for the game and coordinate the game's functions
 *
 * 
 */

class CharacterManager;
class MapManager;
class ClassManager;
class ItemManager;
class MobManager;
class ConditionManager;
class MortalWoundManager;
class PartyManager;
class BaseManager;

enum ManagerType
{
	MANAGER_GAME = -1,
	MANAGER_CHARACTER = 0,
	MANAGER_MOB,
	MANAGER_MAP,
	MANAGER_ITEM,
	MANAGER_CONDITION,
	MANAGER_MORTAL,
	MANAGER_PARTY,
	MANAGER_BASE,
	MANAGER_MAX
};

enum GAME_MODE
{
	GM_MAIN = 0,		// normal play mode
	GM_MENU,
	GM_TARGET,			// target mode, selects from specific set of target Points and triggers something when Enter is pressed (see TARGET_MODE)
	GM_INVENTORY,		// inventory mode, renders inventory window and allows wielding, dropping etc
	GM_CHARACTER,		// character sheet mode, displays character statistics and allows levelling up
//	GM_ABILITY,			// character ability mode (class abilities & proficiencies)
//	GM_SPELL,			// spells mode
//  GM_ARMY,			// army management mode
	GM_DOMAIN,			// domain management mode, includes camping, settlements
//	GM_TEXTENTRY,		// text entry mode, accepts text input
//	GM_LOG,				// view existing game log
//	GM_CHECK,			// "Are you sure?" mode
	GM_END,				// end of game
	GM_QUIT,			// leave game
	GM_MAX
};

enum TARGET_MODE
{
	TARGET_CELL,				// floor location (trap construct, flask/grenade throw) (0:range)
	TARGET_CREATURE,			// creature (eg missile attack, spell target) (0: range, 1:ally/enemy flags)
	//TARGET_NEAREST_X,			// nearest X allies/enemies (eg Sleep, Bless) (0: range, 1:ally/enemy flags, 2: X)
	//TARGET_SPHERE,			// floor location to target a sphere attack (eg Fireball) (0: range, 1:ally/enemy flags, 2:radius)
	//TARGET_CONE				// target cone around character (eg Burning Hands) (0:range, 1:ally/enemy flags, 2:width at widest end)
};

enum TARGET_FLAGS
{
	TF_ENEMY = 1,
	TF_FRIEND = 2,
};

class Game
{
	int currentPartyID;
	int currentCharacterID;
	int currentMapID;
	int currentBaseID;

	int mode;
	
	Map* currentMap;

	const float TORCH_RADIUS = 10;
	const float SQUARED_TORCH_RADIUS = TORCH_RADIUS * TORCH_RADIUS;

	const float SAMPLE_SCREEN_X = 2;
	const float SAMPLE_SCREEN_Y = 2;
	
	bool recomputeFov ; // the player moved. must recompute fov

	bool light_walls;

	std::string playLogString;

	int menuPosition[GM_MAX];
	
	int inventoryPosition = 0;
	int abilityPosition = 0;

	// target mode data
	int targetCursorX, targetCursorY; // target cursor for rendering purposes
	int targetIndex;
	int targetMode;
	std::vector<int> targetIDs; // creature/character IDs to mark
	std::vector<int> targetingData;

	// targeting data setup:
	// 0: Return Manager
	// 1: Return Code
	// 2: Range
	// 3: Ally/Enemy flags (if any)
	// 4: Effect Size (affected creatures, radius, width at cone end)

	int remainingCleaves = -1;

	bool hostilifying = false;

	std::vector<std::string> menuText;
	std::vector<std::function<void()>> menuFunctions;
	int menuSelected;
	
	void MoveCharacter(int new_x, int new_y);

	void RenderOffscreenUI(bool inventory, bool character);

	std::vector<int> GetTargetedEntities();

	void SpawnLevel(int mapID, int spawnPointX, int spawnPointY);
	void QuitGame();
	
public:
	Game()
	{
		sampleConsole = new TCODConsole(SAMPLE_SCREEN_WIDTH, SAMPLE_SCREEN_HEIGHT);
	}

	void StartGame();
	void ClearGame();
	void CreateMenu();
	void CreateTestGame();
	
	bool TargetHandler(int entityID, int returnCode);
	void TriggerTargeting(int targetingMode, int returnManager, int returnCode, int range = -1, int size = 1, bool allies = false, bool enemies = false , std::vector<int>& targets = std::vector<int>());

	void MainLoop();
	
	void RenderMap();
	void RenderScreenFurniture();
	void RenderUI(int selectedCharacterID);
	void RenderActionLog();
	void ClearLookText();
	void UpdateLookText(int x, int y);
	void AddActionLogText(std::string term, bool clear = false);

	void DebugLog(std::string message);

	void RenderCharacterSheet();
	void RenderInventory();

	void RenderTargets();

	void RenderMenu();
	
	// We have to split the keyboard handling between hex and ortho modes.
	// Since our players shouldn't need to shift between modes just to use menu/selector etc controls, we also separate those controls as well as moves
	bool MainGameHandleKeyboard(TCOD_key_t* key);
	bool MenuGameHandleKeyboard(TCOD_key_t* key);
	bool HandleHexKeyboard(TCOD_key_t* key);
	bool HandleOrthoKeyboard(TCOD_key_t* key);

	bool HexKeyboardMove(int move_value);
	bool OrthoKeyboardMove(int move_value);

	bool HexKeyboardTarget(int move_value);
	bool OrthoKeyboardTarget(int move_value);

	int HexToOrtho(int input);
	
	int& GetCurrentMap() { return currentMapID; }
	int GetSelectedCharacterID() { return currentCharacterID; }
	void SetSelectedCharacterID(int characterID) { currentCharacterID = characterID; }
	int GetSelectedPartyID() { return currentPartyID; }
	void SetSelectedPartyID(int partyID) { currentPartyID = partyID; }

	int GetSelectedBaseID() { return currentBaseID; }
	void SetSelectedBaseID(int baseID) { currentBaseID = baseID; }

	bool ResolveAttacks(int attackerManager, int attackerID, int defenderManager, int defenderID, bool missile); // if returns true we're finished so return to GM_MAIN
	bool ResolveAttack(int attackBonus, int damageDie, int damageBonus, int defenderMananger, int defenderID, bool missile);
	bool ResolveDamage(int damageDie, int damageBonus, int targetManager, int targetID);

	void RecalculateFOV() { recomputeFov = true; }

	void CharacterDeath(int characterID); // used when another manager declares a character dead
	
	// publicly accessible managers
	CharacterManager* mCharacterManager;
	ClassManager* mClassManager;
	MapManager* mMapManager;
	TimeManager* mTimeManager;
	ItemManager* mItemManager;
	MobManager* mMobManager;
	ConditionManager* mConditionManager;
	MortalWoundManager* mMortalManager;
	PartyManager* mPartyManager;
	BaseManager* mBaseManager;
	
	TCODConsole* sampleConsole;

	TCODConsole* characterScreen = nullptr;
	TCODConsole* inventoryScreen = nullptr;

	TCODConsole* characterShot = nullptr;
	TCODConsole* inventoryShot = nullptr;

	TCODRandom* randomiser = TCODRandom::getInstance();
};

extern Game* gGame;
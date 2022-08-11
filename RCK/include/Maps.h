#pragma once
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stack>
#include "libtcod.hpp"
#include <vector>
#include <list>
#include <queue>
#include <string>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpath/json_query.hpp>
#include <jsoncons/json_type_traits_macros.hpp>
#include <jsoncons_ext/csv/csv.hpp>
#include <fstream>
#include "OutputLog.h"

// sample screen size
#define SAMPLE_SCREEN_WIDTH 46
#define SAMPLE_SCREEN_HEIGHT 20

#define OUTDOOR_MAP_WIDTH (SAMPLE_SCREEN_WIDTH / 2)
#define OUTDOOR_MAP_HEIGHT (SAMPLE_SCREEN_HEIGHT / 2)

#define INDOOR_MAP_WIDTH SAMPLE_SCREEN_WIDTH
#define INDOOR_MAP_HEIGHT SAMPLE_SCREEN_HEIGHT

// regional cell data is split into two elements - terrain type and content
// terrain is used for movement rate and also for local map generation
enum RegionTerrainTypes
{
	TERRAIN_PLAINS = 0,
	TERRAIN_MOUNTAIN,
	TERRAIN_HILLS,
	TERRAIN_FOREST,
	TERRAIN_JUNGLE,
	TERRAIN_SWAMP,
	TERRAIN_DESERT,
	// TERRAIN_SEA_COAST, // probably doing nothing with ocean travel for... quite some time
	// TERRAN_SEA_DEEP
	TERRAIN_MAX
};

// content covers local structures
// note this does not include flows such as rivers and roads
enum RegionSiteTypes
{
	SITE_NONE,
	SITE_DUNGEON,
	//REGION_SETTLEMENT,
	//REGION_CAMP,
	//REGION_LAIR,
	
};

// local content covers things like doors, furniture, walls, that sort of thing. (Basically everything except entities and items.)
enum LocalContentTypes
{
	CONTENT_NONE = 0,
	CONTENT_TREE,
	CONTENT_ROCKS,
	CONTENT_WALL,
	CONTENT_TRANSITION_STAIRS, // indoor-indoor transition
	CONTENT_TRANSITION_DOOR, // outdoor-indoor-outdoor transition
	CONTENT_TRANSITION_ZONE, // outdoor-outdoor transition
	CONTENT_MAX
};

enum MapTypes
{
	MAP_DUNGEON = 0,	// 1 square = 5ft
	MAP_WILDERNESS,		// 1 hex = 5yd
	MAP_REGION,			// 1 hex = 6 miles
	MAP_MAX
};

enum Hex_Movement
{
	HEX_RIGHTUP = 0,
	HEX_RIGHT,
	HEX_RIGHTDOWN,
	HEX_LEFTDOWN,
	HEX_LEFT,
	HEX_LEFTUP,
	HEX_MAX
};

static int move_map_odd[6][2] =
{
	{1,-1}, // rightup
	{1,0},	// right
	{1,1},	// rightdown
	{0,1},	// leftdown
	{-1,0},	// left
	{0,-1}	// leftup
};

static int move_map_even[6][2] =
{
	{ 0,-1 }, // rightup
	{ 1,0 },	// right
	{ 0,1 },	// rightdown
	{ -1,1 },	// leftdown
	{ -1,0 },	// left
	{ -1,-1 }	// leftup
};

// now handle the keyboard input, which is a little different for hex maps (89UOJK)
static int move_map_ortho[8][2] =
{
	{0 ,-1},    // up
	{1 , 0},	// right
	{0 , 1},	// down
	{-1, 0}, 	// left
	{-1, -1}, 	// up-left
	{-1, 1}, 	// down-left
	{1, 1}, 	// down-right
	{1, -1} 	// up-right
};


enum Ortho_Movement
{
	ORTHO_UP = 0,
	ORTHO_RIGHT,
	ORTHO_DOWN,
	ORTHO_LEFT,
	ORTHO_UPLEFT,
	ORTHO_DOWNLEFT,
	ORTHO_DOWNRIGHT,
	ORTHO_UPRIGHT
};


// load map prefab data
class TerrainType
{
	std::string Name_;
	std::string RegionMapSymbol_;
	int OverlandTravelMultiplier_;
	int EncounterProbability_;
	std::string EncounterTable_;
	std::string Generator_;
	std::vector<std::string> Prefabs_;

public:
	TerrainType(const std::string& Name, const std::string& RegionMapSymbol, const int OverlandTravelMultiplier,
				 const int EncounterProbability, const std::string& EncounterTable, const std::string& Generator,
				 const std::vector<std::string>& Prefabs)
		: Name_(Name), RegionMapSymbol_(RegionMapSymbol), OverlandTravelMultiplier_(OverlandTravelMultiplier),
		  EncounterProbability_(EncounterProbability), EncounterTable_(EncounterTable), Generator_(Generator),
		  Prefabs_(Prefabs)
	{}

	const std::string& Name() { return Name_; }
	const std::string& RegionMapSymbol() { return RegionMapSymbol_; }
	const int OverlandTravelMultiplier() { return OverlandTravelMultiplier_; }
	const int EncounterProbability() { return EncounterProbability_; }
	const std::string& EncounterTable() { return EncounterTable_; }
	const std::string& Generator() { return Generator_; }
	std::vector<std::string> Prefabs() { return Prefabs_; }
};
JSONCONS_ALL_GETTER_CTOR_TRAITS_DECL(TerrainType, Name, RegionMapSymbol, OverlandTravelMultiplier, EncounterProbability, EncounterTable, Generator, Prefabs)

class TerrainTypeSet
{
	std::vector<TerrainType> TerrainTypes_;

public:
	TerrainTypeSet(const std::vector<TerrainType>& TerrainTypes) : TerrainTypes_(TerrainTypes)
	{}

	std::vector<TerrainType> TerrainTypes() { return TerrainTypes_; }
};
JSONCONS_ALL_GETTER_CTOR_TRAITS_DECL(TerrainTypeSet, TerrainTypes)


struct RegionMap
{
	TCODMap* map = NULL;

	int width;
	int height;

	std::vector<int> localMap;
	std::vector<int> terrain;
	std::vector<int> sites;
	std::vector<int> bases;

	int getLocalMap(int x, int y)
	{
		return(localMap[y * width + x]);
	}

	void setLocalMap(int x, int y, int c)
	{
		localMap[y * width + x] = c;
	}

	int getTerrain(int x, int y)
	{
		return(terrain[y * width + x]);
	}

	void setTerrain(int x, int y, int c)
	{
		terrain[y * width + x] = c;
	}

	int getSite(int x, int y)
	{
		return(sites[y * width + x]);
	}

	void setSite(int x, int y, int c)
	{
		sites[y * width + x] = c;
	}

	int getBase(int x, int y)
	{
		return(bases[y * width + x]);
	}

	void setBase(int x, int y, int c)
	{
		bases[y * width + x] = c;
	}
};

// ARRAY OF STRUCTS OF ARRAYS MOFO
// (I'm breaking with the plan a little)
struct Map
{
	TCODMap* map = NULL;

	bool outdoor = false;
	int mapType;

	int width;
	int height;

	// vectors of every cell
	
	std::vector<std::stack<int>> items;
	std::vector<int> content;
	std::vector<int> transition;

	// vectors of lists of things
	
	std::vector<int> reverse_transition_mapindex;
	std::vector<int> reverse_transition_xpos;
	std::vector<int> reverse_transition_ypos;

	std::vector<int> mobs;						// monsters and non-fully-fleshed NPCS, ref to MobManager, position is stored there
	std::vector<int> characters;				// characters uncontrolled by the player, ref to CharacterManager, position is stored there

	int getCharacterAt(int x, int y);
	void setCharacter(int x, int y, int characterID);
	int removeCharacter(int characterID);

	int getMobAt(int x, int y);
	void setMob(int x, int y, int mobID);
	int removeMob(int mobID);

	void getManagedEntityAt(int x, int y, int& manager, int& entityID);
	
	int getContent(int x, int y)
	{
		return(content[y * width + x]);
	}

	void setContent(int x, int y, int c)
	{
		content[y * width + x] = c;
	}

	void addItem(int x, int y, int item)
	{
		items[y * width + x].push(item);
	}

	std::stack<int>* getItems(int x, int y)
	{
		return &items[y * width + x];
	}

	void setTransition(int x, int y, int target)
	{
		transition[y * width + x] = target;
	}

	int getTransition(int x, int y)
	{
		return transition[y * width + x];
	}
};


class MapManager : public ITCODPathCallback
{
	RegionMap* regionMap;
	std::vector<Map*> mapStore;

	std::vector<std::vector<std::vector<std::string>>> terrain_prefabs;
	TerrainTypeSet terrainTypes;

	void GeneratePrefabs();
	
public:
	MapManager(TerrainTypeSet& tts);
	~MapManager();

	Map* getMap(int index);
	RegionMap* getRegionMap();

	//builds a new map, adds it to the map store, returns the id (distinct for indoor and outdoor maps)
	int createMap(bool outdoor);
	
	// builds an empty map of the specified type (useful for open playfields and spawners)
	int buildEmptyMap(int width, int height, int type);

	// builds a map from an array of strings (could be loaded from a file etc)
	int buildMapFromText(std::vector<std::string> hmap, bool outdoor);
	Map* mapFromText(std::vector<std::string> hmap, bool outdoor);

	// region map creation
	void createRegionMap();
	void buildEmptyRegionMap(int width, int height, int base_terrain);
	void BuildRegionMapFromText(std::vector<std::string> hmap_terrain);

	// spawn local map from region map
	int SpawnLocalMap(int x, int y);
	int GenerateMapFromPrefab(int x, int y, std::vector<std::string> hmap, int content);
	int GenerateMapAtLocation(int x, int y);

	int GetMapAtLocation(int x, int y);
	
	// this manager handles the main rendering, since it controls the map status & context (hex/square, lighting etc)
	// if the index is -1, show the region map, if >=0 then show a local map
	void renderMap(TCODConsole* sampleConsole, int index, int centroid_x, int centroid_y);
	void renderRegionMap(TCODConsole* sampleConsole, int centroid_x, int centroid_y);

	// mobile element (player, monster) are rendered through here too
    void renderAtPosition(TCODConsole* sampleConsole, int mapIndex, int centroid_x, int centroid_y, int x, int y, char c, TCODColor foreground = TCODColor::lighterGrey);

	// connects one local map to another at the specified point
	void connectMaps(int map1, int map2, int x1, int y1, int x2, int y2);

	// factory
	static MapManager* LoadMaps();

	// item management
	void AddItem(int mapID, int x, int y, std::string item);
	void AddItem(int mapID, int x, int y, int itemID);
	int TakeTopItem(int mapID, int x, int y);
	std::string ItemDesc(int mapID, int x, int y);

	// movement multiplier (based on map level - higher level maps have larger sized cells, therefore take longer to move through)
	double getMovementTime(int mapID, double speed);

	// hex mobility functions
	float getWalkCost(int xFrom, int yFrom, int xTo, int yTo, void* userData) const;
	void shift(int mapID, int& new_x, int& new_y, int player_x, int player_y, int move_value);

	// bounds checking
	bool isOutOfBounds(int mapID, int x, int y);

	bool isInFOV(int sourceManager, int sourceID, int targetManager, int targetID, int range = 0);
	std::vector<int> filterByFOV(int sourceManager, int sourceID, int targetManager, std::vector<int> targets, int range = 0);

	// handlers
	bool TurnHandler(int entityID, double time);
	bool TargetHandler(int entityID, int returnCode);
	bool TimeHandler(int rounds, int turns, int hours, int days, int weeks, int months);
	
	void DebugLog(std::string message);
};

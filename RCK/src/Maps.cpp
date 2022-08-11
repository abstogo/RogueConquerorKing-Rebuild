#include "Maps.h"
#include <sstream>
#include <string>
#include "Game.h"

void Map::setMob(int x, int y, int mobID)
{
	if (std::find(mobs.begin(), mobs.end(), mobID) == mobs.end())
	{
		mobs.push_back(mobID);
	}

	gGame->mMobManager->SetMobX(mobID, x);
	gGame->mMobManager->SetMobY(mobID, y);
}

void Map::setCharacter(int x, int y, int characterID)
{
	if (std::find(characters.begin(), characters.end(), characterID) == characters.end())
	{
		characters.push_back(characterID);
	}
	// then update it
	gGame->mCharacterManager->SetPlayerX(characterID,x);
	gGame->mCharacterManager->SetPlayerY(characterID,y);
}

int Map::getCharacterAt(int x, int y)
{
	int found = 0;

	auto g = std::find_if(
		characters.begin(), characters.end(),
		[&](int charID) { return ((gGame->mCharacterManager->GetPlayerX(charID) == x) && (gGame->mCharacterManager->GetPlayerY(charID) == y)); }
	);

	if (g != characters.end())
	{
		found = *g;
	}

	return found;
}

int Map::getMobAt(int x, int y)
{
	int found = 0;

	auto g = std::find_if(
		mobs.begin(), mobs.end(),
		[&](int mobID) { return ((gGame->mMobManager->GetMobX(mobID) == x) && (gGame->mMobManager->GetMobY(mobID) == y)); }
	);

	if (g != mobs.end())
	{
		found = *g;
	}

	return found;
}

void Map::getManagedEntityAt(int x, int y, int& manager, int& entityID)
{
	manager = MANAGER_MAX;

	int c = getCharacterAt(x, y);
	if(c != 0)
	{
		manager = MANAGER_CHARACTER;
		entityID = c;
		return;
	}

	int m = getMobAt(x, y);
	if (m != 0)
	{
		manager = MANAGER_MOB;
		entityID = manager;
		return;
	}

	c = 0;
}

int Map::removeCharacter(int characterID)
{
	auto p = std::find(characters.begin(), characters.end(), characterID);

	if(p == characters.end())
	{
		return 0;
	}

	return *p;
}

int Map::removeMob(int mobID)
{
	auto p = std::find(mobs.begin(), mobs.end(), mobID);

	if (p == mobs.end())
	{
		return 0;
	}

	return *p;
}

void MapManager::GeneratePrefabs()
{
	
	/**
	terrain_prefabs.resize(TERRAIN_MAX);

	terrain_prefabs[TERRAIN_PLAINS] = {
		". . . . . . . . . . . . . . . . . . . . . . . ",
		" . . . . . . . . . . . . . . . . . . . . . . .",
		". . . . . . . . . . . . . . . # . . . T . . . ",
		" . . . . . . . . . . . . . . . . . . . . . . .",
		". . . . . . . . . . . . . . . . . . T . . . . ",
		" . . . . . . . # . . . . . . . . T . T T . . .",
		". . . . . . . . . . . . . . . . . . . . . . . ",
		" . . . . . . . . . . . . . . . . . . . . . . .",
		". . . . T . . . . . . . . . . # . . . . T . . ",
		" . . . . . . . . . . . . . T T . . . . . . . .",
	};

	terrain_prefabs[TERRAIN_MOUNTAIN] = {
		". . . . . . . . . . # # # . . . . . . . . . . ",
		" . . . # . . . . . . . . # # . . . . . . . . .",
		". . # # . . . . . # . . . . . # . . . . . . . ",
		" . # . . . . . . # # . . . . # . . . . . # # .",
		". . # . . . . . # # . . . . . # . . . . # # . ",
		" . . # . . . . # # . . . . . # . . . . # # . .",
		". . . # . . . # # . . . . . . # . . . . # . . ",
		" . . # . . . # . . . . . . . . . . . . . . . .",
		". . # . . # # # . . . . . . . # . . . . . . . ",
		" . . # . . # # # . . . . . # # . . . . . . . .",
	};

	terrain_prefabs[TERRAIN_HILLS] = {
		". . . . . . . . . . . . . . . . . . . . . . . ",
		" . . . . . . . . . . . . . . . . . . . . . . .",
		". . . . . # # # . . . . . . . . . . . . . . . ",
		" . . . # # T # # # . . . . . . . . . . . . . .",
		". . . # # T T T T # # . . . . . . . . . . . . ",
		" . . . . . . . . . . # . . . . . . . . . . . .",
		". . . . . . . . . . # # . . . . . . . . . . . ",
		" . . . . . . . . . . # . . . . . . . . . . . .",
		". . . . . . . . . . . . . . . . . . . . . . . ",
		" . . . . . . . . . . . . . . . . . . . . . . .",
	};

	terrain_prefabs[TERRAIN_FOREST] = {
		"T T T . T T T T T T . T T . T T T T . T T . T ",
		" . . . . T . T T T T T . . T T T T T . T T . T",
		"T T . # # . T . T T T . . T T T T . T T T . T ",
		" . T T T . T T T T . . T T # # T T . . . T . T",
		"T T . T . . T . . . . T T T # T T . . . . T T ",
		" T T T T T T T T . . . . T T T T . . . . T T T",
		"T T T T T . T T . . . . . T T T T . . . . T . ",
		" T T T T T T T . . . . . T . . T . . . . . T T",
		". T T T . T T . . . . . T . . . T . . . . . T ",
		" . T . T T T T T T . . T . . . . T . . T . . T",
	};

	/** terrain_prefabs[TERRAIN_JUNGLE] = {
		". . . . . . . . . . . . . . . . . . . . . . . ",
		" . . . . . . . . . . . . . . . . . . . . . . .",
		". . . . . . . . . . . . . . . # . . . T . . . ",
		" . . . . . . . . . . . . . . . . . . . . . . .",
		". . . . . . . . . . . . . . . . . . T . . . . ",
		" . . . . . . . # . . . . . . . . T . T T . . .",
		". . . . . . . . . . . . . . . . . . . . . . . ",
		" . . . . . . . . . . . . . . . . . . . . . . .",
		". . . . T . . . . . . . . . . # . . . . T . . ",
		" . . . . . . . . . . . . . T T . . . . . . . .",
	};
	

	terrain_prefabs[TERRAIN_JUNGLE] = terrain_prefabs[TERRAIN_FOREST];

	/**terrain_prefabs[TERRAIN_SWAMP] = {
		". . . . . . . . . . . . . . . . . . . . . . . ",
		" . . . . . . . . . . . . . . . . . . . . . . .",
		". . . . . . . . . . . . . . . # . . . T . . . ",
		" . . . . . . . . . . . . . . . . . . . . . . .",
		". . . . . . . . . . . . . . . . . . T . . . . ",
		" . . . . . . . # . . . . . . . . T . T T . . .",
		". . . . . . . . . . . . . . . . . . . . . . . ",
		" . . . . . . . . . . . . . . . . . . . . . . .",
		". . . . T . . . . . . . . . . # . . . . T . . ",
		" . . . . . . . . . . . . . T T . . . . . . . .",
	};
	

	terrain_prefabs[TERRAIN_SWAMP] = terrain_prefabs[TERRAIN_PLAINS];

	terrain_prefabs[TERRAIN_DESERT] = {
		". . . . . . . . . . . . . . . . . . . . . . . ",
		" . . . . . . . . . . . . . . . . . . # . . . .",
		". . . . . . . . . . . . . . . . . . . . . . . ",
		" . . . . . . . . . . . . . . . . . . . . . . .",
		". . . . . . . . . . . . . . . . . . . . . . . ",
		" . . . . . . . # . . . . . . . . . . . . . . .",
		". . . . . . . . . . . . . . . . . . . . . . . ",
		" . . . . . . . . . . . . . . . . . . . . . . .",
		". . . . . . . . . . . . . . . . . . . . . . . ",
		" . . . . . . . . . . . . . . . . . . . . # . .",
	};
	*/

	for (TerrainType t : terrainTypes.TerrainTypes())
	{
		gLog->Log("MapManager", "Loading Prefabs for Terrain Type:" + t.Name());

		std::vector<std::vector<std::string>> prefabSet;

		for (std::string prefabPath : t.Prefabs())
		{
			gLog->Log("MapManager", "Loading Prefab:" + prefabPath);
			prefabPath = "RCK/prefabs/" + prefabPath;
			std::vector<std::string> prefabMap;

			// load text file line by line into the prefab
			std::ifstream infile(prefabPath);
			if (infile)
			{
				std::string line;
				for (std::string line; std::getline(infile, line); ) {
					prefabMap.push_back(line);
				}
				prefabSet.push_back(prefabMap);
			}

			
		}

		terrain_prefabs.push_back(prefabSet);
	}
}

MapManager::MapManager(TerrainTypeSet& tts) : terrainTypes(tts)
{
	//GeneratePrefabs();
	mapStore.push_back(NULL);
}

MapManager::~MapManager()
{

}

Map* MapManager::getMap(int index)
{
	return mapStore[index];
}

RegionMap* MapManager::getRegionMap()
{
	return regionMap;
}

bool MapManager::TargetHandler(int entityID, int returnCode)
{
	return true;
}

bool MapManager::TimeHandler(int rounds, int turns, int hours, int days, int weeks, int months)
{
	return true;
}

// region map creation
void MapManager::createRegionMap()
{
	regionMap = new RegionMap();
}

//builds a new map, adds it to the map store, returns the id (distinct for indoor and outdoor maps)
int MapManager::createMap(bool outdoor)
{
	Map* m = new Map();
	m->outdoor = outdoor;
	int id = mapStore.size(); // start at index 1, so 0 means no local map
	mapStore.push_back(m);
	return id;
}

bool MapManager::isInFOV(int sourceManager, int sourceID, int targetManager, int targetID, int range)
{
	int baseX, baseY;

	switch (sourceManager)
	{
		case MANAGER_CHARACTER:
		{
			baseX = gGame->mCharacterManager->GetPlayerX(sourceID);
			baseY = gGame->mCharacterManager->GetPlayerY(sourceID);
		}
		break;

		case MANAGER_MOB:
		{
			baseX = gGame->mMobManager->GetMobX(sourceID);
			baseY = gGame->mMobManager->GetMobY(sourceID);
		}
	break;
	}

	Map* m = getMap(gGame->GetCurrentMap());
	m->map->computeFov(baseX, baseY, range, true, FOV_BASIC);

	int targetX, targetY;
	switch (targetManager)
	{
		case MANAGER_CHARACTER:
		{
			targetX = gGame->mCharacterManager->GetPlayerX(targetID);
			targetY = gGame->mCharacterManager->GetPlayerY(targetID);
		}
		break;

		case MANAGER_MOB:
		{
			targetX = gGame->mMobManager->GetMobX(targetID);
			targetY = gGame->mMobManager->GetMobY(targetID);
		}
		break;
	}

	return m->map->isInFov(targetX, targetY);

}

std::vector<int> MapManager::filterByFOV(int sourceManager, int sourceID, int targetManager, std::vector<int> targets, int range)
{
	std::vector<int> output;

	int baseX, baseY;

	switch (sourceManager)
	{
		case MANAGER_CHARACTER:
		{
			baseX = gGame->mCharacterManager->GetPlayerX(sourceID);
			baseY = gGame->mCharacterManager->GetPlayerY(sourceID);
		}
		break;

		case MANAGER_MOB:
		{
			baseX = gGame->mMobManager->GetMobX(sourceID);
			baseY = gGame->mMobManager->GetMobY(sourceID);
		}
		break;
	}

	Map* m = getMap(gGame->GetCurrentMap());
	m->map->computeFov(baseX, baseY, range, true,FOV_BASIC);

	gGame->RecalculateFOV(); // this makes the player fix the FOV next time we render
	
	for(int target : targets)
	{
		int targetX, targetY;
		switch(targetManager)
		{
		case MANAGER_CHARACTER:
			{
				targetX = gGame->mCharacterManager->GetPlayerX(target);
				targetY = gGame->mCharacterManager->GetPlayerY(target);
			}
			break;

		case MANAGER_MOB:
			{
				targetX = gGame->mMobManager->GetMobX(target);
				targetY = gGame->mMobManager->GetMobY(target);
			}
			break;
		}
		
		bool fov = m->map->isInFov(targetX, targetY);
		if (fov)
			output.push_back(target);
	}

	return output;
}


MapManager* MapManager::LoadMaps()
{
	
	gLog->Log("Map Manager", "Started");

	const std::string mapsFilename = "RCK/prefabs/maps.json";

	std::ifstream is(mapsFilename);

	MapManager* newManager = new MapManager(jsoncons::decode_json<TerrainTypeSet>(is));

	// load prefabs (could do this later?)

	newManager->GeneratePrefabs();


	return newManager;
}

void MapManager::buildEmptyRegionMap(int width, int height, int base_terrain)
{
	createRegionMap();

	regionMap->width = width;
	regionMap->height = height;

	regionMap->map = new TCODMap(width, height);

	regionMap->localMap.resize(width * height);
	for (int i = 0; i < width * height; i++)
		regionMap->localMap[i] = -1;
	regionMap->terrain.resize(width * height);
	regionMap->sites.resize(width * height);
	regionMap->bases.resize(width * height);
	for (int i = 0; i < width * height; i++)
		regionMap->bases[i] = -1;

	regionMap->map->clear(true, true);
}

int MapManager::buildEmptyMap(int width, int height, int type)
{
	bool outdoor = true;
	if (type == MAP_DUNGEON)
		outdoor = false;
	
	int index = createMap(outdoor);

	Map* newMap = mapStore[index];

	newMap->width = width;
	newMap->height = height;

	newMap->mapType = type;
	
	newMap->content.resize(width * height);
	newMap->transition.resize(width * height);
	newMap->items.resize(width * height);

	newMap->mobs.resize(width * height);
	newMap->characters.resize(width * height);
	
	newMap->map = new TCODMap(width, height);
    newMap->map->clear(true, true);
        
	return index;
}



void MapManager::BuildRegionMapFromText(std::vector<std::string> hmap_terrain)
{
	int map_width = SAMPLE_SCREEN_WIDTH;
	int map_height = (SAMPLE_SCREEN_HEIGHT / 2);

	buildEmptyRegionMap(map_width, map_height, TERRAIN_PLAINS);

	for (int y = 0; y < map_height; y++)
	{
		bool stepped = y & 0x1;
		for (int x = (stepped) ? 1 : 0; x < map_width; x += 2)
		{
			int cell_x = (int)(x / 2);
			int cell_y = (int)y;
			
			char terrain_value = hmap_terrain[y][x];
			auto terrains = terrainTypes.TerrainTypes();
			for (int terrain_index=0; terrain_index < terrains.size();terrain_index++)
			{
				TerrainType& t = terrains[terrain_index];
				if (terrain_value == t.RegionMapSymbol().c_str()[0])
				{
					regionMap->map->setProperties(cell_x, cell_y, true, true);	// most terrain types are traversable at slow speeds and view distance is to be variable
					regionMap->setTerrain(cell_x, cell_y, terrain_index);
				}
			}
		}
	}
}

// builds an outdoor map from an array of strings (could be loaded from a file etc)
int MapManager::buildMapFromText(std::vector<std::string> hmap,bool outdoor)
{
	int map_width = hmap[0].size();
	int map_height = hmap.size();

	int index = buildEmptyMap(outdoor ? map_width / 2 : map_width, map_height, outdoor ? MAP_WILDERNESS : MAP_DUNGEON);
	Map* newMap = mapStore[index];
	
	for (int y = 0; y < map_height; y++)
	{
		bool stepped = y & 0x1;
		for (int x = (outdoor && stepped) ? 1 : 0; x < map_width; x += outdoor ? 2 : 1)
		{
			char value = hmap[y][x];
			int cell_x = (int)(outdoor ? x / 2 : x);
			int cell_y = (int)y;
			if (value == '.')
			{
				newMap->map->setProperties(cell_x, cell_y, true, true);	// ground
			}
			if (value == 'T')
			{
				newMap->map->setProperties(cell_x, cell_y, true, false);		// tree
				newMap->setContent(cell_x, cell_y, CONTENT_TREE);
			}
			if (value == '#')
			{
				newMap->map->setProperties(cell_x, cell_y, false, false); // wall
				newMap->setContent(cell_x, cell_y, outdoor ? CONTENT_ROCKS : CONTENT_WALL);
			}
		}
	}
	
	return index;
}

int MapManager::SpawnLocalMap(int x, int y)
{
	int map_width = SAMPLE_SCREEN_WIDTH;
	int map_height = (SAMPLE_SCREEN_HEIGHT / 2);
	
	int mapID = buildEmptyMap(map_width, map_height, MAP_WILDERNESS);

	// 1 map generation function per terrain type
	// this is to be replaced with a JSON file associating local content elements with procgen functions (eg spread, warren, clusters, splats)
	
	regionMap->setLocalMap(x, y, mapID);

	return mapID;
}

int MapManager::GenerateMapFromPrefab(int x, int y, std::vector<std::string> hmap, int site)
{	
	int mapID = buildMapFromText(hmap, true);
	regionMap->setLocalMap(x, y, mapID);
	regionMap->setSite(x, y, site);

	return mapID;
}

int MapManager::GenerateMapAtLocation(int x, int y)
{
	int terrain = regionMap->getTerrain(x, y);

	auto prefabSet = terrain_prefabs[terrain];
	int prefabCount = prefabSet.size();

	int selection = gGame->randomiser->getInt(0, prefabCount - 1);

	int mapID = buildMapFromText(prefabSet[selection], true);
	regionMap->setLocalMap(x, y, mapID);

	return mapID;
}

int MapManager::GetMapAtLocation(int x, int y)
{
	int lmap = regionMap->getLocalMap(x, y);
	if (lmap != -1)
	{
		return lmap;
	}

	// TODO: random generation of wilderness features will go here!

	lmap = GenerateMapAtLocation(x, y);

	return lmap;
}

void MapManager::shift(int mapID, int& new_x, int& new_y, int unit_x, int unit_y, int move_value)
{
	bool outdoor = true;
	if (mapID != -1)
	{
		outdoor = mapStore[mapID]->outdoor;
	}
	if(outdoor)
	{
		bool odd = unit_y & 0x1;
		if (!odd)
		{
			new_x = unit_x + move_map_even[move_value][0];
			new_y = unit_y + move_map_even[move_value][1];
		}
		else
		{
			new_x = unit_x + move_map_odd[move_value][0];
			new_y = unit_y + move_map_odd[move_value][1];
		}
	}
	else
	{
		new_x = unit_x + move_map_ortho[move_value][0];
		new_y = unit_y + move_map_ortho[move_value][1];
	}
}

float MapManager::getWalkCost(int xFrom, int yFrom, int xTo, int yTo, void* userData) const
{
	int* mapID_ptr = (int*)userData;
	int mapID = *mapID_ptr;
	if (mapID != -1)
	{
		int x = xTo - xFrom;
		int y = yTo - yFrom;

		// unwalkable cells get closed off automatically
		if (!mapStore[mapID]->map->isWalkable(xTo, yTo))
		{
			return -1.0f;
		}

		// TODO: modify by content movement modifier
		float baseCost = 1.0f;

		if (mapStore[mapID]->outdoor)
		{
			// if the value is in the hex move map, we can move there, otherwise we can't
			bool odd = !yFrom & 0x1;
			bool found = false;
			for (int i = 0; i < 6; i++)
			{
				if (odd)
				{
					if (move_map_odd[i][0] == x && move_map_odd[i][1] == y)
					{
						found = true;
					}
				}
				else
				{
					if (move_map_even[i][0] == x && move_map_even[i][1] == y)
					{
						found = true;
					}
				}
			}
			return found ? baseCost : -1.0f;
		}
		else
		{
			// if neither value is 0, this is a diagonal
			if (x != 0 && y != 0)
			{
				return sqrt(2) * baseCost;
			}
			else
			{
				return baseCost;
			}
		}
	}
	else
	{
		return 1.0f;
	}
}

void MapManager::connectMaps(int map1, int map2, int x1, int y1, int x2, int y2)
{
	Map* m1 = mapStore[map1];
	Map* m2 = mapStore[map2];

	// work out what kind of transition this is
	int type = 0;
	if(m1->outdoor && m2->outdoor)
	{
		type = CONTENT_TRANSITION_ZONE;
	}
	else if(!m1->outdoor && !m2->outdoor)
	{
		type = CONTENT_TRANSITION_STAIRS;
	}
	else
	{
		type = CONTENT_TRANSITION_DOOR;
	}

	// connect map1 to map2
	m1->map->setProperties(x1, y1, true, true);
	m1->setContent(x1, y1, type);
	m1->setTransition(x1, y1, map2);

	m1->reverse_transition_mapindex.push_back(map2);
	m1->reverse_transition_xpos.push_back(x1);
	m1->reverse_transition_ypos.push_back(y1);

	// connect map2 to map1
	m2->map->setProperties(x2, y2, true, true);
	m2->setContent(x2, y2, type);
	m2->setTransition(x2, y2, map1);

	m2->reverse_transition_mapindex.push_back(map1);
	m2->reverse_transition_xpos.push_back(x2);
	m2->reverse_transition_ypos.push_back(y2);
}

Map* MapManager::mapFromText(std::vector<std::string> hmap, bool outdoor)
{
	int index = buildMapFromText(hmap, outdoor);
	return mapStore[index];
}

void MapManager::renderRegionMap(TCODConsole* sampleConsole, int centroid_x, int centroid_y)
{
	
	int map_width = OUTDOOR_MAP_WIDTH;
	int map_height = OUTDOOR_MAP_HEIGHT;

	TCODColor baseColor = TCODColor::lighterGrey;

	// draw the hex map
	for (int y = 0; y < map_height; y++)
	{
		bool stepped = y & 0x1;
		for (int x = 0; x < map_width; x++)
		{
			int render_x = x * 2;
			int render_y = y * 2;
			render_x += (stepped) ? 1 : 0;

			int terrain = regionMap->getTerrain(x, y);
			
			if (terrain == TERRAIN_PLAINS)
			{
				//sampleConsole.setCharBackground(render_x, y, lightWall, TCOD_BKGND_SET);
				sampleConsole->putChar(render_x, render_y, '.', TCOD_BKGND_NONE);
			}
			else if (terrain == TERRAIN_MOUNTAIN)
			{
				//sampleConsole.setCharBackground(render_x, y, lightTree, TCOD_BKGND_SET);
				sampleConsole->putChar(render_x, render_y, '^', TCOD_BKGND_NONE);
			}
			else if (terrain == TERRAIN_HILLS)
			{
				sampleConsole->putChar(render_x, render_y, '~', TCOD_BKGND_NONE);
			}
			else if (terrain == TERRAIN_FOREST)
			{
				sampleConsole->putChar(render_x, render_y, '*', TCOD_BKGND_NONE);
			}
			else if (terrain == TERRAIN_JUNGLE)
			{
				sampleConsole->putChar(render_x, render_y, '&', TCOD_BKGND_NONE);
			}
			else if (terrain == TERRAIN_SWAMP)
			{
				sampleConsole->putChar(render_x, render_y, 's', TCOD_BKGND_NONE);
			}
			else if (terrain == TERRAIN_DESERT)
			{
				sampleConsole->putChar(render_x, render_y, '_', TCOD_BKGND_NONE);
			}
			else
			{
				//sampleConsole.setCharBackground(render_x, y, lightGround, TCOD_BKGND_SET);
				sampleConsole->putChar(render_x, render_y, ' ', TCOD_BKGND_NONE);
			}

			int base = regionMap->getBase(x, y);

			if(base != -1)
			{
				// we should only show bases if they are not also a Site (we will overwrite it)
				// IDEA: flip between indicators if eg a Camp is in the same hex as a Dungeon
				sampleConsole->putChar(render_x, render_y, TCOD_CHAR_RADIO_SET, TCOD_BKGND_NONE);
			}

			int site = regionMap->getSite(x, y);

			if (site == SITE_DUNGEON)
			{
				sampleConsole->putChar(render_x, render_y, TCOD_CHAR_DCROSS, TCOD_BKGND_NONE);
			}
		}
	}
}

void MapManager::renderMap(TCODConsole* sampleConsole, int index, int centroid_x, int centroid_y)
{
	Map* map = mapStore[index];
	bool outdoor = map->outdoor;

	int map_width = outdoor ? (SAMPLE_SCREEN_WIDTH / 2) : SAMPLE_SCREEN_WIDTH;
	int map_height = outdoor ? (SAMPLE_SCREEN_HEIGHT / 2) : SAMPLE_SCREEN_HEIGHT;

	TCODColor baseColor = TCODColor::lighterGrey;

	int screen_map_x0 = centroid_x - int(map_width / 2);
	int screen_map_x1 = screen_map_x0 + map_width;

	if (screen_map_x0 < 0)
	{
		screen_map_x1 -= (screen_map_x0);
		screen_map_x0 = 0;
	}

	if (screen_map_x1 > map->width)
	{
		screen_map_x0 -= (screen_map_x1 - map->width);
		screen_map_x1 = map_width;
	}

	int screen_map_y0 = centroid_y - int(map_height / 2);
	int screen_map_y1 = screen_map_y0 + map_height;

	if (screen_map_y0 < 0)
	{
		screen_map_y1 -= (screen_map_y0);
		screen_map_y0 = 0;
	}

	if (screen_map_y1 > map->height)
	{
		screen_map_y0 -= (screen_map_y1 - map->height);
		screen_map_y1 = map->height;
	}

	// draw the hex map
	for (int y = screen_map_y0; y < screen_map_y1; y++)
	{
		bool stepped = y & 0x1;
		for (int x = screen_map_x0; x < screen_map_x1; x++)
		{
			int render_x = outdoor ? (x - screen_map_x0) * 2 : (x - screen_map_x0);
			int render_y = outdoor ? (y - screen_map_y0) * 2 : (y - screen_map_y0);
			render_x += (outdoor && stepped) ? 1 : 0;

			int content = map->getContent(x, y);
			auto items = map->getItems(x, y);
			
			bool visible = map->map->isInFov(x, y);
			
			if (!visible)
			{
				//sampleConsole.setCharBackground(x, y, darkGround, TCOD_BKGND_SET);
			}
			else
			{
				if (!items->empty())
				{
					int itemID = items->top();
					std::string s = gGame->mItemManager->getVisual(itemID);
					int c = s[0];
					sampleConsole->putChar(render_x, render_y, c, TCOD_BKGND_NONE);
				}
				else
				{
					if (content == CONTENT_ROCKS)
					{
						//sampleConsole.setCharBackground(render_x, y, lightWall, TCOD_BKGND_SET);
						sampleConsole->putChar(render_x, render_y, 'X', TCOD_BKGND_NONE);
					}
					else if (content == CONTENT_TREE)
					{
						//sampleConsole.setCharBackground(render_x, y, lightTree, TCOD_BKGND_SET);
						sampleConsole->putChar(render_x, render_y, TCOD_CHAR_ARROW_N, TCOD_BKGND_NONE);
					}
					else if (content == CONTENT_WALL)
					{
						sampleConsole->putChar(render_x, render_y, '#', TCOD_BKGND_NONE);
					}
					else if (content == CONTENT_TRANSITION_STAIRS)
					{
						sampleConsole->putChar(render_x, render_y, '>', TCOD_BKGND_NONE);
					}
					else if (content == CONTENT_TRANSITION_DOOR)
					{
						sampleConsole->putChar(render_x, render_y, 'I', TCOD_BKGND_NONE);
					}
					else if (content == CONTENT_TRANSITION_ZONE)
					{
						sampleConsole->putChar(render_x, render_y, '%', TCOD_BKGND_NONE);
					}
					else
					{
						//sampleConsole.setCharBackground(render_x, y, lightGround, TCOD_BKGND_SET);
						sampleConsole->putChar(render_x, render_y, '.', TCOD_BKGND_NONE);
					}
				}
			}
		}
	}

	// now render the mobs
	for(int mob: map->mobs)
	{
		TCODColor baseColor = TCODColor::lighterGrey;
		Creature& c = gGame->mMobManager->GetMonster(mob);
		std::string s = c.GetVisual();
		int cs = s[0];
		if (c.HasCondition("Unconscious")) baseColor = baseColor * TCODColor::grey;

		renderAtPosition(sampleConsole, index, centroid_x, centroid_y, gGame->mMobManager->GetMobX(mob), gGame->mMobManager->GetMobY(mob), cs, baseColor);
	}
}


void MapManager::renderAtPosition(TCODConsole* sampleConsole, int mapIndex, int centroid_x, int centroid_y, int x, int y, char c, TCODColor foreground)
{
	bool outdoor = true;
	int full_map_height, full_map_width;
	if (mapIndex != -1)
	{
		Map* map = mapStore[mapIndex];
		full_map_height = map->height;
		full_map_width = map->width;
		outdoor = map->outdoor;
	}
	else
	{
		full_map_height = getRegionMap()->height;
		full_map_width = getRegionMap()->width;
	}

	int map_width = outdoor ? (SAMPLE_SCREEN_WIDTH / 2) : SAMPLE_SCREEN_WIDTH;
	int map_height = outdoor ? (SAMPLE_SCREEN_HEIGHT / 2) : SAMPLE_SCREEN_HEIGHT;

	TCODColor baseColor = TCODColor::lighterGrey;

	int screen_map_x0 = centroid_x - int(map_width / 2);
	int screen_map_x1 = screen_map_x0 + map_width;

	if (screen_map_x0 < 0)
	{
		screen_map_x1 -= (screen_map_x0);
		screen_map_x0 = 0;
	}

	if (screen_map_x1 > full_map_width)
	{
		screen_map_x0 -= (screen_map_x1 - full_map_width);
		screen_map_x1 = map_width;
	}

	int screen_map_y0 = centroid_y - int(map_height / 2);
	int screen_map_y1 = screen_map_y0 + map_height;

	if (screen_map_y0 < 0)
	{
		screen_map_y1 -= (screen_map_y0);
		screen_map_y0 = 0;
	}

	if (screen_map_y1 > full_map_height)
	{
		screen_map_y0 -= (screen_map_y1 - full_map_height);
		screen_map_y1 = full_map_height;
	}

	bool stepped = y & 0x1;

	int render_x = outdoor ? (x - screen_map_x0) * 2 : (x - screen_map_x0);
	int render_y = outdoor ? (y - screen_map_y0) * 2 : (y - screen_map_y0);
	render_x += (outdoor && stepped) ? 1 : 0;

	sampleConsole->putCharEx(render_x, render_y, c, foreground, TCODColor::black);
}

bool MapManager::TurnHandler(int entityID, double time)
{
	return false;
}

// used to spawn new items on the map
void MapManager::AddItem(int mapID, int x, int y, std::string item)
{
	int itemID = gGame->mItemManager->GenerateItemFromTemplate(item);
	AddItem(mapID, x, y, itemID);
}

// used to add existing items to the map
void MapManager::AddItem(int mapID, int x, int y, int itemID)
{
	mapStore[mapID]->addItem(x, y, itemID);
}

int MapManager::TakeTopItem(int mapID, int x, int y)
{
	auto is = mapStore[mapID]->getItems(x, y);
	if (is->empty()) return -1;
	int item = is->top();
	is->pop();
	return item;
}

std::string MapManager::ItemDesc(int mapID, int x, int y)
{
	std::string output = "";
	auto is = mapStore[mapID]->getItems(x, y);
	if(is->size() > 1)
	{
		output += "There is a pile of items here.";
	}
	else
	{
		int topItem = is->top();
		std::string desc = gGame->mItemManager->getShortDescription(topItem);

		output += "There is a " + desc + " here.";
	}
	return output;
}

// Calculate time to traverse a single cell on the given map at the given speed
double MapManager::getMovementTime(int mapID, double speed)
{
	int type = MAP_REGION;
	if (mapID != -1)
	{
		type = mapStore[mapID]->mapType;
	}

	if(type == MAP_DUNGEON || type == MAP_WILDERNESS)
	{
		// 5 feet or yard per square. Based on ACKS rules, outdoor yds = indoor ft
		return BASE_SCALE / (speed / 3);
	}
	else if (type == MAP_REGION)
	{
		// 6 miles per hex
		double mapSpeed = speed / 5;
		double dayDistance = mapSpeed * 5280;
		return (6 * dayDistance) / mapSpeed;
	}
}

bool MapManager::isOutOfBounds(int mapID, int x, int y)
{
	bool in = false;

	int map_height = OUTDOOR_MAP_HEIGHT;
	int map_width = OUTDOOR_MAP_WIDTH;

	if(mapID != -1)
	{
		map_width = mapStore[mapID]->width;
		map_height = mapStore[mapID]->height;
	}
	

	if (x < 0 || y < 0) in = true;
	if (x > map_width - 1 || y > map_height - 1) in = true;

	return in;
}

void MapManager::DebugLog(std::string message)
{
	gLog->Log("MapManager", message);
}

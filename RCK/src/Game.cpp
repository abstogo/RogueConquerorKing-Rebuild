#include "Game.h"

Game* gGame;

void Game::StartGame()
{
	// Test game init
	// Fixed square and hex maps, fixed mobs, fixed basic character
	
	// create game managers

	DebugLog("Starting Game");
	
	mCharacterManager = CharacterManager::LoadCharacteristics();
	mClassManager = ClassManager::LoadClasses();
	mMapManager = MapManager::LoadMaps();
	mTimeManager = new TimeManager();
	gLog->Log("Time Manager", "Started");
	mItemManager = ItemManager::LoadItemTemplates();
	mMobManager = MobManager::LoadMobData();
	mConditionManager = ConditionManager::LoadConditions();
	mMortalManager = MortalWoundManager::LoadMortalWoundData();
	mPartyManager = PartyManager::LoadPartyData();
	mBaseManager = BaseManager::LoadBaseData();

	DebugLog("Game Managers Created");

	CreateMenu();
	
	mode = GM_MENU;
}

void Game::QuitGame()
{
	mode = GM_QUIT;
}

void Game::CreateMenu()
{
	menuText.push_back("Start Test Game");
	menuFunctions.push_back(std::bind(&Game::CreateTestGame, this));

	menuText.push_back("Quit");
	menuFunctions.push_back(std::bind(&Game::QuitGame, this));

	menuSelected = 0;
}

void Game::CreateTestGame()
{
	DebugLog("Creating Test Game");

	// starting character to make numbers match (0 = false, 0 = no character on map)
	mCharacterManager->GenerateTestCharacter("NULL", "Fighter");

	currentPartyID = mPartyManager->GenerateAITestParty();
	currentCharacterID = mPartyManager->getNextPlayerCharacter(currentPartyID);
	currentBaseID = -1;

	std::vector<std::string> indoorMap = {
		"##############################################",
		"#.................#..........................#",
		"#.................#..........................#",
		"#.................#..........................#",
		"#.................#..........................#",
		"#............................................#",
		"#.................#..........................#",
		"#.................#..........................#",
		"#.................#..........................#",
		"#.................#..........................#",
		"#.................#..........................#",
		"#########.#########..........................#",
		"#.................#..........................#",
		"#.................#..........................#",
		"#.................#..........................#",
		"#.................#..........................#",
		"#.................#..........................#",
		"#.................#..........................#",
		"#.................#..........................#",
		"##############################################",
	};
	int indoorMapID = mMapManager->buildMapFromText(indoorMap, false);

	std::vector<std::string> regionMap = {
		". . . . . . . . . ~ ^ ^ ~ . . . . . . . . . . ",
		" . . . . . . . . . ~ ^ ~ . . . . . . . . . . .",
		". . * * * . . . . ~ ~ ^ ~ . . . . . . * . . . ",
		" . . * * * * . . . ~ ^ ~ . . . . . . . . . . .",
		". . * * * . . . . ~ ~ ~ . . . . . . . . . . . ",
		" . . . * . . . . . . . . . . . . . . . * . . .",
		". . . . . . . . . . . . . . . . . . . . . . . ",
		" s s s . . . . . . . . . . . . . . . . . . . .",
		"s s s . * . . . . . . . . . . . . . . . * . . ",
		" s s s . . . . . . . . . . * * . . . . . . . .",
	};
	mMapManager->BuildRegionMapFromText(regionMap);
	
	std::vector<std::string> outdoorMap = {
		". . . . . . . . . . . . . . . . . . . . . . . ",
		" . . . . . . . . . . . . . . . . . . . . . . .",
		". . . . . . . . . . . . . . . . . . . T . . . ",
		" . . . . . . . . # # . . . . . . . . . . . . .",
		". . . . . . . . . # # # . . . . . . T T . . . ",
		" . . . . . . . # # # # . . . . . T T T T . . .",
		". . . . . . . . # # # . . . . . . . . . . . . ",
		" . . . . . . . . # . . . . . . . . . . . . . .",
		". . . . T . . . . . . . . . . . . . . . T . . ",
		" . . . . . . . . . . . . . T T . . . . . . . .",
	};
	int outdoorMapID = mMapManager->GenerateMapFromPrefab(8,6,outdoorMap,SITE_DUNGEON);

	mPartyManager->SetPartyX(currentPartyID,8);
	mPartyManager->SetPartyY(currentPartyID, 6);

	mMapManager->connectMaps(outdoorMapID, indoorMapID, 3, 3, 8, 15);

	//recomputeFov = true;
	//light_walls = true;

	// add a sword to test character
	int testItem = mItemManager->GenerateItemFromTemplate("Sword");
	int inventoryID = mCharacterManager->AddInventoryItem(currentCharacterID, testItem);
	mCharacterManager->EquipItem(currentCharacterID, inventoryID);

	// add a bow to test character
	int bowItem = mItemManager->GenerateItemFromTemplate("Shortbow");
	int newItemID = mCharacterManager->AddInventoryItem(currentCharacterID, bowItem);
	//mCharacterManager->EquipItem(currentCharacterID, newItemID);

	// drop some items in test map
	mMapManager->AddItem(outdoorMapID, 2, 2, "Chainmail");
	mMapManager->AddItem(outdoorMapID, 2, 3, "Shield");

	// add the test goblins to the test map
	int testGoblinID = mMobManager->GenerateMonster("Goblin", outdoorMapID, 3, 2);;
	int testGoblin2ID = mMobManager->GenerateMonster("Goblin", outdoorMapID, 3, 4);
	int testGoblin3ID = mMobManager->GenerateMonster("Goblin", outdoorMapID, 4, 5);

	// standing level spawner
	SpawnLevel(outdoorMapID, 14, 3);

	// test mortal wounds
	int d20_roll = randomiser->diceRoll("1d20");
	int d6_roll = randomiser->diceRoll("1d6");

	MortalRollResult* result = mMortalManager->RollMortalWound(d20_roll, d6_roll);

	DebugLog("Test Mortal Wound:");
	DebugLog("D20/D6:" + std::to_string(d20_roll) + "/" + std::to_string(d6_roll));
	DebugLog("Results:" + result->effect->PlayerText());

	delete result;

	mode = GM_MAIN;
}

void Game::ClearGame()
{
	
}

void Game::SpawnLevel(int mapID, int spawnPointX, int spawnPointY)
{
	DebugLog("SPAWNING LEVEL #" + std::to_string(mapID));
	
	currentMapID = mapID;
	currentMap = mMapManager->getMap(currentMapID);

	bool outdoor = currentMap->outdoor;
	DebugLog("Level is " + outdoor ? "outdoor." : "indoor.");

	DebugLog("Spawning PCs");
	const int MAX_SPAWN_DIST = 255;
	int dist = 1;
	int dist_mult = outdoor ? 6 : 8;
	// arrange the relevant characters around the entry point. We care about PCs, henches, and animals.
	std::vector<int> pcs =  mPartyManager->getPlayerCharacters(currentPartyID);
	for (int pc_id : pcs)
	{
		if (pc_id == currentCharacterID)
		{
			// the currently controlled character spawns on the spawn point
			mCharacterManager->SetPlayerX(currentCharacterID, spawnPointX);
			mCharacterManager->SetPlayerY(currentCharacterID, spawnPointY);
			mCharacterManager->SetPlayerMap(currentCharacterID, mapID);
			currentMap->setCharacter(spawnPointX, spawnPointY, currentCharacterID);
			
			DebugLog("Spawning player onto position (" + std::to_string(spawnPointX) + "," + std::to_string(spawnPointY) + ")");
		}
		else
		{
			mCharacterManager->SpawnOnMap(pc_id, mapID, spawnPointX, spawnPointY);
		}
	}

	DebugLog("Spawning Henches");
	std::vector<int> henches = mPartyManager->getHenchmen(currentPartyID);
	for (int h_id : henches)
	{
		mCharacterManager->SpawnOnMap(h_id, mapID, spawnPointX, spawnPointY);
	}

	DebugLog("Spawning Animals");
	std::vector<int> animals = mPartyManager->getAnimals(currentPartyID);
	for (int a_id : animals)
	{
		// everyone else is randomly placed around
		if (outdoor || !mMobManager->GetMonster(a_id).HasAbility("NoDungeon"))
		{
			mMobManager->SpawnOnMap(a_id, currentMapID, spawnPointX, spawnPointY);
		}
	}
}

bool Game::MenuGameHandleKeyboard(TCOD_key_t* key)
{
	if (key->vk == TCODK_NONE)
	{
		return false;
	}

	if (key->vk == TCODK_UP)
	{
		menuSelected--;
		if (menuSelected < 0)
		{
			menuSelected = menuText.size() - 1;
		}
	}
	else if (key->vk == TCODK_DOWN)
	{
		menuSelected++;
		if (menuSelected == menuText.size())
		{
			menuSelected = 0;
		}
	}
	else if (key->vk == TCODK_ENTER)
	{
		menuFunctions[menuSelected]();
	}

	return true;
}

bool Game::MainGameHandleKeyboard(TCOD_key_t* key)
{
	// returns true if anything has changed, false if it has not

	int player_x, player_y;
	
	if (currentMapID != -1)
	{
		player_x = mCharacterManager->GetPlayerX(currentCharacterID);
		player_y = mCharacterManager->GetPlayerY(currentCharacterID);
	}
	else
	{
		player_x = mPartyManager->GetPartyX(currentPartyID);
		player_y = mPartyManager->GetPartyY(currentPartyID);
	}
	
	if(key->vk == TCODK_NONE)
	{
		return false;
	}

	// Mode keys! These keys switch between the game's modal windows.
	// Currently these are Inventory, Character, Ability and Spells
	if(key->c == 'v')
	{
		if (mode != GM_INVENTORY)
		{
			mode = GM_INVENTORY;
		}
		else
		{
			mode = GM_MAIN;
		}
	}

	if (key->c == 'c')
	{
		if (mode != GM_CHARACTER)
		{
			mode = GM_CHARACTER;
		}
		else
		{
			mode = GM_MAIN;
		}
	}

	// "a"/"A" is the "ability" button and opens the Character Ability window to use class abilities and proficiencies (where they have discrete activations rather than implicit bonuses etc)
	if (key->c == 'a')
	{
		/*
		if (mode != GM_ABILITY)
		{
			mode = GM_ABILITY;
		}
		else
		{
			mode = GM_MAIN;
		}
		*/
	}

	// "s"/"S" is the "spell" button and opens the Spells window to allow for casting spells
	if (key->c == 's')
	{
		/*
		if (mode != GM_SPELL)
		{
			mode = GM_SPELL;
		}
		else
		{
			mode = GM_MAIN;
		}
		*/
	}

	// "d"/"D" is the "domain" button and opens the Domain window to control camps/settlements/domains.
	// Note this only intended on the Region map currently (and automatically triggers if you enter a settlement)
	// The plan is to have camps/settlements etc explorable eventually, so the Domain mode will trigger the interface while inside them
	if (key->c == 'd')
	{
		if (currentMapID == -1)
		{
			if (mode != GM_DOMAIN)
			{
				mode = GM_DOMAIN;
			}
			else
			{
				mode = GM_MAIN;
			}
		}
	}
	
	
	// ACTION SYSTEM
	// Basic idea is that there are a small number of controls (because the hex/ortho transition is complicated enough)
	// Each control is largely contextual - so movement keys towards an enemy is an attack, toward a non-enemy is a discussion, etc.

	switch (mode)
	{
		case GM_MAIN:
		{

			if (currentMapID == -1)
			{
				// '>' is 'go in'. I use only one button to go up/down etc. The other button is used for region transition
				if (key->c == '.' && key->shift)
				{
					// we're on the region map, so we head 'down' into the local wilderness map
					// this automatically generates a local wilderness map if one does not exist
					int mapID = mMapManager->GetMapAtLocation(player_x, player_y);
					currentMapID = mapID;
					currentMap = mMapManager->getMap(mapID);

					SpawnLevel(mapID, OUTDOOR_MAP_WIDTH / 2, OUTDOOR_MAP_HEIGHT / 2);

					recomputeFov = true;

					return true;
				}

				// "." is the traditional "do nothing" button. In our case, it advances time by 1 hour and returns true.
				if (key->c == '.' && !key->shift)
				{
					double time = 3600.0;
					mTimeManager->AdvanceTimeBy(time);
				}

				if (HandleHexKeyboard(key))
				{
					return true;
				}
			}
			else
			{
				// party dump (can also use in select mode to pick dumps)
				if (key->c == '`')
				{
					mPartyManager->DumpParty(currentPartyID);
				}

				// "." is the traditional "do nothing" button. In our case, it advances time a bit and returns true.
				if (key->c == '.' && !key->shift)
				{
					double time = mMapManager->getMovementTime(currentMapID, mCharacterManager->GetCurrentSpeed(currentCharacterID));
					mTimeManager->AdvanceTimeBy(time);
				}

				// "," is the "pick up" button
				if (key->c == ',' && !key->shift)
				{
					int p = mMapManager->TakeTopItem(currentMapID, player_x, player_y);
					if (p != -1)
					{
						mCharacterManager->AddInventoryItem(currentCharacterID, p);
						AddActionLogText(mCharacterManager->getCharacterName(currentCharacterID) + " picks up a " + mItemManager->getShortDescription(p) + ".");
					}
				}

				// 't'/'T' triggers a missile attack with the current wielded missile or thrown weapon.
				if (key->c == 't')
				{
					int wieldedID = mCharacterManager->GetItemInEquipSlot(currentCharacterID, HAND_MAIN);
					if (wieldedID != -1)
					{
						bool missile = mItemManager->hasTag(wieldedID, "Missile");

						// just ignore this press if we're not using a missile weapon of some kind
						if (missile)
						{
							// set cleave count for current character (remember this is reset if we change, balanced by change delay)
							remainingCleaves = mCharacterManager->GetCleaveCount(currentCharacterID);

							// now select a target
							int range = mItemManager->getMaxRange(wieldedID) / 5;
							int x = mCharacterManager->GetPlayerX(currentCharacterID);
							int y = mCharacterManager->GetPlayerY(currentCharacterID);
							//std::vector<int> monstersInRange = mMobManager->GetAllMonstersInRange(currentMapID, x, y, range, true, true);
							std::vector<int> monsters = mMobManager->GetAllEnemyMonstersOnMap(currentMapID, currentPartyID, true);
							std::vector<int> monstersInView = mMapManager->filterByFOV(MANAGER_CHARACTER, currentCharacterID, MANAGER_MOB, monsters, range);
							if (monstersInView.size() > 0)
								TriggerTargeting(TARGET_CREATURE, -1, 1, range, 1, false, true, monstersInView);
						}
					}
				}

				// Tab switches active character
				if (key->vk == TCODK_TAB)
				{
					int shiftCharacterID = mPartyManager->getNextPlayerCharacter(currentPartyID, currentCharacterID);
					if (shiftCharacterID != -1)
					{
						mCharacterManager->SetBehaviour(currentCharacterID, CHAR_BEHAVIOUR_UNSET);
						mTimeManager->SetEntityTime(currentCharacterID, MANAGER_CHARACTER, 0.01);
						mCharacterManager->SetBehaviour(shiftCharacterID, CHAR_BEHAVIOUR_UNSET);

						DebugLog("Switching to character " + std::to_string(shiftCharacterID));
						currentCharacterID = shiftCharacterID;
						player_x = mCharacterManager->GetPlayerX(currentCharacterID);
						player_y = mCharacterManager->GetPlayerY(currentCharacterID);

						recomputeFov = true;
					}
					else
					{
						DebugLog("Tab pressed but no valid character available.");
					}
				}

				// 'l'/'L' triggers Look Mode
				if (key->c == 'l')
				{
					TriggerTargeting(TARGET_CELL, -1, 0);
					return false;
				}

				// '>' is 'go in'. I use only one button to go up/down etc. The other button is used for region transition
				if (key->c == '.' && key->shift)
				{
					// check for transition between zones
					if (currentMap->getContent(player_x, player_y) >= CONTENT_TRANSITION_STAIRS)
					{
						// there is a transition here. Look it up
						int targetMapIndex = currentMap->getTransition(player_x, player_y);
						Map* targetMap = mMapManager->getMap(targetMapIndex);

						std::vector<int>::iterator iter = std::find(targetMap->reverse_transition_mapindex.begin(), targetMap->reverse_transition_mapindex.end(), currentMapID);

						// sanity check - is it in the vector?
						if (iter != targetMap->reverse_transition_mapindex.end())
						{
							// get the count
							int i = iter - targetMap->reverse_transition_mapindex.begin();
							player_x = targetMap->reverse_transition_xpos[i];
							player_y = targetMap->reverse_transition_ypos[i];

							currentMapID = targetMapIndex;
							currentMap = targetMap;

							recomputeFov = true;
						}
					}

					return true;
				}

				if (key->c == ',' && key->shift)
				{
					if (currentMap->outdoor)
					{
						// we're on the local wilderness map, so head out to the region map

						// TODO: Check for active enemies, don't allow zooming out if there are any (thank you Skyrim)

						currentMapID = -1;
						currentMap = NULL;
						mTimeManager->DeregisterEntities();
						AddActionLogText("", true); // clear the action log
					}

					return true;
				}

				if (currentMap->outdoor)
				{
					if (HandleHexKeyboard(key))
					{
						return true;
					}
				}
				else
				{
					if (HandleOrthoKeyboard(key))
					{
						return true;
					}
				}
			}
		}
		break;
		
		case GM_INVENTORY:
		{
			if (key->vk == TCODK_UP)
			{
				menuPosition[mode]--;
				if (menuPosition[mode] < 0)
				{
					menuPosition[mode] = mCharacterManager->GetInventory(currentCharacterID).size() - 1;
				}
			}
			else if (key->vk == TCODK_DOWN)
			{
				menuPosition[mode]++;
				if (menuPosition[mode] == mCharacterManager->GetInventory(currentCharacterID).size())
				{
					menuPosition[mode] = 0;
				}
			}
			else if (key->vk == TCODK_ENTER)
			{
				// enter is used to equip and unequip
				if (mCharacterManager->GetEquipSlotForInventoryItem(currentCharacterID, menuPosition[mode]) != -1)
				{
					mCharacterManager->UnequipItem(currentCharacterID, menuPosition[mode]);
				}
				else
				{
					mCharacterManager->EquipItem(currentCharacterID, menuPosition[mode]);
				}
			}
		}
		break;

		case(GM_TARGET):
		{
			if (currentMap->outdoor)
			{
				if (HandleHexKeyboard(key))
				{
					return true;
				}
			}
			else
			{
				if (HandleOrthoKeyboard(key))
				{
					return true;
				}
			}
				
			switch (targetMode)
			{
				case(TARGET_CREATURE):
				{
					if (key->vk == TCODK_ENTER)
					{
						// TargetingReturn(targetIDs[targetIndex], targetingData[3], targetingData[1]);
						int managerID = targetingData[0];
						int returnCode = targetingData[1];
						std::vector<int> entityIDs = GetTargetedEntities();

						bool completion = true;

						switch (managerID)
						{
							case MANAGER_GAME:
							{
								for (int entityID : entityIDs)
									if (!gGame->TargetHandler(entityID, returnCode)) completion = false;
							}
							break;
							case MANAGER_CHARACTER:
							{
								for(int entityID : entityIDs)
									if (!gGame->mCharacterManager->TargetHandler(entityID, returnCode)) completion = false;
							}
							break;
							case MANAGER_MOB:
							{
								for (int entityID : entityIDs)
									if (!gGame->mMobManager->TargetHandler(entityID, returnCode)) completion = false;
							}
							break;

							case MANAGER_MAP:
							{
								for (int entityID : entityIDs)
									if (!gGame->mMapManager->TargetHandler(entityID, returnCode)) completion = false;
							}
							break;

							case MANAGER_ITEM:
							{
								// gGame->mItemManager->TurnHandler(*ent_iter, *time_iter);
							}
							break;
						}
					}
				}
				break;

				case(TARGET_CELL):
				{
					// debug dump button
					if (key->c == '`')
					{
						auto items = mMapManager->getMap(currentMapID)->getItems(targetCursorX, targetCursorY);
						if (items->size() > 0)
						{
							//for (int entityID : items)
							//	gGame->mItemManager->DumpItem(entityID);
						}

						int mobID = currentMap->getMobAt(targetCursorX, targetCursorY);
						if (mobID != 0)
						{
							gGame->mMobManager->DumpMob(mobID);
						}

						int charID = currentMap->getCharacterAt(targetCursorX, targetCursorY);
						if (charID != 0)
						{
							gGame->mCharacterManager->DumpCharacter(charID);
						}
					}
					if (key->vk == TCODK_ENTER)
					{
						//TargetingReturn(targetCursorX, targetCursorY, targetingData[3], targetingData[1]);
						TargetHandler(currentCharacterID, 0);
						return true;
					}
				}
				break;
			}
				
		}
		break;

		case(GM_DOMAIN):
		{
			mBaseManager->ControlCommand(key, currentBaseID);
		}
		break;

		case(GM_CHARACTER):
		{
			
		}
		break;
	}

	return false;
}

void Game::TriggerTargeting(int targetingMode, int returnManager, int returnCode, int range, int size, bool allies, bool enemies, std::vector<int>& targets)
{
	mode = GM_TARGET;
	targetMode = targetingMode;
	targetingData.clear();
	targetingData.push_back(returnManager);
	targetingData.push_back(returnCode);
	targetingData.push_back(range);
	int flags = (allies ? TF_FRIEND : 0) | (enemies ? TF_ENEMY : 0);
	targetingData.push_back(flags);
	targetingData.push_back(size);

	targetIDs.clear();
	targetIDs = std::vector<int>(targets);

	if (targetingMode == TARGET_CREATURE)
	{
		targetIndex = 0;
	}
	else if (targetingMode == TARGET_CELL)
	{
		targetCursorX = mCharacterManager->GetPlayerX(currentCharacterID);
		targetCursorY = mCharacterManager->GetPlayerY(currentCharacterID);
	}
	
}


bool Game::TargetHandler(int entityID, int returnCode)
{
	// target handler returns true if we're done and should return to normal mode.

	// return code 0: Look/Examine response
	if (returnCode == 0)
	{
		// we only come here if we don't have an Examine set up for the object under the cursor, so return to main mode
		mode = GM_MAIN;
		return true;
	}

	// return code 1: Missile Attack response
	if (returnCode == 1)
	{
		if(ResolveAttacks(MANAGER_CHARACTER, gGame->currentCharacterID, MANAGER_MOB, entityID, true))
			mode = GM_MAIN;
		return true;
	}

	// return code 2: Melee cleave response
	if (returnCode == 2)
	{
		if(ResolveAttacks(MANAGER_CHARACTER, gGame->currentCharacterID, MANAGER_MOB, entityID, false))
			mode = GM_MAIN;
		return true;
	}

	return true;
}

void Game::MoveCharacter(int new_x, int new_y)
{
	// a somewhat modified version of the CharacterManager's "MoveTo" function used for the current character under player control
	if (!gGame->mMapManager->isOutOfBounds(currentMapID, new_x, new_y))
	{
		if (currentMapID != -1)
		{
			if (currentMap->map->isWalkable(new_x, new_y))
			{
				int character = currentMap->getCharacterAt(new_x, new_y);
				if (currentMap->getCharacterAt(new_x, new_y))
				{
					// there's a character there. Check if we want to attack them
					//if (c.IsHostile())
					//{
						// close-quarters attack!
					//}
					//else
					//{
						// we don't want to attack, so we just don't move
					//}
					//

					// if we're not attacking, check if the character is blocking the way
					if (mCharacterManager->getCharacterHasCondition(character, "Unconscious"))
					{
						// unconscious character. Check if they have an unresolved injury
						if (mCharacterManager->getCharacterHasCondition(character, "Injured"))
						{
							if (mCharacterManager->getCharacterCapabilityFlag(currentCharacterID, "TreatWounds"))
							{
								std::string charName = mCharacterManager->getCharacterName(character);
								DebugLog("Performing mortal wounds check on " + charName + ".");
								std::string text2 = mCharacterManager->getCharacterName(currentCharacterID) + " checks " + charName + "'s wounds.";
								AddActionLogText(text2);
								// unresolved injury. Calculate modifiers for roll
								// TODO: Account for time since injury. Probably replace "Injured" condition.
								// Or track time since condition? Useful for temporary conditions!
								int bonus = mCharacterManager->getCharacterAbilityBonus(character, "Constitution");
								int hp = mCharacterManager->getCharacterCurrentHitPoints(character);
								if (hp == 0)
								{
									bonus += 5;
								}
								else
								{
									int hp_floor = -mCharacterManager->getCharacterTotalHitPoints(character);
									if ((hp <= (hp_floor / 4)) && (hp > hp_floor / 2))
									{
										bonus -= 2;
									}
									else if (hp <= (hp_floor / 2))
									{
										bonus -= 5;
									}
								}
								bonus += mCharacterManager->getTagValue(currentCharacterID, "Healing:MortalWound:Bonus");

								DebugLog("Severity roll at " + std::to_string(bonus));
								// test mortal wounds
								int d20_roll = randomiser->diceRoll("1d20") + bonus;
								int d6_roll = randomiser->diceRoll("1d6");

								DebugLog("D20/D6:" + std::to_string(d20_roll) + "/" + std::to_string(d6_roll));

								MortalRollResult* result = mMortalManager->RollMortalWound(d20_roll, d6_roll);

								std::string text = result->effect->PlayerText();

								text.replace(text.find("%1%"), sizeof("%1%") - 1, charName);

								DebugLog("Results:" + text);

								AddActionLogText(text);

								mCharacterManager->AddMortalEffect(character, result->effect);
								// injury is resolved now (one way or the other). If we get attacked again, we set this again.
								mCharacterManager->RemoveCondition(character, "Injured");

								// there are 3 possible wound states: Recover, Wounded (ie Dying) or Dead.
								if (result->status == "Recover")
								{
									// character gets back up; remove Injured and Unconscious
									mCharacterManager->RemoveCondition(character, "Unconscious");
								}

								if (result->status == "Wounded")
								{
									std::string p = "They are wounded and will die within 1 " + result->recovery + " without healing.";
									AddActionLogText(p);

									long double time = 0.0L;
									if (result->recovery == "Round") time = TimeManager::GetTimePeriodInSeconds(TIME_ROUND);
									if (result->recovery == "Turn") time = TimeManager::GetTimePeriodInSeconds(TIME_TURN);
									if (result->recovery == "Day") time = TimeManager::GetTimePeriodInSeconds(TIME_DAY);

									mCharacterManager->SetCondition(character, "Dying", time);
								}

								if (result->status == "Dead")
								{
									CharacterDeath(character);
								}
								else
								{
									// if the character isn't dead, check for bed rest requirements
									if (result->bedRest > 0)
									{
										mCharacterManager->SetCondition(character, "Recovering", TimeManager::GetTimePeriodInSeconds(TIME_DAY) * result->bedRest);
									}
								}
							}
						}
					}
				}
				else if (currentMap->getMobAt(new_x, new_y))
				{
					// there's a monster there
					int c_id = currentMap->getMobAt(new_x, new_y);
					Creature& c = gGame->mMobManager->GetMonster(c_id);

					if (c.IsBlocking())
					{
						// set cleave count for current character (remember this is reset if we change, balanced by change delay)
						remainingCleaves = mCharacterManager->GetCleaveCount(currentCharacterID);

						if (c.IsHostile())
						{
							// the monster is hostile, automatically attack
							if (ResolveAttacks(MANAGER_CHARACTER, gGame->currentCharacterID, MANAGER_MOB, c_id, false))
								mode = GM_MAIN;
						}
						else
						{
							// TODO: Options controls for non-hostiles (allow/confirm/deny, currently only confirm)
							if (!mPartyManager->IsInParty(currentPartyID, MANAGER_MOB, c_id))
							{
								if (hostilifying)
								{
									AddActionLogText(c.GetName() + " is now hostile.");
									c.SetHostile(true);
									if (ResolveAttacks(MANAGER_CHARACTER, gGame->currentCharacterID, MANAGER_MOB, c_id, false))
										mode = GM_MAIN;
									hostilifying = false;
								}
								else
								{
									AddActionLogText("Are you sure? " + c.GetName() + " is not hostile. Attack again to confirm.");
									hostilifying = true;
								}
							}
						}
					}
					else
					{
						// not attacking anything, so calm down
						hostilifying = false;

						// there isn't another creature there, so move
						mCharacterManager->SetPlayerX(currentCharacterID, new_x);
						mCharacterManager->SetPlayerY(currentCharacterID, new_y);
						recomputeFov = true;

						UpdateLookText(new_x, new_y);
						double time = mMapManager->getMovementTime(currentMapID, mCharacterManager->GetCurrentSpeed(currentCharacterID));
						mTimeManager->AdvanceTimeBy(time);
					}

				}
				else
				{
					// not attacking anything, so calm down
					hostilifying = false;

					// there isn't another creature there, so move
					mCharacterManager->SetPlayerX(currentCharacterID, new_x);
					mCharacterManager->SetPlayerY(currentCharacterID, new_y);
					recomputeFov = true;

					UpdateLookText(new_x, new_y);
					double time = mMapManager->getMovementTime(currentMapID, mCharacterManager->GetCurrentSpeed(currentCharacterID));
					mTimeManager->AdvanceTimeBy(time);
				}
			}
		}
		else
		{
			if (mMapManager->getRegionMap()->map->isWalkable(new_x, new_y))
			{
				mPartyManager->SetPartyX(currentPartyID, new_x);
				mPartyManager->SetPartyY(currentPartyID, new_y);
				
				//UpdateLookText(new_x, new_y);
				double time = mMapManager->getMovementTime(currentMapID, mCharacterManager->GetCurrentSpeed(currentCharacterID));
				mTimeManager->AdvanceTimeBy(time);

				int baseID = mBaseManager->GetBaseAt(new_x, new_y);
				if (baseID != -1)
				{
					// we just moved onto a base
					if (mBaseManager->GetBaseOwner(baseID) == currentPartyID)
					{
						// and it's ours
						currentBaseID = baseID;
					}
				}
				else
				{
					int partyID = mPartyManager->GetPartyAt(new_x, new_y);
					if (partyID != -1)
					{
						// we just moved onto a party
					}
				}

				

			}
		}
	}
}

bool Game::HexKeyboardMove(int move_value)
{
	// split off from HandleHexKeyboard as we need to handle multiple contexts
	int player_x, player_y;
	if(currentMapID != -1)
	{
		player_x = mCharacterManager->GetPlayerX(currentCharacterID);
		player_y = mCharacterManager->GetPlayerY(currentCharacterID);
	}
	else
	{
		player_x = mPartyManager->GetPartyX(currentPartyID);
		player_y = mPartyManager->GetPartyY(currentPartyID);
	}
	int new_x, new_y;
	
	mMapManager->shift(currentMapID, new_x, new_y, player_x, player_y, move_value);

	MoveCharacter(new_x, new_y);

	return true;
}

bool Game::HandleHexKeyboard(TCOD_key_t* key)
{
	// return true if we had a move-action here, false if we didn't

	int move_value = -1;

	if (key->c == '8')
	{
		move_value = HEX_LEFTUP;
	}

	if (key->c == '9')
	{
		move_value = HEX_RIGHTUP;
	}

	if (key->c == 'U' || key->c == 'u')
	{
		move_value = HEX_LEFT;
	}

	if (key->c == 'O' || key->c == 'o')
	{
		move_value = HEX_RIGHT;
	}

	if (key->c == 'J' || key->c == 'j')
	{
		move_value = HEX_LEFTDOWN;
	}

	if (key->c == 'K' || key->c == 'k')
	{
		move_value = HEX_RIGHTDOWN;
	}

	if (move_value != -1)
	{
		if(mode == GM_MAIN)
			return HexKeyboardMove(move_value);

		if(mode == GM_TARGET)
		{
			return HexKeyboardTarget(move_value);
		}
	}

	return false;
}

bool Game::HexKeyboardTarget(int move_value)
{
	switch(targetMode)
	{
		case TARGET_CREATURE:
		{
			// creatures are treated as a list. As a result, we translate hex to ortho and send it there.
			return OrthoKeyboardTarget(HexToOrtho(move_value));
		}
		break;

		case TARGET_CELL:
		{
			// cell cursor needs to be moved around directly, so we treat this similarly to a move command
			int new_x, new_y;
			mMapManager->shift(currentMapID, new_x, new_y, targetCursorX, targetCursorY, move_value);
			targetCursorX = new_x;
			targetCursorY = new_y;

			UpdateLookText(targetCursorX, targetCursorY);
		}
		break;
		
	}

	return false;
}

bool Game::OrthoKeyboardTarget(int move_value)
{
	switch (targetMode)
	{
		case TARGET_CREATURE:
		{
			if (move_value == ORTHO_UP)
			{
				// move between targetIDs
				targetIndex++;
				if (targetIndex > targetIDs.size()-1) targetIndex = 0;
			}
			else if (move_value == ORTHO_DOWN)
			{
				targetIndex--;
				if (targetIndex < 0) targetIndex = targetIDs.size()-1;
			}
		}
		break;

		case TARGET_CELL:
		{
			// cell cursor needs to be moved around directly, so we treat this similarly to a move command
			int new_x, new_y;
			mMapManager->shift(currentMapID, new_x, new_y, targetCursorX, targetCursorY, move_value);
			targetCursorX = new_x;
			targetCursorY = new_y;

			UpdateLookText(targetCursorX, targetCursorY);
		}
		break;
	}
	
	return false;
}

int Game::HexToOrtho(int input)
{
	// Translates HEX_ move codes to ORTHO_ move codes.
	// Can't do this in reverse simply as we need to move upleft/upright etc
	// TODO: OrthoToHex

	if (input == HEX_LEFT)
		return ORTHO_LEFT;

	if (input == HEX_RIGHT)
		return ORTHO_RIGHT;

	if (input == HEX_LEFTUP || input == HEX_RIGHTUP)
		return ORTHO_UP;

	if (input == HEX_LEFTDOWN || input == HEX_RIGHTDOWN)
		return ORTHO_DOWN;

	return -1;
}

bool Game::OrthoKeyboardMove(int move_value)
{
	int player_x = mCharacterManager->GetPlayerX(currentCharacterID);
	int player_y = mCharacterManager->GetPlayerY(currentCharacterID);

	int new_x, new_y;

	mMapManager->shift(currentMapID, new_x, new_y, player_x, player_y, move_value);

	MoveCharacter(new_x, new_y);

	return true;
}

bool Game::HandleOrthoKeyboard(TCOD_key_t* key)
{
	int move_value = -1;

	if (key->c == 'U' || key->c == 'u')
	{
		move_value = ORTHO_UP;
	}

	if (key->c == 'K' || key->c == 'k')
	{
		move_value = ORTHO_RIGHT;
	}

	if (key->c == 'J' || key->c == 'j')
	{
		move_value = ORTHO_DOWN;
	}

	if (key->c == 'H' || key->c == 'h')
	{
		move_value = ORTHO_LEFT;
	}

	if (move_value != -1)
	{
		if (mode == GM_MAIN)
			return OrthoKeyboardMove(move_value);

		if (mode == GM_TARGET)
		{
			return OrthoKeyboardTarget(move_value);
		}
	}

	return false;
}

bool Game::ResolveAttacks(int attackerManager, int attackerID, int defenderManager, int defenderID, bool missile)
{
	// can be used for different entity types (mob and character) using managers
	// we'll use this for traps using the map manager later too
	
	// Many monsters have an attack sequence, and any creature can Cleave (up to certain limitations).
	// Attack sequences always go off, and Cleaves go off if we fell an enemy with an attack.
	// Finally, we can have a couple of different kind of magical and non-magical ranged attack, some of which have attack rolls and some don't

	// How do Cleaves interact with multi-attack sequences?
	// Essentially every creature gets a given number of Cleaves each turn, and they can be used on any given attack until they run out.

	int attackerAttackBonus, attackerDamageDieType, attackerDamageDice, attackerDamageBonus, attackerCleaveCount;
	
	switch (attackerManager)
	{
		case MANAGER_CHARACTER:
			{
				Creature& c = mMobManager->GetMonster(defenderID);
				std::string attackText = gGame->mCharacterManager->getCharacterName(attackerID) + " attacks " + c.GetName() + ".";
				gGame->AddActionLogText(attackText);
				
				// PCs/NPCs get only one attack each, barring things like "Haste".
				attackerAttackBonus = mCharacterManager->UpdateCurrentAttackValue(attackerID, missile);

				// retrieve current weapon
				int weaponID = mCharacterManager->GetItemInEquipSlot(attackerID, HAND_MAIN);
				int offhandID = mCharacterManager->GetItemInEquipSlot(attackerID, HAND_OFF);

				if (weaponID != -1)
				{
					if (weaponID != offhandID)
					{
						// if the weapon is in one hand and has the "Grab" tag, the die is d2 (bolas, whips etc)
						// if the weapon is in one hand and has the "Light" tag, the die is d4 (Clubs/Daggers etc)
						// if the weapon is in one hand only, the die is d6 (one handed weapon only)
						attackerDamageDieType = 6;
						if (mItemManager->hasTag(weaponID, "Light")) attackerDamageDieType = 4;
						if (mItemManager->hasTag(weaponID, "Grab")) attackerDamageDieType = 2;
					}
					else
					{
						// if the weapon is in two hands and has the "One-Handed" tag, the die is d8 (bastard weapon wielded in two hands)
						// if the weapon is in two hands and does not have the "One-Handed" tag, the die is d10 (full two-hander)
						if (mItemManager->hasTag(weaponID, "One-Handed"))
						{
							attackerDamageDieType = 8;
						}
						else
						{
							attackerDamageDieType = 10;
						}
					}
				

					// range modifiers
					if(missile)
					{
						int x = mCharacterManager->GetPlayerX(attackerID);
						int y = mCharacterManager->GetPlayerY(attackerID);

						int tx = mMobManager->GetMobX(defenderID);
						int ty = mMobManager->GetMobY(defenderID);

						float dist = sqrt(pow(x - tx,2) +  pow(y - ty,2))  * 5; // each square or hex is 5ft or 5yd (which are treated the same by ACKS rules indoor/outdoor)
						int rangePenalty = mItemManager->getRangePenalty(weaponID, dist);
						attackerAttackBonus += rangePenalty;
					}
					
					// bow weapons are an exception to the usual damage pattern.
					if(mItemManager->hasTag(weaponID, "Bows") || mItemManager->hasTag(weaponID, "Crossbows"))
					{
						attackerDamageDieType = 6;
					}

					// barring special circumstances (criticals, spear charges etc) this is always 1 damage die
					attackerDamageDice = 1;

					// attacker damage bonus
					attackerDamageBonus = mCharacterManager->GetCurrentDamageBonus(attackerID, missile);

					bool slain = ResolveAttack(attackerAttackBonus, attackerDamageDieType, attackerDamageBonus, defenderManager, defenderID, missile);
					if(slain && remainingCleaves > 0)
					{
						// If we're in missile mode, act as though we just pressed "t" but exclude the current target from the list
						// If we're in melee mode, spawn an entity list consisting of all adjacent monsters but exclude the current target from the list
						gGame->AddActionLogText("Cleave!");
						remainingCleaves--;
						if(missile)
						{
							int wieldedID = mCharacterManager->GetItemInEquipSlot(currentCharacterID, HAND_MAIN);
							
							// on-hand weapon is missile, so select a target
							int range = mItemManager->getMaxRange(wieldedID);
							int x = mCharacterManager->GetPlayerX(currentCharacterID);
							int y = mCharacterManager->GetPlayerY(currentCharacterID);
							std::vector<int> monstersInRange = mMobManager->GetAllMonstersInRange(currentMapID, x, y, range, true);
							std::remove(monstersInRange.begin(), monstersInRange.end(), defenderID);

							// What happens next depends on the number of available targets.
							// If there are none, we are done with the attack sequence.
							if(monstersInRange.size()<1)
							{
								return true;
							}
							// If there is only one, we don't need to target, so immediately run that attack sequence
							if(monstersInRange.size() == 1)
							{
								ResolveAttacks(attackerManager, attackerID, MANAGER_MOB, monstersInRange[0], missile);
							}
							// otherwise, trigger the targeting sequence to pick the cleave target
							TriggerTargeting(TARGET_CREATURE, -1, 1, range, 1, false, true, monstersInRange);
							return false;
						}
						else
						{
							int new_x = mMobManager->GetMobX(defenderID);
							int new_y = mMobManager->GetMobY(defenderID);

							mCharacterManager->SetPlayerX(currentCharacterID, new_x);
							mCharacterManager->SetPlayerY(currentCharacterID, new_y);
							recomputeFov = true;

							UpdateLookText(new_x, new_y);

							std::vector<int> monstersInRange = mMobManager->GetAllMonstersInRange(currentMapID, new_x, new_y, 1, true);
							std::remove(monstersInRange.begin(), monstersInRange.end(), defenderID);

							// What happens next depends on the number of available targets.
							// If there are none, we are done with the attack sequence.
							if (monstersInRange.size() < 1)
							{
								return true;
							}
							// If there is only one, we don't need to target, so immediately run that attack sequence
							if (monstersInRange.size() == 1)
							{
								ResolveAttacks(attackerManager, attackerID, MANAGER_MOB, monstersInRange[0], missile);
							}
							
							TriggerTargeting(TARGET_CREATURE, -1, 2, 1, 1, false, true, monstersInRange);
							return false;
						}
					}
				}
			}
			break;
		case MANAGER_MOB:
			{
				// Monsters usually either get 1 weapon attack (like PCs) or get an attack sequence like Claw/Claw/Bite
				Creature& c = mMobManager->GetMonster(attackerID);

				std::string attackText = c.GetName() + " attacks " + gGame->mCharacterManager->getCharacterName(defenderID) + ".";
				gGame->AddActionLogText(attackText);
				
				attackerCleaveCount = c.GetCleaveCount();
				
				AdvancementStore* as = mClassManager->GetAdvancementStore();
				attackerAttackBonus = as->AttackBonusLookup["Monster"][c.GetHitDie()];

				std::vector <std::vector<std::string>>& attackSequences = c.GetAttackSequences();
				int sequence = randomiser->getInt(0, attackSequences.size()-1);
				std::vector<std::string>& attackSequence = attackSequences[sequence];
				
				for(std::string attack : attackSequence)
				{
					AttackType at = c.GetAttack(attack);
					attackerDamageBonus = at.DamageBonus();
					attackerDamageDieType = at.DamageDie();
					// TODO: Damage Dice in AttackType
					attackerDamageDice = 1;
					std::string attackName = at.Name();
					// TODO: Missile attacks in attackType
					bool slain = ResolveAttack(attackerAttackBonus, attackerDamageDieType, attackerDamageBonus, defenderManager, defenderID, missile);
					while(slain && attackerCleaveCount > 0)
					{
						// TODO: Cleaves
					}
				}
			}
			break;
		default:
			{
			}
			break;
		
	}
	return true; // if we didn't go out any other way, we need to return to GM_MAIN
}

bool Game::ResolveAttack(int attackBonus, int damageDie, int damageBonus, int defenderMananger, int defenderID, bool missile)
{
	// This function resolves the results of a single attack and returns if the target was killed by this attack.
	// attack bonus is calculated before entering this function

	int defenderAC = 0; // damage reduction?

	// retrieve attacker's stats
	switch(defenderMananger)
	{
		case MANAGER_CHARACTER:
			{
				defenderAC = mCharacterManager->getCharacterCurrentArmourClass(defenderID);
			}
			break;
		case MANAGER_MOB:
			{
				Creature& c = mMobManager->GetMonster(defenderID);
				defenderAC = c.GetArmourClass();
			}
			break;
		default:
			{
				// TODO: Add attack against object
			}
		break;
	}

	// Now we know the attacker's attack bonus and the defender's AC, we can perform the roll.
	// "Attack Bonus" here is actually a roll-up value. We add the AC to that value, and try to roll equal to or above it on a d20.

	int finalTargetValue = attackBonus + defenderAC;

	//TCOD_dice_t attackDie;
	//attackDie.nb_faces = 20;
	//attackDie.nb_rolls = 1;
	int roll = randomiser->diceRoll("1d20");
	if(roll<finalTargetValue)
	{
		// a miss!
		gGame->AddActionLogText("The attack misses!");
		return false;
	}
	else
	{
		// a hit, a palpable hit!
		gGame->AddActionLogText("The attack hits!");
		return ResolveDamage(damageDie, damageBonus, defenderMananger, defenderID);
	}
}

bool Game::ResolveDamage(int damageDie, int damageBonus, int defenderMananger, int defenderID)
{
	// this function resolves the damage applied to a given opponent and returns whether they were killed.
	// It can be used directly for non-rolled attacks (eg Magic Missile, Fireball, Lightning Bolt)
	// TODO: add specific tag effects to this damage for eg elemental resistance

	TCOD_dice_t damageRoller;
	damageRoller.nb_faces = damageDie;
	damageRoller.nb_rolls = 1; // needs updating for eg monster attacks for 2d6, lightning bolt etc
	damageRoller.addsub = damageBonus;
	damageRoller.multiplier = 1;

	// and make the roll
	int result = randomiser->diceRoll(damageRoller);

	// subtract that many hits from the target
	bool disabled = false;
	switch (defenderMananger)
	{
	case MANAGER_CHARACTER:
	{
		int currentHP = mCharacterManager->getCharacterCurrentHitPoints(defenderID);
		currentHP -= result;
		disabled = (currentHP < 1);
		mCharacterManager->setCharacterCurrentHitPoints(defenderID, currentHP);
		if (disabled)
		{
			std::string c = mCharacterManager->getCharacterName(defenderID);
			gGame->AddActionLogText(c + " falls!");
			mCharacterManager->SetCondition(defenderID,"Unconscious",-255);
			mCharacterManager->SetCondition(defenderID, "Injured",-255);
			mCharacterManager->SetBehaviour(defenderID, "Unconscious"); 
			// if the defender is the currently active character
			if(defenderID == currentCharacterID)
			{
				int nextChar = mPartyManager->getNextPlayerCharacter(currentPartyID, currentCharacterID);
				if(nextChar == -1)
				{
					// GAME OVER!
					gGame->AddActionLogText("Game over!");
					// TODO: end game element? Traditionally there's an endgame screen or summat
					ClearGame();
					mode = GM_MENU;
				}
				else
				{
					// we've moved to the next character in the stack
					currentCharacterID = nextChar;
				}
			}
		}
	}
	break;
	case MANAGER_MOB:
	{
		Creature& c = mMobManager->GetMonster(defenderID);
		int currentHP = c.GetHitPoints();
		currentHP -= result;
		disabled = (currentHP < 1);
		c.SetHitPoints(currentHP);
		if(disabled)
		{
			gGame->AddActionLogText(c.GetName() + " falls!");
			c.SetCondition("Unconscious");
			c.SetCondition("Injured");
			mMobManager->SetBehaviour(defenderID, "Unconscious"); // will be moved into the condition management eventually
		}
	}
	break;
	default:
	{
		// TODO: Add attack against object
	}
	break;
	}
	
	return disabled;
}

void Game::CharacterDeath(int characterID)
{
	// sad times :(
	// TODO: drop all items
	mCharacterManager->DeactivateCharacter(characterID);
	mPartyManager->RemoveCharacter(currentPartyID, characterID);
	// TODO: drop corpse item
}


void Game::MainLoop()
{
	TCOD_key_t key = { TCODK_NONE,0 };
	TCOD_mouse_t mouse;
	
	do {
		// render current sample

		TCODConsole::root->clear();

		if(mode == GM_MENU)
		{
			MenuGameHandleKeyboard(&key);
			RenderMenu();
		}
		else
		{
			MainGameHandleKeyboard(&key);

			RenderScreenFurniture();

			RenderMap();

			switch (mode)
			{
				case GM_CHARACTER:
				{
					RenderUI(currentCharacterID);
					RenderCharacterSheet();
				}
				break;
				case GM_INVENTORY:
				{
					RenderUI(currentCharacterID);
					RenderInventory();
				}
				break;
				case GM_TARGET:
				{
					RenderUI(currentCharacterID);
					RenderTargets();
				}
				break;
				case GM_DOMAIN:
				{
					// RenderUI is called from inside, to give selected character info
					mBaseManager->RenderBaseMenu(currentBaseID);
				}
			}


			RenderOffscreenUI(mode == GM_INVENTORY, mode == GM_CHARACTER);

			RenderActionLog();
		}

		// blit the sample console on the root console
		TCODConsole::blit(gGame->sampleConsole, 0, 0, SAMPLE_SCREEN_WIDTH, SAMPLE_SCREEN_HEIGHT, // the source console & zone to blit
			TCODConsole::root, SAMPLE_SCREEN_X, SAMPLE_SCREEN_Y // the destination console & position
		);
		// erase the renderer in debug mode (needed because the root console is not cleared each frame)
		TCODConsole::root->print(1, 1, "        ");

		TCODConsole::root->setDefaultForeground(TCODColor::lighterGrey);
		TCODConsole::root->setDefaultBackground(TCODColor::black);

		// update the game screen
		TCODConsole::flush();

		// did the user hit a key ?
		TCODSystem::checkForEvent((TCOD_event_t)(TCOD_EVENT_KEY_PRESS | TCOD_EVENT_MOUSE), &key, &mouse);
		if (key.vk == TCODK_ENTER && key.lalt) {
			// ALT-ENTER : switch fullscreen
			TCODConsole::setFullscreen(!TCODConsole::isFullscreen());
#ifdef TCOD_LINUX
		}
		else if (key.c == 'p') {
#else
		}
		else if (key.vk == TCODK_PRINTSCREEN) {
#endif
			if (key.lalt) {
				// ALT-PrintScreen : save to .asc format
				TCODConsole::root->saveApf("samples.apf");
			}
			else {
				// save screenshot 
				TCODSystem::saveScreenshot(NULL);
			}
		}
	} while (!TCODConsole::isWindowClosed() && mode != GM_QUIT);
}

void Game::RenderMap()
{
	int player_x, player_y;
	if(currentMapID == -1)
	{
		player_x = mPartyManager->GetPartyX(currentPartyID);
		player_y = mPartyManager->GetPartyY(currentPartyID);
	}
	else
	{
		player_x = mCharacterManager->GetPlayerX(currentCharacterID);
		player_y = mCharacterManager->GetPlayerY(currentCharacterID);
	}
	sampleConsole->clear();

	sampleConsole->setDefaultForeground(TCODColor::lighterGrey);
	//sampleConsole.print(1, 0, "89UOJK : move around\nT : light walls %s\n+-: ", light_walls ? "on " : "off");

	if (recomputeFov) {
		// calculate the field of view from the player position
		recomputeFov = false;
		//currentMap->map->computeFov(player_x, player_y, 0, light_walls, FOV_PERMISSIVE_1);
		currentMap->map->computeFov(player_x, player_y, 0, light_walls, FOV_BASIC);
	}

	// why did I remove the torch variation effect? 
	// Because the wilderness is intended to be more "open-feeling" than the dungeon. If we keep the torch effect in the dungeons, 
	// that adds to the sense of claustrophobia. But outdoors should feel airy and open, even in the dark. 

	if (currentMapID == -1)
	{
		mMapManager->renderRegionMap(sampleConsole,player_x,player_y);
		mMapManager->renderAtPosition(sampleConsole, -1, player_x, player_y, player_x, player_y, '@', TCODColor::white);
	}
	else
	{
		mMapManager->renderMap(sampleConsole, currentMapID,player_x,player_y);

		std::vector<int> chars = mPartyManager->getPlayerCharacters(currentPartyID);
		for (int ch : chars)
		{
			if (mCharacterManager->GetPlayerX(ch) != -1)
			{
				TCODColor baseColor = TCODColor::lighterGrey;
				if (mCharacterManager->getCharacterHasCondition(ch, "Unconscious")) baseColor = baseColor * TCODColor::grey;

				if (ch == currentCharacterID)
				{
					mMapManager->renderAtPosition(sampleConsole, currentMapID, player_x, player_y, mCharacterManager->GetPlayerX(ch), mCharacterManager->GetPlayerY(ch), '@', TCODColor::white);
				}
				else
				{
					mMapManager->renderAtPosition(sampleConsole, currentMapID, player_x, player_y, mCharacterManager->GetPlayerX(ch), mCharacterManager->GetPlayerY(ch), '@', baseColor);
				}
			}
		}

		std::vector<int> henches = mPartyManager->getHenchmen(currentPartyID);
		for (int ch : henches)
		{
			TCODColor baseColor = TCODColor::lighterGrey;
			if (mCharacterManager->getCharacterHasCondition(ch, "Unconscious")) baseColor = baseColor * TCODColor::grey;

			if (mCharacterManager->GetPlayerX(ch) != -1)
			{
				mMapManager->renderAtPosition(sampleConsole, currentMapID, player_x, player_y, mCharacterManager->GetPlayerX(ch), mCharacterManager->GetPlayerY(ch), '@', baseColor);
			}
		}
	}
}

std::vector<int> Game::GetTargetedEntities()
{
	// can't have more entities targeted than were in the original selection set
	int effect_size = targetingData[4];
	if (effect_size > targetIDs.size()) effect_size = targetIDs.size();

	// the ids need to lap around. So if we're selecting 3 from a set of {1,2,3,4,5}, and our targeting starts at 4, then we want 4,5,1
	std::vector<int> output;

	size_t i = targetIndex; 
	
	while(effect_size > 0)
	{
		output.push_back(targetIDs[i]);
		effect_size--;
		i++;
		if (i > (targetIDs.size() - 1)) i = 0;
	}
	
	return output;
}

void Game::RenderTargets()
{
	// what we render depends on the targeting mode. The most basic approach is to render Xs over the targeted units or positions
	// targeting data setup:
	// 0: Return Manager
	// 1: Return Code
	// 2: Range
	// 3: Ally/Enemy flags (if any)
	// 4: Effect Size (affected creatures, radius, width at cone end)
	// 
	switch (targetMode)
	{
	case(TARGET_CELL):
	{
		mMapManager->renderAtPosition(sampleConsole, currentMapID, targetCursorX, targetCursorY, targetCursorX, targetCursorY, 'X');
	}
	break;

	case(TARGET_CREATURE):
	{
		auto targets = GetTargetedEntities();
		if (targetingData[3] & TARGET_FLAGS::TF_ENEMY)
		{
			for (int beastie : targets)
			{
				//Creature& c = mMobManager->GetMonster(beastie);
				
				mMapManager->renderAtPosition(sampleConsole, currentMapID, mMobManager->GetMobX(beastie), mMobManager->GetMobX(beastie), mMobManager->GetMobX(beastie), mMobManager->GetMobX(beastie), 'X');
			}
		}

		if (targetingData[3] & TARGET_FLAGS::TF_FRIEND)
		{
			for (int wossname : targets)
			{
				mMapManager->renderAtPosition(sampleConsole, currentMapID, mCharacterManager->GetPlayerX(wossname), mCharacterManager->GetPlayerY(wossname), mCharacterManager->GetPlayerX(wossname), mCharacterManager->GetPlayerY(wossname), 'X');
			}
		}
	}
	break;
	}
}

void Game::RenderScreenFurniture()
{
	// currently none. May add some later
}

void Game::RenderUI(int selectedCharacterID)
{
	// Three main screen regions. Top left and middle is the main map, the bottom is the log, right side is a stats summary
	std::string name = mCharacterManager->getCharacterName(selectedCharacterID);
	std::string char_class = mCharacterManager->getCharacterClass(selectedCharacterID)->Name();
	std::string level = std::to_string(mCharacterManager->getCharacterLevel(selectedCharacterID));
	std::string hp = std::to_string(mCharacterManager->getCharacterCurrentHitPoints(selectedCharacterID)) + "/" + std::to_string(mCharacterManager->getCharacterTotalHitPoints(selectedCharacterID));

	TCODConsole::root->printf(60, 4, name.c_str());
	TCODConsole::root->printf(60, 5, char_class.c_str());
	TCODConsole::root->printf(60, 6, level.c_str());
	TCODConsole::root->printf(60, 8, hp.c_str());

	// TODO: Status indicators
}

void Game::UpdateLookText(int x, int y)
{
	playLogString = "";


	if (currentMapID != -1)
	{
		auto items = mMapManager->getMap(currentMapID)->getItems(x, y);
		if (items->size() > 0)
		{
			playLogString += mMapManager->ItemDesc(currentMapID, x, y);
		}

		int mobID = currentMap->getMobAt(x, y);
		if (mobID != 0)
		{
			Creature& c = mMobManager->GetMonster(mobID);
			if (c.HasCondition("Unconscious"))
			{
				playLogString += "There is an unconscious " + c.GetName() + ".";
			}
			else
			{
				playLogString += "There is a " + c.GetName() + ".";
			}
		}

		int charID = currentMap->getCharacterAt(x, y);
		if (charID != 0 && charID != currentCharacterID)
		{
			if (mCharacterManager->getCharacterHasCondition(charID, "Unconscious"))
			{
				playLogString += mCharacterManager->getCharacterName(charID) + "lies here, unconscious.";
			}
			else
			{
				playLogString += mCharacterManager->getCharacterName(charID) + " is here.";
			}
		}
	}
	else
	{
		// overland map 
		if (currentBaseID != -1)
		{
			if (mBaseManager->GetBaseOwner(currentBaseID) == currentPartyID)
			{
				// this is our base!
				playLogString += "Our " + mBaseManager->GetBaseType(currentBaseID) + " is here.";
			}
			else
			{
				// this is someone else's base!
				playLogString += "A " + mBaseManager->GetBaseType(currentBaseID) + " is here.";
			}
		}
	}

	DebugLog("Action Log changed to:" + playLogString);
}

void Game::AddActionLogText(std::string term, bool clear)
{
	if(clear)
	{
		playLogString = "";
	}
	playLogString += " " + term;

	if (clear)
	{
		DebugLog("Action Log cleared and added:" + term);
	}
	else
	{
		DebugLog("Action Log added:" + term);
	}
}

void Game::RenderActionLog()
{
	const int BOX_HEIGHT = 5;
	std::string logText = playLogString;
	int size = TCODConsole::root->getHeightRect(2, 25, SAMPLE_SCREEN_WIDTH, BOX_HEIGHT, logText.c_str());

	// now truncate our text line by line until it fits.

	while (size > BOX_HEIGHT)
	{
		std::string::size_type n = 0;
		n = logText.find_first_not_of(" \t", n);
		n = logText.find_first_of(" \t", n);
		logText.erase(0, logText.find_first_not_of(" \t", n));

		size = TCODConsole::root->getHeightRect(2, 25, SAMPLE_SCREEN_WIDTH, BOX_HEIGHT, logText.c_str());
	}
	
	TCODConsole::root->printRect(2, 25, SAMPLE_SCREEN_WIDTH, 5, logText.c_str());
}

void Game::ClearLookText()
{
	TCODConsole::root->printRect(0, 24, 50, 5, " ");
	//TCODConsole::root->c
}

void Game::RenderCharacterSheet()
{
	if (characterScreen == nullptr)
	{
		characterScreen = new TCODConsole(SAMPLE_SCREEN_WIDTH , SAMPLE_SCREEN_HEIGHT);
	}
	auto acks_class = mCharacterManager->getCharacterClass(currentCharacterID);
	int level = mCharacterManager->getCharacterLevel(currentCharacterID);
	
	auto ranks = acks_class->LevelTitles;
	std::string rank = ranks[level];
	std::string name = mCharacterManager->getCharacterName(currentCharacterID);
	std::string title = name + "," + rank;
	
	characterScreen->printFrame(0, 0, SAMPLE_SCREEN_WIDTH, SAMPLE_SCREEN_HEIGHT, false, TCOD_BKGND_SET, title.c_str());
	// characterScreen->printRectEx(SAMPLE_SCREEN_WIDTH / 2, 2, SAMPLE_SCREEN_WIDTH, SAMPLE_SCREEN_HEIGHT, TCOD_BKGND_NONE, TCOD_CENTER, "This will be the character sheet screen.");

	characterScreen->printEx(25, 2, TCOD_BKGND_NONE, TCOD_LEFT, "Attributes");
	characterScreen->printEx(25,4, TCOD_BKGND_NONE, TCOD_LEFT, "Strength");
	characterScreen->printEx(25, 5, TCOD_BKGND_NONE, TCOD_LEFT, "Dexterity");
	characterScreen->printEx(25, 6, TCOD_BKGND_NONE, TCOD_LEFT, "Constitution");
	characterScreen->printEx(25, 7, TCOD_BKGND_NONE, TCOD_LEFT, "Intelligence");
	characterScreen->printEx(25, 8, TCOD_BKGND_NONE, TCOD_LEFT, "Wisdom");
	characterScreen->printEx(25, 9, TCOD_BKGND_NONE, TCOD_LEFT, "Charisma");

	characterScreen->printEx(40, 4, TCOD_BKGND_NONE, TCOD_LEFT, std::to_string(mCharacterManager->getCharacterCharacteristic(currentCharacterID,0)).c_str());
	characterScreen->printEx(40, 5, TCOD_BKGND_NONE, TCOD_LEFT, std::to_string(mCharacterManager->getCharacterCharacteristic(currentCharacterID, 1)).c_str());
	characterScreen->printEx(40, 6, TCOD_BKGND_NONE, TCOD_LEFT, std::to_string(mCharacterManager->getCharacterCharacteristic(currentCharacterID, 2)).c_str());
	characterScreen->printEx(40, 7, TCOD_BKGND_NONE, TCOD_LEFT, std::to_string(mCharacterManager->getCharacterCharacteristic(currentCharacterID, 3)).c_str());
	characterScreen->printEx(40, 8, TCOD_BKGND_NONE, TCOD_LEFT, std::to_string(mCharacterManager->getCharacterCharacteristic(currentCharacterID, 4)).c_str());
	characterScreen->printEx(40, 9, TCOD_BKGND_NONE, TCOD_LEFT, std::to_string(mCharacterManager->getCharacterCharacteristic(currentCharacterID, 5)).c_str());
	
	std::string clas = "Class:" + acks_class->Name();
	std::string lev = "Level:" + std::to_string(level);

	characterScreen->printEx(2, 2, TCOD_BKGND_NONE, TCOD_LEFT, clas.c_str());
	characterScreen->printEx(2, 3, TCOD_BKGND_NONE, TCOD_LEFT, lev.c_str());

	int xp = mCharacterManager->getCharacterExperience(currentCharacterID);
	auto maxps = acks_class->LevelXPValues;
	std::string nextp = "---";
	if(maxps.size() != level)
	{
		nextp = std::to_string(maxps[level + 1]);
	}
	std::string exp = "Experience:" + std::to_string(xp) + "/" + nextp;
	
	characterScreen->printEx(2, 4, TCOD_BKGND_NONE, TCOD_LEFT, exp.c_str());

	auto advancement = mClassManager->GetAdvancementStore();
	
	auto attack_bonuses = advancement->AttackBonusLookup[acks_class->AttackProgression()];
	int attack_bonus = attack_bonuses[level];
	std::string ab_melee = "Melee Attack:" + std::to_string(mCharacterManager->UpdateCurrentAttackValue(currentCharacterID, false));
	std::string ab_missile = "Missile Attack:" + std::to_string(mCharacterManager->UpdateCurrentAttackValue(currentCharacterID, true));

	characterScreen->printEx(2, 6, TCOD_BKGND_NONE, TCOD_LEFT, ab_melee.c_str());
	characterScreen->printEx(2, 7, TCOD_BKGND_NONE, TCOD_LEFT, ab_missile.c_str());

	characterScreen->printEx(25, 11, TCOD_BKGND_NONE, TCOD_LEFT, "Saves");
	
	for(int i=0;i<5;i++)
	{
		std::string save_name = saveTypes[i];

		std::string display_name = save_name.substr(0, 13);
		
		characterScreen->printEx(25, 12+i, TCOD_BKGND_NONE, TCOD_LEFT, display_name.c_str());

		int saveVal = mCharacterManager->UpdateCurrentSaveValue(currentCharacterID, save_name);

		characterScreen->printEx(40, 12+i, TCOD_BKGND_NONE, TCOD_LEFT, std::to_string(saveVal).c_str());
	}

	std::string ac = "AC:" + std::to_string(mCharacterManager->GetCurrentAC(currentCharacterID));
	characterScreen->printEx(2, 9, TCOD_BKGND_NONE, TCOD_LEFT, ac.c_str());
	std::string enc = "Encumbrance:" + mCharacterManager->GetCurrentEncumbranceType(currentCharacterID);
	characterScreen->printEx(2, 10, TCOD_BKGND_NONE, TCOD_LEFT, enc.c_str());
}

void Game::RenderMenu()
{
	sampleConsole->clear();

	sampleConsole->setDefaultForeground(TCODColor::white);

	sampleConsole->printFrame(0, 0, SAMPLE_SCREEN_WIDTH, SAMPLE_SCREEN_HEIGHT, false, TCOD_BKGND_SET, "Rogue Conqueror King");

	int start_ypos = 4;
	int ypos = start_ypos;
	
	for (std::string s : menuText)
	{
		sampleConsole->printEx(8, ypos, TCOD_BKGND_NONE, TCOD_LEFT, s.c_str());
		ypos++;
	}

	int select_y = start_ypos + menuSelected;
	for (int x = 0; x < SAMPLE_SCREEN_WIDTH; x++)
	{
		sampleConsole->setCharBackground(x, select_y, TCODColor::white, TCOD_BKGND_SET);
		sampleConsole->setCharForeground(x, select_y, TCODColor::black);
	}
}


void Game::RenderInventory()
{
	if(inventoryScreen == nullptr)
	{
		inventoryScreen = new TCODConsole(SAMPLE_SCREEN_WIDTH, SAMPLE_SCREEN_HEIGHT);
	}

	inventoryScreen->clear();

	inventoryScreen->printFrame(0, 0, SAMPLE_SCREEN_WIDTH, SAMPLE_SCREEN_HEIGHT, false, TCOD_BKGND_SET, "Inventory");
	// inventoryScreen->printRectEx(SAMPLE_SCREEN_WIDTH / 2, 2, SAMPLE_SCREEN_WIDTH, SAMPLE_SCREEN_HEIGHT, TCOD_BKGND_NONE, TCOD_CENTER, "This will be the inventory screen.");

	auto inv = mCharacterManager->GetInventory(currentCharacterID);

	int page = 0;

	const int MAX_ITEMS = SAMPLE_SCREEN_HEIGHT - 3;

	// figure out which page the current position is on

	if(inventoryPosition > MAX_ITEMS)
	{
		// we're off the first page. calculate which page we're actually on
		page = inventoryPosition % MAX_ITEMS;
	}
	
	auto inv_iter = inv.begin();
	std::advance(inv_iter, page* MAX_ITEMS);

	for(int i = page * MAX_ITEMS;(i<(page+1)*MAX_ITEMS && i<inv.size());i++)
	{
		int itemID = *inv_iter++;
		std::string item = mItemManager->getName(itemID);
		int slot = mCharacterManager->GetEquipSlotForInventoryItem(currentCharacterID,i);
		if (slot != -1)
			item += " (Equipped)";
		int y_pos = i - page * MAX_ITEMS + 2;
		TCOD_bkgnd_flag_t backg = TCOD_BKGND_NONE;
		inventoryScreen->printEx(2, y_pos, backg, TCOD_LEFT, item.c_str());
	}

	int select_y = inventoryPosition - page * MAX_ITEMS + 2;
	for(int x = 0;x< SAMPLE_SCREEN_WIDTH;x++)
	{
		inventoryScreen->setCharBackground(x, select_y, TCODColor::white, TCOD_BKGND_SET);
		inventoryScreen->setCharForeground(x, select_y, TCODColor::black);
	}
}

void Game::RenderOffscreenUI(bool inventory, bool character)
{
	static int x = 0, y = 0; // secondary screen position

	TCODSystem::setFps(30); // fps limited to 30

	if (character)
	{
		TCODConsole::blit(characterScreen, 0, 0, SAMPLE_SCREEN_WIDTH, SAMPLE_SCREEN_HEIGHT,
			sampleConsole, x, y, 1.0f, 0.75f);
	}

	if (inventory)
	{
		TCODConsole::blit(inventoryScreen, 0, 0, SAMPLE_SCREEN_WIDTH, SAMPLE_SCREEN_HEIGHT,
			sampleConsole, x, y, 1.0f, 0.75f);
	}

}


void Game::DebugLog(std::string message)
{
	gLog->Log("Game", message);
}

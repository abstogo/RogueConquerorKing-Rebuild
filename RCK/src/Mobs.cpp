#include "Mobs.h"
#include "Game.h"
#include <string>
#include <locale>
#include <cmath>

MobManager* MobManager::LoadMobData()
{
	gLog->Log("Monster Loader", "Started");

	std::string mobFilename = "RCK/scripts/creatures.json";
	
	std::ifstream is(mobFilename);

	MobManager* output = new MobManager(jsoncons::decode_json<CreatureSet>(is));

	is.close();

	gLog->Log("Monster Loader", "Decoded " + mobFilename);
	
	const std::vector<CreatureTemplate>& creatureList = output->CreatureTemplates().CreatureTemplates();

	for (int i = 0; i < creatureList.size(); i++)
	{
		const CreatureTemplate& c = creatureList[i];
		
		// add this class to the index list
		output->creatureNameLookup[c.Name()] = i;
	}

	gLog->Log("Monster Loader", "Creature Lookup Populated");
	
	return output;
}


void MobManager::SetMobX(int entityID, int x)
{
	mobXPos[entityID] = x;
}

void MobManager::SetMobY(int entityID, int y)
{
	mobYPos[entityID] = y;
}

int MobManager::GetMobX(int entityID)
{
	return mobXPos[entityID];
}

int MobManager::GetMobY(int entityID)
{
	return mobYPos[entityID];
}

bool MobManager::TurnHandler(int entityID, double time)
{
	// this fires every time one of our monster is able to move or act again

	// If we don't have a current behaviour, pick one.
	bool pickNew = false;
	double timeToMove = 0.0;
	Creature& c = Monsters_[entityID];
	if(currentBehaviour[entityID] == -1)
	{
		pickNew = true;
	}
	else
	{
		// Perform our current behaviour

		// A switch, I hear you cry? Yeah, yeah, I know, not very DOD of me. If I was writing this to manage 2 million enemies I might take it the other way
		// (Actually that sounds like a bloody amazing idea if there's some way to make them visible in a Roguelike. Maybe a battle sim where every unit is calculated?)
		// But in all seriousness there's not a lot of call to split the timing table n ways for existence-based processing if we only call this at each timing point, and it's complicated enough
		
		switch(currentBehaviour[entityID])
		{
		case MOB_BEHAVIOUR_IDLE:
			{
				// we simply don't move.
				// at some point later we can put in observations - "scratches its nose" etc

				// randomly determine how long we do this for
				timeToMove = gGame->randomiser->getDouble(1.0, 17.0);
				currentBehaviour[entityID] = MOB_BEHAVIOUR_UNSET;
			}
			break;

		case MOB_BEHAVIOUR_WANDER:
			{
				// move in a random direction
				int moveX, moveY, move_value;
				int mapID = gGame->GetCurrentMap();
				if (gGame->mMapManager->getMap(mapID)->outdoor)
				{
					move_value = gGame->randomiser->getInt(0, 5);
				}
				else
				{
					move_value = gGame->randomiser->getInt(0, 7);
				}
				gGame->mMapManager->shift(gGame->GetCurrentMap(), moveX, moveY, GetMobX(entityID), GetMobY(entityID), move_value);

				timeToMove = MoveTo(entityID, moveX, moveY, time);
			}
			break;

		case MOB_BEHAVIOUR_FOLLOW:
		{
			// try to move towards a specific character and then stay at a distance of 2-3 squares

			// first, see if we need to generate a new path
			// We need to generate a new path if we can see the enemy or if we reached our previous destination. Restore the pathing check iff we don't have sensory lock
			//if (paths[entityID] == NULL)
			//{
				// do we have a target already selected? If not, pick the player
			if (targetID[entityID] == -1 && targetManager[entityID] == -1)
			{
				targetManager[entityID] = MANAGER_CHARACTER;
				targetID[entityID] == gGame->GetSelectedCharacterID();
			}

			int ox = GetMobX(entityID);
			int oy = GetMobY(entityID);

			int dx, dy;
			if (targetManager[entityID] == MANAGER_CHARACTER)
			{
				dx = gGame->mCharacterManager->GetPlayerX(gGame->GetSelectedCharacterID());
				dy = gGame->mCharacterManager->GetPlayerY(gGame->GetSelectedCharacterID());
			}
			else if (targetManager[entityID] == MANAGER_MOB)
			{
				Creature t = Monsters_[targetID[entityID]];
				dx = GetMobX(targetID[entityID]);
				dy = GetMobY(targetID[entityID]);
			}
			else
			{
				// do nothing. We should normally have a different element for seeking an item?
			}

			int dist = sqrt(pow(abs(dx - ox), 2) + pow(abs(dy - oy), 2));
			if (dist > 2)
			{
				int* mapID_ptr = &(gGame->GetCurrentMap());
				int mapID = gGame->GetCurrentMap();
				Map* map = gGame->mMapManager->getMap(mapID);
				paths[entityID] = new TCODPath(map->width, map->height, gGame->mMapManager, (void*)mapID_ptr, 1.0f);
				paths[entityID]->compute(ox, oy, dx, dy);
				//}

				if (paths[entityID]->isEmpty())
				{
					// we've reached our destination, so now we need a different behaviour
					delete paths[entityID];
					paths[entityID] = NULL;
					timeToMove = 1.0;
					pickNew = true;
				}
				else
				{
					int tx, ty;
					bool canWalk = paths[entityID]->walk(&tx, &ty, true);
					if (canWalk)
					{
						// go there!
						timeToMove = MoveTo(entityID, tx, ty, time);
					}
					else
					{
						// we can't find a route, so pick another behaviour
						delete paths[entityID];
						paths[entityID] = NULL;
						timeToMove = 3.0;
						pickNew = true;
					}
				}
			}
		}
		break;

		case MOB_BEHAVIOUR_SEEK_PLAYER:
		{
			// try to move towards a specific character

			// first, see if we need to generate a new path
			// We need to generate a new path if we can see the enemy or if we reached our previous destination. Restore the pathing check iff we don't have sensory lock
			//if (paths[entityID] == NULL)
			//{
				// do we have a target already selected? If not, pick the current character
			if (targetID[entityID] == -1 && targetManager[entityID] == -1)
			{
				targetManager[entityID] = MANAGER_CHARACTER;
				targetID[entityID] == gGame->GetSelectedCharacterID();
			}

			int ox = GetMobX(entityID);
			int oy = GetMobY(entityID);

			int dx, dy;
			if (targetManager[entityID] == MANAGER_CHARACTER)
			{
				dx = gGame->mCharacterManager->GetPlayerX(gGame->GetSelectedCharacterID());
				dy = gGame->mCharacterManager->GetPlayerY(gGame->GetSelectedCharacterID());
			}
			else if (targetManager[entityID] == MANAGER_MOB)
			{
				Creature t = Monsters_[targetID[entityID]];
				dx = GetMobX(targetID[entityID]);
				dy = GetMobY(targetID[entityID]);
			}
			else
			{
				// do nothing. We should normally have a different element for seeking an item?
			}

			int* mapID_ptr = &(gGame->GetCurrentMap());
			int mapID = gGame->GetCurrentMap();
			Map* map = gGame->mMapManager->getMap(mapID);
			paths[entityID] = new TCODPath(map->width, map->height, gGame->mMapManager, (void*)mapID_ptr, 1.0f);
			paths[entityID]->compute(ox, oy, dx, dy);
			//}

			if (paths[entityID]->isEmpty())
			{
				// we've reached our destination, so now we need a different behaviour
				delete paths[entityID];
				paths[entityID] = NULL;
				timeToMove = 1.0;
				pickNew = true;
			}
			else
			{
				int tx, ty;
				bool canWalk = paths[entityID]->walk(&tx, &ty, true);
				if (canWalk)
				{
					// go there!
					timeToMove = MoveTo(entityID, tx, ty, time);
				}
				else
				{
					// we can't find a route, so pick another behaviour
					delete paths[entityID];
					paths[entityID] = NULL;
					timeToMove = 3.0;
					pickNew = true;
				}
			}
		}
		break;
			
		case MOB_BEHAVIOUR_SEEK_ENEMY:
			{
				// try to move towards the nearest enemy and attack them

				int mapID = gGame->GetCurrentMap();
				Map* map = gGame->mMapManager->getMap(mapID);

				int ox = GetMobX(entityID);
				int oy = GetMobY(entityID);
				
				// do we have a target already selected? If not, pick the nearest enemy
				if (targetID[entityID] == -1 && targetManager[entityID] == -1)
				{
					// take the total set of conscious PCs and Henchmen, then filter them by POV
					std::vector<int> characters = gGame->mCharacterManager->GetConditionCharactersOnMap(mapID, "Unconscious", false);
					std::vector<int> targets = gGame->mMapManager->filterByFOV(MANAGER_MOB, entityID, MANAGER_CHARACTER, characters);
					
					int selected = -1;

					// if we have no target options, set off another action select
					if (targets.size() > 0)
					{
						// get the closest
						double closest_sqr_dist = 10e10;
						for (int j : targets)
						{
							int playerX = gGame->mCharacterManager->GetPlayerX(j);
							int playerY = gGame->mCharacterManager->GetPlayerY(j);

							double dist = ((playerX - ox) * (playerX - ox)) + ((playerY - oy) * (playerY - oy));
							if (dist < closest_sqr_dist)
							{
								selected = j;
								closest_sqr_dist = dist;
							}
						}
						// if no closest found somehow, pick the first
						if (selected == -1) selected = targets[0];

						targetManager[entityID] = MANAGER_CHARACTER;
						targetID[entityID] = selected;
					}
				}

				// if we still don't have a target selected, pick a new behaviour.
				if (targetID[entityID] == -1 && targetManager[entityID] == -1)
				{
					pickNew = true;
					timeToMove = 3.0;
				}
				else
				{
					// otherwise, continue tracking our target.
					
					int dx, dy;
					bool unconscious = false;
					if (targetManager[entityID] == MANAGER_CHARACTER)
					{
						dx = gGame->mCharacterManager->GetPlayerX(targetID[entityID]);
						dy = gGame->mCharacterManager->GetPlayerY(targetID[entityID]);
						unconscious = gGame->mCharacterManager->getCharacterHasCondition(targetID[entityID], "Unconscious");
					}
					else if (targetManager[entityID] == MANAGER_MOB)
					{
						Creature t = Monsters_[targetID[entityID]];
						dx = GetMobX(targetID[entityID]);
						dy = GetMobY(targetID[entityID]);
						unconscious = t.GetHitPoints() <= 0;
					}
					else
					{
						// do nothing. We should normally have a different element for seeking an item?
					}

					paths[entityID] = new TCODPath(map->width, map->height, gGame->mMapManager, (void*)&mapID, 1.0f);
					paths[entityID]->compute(ox, oy, dx, dy);

					if (paths[entityID]->isEmpty() || unconscious)
					{
						// we've reached our destination, so now we need a different behaviour
						delete paths[entityID];
						paths[entityID] = NULL;
						timeToMove = 1.0;
						pickNew = true;
						targetID[entityID] = -1;
						targetManager[entityID] = -1;
					}
					else
					{
						int tx, ty;
						bool canWalk = paths[entityID]->walk(&tx, &ty, true);
						if (canWalk)
						{
							// go there!
							timeToMove = MoveTo(entityID, tx, ty, time);
						}
						else
						{
							// we can't find a route, so pick another behaviour
							delete paths[entityID];
							paths[entityID] = NULL;
							targetID[entityID] = -1;
							targetManager[entityID] = -1;
							timeToMove = 3.0;
							pickNew = true;
						}
					}
				}
			}
			break;

		case MOB_BEHAVIOUR_UNCONSCIOUS:
			{
				// do absolutely nothing. We still run the tick cycle so we get restarted if we're woken
			}
			break;
		}
	}

	if (timeToMove == 0.0)
	{
		// we didn't move. Wait a while, then do it again
		// how long? Depends on the map. Wait about 150 units of movement
		timeToMove = gGame->mMapManager->getMovementTime(mapIDs[entityID], c.GetMovement());
	}
	gGame->mTimeManager->SetEntityTime(entityID, MANAGER_MOB, time + timeToMove);

	
	// now check if we have concluded our activity. If so, unset our behaviour so we can pick a new one

	// pick a new behaviour
	if(pickNew)
	{
		currentBehaviour[entityID] = SelectBehaviour(entityID);
		std::string bhname = Monsters_[entityID].GetName() + " sets behaviour:" + MobBehaviourNames[currentBehaviour[entityID]] + ".";
		gGame->AddActionLogText(bhname);
	}

	// don't interrupt - carry on running through the time list
	return false;
}

int MobManager::SelectBehaviour(int entityID)
{
	int result = -1;
	// TODO: Add sensorium checks here

	// temporary version - pick randomly from available set
	// unless we're unconscious, in which case just fucking lie there biznitch

	Creature& c = Monsters_[entityID];
	if (c.HasCondition("Unconscious")) return MOB_BEHAVIOUR_UNCONSCIOUS;
	std::vector<int> behaviours = c.GetBehaviours();
	if (behaviours.size() > 0)
	{
		int select = gGame->randomiser->get(0, behaviours.size() - 1);
		result = behaviours[select];
	}
	return result;
}

void MobManager::SetBehaviour(int entityID, int behaviourType)
{
	currentBehaviour[entityID] = behaviourType;
}

double MobManager::MoveTo(int entityID, int new_x, int new_y, int currentTime)
{
	Creature& c = Monsters_[entityID];
	int mapID = mapIDs[entityID];
	Map* m = gGame->mMapManager->getMap(mapID);
	double timeExpended = 0.0;
	if (!gGame->mMapManager->isOutOfBounds(mapID, new_x, new_y))
	{
		if (m->map->isWalkable(new_x, new_y))
		{
			if (new_x >= 0 && new_y >= 0)
			{
				int baseCharacter = 0;
				if ((gGame->mCharacterManager->GetPlayerX(gGame->GetSelectedCharacterID()) == new_x) && (gGame->mCharacterManager->GetPlayerY(gGame->GetSelectedCharacterID()) == new_y))
				{
					baseCharacter = gGame->GetSelectedCharacterID();
				}
				else
				{
					baseCharacter = m->getCharacterAt(new_x, new_y);
				} 
				if (baseCharacter)
				{
					// there's a character there. Check if we want to attack them
					if(c.IsHostile())
					{
						// close-quarters attack!
						gGame->ResolveAttacks(MANAGER_MOB, entityID, MANAGER_CHARACTER, baseCharacter, false);
					}
					else
					{
						// we don't want to attack, so we just don't move
					}
				}
				else if (m->getMobAt(new_x, new_y))
				{
					// there's a monster there
				}
				else
				{
					// there isn't another creature there, so move
					
					// recomputeFov = true; // replace with awareness scan
					m->setMob(new_x, new_y, entityID);

					timeExpended = gGame->mMapManager->getMovementTime(mapID, c.GetMovement());
				}
			}
		}
	}
	return timeExpended;
}


int MobManager::GetTemplateIndex(const std::string creatureName)
{
	return creatureNameLookup[creatureName];
}

void MobManager::EmptyCreature(Creature c)
{
	Monsters_.push_back(c);
	mapIDs.push_back(-1);
	// behaviour data
	targetID.push_back(-1);
	targetManager.push_back(-1);
	currentBehaviour.push_back(-1);

	mobXPos.push_back(-1);
	mobYPos.push_back(-1);

	paths.push_back(NULL);
}

void MobManager::SpawnOnMap(int entityID, int mapID, int spawn_x, int spawn_y)
{
	DebugLog("Spawning " + GetMonster(entityID).GetName() + " onto map #" + std::to_string(mapID));
	
	mapIDs[entityID] = mapID;

	const int MAX_SPAWN_DIST = 255;

	if (mapID != -1)
	{
		// note that we keep trying until we find somewhere, same as character launches
		Map* currentMap = gGame->mMapManager->getMap(mapID);
		bool outdoor = currentMap->outdoor;
		int dist_mult = outdoor ? 6 : 8;
		int dist = 1;
		
		bool found = false;
		while (!found && dist < MAX_SPAWN_DIST)
		{
			int ticker = dist * dist_mult;
			while (!found && ticker > 0)
			{
				int new_x = spawn_x + (dist * gGame->randomiser->getInt(-1, 1));
				int new_y = spawn_y + (dist * gGame->randomiser->getInt(-1, 1));

				if (!gGame->mMapManager->isOutOfBounds(mapID, new_x, new_y))
				{
					if (currentMap->map->isWalkable(new_x, new_y))
					{
						if (!currentMap->getCharacterAt(new_x, new_y) && !currentMap->getMobAt(new_x, new_y))
						{
							// square is clear, put it there
							double moveTime = MoveTo(entityID, new_x, new_y, 0);
							gGame->mTimeManager->SetEntityTime(entityID, MANAGER_MOB, moveTime);
							found = true;

							DebugLog("Spawned " + GetMonster(entityID).GetName() + " onto map #" + std::to_string(mapID) + " at position (" + std::to_string(new_x) + "," + std::to_string(new_y) + ")");
						}
					}
				}
				ticker--;
			}
			if (!found) dist++;
		}
	}
}

int MobManager::GenerateMonster(int templateIndex, int mapID, int x, int y, bool hostile)
{
	Creature c = Creature::GenerateCreature(templateIndex);
	c.SetHostile(hostile);
	EmptyCreature(c);
	
	if(mapID != -1)
		SpawnOnMap(nextMonsterIndex, mapID, x, y);

	return nextMonsterIndex++;
}

void Creature::AddAttack(std::string name, int damageDie, int damageBonus)
{
	
}

//Creature Creature::GenerateCreature(const CreatureTemplate& ct)
Creature Creature::GenerateCreature(int templateIndex)
{
	Creature output;
	CreatureTemplate ct = gGame->mMobManager->CreatureTemplates().CreatureTemplates()[templateIndex];
	output.SetName(ct.Name());			// potentially allow for monster name generation?
	output.SetVisual(ct.Visual());
	output.SetSaveAs(ct.SaveAs());
	output.SetAlignment(ct.Alignment()); // replace with actual saves?

	output.SetHitDie(ct.HitDie());

	// Generate HitPoints
	TCOD_dice_t hitDice;
	hitDice.nb_faces = 8;
	hitDice.nb_rolls = ct.HitDie();
	hitDice.addsub = ct.HitDieModifier();
	hitDice.multiplier = 1;
	int hitPoints = gGame->randomiser->diceRoll(hitDice);
	if (hitPoints < 1) hitPoints = 1; // can't have less than 1 HP at character gen!
	output.SetHitPoints(hitPoints);

	
	output.SetArmourClass(ct.ArmourClass());
	output.SetMorale(ct.Morale());
	output.SetMovement(ct.Movement());
	output.SetXP(ct.XP());					// generate from HD and abilities?

	// break out attacks into a map for lookups
	for(AttackType at : ct.Attacks())
	{
		output.Attacks_[at.Name()] = at;
	}
	
	output.Abilities_ = ct.Abilities();
	output.AttackSequences_ = ct.AttackSequences();

	output.CreatureType_ = templateIndex;
	
	std::vector<std::string> behaviourStrings = ct.Behaviours();
	for(std::string bs : ct.Behaviours())
	{
		output.Behaviours_.push_back(gGame->mMobManager->behaviourLookup[bs]);
	}

	output.SetHostile(true);

	return output;
}

bool Creature::IsBlocking()
{
	// currently this is just "are we unconscious"
	return !HasCondition("Unconscious");
}

bool Creature::HasCondition(std::string condition)
{
	int index = gGame->mConditionManager->GetConditionIndex(condition);
	return HasCondition(index);
}


int Creature::SetCondition(std::string condition)
{
	int index = gGame->mConditionManager->GetConditionIndex(condition);
	return SetCondition(index);
}

// Set a condition. Usually inflicted on us by others.
int Creature::SetCondition(int condition)
{
	// don't duplicate
	if (!HasCondition(condition))
	{
		Conditions.push_back(condition);
		return condition;
	}

	return -1;
}

// Used either by the timing system or via an action to explicitly remove the condition
// Eg - turn-length condition removed at end of turn, "Prone" removed by "stand up" action
//

int Creature::RemoveCondition(std::string condition)
{
	int index = gGame->mConditionManager->GetConditionIndex(condition);
	return RemoveCondition(index);
}

int Creature::RemoveCondition(int condition)
{
	std::vector<int>::iterator iter = std::find(Conditions.begin(), Conditions.end(), condition);
	if (iter != Conditions.end())
	{
		int value = *iter;
		Conditions.erase(iter);
		return value;
	}

	return -1;
}


bool MobManager::TargetHandler(int entityID, int returnCode)
{
	return true;
}

bool MobManager::TimeHandler(int rounds, int turns, int hours, int days, int weeks, int months)
{
	return true;
}

std::vector<int> MobManager::GetAllMonstersOnMap(int mapID, bool hostileOnly, bool liveOnly)
{
	std::vector<int> output;

	for (int i = 0; i < mapIDs.size(); i++)
	{
		if (mapIDs[i] == mapID)
		{
			if (!hostileOnly || Monsters_[i].IsHostile())
			{
				if (liveOnly && Monsters_[i].IsBlocking())
				{
					output.push_back(i);
				}
			}
		}
	}

	return output;
}

std::vector<int> MobManager::GetAllEnemyMonstersOnMap(int mapID, bool partyId, bool liveOnly)
{
	std::vector<int> output;

	for (int i = 0; i < mapIDs.size(); i++)
	{
		if (mapIDs[i] == mapID)
		{
			if (!gGame->mPartyManager->IsAnAnimal(partyId, i))
			{
				if (liveOnly && Monsters_[i].IsBlocking())
				{
					output.push_back(i);
				}
			}
		}
	}

	return output;
}

std::vector<int> MobManager::GetAllMonstersInRange(int mapID, int centerX, int centerY, int range, bool hostileOnly, bool liveOnly)
{
	std::vector<int> base = GetAllMonstersOnMap(mapID, hostileOnly);
	int rangeSquared = pow(range, 2);
	std::vector<int> output;

	for(int i=0;i<base.size();i++)
	{
		int monsterID = base[i];
		int monsterX = GetMobX(monsterID);
		int monsterY = GetMobY(monsterID);

		int distanceSquared = pow(monsterX - centerX, 2) + pow(monsterX - centerX, 2);
		if(distanceSquared <= rangeSquared)
		{
			output.push_back(monsterID);
		}
	}

	return output;
}

void MobManager::DebugLog(std::string message)
{
	gLog->Log("MobManager", message);
}

void MobManager::DumpMob(int mobID)
{
	// this dumps all mob details

	DebugLog("Dumping Mob #" + std::to_string(mobID));

	Creature mob = GetMonster(mobID);

	std::string name = mob.GetName();

	std::string filename = "dump_" + name + "_" + std::to_string(mobID) + ".txt";

	std::ofstream dumpFile;

	dumpFile.open(filename);

	std::string dumpLine;

	dumpLine = "Name:" + name;
	dumpFile << dumpLine << std::endl;

	CreatureTemplate ct = creatureTemplateSet.CreatureTemplates()[mob.GetCreatureType()];
	std::string templateName = ct.Name();
	dumpLine = "Template:" + templateName;
	dumpFile << dumpLine << std::endl;

	std::string visual = mob.GetVisual();
	dumpLine = "Symbol:" + visual;
	dumpFile << dumpLine << std::endl;

	std::string saveas = mob.GetSaveAs();
	dumpLine = "Saves As:" + saveas;
	dumpFile << dumpLine << std::endl;

	std::string hitDie = std::to_string(mob.GetHitDie());
	std::string hitPoints = std::to_string(mob.GetHitPoints());
	dumpLine = "HD/HP:" + hitDie + "/" + hitPoints + " remaining";
	dumpFile << dumpLine << std::endl;

	std::string ac = std::to_string(mob.GetArmourClass());
	dumpLine = "AC:" + ac;
	dumpFile << dumpLine << std::endl;
	
	int map = mapIDs[mobID];
	std::string map_text = "Current Map:" + (map == -1 ? "None" : std::to_string(map));
	dumpFile << map_text << std::endl;

	std::string behaviour_name = "Current Behaviour:";
	int behaviour = currentBehaviour[mobID];
	if (behaviour == -1)
	{
		behaviour_name += "None";
	}
	else
	{
		behaviour_name += MobBehaviourNames[behaviour];
	}
	dumpFile << behaviour_name << std::endl;

	dumpFile << DumpConditions(mobID);
	dumpFile << std::endl;

	// inventory?
	// attack sequences?
	// reactions?
	
	dumpFile.close();
}

std::string MobManager::DumpConditions(int mobID)
{
	std::string output = "Conditions:";

	Creature mob = GetMonster(mobID);
	std::vector<int> conditions = mob.GetConditions();
	for (int i = 0; i < conditions.size(); i++)
	{
		if (i != 0) output += ",";
		int s = conditions[i];
		std::string name = gGame->mConditionManager->GetNameFromIndex(s);
		output += name;
	}
	output += ".";
	return output;
}
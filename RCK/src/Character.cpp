#include "Character.h"
#include "Class.h"
#include "Game.h"

CharacterManager* CharacterManager::LoadCharacteristics()
{
	gLog->Log("Characteristic Loader","Started");

	const std::string statsFilename = "RCK/scripts/statistics.json";
	
	std::ifstream is(statsFilename);

	CharacterManager* cm = new CharacterManager(jsoncons::decode_json<CharacteristicData>(is));

	// resort the characteristics into a map for lookup#

	const std::vector<Statistic>& ss = cm->cd.statistics();
	
	for(int i=0;i<ss.size();i++)
	{
		const Statistic& ssn = ss[i];
		cm->StatisticLookup[ssn.name()] = i;
	}
	
	// filter our tags into a map for referencing
	std::vector<Statistic>& stats = (std::vector<Statistic>&)cm->cd.statistics();
	for(int i=0;i<stats.size();i++)
	{
		const std::vector<std::string>& tags = stats[i].bonusTo();
		for(std::vector<std::string>::const_iterator tag = tags.begin(); tag != tags.end(); tag++)
		{
			cm->CharacteristicTags[*tag] = i;
		}
	}

	is.close();

	gLog->Log("Characteristic Loader", "Decoded " + statsFilename);
	
	// read the characteristic bonuses from ability_bonus.csv

	const std::string abilityBonusFilename = "RCK/scripts/ability_bonus.csv";

	std::ifstream cs(abilityBonusFilename);
	
	jsoncons::csv::csv_options options;
	options.assume_header(true);

	jsoncons::ojson j = jsoncons::csv::decode_csv<jsoncons::ojson>(cs, options);

	cm->AbilityBonuses.resize(19);
	
	for (const auto& row : j.array_range())
	{
		int dice = row["dice"].as<int>();
		int bonus = row["bonus"].as<int>();

		cm->AbilityBonuses[dice] = bonus;
	}

	cs.close();

	gLog->Log("Characteristic Loader", "Decoded " + abilityBonusFilename);

	const std::string abilityRequisiteFilename = "RCK/scripts/ability_prime_req.csv";
	
	std::ifstream prs(abilityRequisiteFilename);

	jsoncons::ojson jprs = jsoncons::csv::decode_csv<jsoncons::ojson>(prs, options);

	cm->PrimeReqMultiplier.resize(19);

	for(const auto& row : jprs.array_range())
	{
		int score = row["score"].as<int>();
		float multiplier = row["multiplier"].as<float>();

		cm->PrimeReqMultiplier[score] = multiplier;
	}

	prs.close();

	gLog->Log("Characteristic Loader", "Decoded " + abilityRequisiteFilename);

	gLog->Log("Characteristic Loader", "Completed");

	return cm;
}

bool CharacterManager::TurnHandler(int entityID, double time)
{
	// this fires every time one of our monster is able to move or act again

	if (gGame->GetSelectedCharacterID() == entityID)
	{
		// selected characters do not run turns, so do nothing
		return false;
	}
	
	// If we don't have a current behaviour, pick one.
	bool pickNew = false;
	double timeToMove = 0.0;

	if (pcCurrentBehaviour[entityID] == -1)
	{
		pickNew = true;
	}
	else
	{
		// Perform our current behaviour

		// A switch, I hear you cry? Yeah, yeah, I know, not very DOD of me. If I was writing this to manage 2 million enemies I might take it the other way
		// (Actually that sounds like a bloody amazing idea if there's some way to make them visible in a Roguelike. Maybe a battle sim where every unit is calculated?)
		// But in all seriousness there's not a lot of call to split the timing table n ways for existence-based processing if we only call this at each timing point, and it's complicated enough

		switch (pcCurrentBehaviour[entityID])
		{
		case CHAR_BEHAVIOUR_IDLE:
		{
			// we simply don't move.
			// at some point later we can put in observations - "scratches its nose" etc

			// randomly determine how long we do this for
			timeToMove = gGame->randomiser->getDouble(1.0, 17.0);
			pcCurrentBehaviour[entityID] = CHAR_BEHAVIOUR_UNSET;
		}
		break;

		case CHAR_BEHAVIOUR_WANDER:
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
			gGame->mMapManager->shift(gGame->GetCurrentMap(), moveX, moveY, GetPlayerX(entityID), GetPlayerY(entityID), move_value);

			timeToMove = MoveTo(entityID, moveX, moveY, time);
		}
		break;
		}
	}

	if (timeToMove == 0.0)
	{
		// we didn't move. Wait a while, then do it again
		// how long? Depends on the map. Wait about 150 units of movement
		if(pcMapID[entityID] != -1)
			timeToMove = gGame->mMapManager->getMovementTime(pcMapID[entityID], GetCurrentSpeed(entityID));
	}
	gGame->mTimeManager->SetEntityTime(entityID, MANAGER_CHARACTER, time + timeToMove);


	// now check if we have concluded our activity. If so, unset our behaviour so we can pick a new one

	// pick a new behaviour
	if (pickNew)
	{
		pcCurrentBehaviour[entityID] = SelectBehaviour(entityID);
		std::string bhname = getCharacterName(entityID) + " sets behaviour:" + CharBehaviourNames[pcCurrentBehaviour[entityID]] + ".";
		gGame->AddActionLogText(bhname);
	}

	// don't interrupt - carry on running through the time list
	return false;
}


bool CharacterManager::TargetHandler(int entityID, int returnCode)
{
	return true;
}

bool CharacterManager::TimeHandler(int rounds, int turns, int hours, int days, int weeks, int months)
{
	// check condition timeouts

	for(int c=0;c<pcConditions.size();c++)
	{
		bool updateNeeded = false;
		std::vector<int> expired;
		for (int d = 0; d < pcConditions[c].size(); d++)
		{
			std::string recoveryType = gGame->mConditionManager->GetRecovery(pcConditions[c][d].first);
			// special conditions do not degrade over time
			if (recoveryType != "Special")
			{
				if (pcConditions[c][d].second != -255)
				{
					pcConditions[c][d].second -= rounds * 10.0L;
					if (pcConditions[c][d].second < 0)
					{
						expired.push_back(d);
					}
				}
				for (int e = 0; e < expired.size(); e++)
				{
					// check for the specific case where we were bleeding to death
					if (gGame->mConditionManager->GetNameFromIndex(expired[e]) == "Dying")
					{
						// bled out
						gGame->CharacterDeath(c);
					}
					else
					{
						pcConditions[c].erase(pcConditions[c].begin() + expired[e]);
						updateNeeded = true;
					}
				}
			}
		}
		if(updateNeeded)
		{
			UpdateCapabilities(c);
		}
	}

	return true;
}

int CharacterManager::AbilityBonus(int characteristicValue)
{
	return AbilityBonuses[characteristicValue];
}

int CharacterManager::GetAbilityIndexForTag(std::string tag)
{
	// if this tag exists
	if(CharacteristicTags.count(tag) > 0)
	{
		// return the associated ability 
		return CharacteristicTags[tag];
	}
	else
	{
		// if the tag doesn't exist, then no bonus
		return 0;
	}
}

int CharacterManager::GenerateFighter(std::string name)
{
	return GenerateTestCharacter(name,"Fighter");
}

const ACKSClass* CharacterManager::getCharacterClass(int id)
{
	int classIndex = pcClass[id];
	return &(gGame->mClassManager->Classes().Classes()[classIndex]);
}

void CharacterManager::BaseGenerate()
{
	// 0 xp
	pcExperience.push_back(0);

	// no armour (to begin with)
	pcCurrentArmourClass.push_back(0);

	// no condition
	std::vector<std::pair<int,int>> b;
	pcConditions.push_back(b);

	// basic capabilities
	pcCapabilityFlags.push_back(GenerateBaseCapabilityFlags());

	// empty inventory and equipment
	std::list<int> a;
	pcInventory.push_back(a);

	std::vector<int> e;
	e.resize(EQUIP_MAX, -1);
	pcEquipped.push_back(e);

	// tag cache
	pcCollectedTags.push_back(std::map<std::string, int>());

	// copy class armour and weapon proficiencies to the cache
	pcArmourProficiencies.push_back(std::vector<std::string>());
	pcWeaponProficiencies.push_back(std::vector<std::string>());
	pcFightingStyles.push_back(std::vector<std::string>());

	// set default (non-real) position and map
	pcMapID.push_back(-1);
	pcXPos.push_back(-1);
	pcYPos.push_back(-1);

	// set no behaviour
	pcCurrentBehaviour.push_back(-1);

	// set no domain action/travel mode
	pcDomainAction.push_back("");
	pcTravelModes.push_back({"Normal"});

	// remaining cleaves
	pcRemainingCleaves.push_back(0);

	std::vector<MortalEffect*> meff;
	pcMortalWounds.push_back(meff);
}

int CharacterManager::GenerateNormalMan(std::string name)
{
	int output = nextCharacterIndex++;

	DebugLog("Generating a Normal Man as #" + std::to_string(output));

	// we need to add something to every vector, to make sure we line up

	// Standard test character has a 9 in everything, allowing them to play most standard classes
	std::vector<int> characs = std::vector<int>();
	for (int i = 0; i < 6; i++)
		characs.push_back(9);
	pcCharacteristics.push_back(characs);

	// class is basically Fighter except level 0
	int characterClass = gGame->mClassManager->GetClassIndex("Fighter");
	const ACKSClass& acksClass = gGame->mClassManager->Classes().Classes()[characterClass];
	pcClass.push_back(characterClass);

	pcName.push_back(name);

	// NM is d4
	int hitDie = 4;

	// max hit die at first level
	pcTotalHitPoints.push_back(hitDie);
	pcCurrentHitPoints.push_back(hitDie);

	// level 0
	pcLevel.push_back(0);
	
	BaseGenerate();

	// and build the caches
	UpdateProficiencyCache(output);
	UpdateTagCache(output);
	UpdateCapabilities(output);

	// dump debug output of character

	DumpProficiencyCache(output);
	DumpTagCache(output);
	
	return output;
}

int CharacterManager::GenerateTestCharacter(std::string name, const std::string _class)
{
	int output = nextCharacterIndex++;

	DebugLog("Generating a " + _class +"as #" + std::to_string(output));

	// we need to add something to every vector, to make sure we line up

	// Standard test character has a 9 in everything, allowing them to play most standard classes
	std::vector<int> characs = std::vector<int>();
	for (int i = 0; i < 6; i++) 
		characs.push_back(9);
	pcCharacteristics.push_back(characs);

	// class
	int characterClass = gGame->mClassManager->GetClassIndex(_class);
	const ACKSClass& acksClass = gGame->mClassManager->Classes().Classes()[characterClass];
	pcClass.push_back(characterClass);

	pcName.push_back(name);

	// level 1
	pcLevel.push_back(1);


	// get hit points from class
	int hitDie = acksClass.HitDie();

	// max hit die at first level
	pcTotalHitPoints.push_back(hitDie);
	pcCurrentHitPoints.push_back(hitDie);
	
	BaseGenerate();
	
	// and build the caches
	UpdateProficiencyCache(output);
	UpdateTagCache(output);
	UpdateCapabilities(output);

	// dump debug output of character

	DumpProficiencyCache(output);
	DumpTagCache(output);

	
	return output;
}

int CharacterManager::SelectBehaviour(int entityID)
{
	int result = -1;
	// TODO: Add sensorium checks here

	if (getCharacterHasCondition(entityID, "Unconscious")) return CHAR_BEHAVIOUR_UNCONSCIOUS;

	// doing nothing special yet, set wander

	result = behaviourLookup["Wander"];
	
	SetBehaviour(entityID, result);
	
	return result;
}

void CharacterManager::SetBehaviour(int entityID, int behaviourType)
{
	pcCurrentBehaviour[entityID] = behaviourType;
}

void CharacterManager::UpdateProficiencyCache(int characterID)
{
	ACKSClass ac = gGame->mClassManager->Classes().Classes()[pcClass[characterID]];
	
	auto& armourCache = pcArmourProficiencies[characterID];
	auto& weaponCache = pcWeaponProficiencies[characterID];
	auto& styleCache = pcFightingStyles[characterID];

	armourCache.clear();
	weaponCache.clear();
	styleCache.clear();
	
	auto classArmourProfs = ac.ArmourProficiencies();
	auto classWeaponProfs = ac.WeaponProficiencies();
	auto classFightingStyles = ac.FightingStyles();

	armourCache.insert(armourCache.end(), classArmourProfs.begin(), classArmourProfs.end());
	weaponCache.insert(weaponCache.end(), classWeaponProfs.begin(), classWeaponProfs.end());
	styleCache.insert(styleCache.end(), classFightingStyles.begin(), classFightingStyles.end());
}

void CharacterManager::UpdateCapabilities(int characterID)
{
	// start with base capabilities, universal to all characters
	unsigned long long capabilityFlags = GenerateBaseCapabilityFlags();

	// add all class capabilities
	ACKSClass ac = gGame->mClassManager->Classes().Classes()[pcClass[characterID]];

	for(LevelledAbility la : ac.LevelledAbilities())
	{
		std::string name = la.Type();
		if(name.compare(0,11,"Capability:") == 0)
		{
			// this is a capability flag
			std::string capability = name.substr(11, name.length() - 11);
			unsigned long long capability_index = CapabilityLookup.at(capability);
			capabilityFlags |= capability_index;
		}
	}
	
	// add all fighting style capabilities
	for(std::string s : pcFightingStyles[characterID])
	{
		if(s=="Paired Weapon")
		{
			capabilityFlags |= CAPABILITY_DUAL_WIELD;
		}
		else if(s=="Weapon And Shield")
		{
			capabilityFlags |= CAPABILITY_USE_SHIELD;
		}
		else if(s=="Two-Handed Weapon")
		{
			capabilityFlags |= CAPABILITY_USE_TWO_HANDED;
		}
	}
	
	// add all proficiency capabilities

	// restrict based on conditions
	
	// restrict based on mortal effects

	for(MortalEffect* effect : pcMortalWounds[characterID])
	{
		for(SpecialPenalty penalty : effect->SpecialPenalties())
		{
			std::string name = penalty.Name();
			if (name.compare(0, 11, "Capability:") == 0)
			{
				std::string capability = name.substr(11, name.length() - 11);
				unsigned long long capability_index = CapabilityLookup.at(capability);
				capabilityFlags &= ~capability_index;
			}
		}
	}

	// store results
	pcCapabilityFlags[characterID] = capabilityFlags;
}

std::string CharacterManager::DumpProficiencyCache(int characterID)
{
	DebugLog("Dumping Proficiencies for " + this->getCharacterName(characterID) + ":");

	std::string aProfList = "Armour:";
	for(std::string aProf : pcArmourProficiencies[characterID])
	{
		aProfList += aProf + ",";
	}
	aProfList += "\n";

	std::string wProfList = "Weapon:";
	for (std::string wProf : pcWeaponProficiencies[characterID])
	{
		wProfList += wProf + ",";
	}
	wProfList += "\n";

	std::string sProfList = "Fighting Styles:";
	for (std::string sProf : pcFightingStyles[characterID])
	{
		sProfList += sProf + ",";
	}
	sProfList = "\n";

	std::string output = aProfList + wProfList + sProfList;
	DebugLog(output);
	return output;
}

std::string CharacterManager::DumpConditions(int characterID)
{
	std::string output = "Conditions:";

	for(int i=0; i<pcConditions[characterID].size(); i++)
	{
		if (i != 0) output += ",";
		std::pair<int,int> s = pcConditions[characterID][i];
		int idx = s.first;
		std::string name = gGame->mConditionManager->GetNameFromIndex(idx);
		output += name;
	}
	output += ".";
	return output;
}

std::string CharacterManager::DumpCapabilities(int characterID)
{
	std::string output = "Capabilities:";

	unsigned long long capabilityIndex = 1ULL;
	int nameIndex = 0;
	while(nameIndex < CAPABILITY_MAX)
	{
		if (getCharacterCapabilityFlag(characterID, (CapabilityFlags)capabilityIndex))
		{
			if (capabilityIndex != 0x01) output += ",";
			output += CapabilityNames[nameIndex];
		}
		nameIndex++;
		capabilityIndex = capabilityIndex << 1;
	}
	output += ".";
	return output;
}

unsigned long long CharacterManager::GenerateBaseCapabilityFlags()
{
	return	CAPABILITY_VISION_NORMAL | CAPABILITY_HEARING_NORMAL | CAPABILITY_SMELL | CAPABILITY_AWARENESS_NORMAL |
		CAPABILITY_COGNITION | CAPABILITY_FULL_MOVEMENT | CAPABILITY_FORCED_MARCH | CAPABILITY_DEFENCE | 
		CAPABILITY_ATTACK | CAPABILITY_USE_ITEM | CAPABILITY_ACTION | CAPABILITY_SPEECH | CAPABILITY_TREAT_WOUNDS | CAPABILITY_NATURAL_HEALING;
}

int CharacterManager::EquipShield(int characterID, int itemID)
{
	// we can only equip if we have the Weapon & Shield fighting style
	if (CanUseStyle(characterID, "Weapon And Shield"))
	{
		pcEquipped[characterID][HAND_OFF] = itemID;
		DebugLog("Equipped " + gGame->mItemManager->getName(itemID) + " in off-hand");
		return HAND_OFF;
	}

	DebugLog("Can't equip shield without proficiency");
	return -1;
}

int CharacterManager::EquipArmour(int characterID, int itemID)
{
	// armour proficiencies check

	if (CanUseItem(characterID, itemID))
	{
		pcEquipped[characterID][ARMOUR] = itemID;
		DebugLog("Equipped " + gGame->mItemManager->getName(itemID));
		return ARMOUR;
	}

	DebugLog("Can't equip armour without proficiency");
	return -1;
}

int CharacterManager::GetCleaveCount(int characterID)
{
	auto acks_class = gGame->mCharacterManager->getCharacterClass(characterID);
	const std::string progression = acks_class->AttackProgression();
	int level = pcLevel[characterID];

	if(progression == "Fighter")
	{
		return level;
	}

	if (progression == "Thief" || progression == "Cleric")
	{
		return level / 2;
	}

	return 0;	
}

int CharacterManager::EquipWeapon(int characterID, int itemID)
{
	// Note that this function will NOT unequip existing items, you need to do that upstream

	// with all weapons, shields, armour etc we need to check that we have proficiency with them and can use the appropriate style.	
	bool one_h = gGame->mItemManager->hasTag(itemID, "One-Handed");
	bool two_h = gGame->mItemManager->hasTag(itemID, "Two-Handed");

	std::string item_name = gGame->mItemManager->getName(itemID);

	int output = -1;

	if (two_h && !one_h)
	{
		// two-hander only
		if (pcEquipped[characterID][HAND_OFF] == -1 && pcEquipped[characterID][HAND_MAIN] == -1)
		{
			if (CanUseStyle(characterID, "Two-Handed Weapon"))
			{
				// we can use the two-handed style, so lets wield this two-handed!
				pcEquipped[characterID][HAND_MAIN] = itemID;
				pcEquipped[characterID][HAND_OFF] = itemID;
				DebugLog(getCharacterName(characterID) + " equips " + item_name + " two-handed.");
				return HAND_MAIN;
			}
		}
	}
	
	if (one_h)
	{
		if (two_h)
		{
			// 1 and 2 hands: bastard weapon
			// if we have both our hands currently free, equip this as a 2-hander
			if (pcEquipped[characterID][HAND_OFF] == -1 && pcEquipped[characterID][HAND_MAIN] == -1)
			{
				if (CanUseStyle(characterID, "Two-Handed Weapon"))
				{
					// we can use the two-handed style, so lets wield this two-handed!
					pcEquipped[characterID][HAND_MAIN] = itemID;
					pcEquipped[characterID][HAND_OFF] = itemID;
					DebugLog(getCharacterName(characterID) + " equips " + item_name + " two-handed.");
					return HAND_MAIN;
				}
			}
			// otherwise drop through to main 1-h logic
		}
		if (pcEquipped[characterID][HAND_MAIN] != -1)
		{
			// we have something in our on-hand, so wield this with our off-hand if we can
			if (CanUseStyle(characterID, "Paired Weapon") && pcEquipped[characterID][HAND_OFF] != -1)
			{
				// we can do paired weapons, so lets do it
				pcEquipped[characterID][HAND_OFF] = itemID;
				DebugLog(getCharacterName(characterID) + " equips " + item_name + " in off-hand.");
				return HAND_OFF;
			}
			else
			{
				// we tried to equip a second weapon and we can't use paired, so nowt
				DebugLog(getCharacterName(characterID) + " can't dual wield " + item_name);
				return -1;
			}
		}

		// if we reach here, we have nothing in our on-hand

		if (pcEquipped[characterID][HAND_OFF] != -1)
		{
			// we have something in our off-hand but not our main hand.
			// if it's a shield and we can sword & board, or if it's a weapon and we can pair, then equip the weapon one-handed in our main hand.

			int offhandItem = pcEquipped[characterID][HAND_OFF];

			if (gGame->mItemManager->hasTag(offhandItem, "Shield"))
			{
				if (CanUseStyle(characterID, "Weapon And Shield"))
				{
					pcEquipped[characterID][HAND_MAIN] = itemID;
					DebugLog(getCharacterName(characterID) + " equips " + item_name + " alongside their shield.");
					return HAND_MAIN;
				}
				else
				{
					// wield nothing
					DebugLog(getCharacterName(characterID) + " cannot wield " + item_name + " alongside their shield.");
					return -1;
				}
			}

			if (gGame->mItemManager->hasTag(offhandItem, "Weapon"))
			{
				if (CanUseStyle(characterID, "Paired Weapon"))
				{
					pcEquipped[characterID][HAND_MAIN] = itemID;
					DebugLog(getCharacterName(characterID) + " equips " + item_name + " paired.");
					return HAND_MAIN;
				}
				else
				{
					// wield nothing
					DebugLog(getCharacterName(characterID) + " cannot wield " + item_name + " paired.");
					return -1;
				}
			}

			// if we reach here, then our other item is some misc nonsense, so wield normally

			pcEquipped[characterID][HAND_MAIN] = itemID;
			DebugLog(getCharacterName(characterID) + " equips " + item_name + " in on-hand.");
			return HAND_MAIN;
		}
	}
	return output;
}

int CharacterManager::GetItemIndex(int characterID, int inventoryID)
{
	if (pcInventory[characterID].size() == 0)
		return -1;

	std::list<int>& inv = pcInventory[characterID];

	std::list<int>::iterator inv_it = inv.begin();
	std::advance(inv_it, inventoryID);

	return *inv_it;

}

void CharacterManager::UnequipItem(int characterID, int inventoryID)
{
	int itemID = GetItemIndex(characterID, inventoryID);

	for(int i=0;i<pcEquipped[characterID].size();i++)
	{
		if(itemID == pcEquipped[characterID][i])
		{
			pcEquipped[characterID][i] = -1;
		}
	}
	std::string item_name = gGame->mItemManager->getName(itemID);
	DebugLog(getCharacterName(characterID) + " unequips " + item_name);
}

int CharacterManager::EquipItem(int characterID, int inventoryID)
{
	if (pcInventory[characterID].size() == 0)
		return -1;
	
	std::list<int>& inv = pcInventory[characterID];

	std::list<int>::iterator inv_it = inv.begin();
	std::advance(inv_it, inventoryID);

	int itemID = *inv_it;

	int output = -1;

	if (CanUseItem(characterID, itemID))
	{
		if (gGame->mItemManager->hasTag(itemID, "Weapon"))
		{
			output = EquipWeapon(characterID, itemID);
			UpdateCurrentAttackValue(characterID, gGame->mItemManager->hasTag(itemID, "Missile"));
		}

		if (gGame->mItemManager->hasTag(itemID, "Shield"))
		{
			output = EquipShield(characterID, itemID);
		}

		if (gGame->mItemManager->hasTag(itemID, "Armour"))
		{
			output = EquipArmour(characterID, itemID);
		}
	}
	return output;
}

bool CharacterManager::CanUseStyle(int characterID, const std::string style)
{
	int classID = pcClass[characterID];
	auto p = gGame->mClassManager->Classes().Classes()[classID];
	auto v = p.FightingStyles();
	auto n = std::find(v.begin(), v.end(), style);
	return (n != v.end());
}


bool CharacterManager::CanUseItem(int characterID, int itemID)
{
	// this function checks for proficiencies. magic items will use something else

	bool is_weapon = gGame->mItemManager->hasTag(itemID, "Weapon");
	if(is_weapon)
	{
		// this is a weapon. If any of the weapon's tags match the user's class proficiencies, we can use this weapon
		if(std::find_if(pcWeaponProficiencies[characterID].begin(), pcWeaponProficiencies[characterID].end(), [itemID](const std::string& prof) -> bool {return gGame->mItemManager->hasTag(itemID, prof); }) != pcWeaponProficiencies[characterID].end())
		{
			// match found!
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		bool is_armour = gGame->mItemManager->hasTag(itemID, "Armour");
		if(is_armour)
		{
			if (std::find_if(pcArmourProficiencies[characterID].begin(), pcArmourProficiencies[characterID].end(), [itemID](const std::string& prof) -> bool {return gGame->mItemManager->hasTag(itemID, prof); }) == pcArmourProficiencies[characterID].end())
			{
				// match found!
				return true;
			}
			else
			{
				return false;
			}
		}
	}

	bool is_shield = gGame->mItemManager->hasTag(itemID, "Shield");
	if(is_shield)
	{
		if(CanUseStyle(characterID, "Weapon And Shield"))
		{
			return true;
		}
	}

	return false;
}


int CharacterManager::AddInventoryItem(int characterID, int itemID)
{
	int newIndex = pcInventory[characterID].size();
	pcInventory[characterID].push_back(itemID);
	return newIndex;
}

int CharacterManager::RemoveInventoryItem(int characterID, int inventoryID)
{
	auto list = pcInventory[characterID];

	if (list.size() == 0)
		return -1;
	auto iter = list.begin();
	std::advance(iter, inventoryID);

	int output = *iter;

	list.erase(iter);
	
	return output;
}

void CharacterManager::UpdateTagCache(int characterID)
{
	// tag values come from a variety of places
	// 1) Your Characteristics
	// 2) Your Class and Level
	// 3) Your Equipment
	// 4) Your Mortal Wounds (usually negative)
	// These are used as the value modifiers for a variety of values and are totalled here.

	auto &tags = pcCollectedTags[characterID];
	
	// abilities unlocked per-level (as opposed to levelled bonuses)
	// these are not summed
	for (auto kv : getCharacterClass(characterID)->LevelledAbilities())
	{
		if (getCharacterLevel(characterID) > kv.Level())
		{
			tags[kv.Type()] = kv.Value();
		}
	}

	// characteristic bonuses
	for(auto kv : CharacteristicTags)
	{
		tags[kv.first] += getCharacterAbilityBonus(characterID, kv.second);
	}

	// levelled bonuses from class
	for (auto kv : getCharacterClass(characterID)->LevelTagBonuses)
	{
		tags[kv.first] += kv.second[getCharacterLevel(characterID)];
	}

	for (MortalEffect* effect : pcMortalWounds[characterID])
	{
		for (SpecialPenalty penalty : effect->SpecialPenalties())
		{
			std::string name = penalty.Name();
			// if it's not a Capability, it's a Tag
			if (name.compare(0, 11, "Capability:"))
			{
				tags[name] = penalty.Value();
			}
		}
	}
}

std::string CharacterManager::DumpTagCache(int characterID)
{
	DebugLog("Dumping Tags for " + this->getCharacterName(characterID));

	std::string tagList;
	for (auto tag : pcCollectedTags[characterID])
	{
		tagList += tag.first + ":" + std::to_string(tag.second) + ", ";
	}
	tagList += "\n";
	DebugLog(tagList);
	return tagList;
}

int CharacterManager::getTagValue(int characterID, std::string tag)
{
	auto &tags = pcCollectedTags[characterID];
	return tags[tag];
}

int CharacterManager::UpdateCurrentAttackValue(int characterID, bool missile)
{
	// TODO: Fighting style bonuses to attack value
	
	auto advancement = gGame->mClassManager->GetAdvancementStore();
	auto acks_class = gGame->mCharacterManager->getCharacterClass(characterID);
	int level = gGame->mCharacterManager->getCharacterLevel(characterID);

	std::string debugOutput = this->getCharacterName(characterID) + " calculating attack value with base ";
	
	auto attack_bonuses = advancement->AttackBonusLookup[acks_class->AttackProgression()];
	int attack_bonus = attack_bonuses[level];

	debugOutput += std::to_string(attack_bonus);

	std::string attack_tag = "Attack:Melee";
	if(missile)
	{
		attack_tag = "Attack:Missile";
	}

	int tagValue = getTagValue(characterID, attack_tag);
	debugOutput += " and tag bonus " + std::to_string(tagValue);
	
	int output = attack_bonus - tagValue; // why minus? Because our attack values are roll-above, eg 7 gives "7+". So a bonus of 2 gives "5+"
	debugOutput += " for a total of " + std::to_string(output);
	DebugLog(debugOutput);
	return output;
}

int CharacterManager::UpdateCurrentSaveValue(int characterID, std::string save)
{
	auto advancement = gGame->mClassManager->GetAdvancementStore();
	auto classes = advancement->SaveLookup[save];
	int level = gGame->mCharacterManager->getCharacterLevel(characterID);
	auto acks_class = gGame->mCharacterManager->getCharacterClass(characterID);

	auto by_level = classes[acks_class->Name()];

	int save_value = by_level[level];

	std::string save_tag = "Saves:" + save;

	int output = save_value + getTagValue(characterID, save_tag);
	DebugLog(this->getCharacterName(characterID) + " updated current " + save + " value to " + std::to_string(output));
	return output;
}

double CharacterManager::GetCurrentEncumbrance(int characterID)
{
	auto inv = GetInventory(characterID);
	return gGame->mItemManager->getWeight(inv);
}

int CharacterManager::GetCurrentSpeed(int characterID)
{
	// base of encumbrance for humans/elves/dwarves (PCs in general) is fixed for laziness reasons
	// If we ever add any other PC races we should prooooobably put this in a data file

	// when we redo this function in the mob manager we will need to load these in from the mob JSON
	const double BASE_MOVEMENT = 30.0;
	

	// calculations for this.
	// encumbrance load can be 0-1/2 Base (Minimal, full speed), 1/2 - 3/4 Base (Light), 3/4 - Base (Medium), Base - 2xBase (Heavy)
	// Exploration Speeds are 120/90/60/30 for each category

	// first get the encumbrance class (split off so we can also use this for text response)
	int encumbrance = GetEncumbranceClass(characterID);

	double calcMovement = (4 - encumbrance) * BASE_MOVEMENT;

	return calcMovement;
	
}

int CharacterManager::GetEncumbranceClass(int characterID)
{
	const double BASE_LOAD = 10.0;

	const double minimal = BASE_LOAD * 0.5;
	const double light = BASE_LOAD * 0.75;
	const double medium = BASE_LOAD * 1.0;
	const double heavy = BASE_LOAD * 2.0;

	double currentEnc = GetCurrentEncumbrance(characterID);

	if(currentEnc <= minimal)
	{
		return 0;
	}
	else if(currentEnc > minimal && currentEnc <= light )
	{
		return 1;
	}
	else if(currentEnc > light && currentEnc <= medium )
	{
		return 2;
	}
	else
	{
		return 3;
	}
}

std::string CharacterManager::GetCurrentEncumbranceType(int characterID)
{
	const std::string thing[] = { "Minimal","Light","Medium","Heavy" };
	int encumbranceClass = GetEncumbranceClass(characterID);
	DebugLog(this->getCharacterName(characterID) + " recalculated current encumbrance class as " + thing[encumbranceClass]);
	return thing[encumbranceClass];
}

int CharacterManager::GetCurrentDamageBonus(int characterID, bool missile)
{
	// TODO: Fighting style bonuses to damage

	int damageBonus = 0;

	std::string damageTag = missile ? "Damage:Missile" : "Damage:Melee";

	damageBonus += getTagValue(characterID, damageTag);

	DebugLog(this->getCharacterName(characterID) + " recalculated current damage bonus:" + std::to_string(damageBonus));
	
	return damageBonus;
}

int CharacterManager::GetCurrentAC(int characterID)
{
	// TODO: Fighting style bonuses to AC
	
	// AC calculation: Armour AC + Shield AC + Tags AC (including Dex), minimum AC 0
	int armourID = GetItemInEquipSlot(characterID, ARMOUR);
	int shieldID = GetItemInEquipSlot(characterID, HAND_OFF);

	std::string debugOut = this->getCharacterName(characterID) + " recalculating AC with ";
	
	// if the thing in our offhand isn't some form of shield, ignore it
	if(shieldID != -1 && !gGame->mItemManager->hasTag(shieldID, "Shield"))
	{
		shieldID = -1;
	}

	int AC = 0;
	
	if(armourID != -1)
	{
		debugOut += gGame->mItemManager->getName(armourID) + ", ";
		auto tags = gGame->mItemManager->getTags(armourID);
		auto ac_tag = std::find_if(tags.begin(), tags.end(), [](std::string s) -> bool { const std::string pre = "AC|"; return (s.compare(0, pre.size(), pre) == 0); });
		if(ac_tag != tags.end())
		{
			std::string value = (*ac_tag).substr(3);
			int ac_value = std::stoi(value);
			AC += ac_value;
		}
	}

	if (shieldID != -1)
	{
		debugOut += gGame->mItemManager->getName(shieldID) + " ";
		auto tags = gGame->mItemManager->getTags(shieldID);
		auto ac_tag = std::find_if(tags.begin(), tags.end(), [](std::string s) -> bool { const std::string pre = "AC|"; return (s.compare(0, pre.size(), pre) == 0); });
		if (ac_tag != tags.end())
		{
			std::string value = (*ac_tag).substr(3);
			int ac_value = std::stoi(value);
			AC += ac_value;
		}
	}

	int ac_value = getTagValue(characterID, "Defence:AC");
	debugOut += "and AC bonus of " + std::to_string(ac_value);
	AC += ac_value;
	debugOut += " for a total AC of " + std::to_string(AC);
	pcCurrentArmourClass[characterID] = AC;
	DebugLog(debugOut);
	return AC;
}

int CharacterManager::GetEquipSlotForInventoryItem(int characterID, int inventoryID)
{
	auto p = pcInventory[characterID].begin();
	std::advance(p, inventoryID);
	int itemID = *p;
	for (int j = 0; j < EQUIP_MAX; j++)
	{
		if (GetItemInEquipSlot(characterID, j) == itemID)
		{
			return j;
		}
	}
	return -1;
}

bool CharacterManager::getCharacterHasCondition(int id, int condition)
{
	auto p = std::find_if(pcConditions[id].begin(), pcConditions[id].end(), [&](std::pair<int, int> t_cond) { return t_cond.first == condition; });
	return p != pcConditions[id].end();
}

bool CharacterManager::getCharacterHasCondition(int id, std::string condition)
{
	int index = gGame->mConditionManager->GetConditionIndex(condition);
	return getCharacterHasCondition(id, index);
}

// Set a condition. Usually inflicted on us by others.
int CharacterManager::SetCondition(int id, int condition,int time)
{
	DebugLog(this->getCharacterName(id) + " setting condition " + gGame->mConditionManager->GetNameFromIndex(condition));
	
	// don't duplicate
	if(!getCharacterHasCondition(id, condition))
	{
		std::pair<int, int> entry(condition,time);
		pcConditions[id].push_back(entry);
		return condition;
	}

	return -1;
}

// Used either by the timing system or via an action to explicitly remove the condition
// Eg - turn-length condition removed at end of turn, "Prone" removed by "stand up" action
int CharacterManager::RemoveCondition(int id, int condition)
{
	std::vector<std::pair<int,int>>::iterator iter = std::find_if(pcConditions[id].begin(), pcConditions[id].end(), [&](std::pair<int, int> t_cond) { return t_cond.first == condition; });
	if(iter != pcConditions[id].end())
	{
		DebugLog(this->getCharacterName(id) + " removing condition " + gGame->mConditionManager->GetNameFromIndex(condition));
		std::pair<int,int> value = *iter;
		pcConditions[id].erase(iter);
		return value.first;
	}

	return -1;
}

int CharacterManager::ReduceCondition(int id, int condition, int timeToReduce)
{
	std::vector<std::pair<int, int>>::iterator iter = std::find_if(pcConditions[id].begin(), pcConditions[id].end(), [&](std::pair<int, int> t_cond) { return t_cond.first == condition; });
	if (iter != pcConditions[id].end())
	{
		(*iter).second -= timeToReduce;
		if ((*iter).second <= 0.0L)
		{
			// counted down so remove it
			return RemoveCondition(id, condition);
		}
	}
	return -1;
}

int CharacterManager::SetCondition(int id, std::string condition,int time)
{
	int index = gGame->mConditionManager->GetConditionIndex(condition);
	return SetCondition(id, index,time);
}

int CharacterManager::RemoveCondition(int id, std::string condition)
{
	int index = gGame->mConditionManager->GetConditionIndex(condition);
	return RemoveCondition(id, index);
}

int CharacterManager::ReduceCondition(int id, std::string condition, int timeToReduce)
{
	int index = gGame->mConditionManager->GetConditionIndex(condition);
	return ReduceCondition(id, index, timeToReduce);
}

void CharacterManager::DebugLog(std::string message)
{
	gLog->Log("CharacterManager", message);
}

void CharacterManager::DumpCharacter(int characterID)
{
	// this dumps all character details

	DebugLog("Dumping Character #" + std::to_string(characterID));

	std::string name = pcName[characterID];

	std::string filename = "dump_" + name + "_" + std::to_string(characterID) + ".txt";
	
	std::ofstream dumpFile;

	dumpFile.open(filename);

	std::string dumpLine;

	dumpLine = "Name:" + name;
	dumpFile << dumpLine << std::endl;

	int cl = pcClass[characterID];
	ACKSClass acks_class = gGame->mClassManager->Classes().Classes()[cl];
	dumpLine = "Class:" + acks_class.Name();
	dumpFile << dumpLine << std::endl;
	int level = pcLevel[characterID];
	dumpLine = "Level " + std::to_string(level) + " (" + acks_class.LevelTitles[level] + ")";
	dumpFile << dumpLine << std::endl;
	int xp = pcExperience[characterID];
	if (level < acks_class.LevelXPValues.size() - 1)
	{
		int next_xp = acks_class.LevelXPValues[level + 1];
		dumpLine = "Experience:" + std::to_string(xp) + " (" + std::to_string(next_xp) + " to next level)";
	}
	else
	{
		dumpLine = "Experience:" + std::to_string(xp);
	}
	dumpFile << dumpLine << std::endl;
	int hp = pcCurrentHitPoints[characterID];
	int total_hp = pcTotalHitPoints[characterID];
	dumpLine = "HP:" + std::to_string(hp) + "/" + std::to_string(total_hp);
	dumpFile << dumpLine << std::endl;

	std::string ab_melee = "Melee Attack:" + std::to_string(UpdateCurrentAttackValue(characterID, false));
	dumpFile << ab_melee << std::endl;
	std::string ab_missile = "Missile Attack:" + std::to_string(UpdateCurrentAttackValue(characterID, true));
	dumpFile << ab_missile << std::endl;

	for (int i = 0; i < 5; i++)
	{
		std::string save_name = saveTypes[i];

		std::string display_name = save_name.substr(0, 13);

		int saveVal = UpdateCurrentSaveValue(characterID, save_name);
		std::string save = display_name + ":" + std::to_string(saveVal);
		dumpFile << save << std::endl;
	}

	std::string ac = "AC:" + std::to_string(GetCurrentAC(characterID));
	dumpFile << ac << std::endl;
	std::string enc = "Encumbrance:" + GetCurrentEncumbranceType(characterID);
	dumpFile << enc << std::endl;

	dumpFile << std::endl;

	dumpFile << DumpProficiencyCache(characterID);
	dumpFile << std::endl;

	dumpFile << DumpTagCache(characterID);
	dumpFile << std::endl;

	dumpFile << DumpCapabilities(characterID);
	dumpFile << std::endl;

	dumpFile << DumpConditions(characterID);
	dumpFile << std::endl;

	std::string behaviour_name = "Current Behaviour:";
	int behaviour = pcCurrentBehaviour[characterID];
	if(behaviour==-1)
	{
		behaviour_name += "None";
	}
	else
	{
		behaviour_name += CharBehaviourNames[behaviour];
	}
	dumpFile << behaviour_name << std::endl;

	int map = pcMapID[characterID];
	std::string map_text = "Current Map:" + (map == -1 ? "None" : std::to_string(map));
	dumpFile << map_text << std::endl;
	// inventory?

	dumpFile.close();
}

void CharacterManager::SpawnOnMap(int entityID, int mapID, int spawn_x, int spawn_y)
{
	gLog->Log("Character Manager", "Spawning " + getCharacterName(entityID) + " onto map #" + std::to_string(mapID));
	
	pcMapID[entityID] = mapID;

	const int MAX_SPAWN_DIST = 255;

	if (mapID != -1)
	{
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
							gGame->mTimeManager->SetEntityTime(entityID, MANAGER_CHARACTER, moveTime);
							found = true;
							gLog->Log("Character Manager", "Spawned " + getCharacterName(entityID) + " onto map #" + std::to_string(mapID) + " at position (" + std::to_string(new_x) + "," + std::to_string(new_y) + ")");
						}
					}
				}
				ticker--;
			}
			if (!found) dist++;
		}
	}
}

double CharacterManager::MoveTo(int entityID, int new_x, int new_y, int currentTime)
{
	int mapID = pcMapID[entityID];
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
					// there's another character there. In future check for party membership
				}
				else
				{
					int mobId = m->getMobAt(new_x, new_y);
					if (mobId)
					{
						// there's a monster there. Check for MURDERIZATION!
						Creature& c = gGame->mMobManager->GetMonster(mobId);

						if (c.IsBlocking())
						{
							// set cleave count for current character (remember this is reset if we change, balanced by change delay)
							pcRemainingCleaves[entityID] = GetCleaveCount(entityID);

							if (c.IsHostile())
							{
								// the monster is hostile, automatically attack
								gGame->ResolveAttacks(MANAGER_CHARACTER, entityID, MANAGER_MOB, mobId, false);
							}
							else
							{
								// do not attack non-hostiles

							}
						}
					}
					else
					{
						// there isn't another creature there, so move
						int oldX = GetPlayerX(entityID);
						int oldY = GetPlayerY(entityID);
						if (oldX > -1 && oldY > -1)
							m->setCharacter(oldX, oldY, 0);
						SetPlayerX(entityID,new_x);
						SetPlayerY(entityID, new_y);
						
						m->setCharacter(new_x, new_y, entityID);

						timeExpended = gGame->mMapManager->getMovementTime(mapID, GetCurrentSpeed(entityID));
					}
				}
			}
		}
	}
	return timeExpended;
}

std::vector<MortalEffect*> CharacterManager::GetMortalEffects(int id)
{
	return pcMortalWounds[id];
}

void CharacterManager::AddMortalEffect(int id, MortalEffect* effect)
{
	// check to see if we already have this Mortal Effect. If we have it already, we don't add it, but we check DoubleTo to see if it upgrades
	bool found = false;
	std::vector<MortalEffect*> newEffects;
	for(MortalEffect* eff : pcMortalWounds[id])
	{
		if((eff->Code() == effect->Code()) && (eff->DoubleTo() != ""))
		{
			// we upgrade the existing effect to this, keeping the existing timeframe
			MortalEffect* meff = gGame->mMortalManager->GetMortalEffectFromCode(eff->DoubleTo());
			newEffects.push_back(meff);
			found = true;
		}
		else
		{
			// add this to the new list
			newEffects.push_back(eff);
		}
	}
	// now add the new one if we didn't find a doubleup
	if(!found)
	{
		newEffects.push_back(effect);
	}

	pcMortalWounds[id] = newEffects;

	// because we've added a Mortal Wound, we must update the Capabilities and Tag Cache to reflect the changes to the character
	UpdateCapabilities(id);
	UpdateTagCache(id);
}

void CharacterManager::DeactivateCharacter(int characterID)
{
	// when a character dies, we don't delete it from the lists as that would produce all manner of weird effects and is unnecessary.
	// instead, we unset it from the map and remove all capabilities. This will prevent the character from "ticking" etc
	// and retain the character info for future use (eg RL&L, stored XP, memoriam screens etc)

	if(pcMapID[characterID] != -1)
	{
		gGame->mMapManager->getMap(pcMapID[characterID])->removeCharacter(characterID);
	}
	
	pcMapID[characterID] = -1;
	pcXPos[characterID] = -1;
	pcYPos[characterID] = -1;
	pcCapabilityFlags[characterID] = 0;
}

std::vector<int> CharacterManager::GetCharactersOnMap(int mapID)
{
	std::vector<int> output;
	std::vector<int>::iterator it = pcMapID.begin();
	while ((it = std::find_if(it, pcMapID.end(), [mapID](int x) {return x == mapID; })) != pcMapID.end())
	{
		output.push_back(std::distance(pcMapID.begin(), it));
		// ReSharper disable once CppDiscardedPostfixOperatorResult
		it++;
	}
	return output;
}

std::vector<int> CharacterManager::GetTaggedCharactersOnMap(int mapID, std::string tag, bool value)
{
	std::vector<int> output;
	std::vector<int> input = GetCharactersOnMap(mapID);
	std::vector<int>::iterator it = input.begin();
	if(value)
	{
		while ((it = std::find_if(it, input.end(), [=](int c) {return getTagValue(c, tag) != 0; })) != input.end())
		{
			output.push_back(*it);
			// ReSharper disable once CppDiscardedPostfixOperatorResult
			it++;
		}
	}
	else
	{
		while ((it = std::find_if(it, input.end(), [=](int c) {return getTagValue(c, tag) == 0; })) != input.end())
		{
			output.push_back(*it);
			// ReSharper disable once CppDiscardedPostfixOperatorResult
			it++;
		}
	}
	return output;
}

std::vector<int> CharacterManager::GetConditionCharactersOnMap(int mapID, std::string condition, bool value)
{
	std::vector<int> output;
	std::vector<int> input = GetCharactersOnMap(mapID);
	std::vector<int>::iterator it = input.begin();

	while ((it = std::find_if(it, input.end(), [=](int c) {return getCharacterHasCondition(c, condition) == value; })) != input.end())
	{
		output.push_back(*it);
		// ReSharper disable once CppDiscardedPostfixOperatorResult
		it++;
	}
	
	return output;
}

std::string CharacterManager::getCharacterDomainAction(int id)
{
	return pcDomainAction[id];
}

void CharacterManager::setCharacterDomainAction(int id, std::string action)
{
	pcDomainAction[id] = action;
}

bool CharacterManager::getCharacterTravelModeSet(int id, std::string mode)
{
	std::set<std::string> g = pcTravelModes[id];
	return g.find(mode) != g.end();
}

void CharacterManager::setCharacterTravelMode(int id, std::string mode)
{
	pcTravelModes[id].insert(mode);
}

void CharacterManager::unsetCharacterTravelMode(int id, std::string mode)
{
	pcTravelModes[id].erase(mode);
}

void CharacterManager::toggleCharacterTravelMode(int id, std::string mode)
{
	if (getCharacterTravelModeSet(id, mode))
	{
		unsetCharacterTravelMode(id, mode);
	}
	else
	{
		setCharacterTravelMode(id, mode);
	}
}
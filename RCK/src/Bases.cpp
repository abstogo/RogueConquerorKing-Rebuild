#include "Bases.h"

BaseManager* BaseManager::LoadBaseData()
{
	gLog->Log("Base Loader", "Started");
	
	std::string baseFilename = "RCK/scripts/bases.json";
	std::ifstream is(baseFilename);

	BaseManager* output = new BaseManager(jsoncons::decode_json<BaseInfoSet>(is));

	is.close();

	gLog->Log("Base Loader", "Decoded " + baseFilename);

	// feed reverse lookup
	for (int i = 0; i < output->baseInfoSet.BaseTypes().size(); i++)
	{
		const BaseType& it = output->baseInfoSet.BaseTypes()[i];
		output->reverseBaseTypeDictionary[it.Name()] = i;
	}

	for (int i = 0; i < output->baseInfoSet.Tags().size(); i++)
	{
		const BaseTag& it = output->baseInfoSet.Tags()[i];
		output->reverseBaseTagDictionary[it.Tag()] = i;
	}
	
	output->nextBaseID = 0;

	gLog->Log("Base Loader", "Completed");
	
	return output;
}

int BaseManager::shellGenerate()
{
	int output = nextBaseID++;

	DebugLog("Generating Empty Base #" + std::to_string(output));

	baseType.push_back(0); // default is camp
	baseXPos.push_back(-1);
	baseYPos.push_back(-1);
	basePartyID.push_back(-1);
	ownerPartyID.push_back(-1);

	pcActiveTags.push_back({});

	return output;
}

// manipulate the internal "base" party

std::vector<int>& BaseManager::getPlayerCharacters(int baseID)
{ 
	return gGame->mPartyManager->getPlayerCharacters(basePartyID[baseID]); 
}

std::vector<int>& BaseManager::getHenchmen(int baseID)
{ 
	return gGame->mPartyManager->getHenchmen(basePartyID[baseID]);; 
}

std::vector<int>& BaseManager::getAnimals(int baseID)
{ 
	return gGame->mPartyManager->getAnimals(basePartyID[baseID]);
}

void BaseManager::AddPlayerCharacter(int characterID, int baseID)
{
	gGame->mPartyManager->AddPlayerCharacter(basePartyID[baseID], characterID);
	DebugLog("PC (character ID #" + std::to_string(characterID) + ") added to party");
}

void BaseManager::AddHenchman(int characterID, int baseID)
{
	gGame->mPartyManager->AddHenchman(basePartyID[baseID], characterID);
	DebugLog("Henchman (character ID #" + std::to_string(characterID) + ") added to party");
}

void BaseManager::AddAnimal(int mobID, int baseID)
{
	gGame->mPartyManager->AddAnimal(basePartyID[baseID], mobID);
	DebugLog("Animal (mob ID #" + std::to_string(mobID) + ") added to party");
}

void BaseManager::getBasePartyCharacters(std::vector<int>& out, int baseID)
{
	int baseParty = basePartyID[baseID];

	std::vector<int>& basePartyPCs = gGame->mPartyManager->getPlayerCharacters(baseParty);
	std::vector<int>& basePartyHenches = gGame->mPartyManager->getHenchmen(baseParty);

	out.insert(out.begin(), basePartyPCs.begin(), basePartyPCs.end());
	out.insert(out.begin(), basePartyHenches.begin(), basePartyHenches.end());
}

void BaseManager::getVisitingPartyCharacters(std::vector<int>& out, int partyID)
{
	std::vector<int>& visitPartyPCs = gGame->mPartyManager->getPlayerCharacters(partyID);
	std::vector<int>& visitPartyHenches = gGame->mPartyManager->getHenchmen(partyID);

	out.insert(out.begin(), visitPartyPCs.begin(), visitPartyPCs.end());
	out.insert(out.begin(), visitPartyHenches.begin(), visitPartyHenches.end());
}

int BaseManager::getBasePartyCharacterCount(int baseID)
{
	int baseParty = basePartyID[baseID];

	int size = gGame->mPartyManager->getPlayerCharacters(baseParty).size();
	size += gGame->mPartyManager->getHenchmen(baseParty).size();

	return size;
}

int BaseManager::getVisitingCharacterCount(int partyID)
{
	int size = gGame->mPartyManager->getPlayerCharacters(partyID).size();
	size += gGame->mPartyManager->getHenchmen(partyID).size();

	return size;
}


void BaseManager::RemoveCharacter(int entityID, int baseID)
{
	gGame->mPartyManager->RemoveCharacter(basePartyID[baseID], entityID);
}

void BaseManager::RemoveAnimal(int entityID, int baseID)
{
	gGame->mPartyManager->RemoveAnimal(basePartyID[baseID], entityID);
}

bool BaseManager::IsAPC(int entityID, int baseID)
{
	return gGame->mPartyManager->IsAPC(basePartyID[baseID], entityID);
}

bool BaseManager::IsAHenchman(int entityID, int baseID)
{
	return gGame->mPartyManager->IsAHenchman(basePartyID[baseID], entityID);
}

bool BaseManager::IsAnAnimal(int entityID, int baseID)
{
	return gGame->mPartyManager->IsAnAnimal(basePartyID[baseID], entityID);
}

int BaseManager::GetBaseAt(int find_x, int find_y)
{
	int output = -1;
	int i = 0;
	for (int x : baseXPos)
	{
		if (x == find_x)
		{
			if (baseYPos[i] == find_y)
			{
				output = i;
				break;
			}
		}
		i++;
	}

	return output;
}

std::string BaseManager::GetBaseType(int baseID)
{
	int bT = baseType[baseID];
	return baseInfoSet.BaseTypes()[bT].Name();
}

int BaseManager::GenerateCampAtLocation(int partyID, int basePosX, int basePosY)
{
	DebugLog("Creating a camp controlled by party #" + std::to_string(partyID) + " at (" + std::to_string(basePosX) + "," + std::to_string(basePosY) + ")");
	int output = shellGenerate();

	basePartyID[output] = gGame->mPartyManager->GenerateEmptyParty(); // new PartyID
	ownerPartyID[output] = partyID;

	baseXPos[output] = basePosX;
	baseYPos[output] = basePosY;

	// transfer all characters, animals and items to base
	// TODO: Make this an option
	gGame->mPartyManager->TransferParty(partyID, basePartyID[output]);

	int campID = 0;
	try
	{
		campID = reverseBaseTypeDictionary.at("Camp");
	}
	catch(std::out_of_range)
	{
		campID = 0; // default first one
	}
	baseType[output] = campID;

	// add base to region map

	gGame->mMapManager->getRegionMap()->setBase(basePosX, basePosY, output);

	// add base to timing system
	// next flag happens at the end of the current day

	long double day = TimeManager::GetTimePeriodInSeconds(TIME_DAY);
	gGame->mTimeManager->SetEntityTime(output, MANAGER_BASE, day - fmod(gGame->mTimeManager->GetRunningTime(), day));
	
	return output;
}

void BaseManager::DebugLog(std::string message)
{
	gLog->Log("Base Manager", message);
}

bool BaseManager::TurnHandler(int baseID, double time)
{
	// this triggers at the end of every day, so we can handle daily activities

	// all base actions require the character to be in the base party; other daily actions are managed by PartyManager.
	std::vector<int> basePartyCharIDs;
	getBasePartyCharacters(basePartyCharIDs, baseID);

	// Bed rest heals 1d3 hit points and reduces bed rest requirement.
	std::vector<int> bedResters;
	
	std::copy_if(basePartyCharIDs.begin(), basePartyCharIDs.end(), bedResters.begin(), [](int c) {return (gGame->mCharacterManager->getCharacterDomainAction(c) == "BedRest"); });
	for (int c : bedResters)
	{
		if (gGame->mCharacterManager->getCharacterHasCondition(c, "Recovering"))
		{
			// bed rest timer reduced by 1 day
			gGame->mCharacterManager->ReduceCondition(c, "Recovering", TimeManager::GetTimePeriodInSeconds(TIME_DAY));
		}

		// roll 1d3
		TCOD_dice_t damageRoller;
		damageRoller.nb_faces = 3;
		damageRoller.nb_rolls = 1; // needs updating for eg monster attacks for 2d6, lightning bolt etc
		damageRoller.addsub = 0;
		damageRoller.multiplier = 1;

		int healHP = gGame->randomiser->diceRoll(damageRoller);
		int currentHP = gGame->mCharacterManager->getCharacterCurrentHitPoints(c);
		int maxHP = gGame->mCharacterManager->getCharacterTotalHitPoints(c);

		currentHP += healHP;
		if (currentHP > maxHP) currentHP = maxHP;

		gGame->mCharacterManager->setCharacterCurrentHitPoints(c, currentHP);
	}
	
	// always interrupt, so we run for 1 day per press of advance day key
	// TODO: Allow advancing more than a single day
	return true;
}

bool BaseManager::TimeHandler(int rounds, int turns, int hours, int days, int weeks, int months)
{
	return false;
}

void BaseManager::DumpBase(int baseID)
{
	// TODO: Dump local party

	DebugLog("Dumping virtual out-party");
	
	for(int character : gGame->mPartyManager->getPlayerCharacters(basePartyID[baseID]))
	{
		gGame->mCharacterManager->DumpCharacter(character);
	}
	
	for(int hench : gGame->mPartyManager->getHenchmen(basePartyID[baseID]))
	{
		gGame->mCharacterManager->DumpCharacter(hench);
	}
	
	for(int beast : gGame->mPartyManager->getAnimals(basePartyID[baseID]))
	{
		gGame->mMobManager->DumpMob(beast);
	}

	// TODO: Dump party inventory
	
}

bool BaseManager::ControlCommand(TCOD_key_t* key,int baseID)
{
	// if we don't have a base here (for whatever reason)
	if (baseID == -1)
	{
		// "A" key creates a base
		if (key->c == 'a')
		{
			// make a base! (We don't bother doing a prof check here as there is no circumstance in which no character with
			// Adventuring will be in the party.)
			int partyID = gGame->GetSelectedPartyID();
			int partyX = gGame->mPartyManager->GetPartyX(partyID);
			int partyY = gGame->mPartyManager->GetPartyY(partyID);

			int newBaseID = this->GenerateCampAtLocation(partyID, partyX, partyY);
			
			gGame->SetSelectedBaseID(newBaseID);

			return true;
		}
	}
	else
	{

		if (key->vk == TCODK_LEFT || key->vk == TCODK_RIGHT)
		{
			// switch between "base party" and virtual "out party"
			if (controlPane == PANE_BASE_CHARACTERS)
			{
				int visitorCount = getVisitingCharacterCount(gGame->GetSelectedPartyID());
				if (visitorCount > 0)
				{
					controlPane = PANE_PARTY_CHARACTERS;
					if (menuPosition > visitorCount - 1) menuPosition = visitorCount - 1;
				}
			}
			else
			{
				if (controlPane == PANE_PARTY_CHARACTERS)
				{
					int baseCount = getBasePartyCharacterCount(baseID);
					if(baseCount > 0)
					{
						controlPane = PANE_BASE_CHARACTERS;
						if (menuPosition > baseCount - 1) menuPosition = baseCount - 1;
					}
				}
			}

			// switch between "base store" and party inventory
			/**
			if (controlPane == PANE_PARTY_INVENTORY)
			{
				controlPane = PANE_BASE_INVENTORY;
			}
			else
			{
				if (controlPane == PANE_BASE_INVENTORY) controlPane = PANE_PARTY_INVENTORY;
			}
			*/

		}

		if (key->vk == TCODK_UP)
		{
			menuPosition--;
			// this is entirely dependent on what's inside the pane we're controlling

			if (controlPane == PANE_BASE_CHARACTERS)
			{
				if (menuPosition < 0)
				{
					// move to end
					int total = getBasePartyCharacterCount(baseID);

					menuPosition = total - 1;
				}
			}

			if (controlPane == PANE_PARTY_CHARACTERS)
			{
				if (menuPosition < 0)
				{
					// move to end
					int total = getVisitingCharacterCount(gGame->GetSelectedPartyID());
					menuPosition = total - 1;
				}
			}

		}
		else if (key->vk == TCODK_DOWN)
		{
			menuPosition++;
			if (controlPane == PANE_BASE_CHARACTERS)
			{
				if (menuPosition > getBasePartyCharacterCount(baseID) - 1)
				{
					menuPosition = 0;
				}
			}

			if (controlPane == PANE_PARTY_CHARACTERS)
			{
				if (menuPosition > getVisitingCharacterCount(gGame->GetSelectedPartyID()) - 1)
				{
					menuPosition = 0;
				}
			}
			
		}
		else if (key->vk == TCODK_ENTER)
		{
			int focus = controlPane;
			int charID = GetSelectedCharacter(baseID);
			int partyID = basePartyID[baseID];
			int visitingPartyID = gGame->GetSelectedPartyID();
			// enter moves characters and equipment between "base" and "party"
			if (focus == PANE_BASE_CHARACTERS)
			{
				// move character from base party to visiting party

				bool hench = IsAHenchman(charID, baseID);
				RemoveCharacter(charID,baseID);
				hench ? gGame->mPartyManager->AddHenchman(visitingPartyID,charID) : gGame->mPartyManager->AddPlayerCharacter(visitingPartyID, charID);

				if (getBasePartyCharacterCount(baseID) == 0)
				{
					// all characters moved to other screen, so switch focus
					controlPane = PANE_PARTY_CHARACTERS;
					menuPosition = 0;
				}
				else
				{
					menuPosition -= 1;
					if (menuPosition < 0) menuPosition = 0;
				}
			}
			else if (focus == PANE_PARTY_CHARACTERS)
			{
				// move character from visiting party to base party

				bool hench = gGame->mPartyManager->IsAHenchman(visitingPartyID,charID);
				gGame->mPartyManager->RemoveCharacter(visitingPartyID, charID);
				hench ? AddHenchman(charID, baseID) : AddPlayerCharacter(charID, baseID);

				if (getVisitingCharacterCount(gGame->GetSelectedPartyID()) == 0)
				{
					// all characters moved to other screen, so switch focus
					controlPane = PANE_BASE_CHARACTERS;
					menuPosition = 0;
				}
				else
				{
					menuPosition -= 1;
					if (menuPosition < 0) menuPosition = 0;
				}
			}

			//if (mCharacterManager->GetEquipSlotForInventoryItem(currentCharacterID, menuPosition[mode]) != -1)
			//{
			//	mCharacterManager->UnequipItem(currentCharacterID, menuPosition[mode]);
			//}
			//else
			//{
			//	mCharacterManager->EquipItem(currentCharacterID, menuPosition[mode]);
			//}
		}
		else if (key->c >= '1' && key->c <= '9')
		{
			// character option selected
			int charID = GetSelectedCharacter(baseID);
			std::vector<BaseTag> bts = GetCharacterActionList(baseID, charID);
			// set character to perform action
			int idx = key->c - '1';
			if (idx >= 0 && idx < bts.size())
			{
				BaseTag& bt = bts[idx];
				if (bt.Type() == "CharacterAction")
					gGame->mCharacterManager->setCharacterDomainAction(charID, bts[idx].Tag());

				if (bt.Type() == "TravelMode")
				{
					std::vector<std::string> excludes = bts[idx].Excludes();
					for (std::string exc : excludes)
					{
						if (gGame->mCharacterManager->getCharacterDomainAction(charID) == exc)
						{
							// this travel mode deactivates the domain action
							gGame->mCharacterManager->setCharacterDomainAction(charID, "");
						}

						if (gGame->mCharacterManager->getCharacterTravelModeSet(charID, exc))
						{
							gGame->mCharacterManager->unsetCharacterTravelMode(charID, exc);
						}
					}
					gGame->mCharacterManager->toggleCharacterTravelMode(charID, bts[idx].Tag());
				}
			}
		}
		else if (key->c == 'X' || key->c == 'x')
		{
			// advance time to next day boundary, triggering daily domain action turn

			long double day = TimeManager::GetTimePeriodInSeconds(TIME_DAY);
			gGame->mTimeManager->AdvanceTimeBy(day - fmod(gGame->mTimeManager->GetRunningTime(), day));
		}
		else if (key->c == 'F' || key->c == 'f')
		{
			// move all non-Unconscious characters into out-party and leave Domain mode
		}
	}

	return false;
}

void BaseManager::RenderBaseMenu(int baseID)
{
	if (baseID == -1)
	{
		// no base here
		gGame->sampleConsole->printFrame(0, 0, SAMPLE_SCREEN_WIDTH, SAMPLE_SCREEN_HEIGHT, true, TCOD_BKGND_SET, "");

		gGame->sampleConsole->printf(2, 2, "There is no camp or settlement here.");
		gGame->sampleConsole->printf(2, 4, "Press A to have your party");
		gGame->sampleConsole->printf(2, 5, "build a camp here.");
	}
	else
	{
		int baseOwner = this->GetBaseOwner(baseID);
		if (baseOwner != gGame->GetSelectedPartyID())
		{
			// another party's base!
			gGame->sampleConsole->printFrame(0, 0, SAMPLE_SCREEN_WIDTH, SAMPLE_SCREEN_HEIGHT, true, TCOD_BKGND_SET, "");
			std::string message = "There is a camp here belonging to another party.";
			gGame->sampleConsole->printf(2, 2, message.c_str());
		}
		else
		{
			gGame->sampleConsole->printFrame(0, 0, SAMPLE_SCREEN_WIDTH / 2, SAMPLE_SCREEN_HEIGHT / 2, true, TCOD_BKGND_SET, "");
			gGame->sampleConsole->printFrame(SAMPLE_SCREEN_WIDTH / 2, 0, SAMPLE_SCREEN_WIDTH / 2, SAMPLE_SCREEN_HEIGHT / 2, true, TCOD_BKGND_SET, "");
			gGame->sampleConsole->printFrame(0, SAMPLE_SCREEN_HEIGHT / 2, SAMPLE_SCREEN_WIDTH, SAMPLE_SCREEN_HEIGHT, true, TCOD_BKGND_SET, "");

			int bT = baseType[baseID];
			int partyID = basePartyID[baseID];

			if (controlPane < PANE_PARTY_INVENTORY)
			{
				// Character Screen:
				// Top Left: Characters in Base Party
				// Top Right: Characters in Visiting Party
				// Bottom: Choices available to selected character

				// draw out the characters from the managers
				std::vector<int> base_cs, out_cs;
				getBasePartyCharacters(base_cs, baseID);
				getVisitingPartyCharacters(out_cs, gGame->GetSelectedPartyID());

				// add overall frame with title
				gGame->sampleConsole->printFrame(0, 0, SAMPLE_SCREEN_WIDTH, SAMPLE_SCREEN_HEIGHT, false, TCOD_BKGND_SET, "Party Management");

				// output base characters
				for (int i = 0; i < base_cs.size(); i++)
				{
					int charID = base_cs[i];
					std::string name = gGame->mCharacterManager->getCharacterName(charID);
					if (gGame->mPartyManager->IsAPC(partyID, charID)) name += "*";
					std::string currentDomainAction = gGame->mCharacterManager->getCharacterDomainAction(charID);
					std::vector<BaseTag> cal = GetCharacterActionList(baseID, charID);
					std::string status = "";
					for (BaseTag bt : cal)
					{
						// we have 1 domain action at a time, so if we match on that, indicate
						if (bt.Tag() == currentDomainAction)
						{
							status += bt.Indicator();
						}

						// we can have multiple domain travel modes, indicate them
						if (gGame->mCharacterManager->getCharacterTravelModeSet(charID, bt.Tag()))
						{
							status += bt.Indicator();
						}
					}

					if (status != "") name += " " + status;
					
					TCOD_bkgnd_flag_t backg = TCOD_BKGND_NONE;
					int y_pos = 3 + i;
					gGame->sampleConsole->printEx(2, y_pos, backg, TCOD_LEFT, name.c_str());
				}

				// output party characters
				for (int i = 0; i < out_cs.size(); i++)
				{
					int charID = out_cs[i];
					std::string name = gGame->mCharacterManager->getCharacterName(out_cs[i]);
					if (IsAPC(out_cs[i],baseID)) name += "*";
					std::string currentDomainAction = gGame->mCharacterManager->getCharacterDomainAction(charID);
					std::vector<BaseTag> cal = GetCharacterActionList(baseID, charID);
					for (BaseTag bt : cal)
					{
						// we have 1 domain action at a time, so if we match on that, indicate
						if (bt.Tag() == currentDomainAction)
							name += " " + bt.Indicator();

						// we can have multiple domain travel modes, indicate them
						if (gGame->mCharacterManager->getCharacterTravelModeSet(charID, bt.Tag()))
							name += " " + bt.Indicator();
					}

					TCOD_bkgnd_flag_t backg = TCOD_BKGND_NONE;
					int y_pos = 3 + i;
					gGame->sampleConsole->printEx(2 + SAMPLE_SCREEN_WIDTH / 2, y_pos, backg, TCOD_LEFT, name.c_str());
				}

				// highlight selected character
				int select_y = menuPosition + 3;
				int base_x = (controlPane == PANE_BASE_CHARACTERS) ? 0 : SAMPLE_SCREEN_WIDTH / 2;
				for (int x = base_x; x < (base_x + SAMPLE_SCREEN_WIDTH / 2); x++)
				{
					gGame->sampleConsole->setCharBackground(x, select_y, TCODColor::white, TCOD_BKGND_SET);
					gGame->sampleConsole->setCharForeground(x, select_y, TCODColor::black);
				}

				// selected character ID
				int charID = GetSelectedCharacter(baseID);

				if (charID != -1)
				{
					// controls for selected character

					std::vector<BaseTag> bt_list = GetCharacterActionList(baseID, charID);

					for (int i = 0; i < bt_list.size(); i++)
					{
						BaseTag t = bt_list[i];
						int y = (SAMPLE_SCREEN_HEIGHT / 2) + 2 + i;
						std::string outp = std::to_string(i + 1) + " : " + t.MenuText();
						if (t.Indicator() != "")
							outp += " (" + t.Indicator() + ")";
						TCOD_bkgnd_flag_t backg = TCOD_BKGND_NONE;
						gGame->sampleConsole->printEx(3, y, backg, TCOD_LEFT, outp.c_str());
					}

					// UI for selected character

					gGame->RenderUI(charID);
				}

				// timeskip controls
				std::string time_control = "X to skip 1 day \n F to take all and exit \n D to leave";
				gGame->sampleConsole->printEx(SAMPLE_SCREEN_WIDTH - 2, SAMPLE_SCREEN_HEIGHT - 4, TCOD_BKGND_NONE, TCOD_RIGHT, time_control.c_str());
			}
			else
			{
				// Inventory Screen:
				// Top Left: Party Inventory
				// Top Right: Base Inventory
				// Bottom: Purchasable Items
			}
		}
	}
}

std::vector<BaseTag> BaseManager::GetCharacterActionList(int baseID, int charID)
{
	std::vector<BaseTag> output;

	int bT = baseType[baseID];
	int partyID = basePartyID[baseID];

	BaseType b = baseInfoSet.BaseTypes()[bT];

	for (int i = 0; i < b.Core().size(); i++)
	{
		std::string c = b.Core()[i];
		int tagID = reverseBaseTagDictionary[c];
		BaseTag t = baseInfoSet.Tags()[tagID];
		if(t.Type() == "CharacterAction" || t.Type() == "TravelMode")
		{
			if (CharacterCanUseAction(baseID, charID, tagID))
			{
				output.push_back(t);
			}
		}
	}

	return output;
}

int BaseManager::GetSelectedCharacter(int baseID)
{
	std::vector<int> base_cs, out_cs;

	getBasePartyCharacters(base_cs, baseID);
	getVisitingPartyCharacters(out_cs, gGame->GetSelectedPartyID());
	
	// selected character ID
	int charID = -1;
	if (controlPane == PANE_BASE_CHARACTERS)
	{
		if (base_cs.size() == 0) return -1;
		charID = base_cs[menuPosition];
	}
	else
	{
		if (out_cs.size() == 0) return -1;
		charID = out_cs[menuPosition];
	}

	return charID;
}

bool BaseManager::CharacterCanUseAction(int baseID, int characterID, int tag)
{
	int bT = baseType[baseID];
	BaseType b = baseInfoSet.BaseTypes()[bT];
	BaseTag t = baseInfoSet.Tags()[tag];
	
	/// TODO: action requirements (player tags, items, situations)
	if (t.Requires().size() > 0)
		return false;

	for(int i=0;i<t.Requires().size();i++)
	{
		std::string tagRequire = t.Requires()[i];
		std::string opt = "Options";
		if (std::equal(opt.begin(), opt.end(), tagRequire.begin()))
		{
			int activeTag = pcActiveTags[baseID][characterID][tagRequire.substr(9)];
		}
	}
	
	return true;
}

void BaseManager::RenderBaseMenu(int xpos, int ypos)
{
	for (int i = 0; i < baseXPos.size(); i++)
	{
		if ((xpos == baseXPos[i]) && (ypos == baseYPos[i]))
		{
			if(gGame->GetSelectedPartyID() == basePartyID[i])
				RenderBaseMenu(i);
		}
	}
}
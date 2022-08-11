#include "Conditions.h"

ConditionManager* ConditionManager::LoadConditions()
{
	gLog->Log("Condition Loader", "Started");
	
	std::string conditionsFilename = "RCK/scripts/conditions.json";
	std::ifstream is(conditionsFilename);

	ConditionManager* cm = new ConditionManager(jsoncons::decode_json<ConditionData>(is));

	gLog->Log("Condition Loader", "Decoded " + conditionsFilename);

	int nextConditionIndex = 0;

	// feed out data into structures for use

	for(Condition c : cm->cd.Conditions_NC())
	{
		cm->Names.push_back(c.Name());
		// translate to bool
		cm->CanTakeActions.push_back(c.CanTakeActions() == "yes" || c.CanTakeActions() == "Yes" || c.CanTakeActions() == "true" || c.CanTakeActions() == "True");
		cm->CanAttack.push_back(c.CanFight() == "yes" || c.CanFight() == "Yes" || c.CanFight() == "true" || c.CanFight() == "True");
		cm->CanCastSpells.push_back(c.CanCastSpells() == "yes" || c.CanCastSpells() == "Yes" || c.CanCastSpells() == "true" || c.CanCastSpells() == "True");
		cm->CanBeBackstabbed.push_back(c.CanBeBackstabbed() == "yes" || c.CanBeBackstabbed() == "Yes" || c.CanBeBackstabbed() == "true" || c.CanBeBackstabbed() == "True");

		cm->MoveRate.push_back(c.MoveRate());

		cm->MoveLimit.push_back(c.MoveLimit());
		cm->ToHitMeBonus.push_back(c.ToHitMeBonus());
		cm->ToHitOthersBonus.push_back(c.ToHitOthersBonus());
		cm->ArmorClassBonus.push_back(c.ArmorClassBonus());
		cm->SurpriseBonus.push_back(c.SurpriseBonus());

		// all copied across, now fill up the lookups

		cm->NameLookup[c.Name()] = nextConditionIndex++;
	}

	// now we have the name lookup sorted out, run the includes
	for (Condition c : cm->cd.Conditions_NC())
	{
		std::vector<int> conditionIndicies;
		for(std::string included_condition : c.Includes())
		{
			conditionIndicies.push_back(cm->GetConditionIndex(included_condition));
		}
		cm->Includes.push_back(conditionIndicies);
	}

	gLog->Log("Condition Loader", "Completed");
	
	return cm;
}

bool ConditionManager::HasCondition(std::vector<int>& conditions, int findVal)
{
	if (std::find(conditions.begin(), conditions.end(), findVal) != conditions.end())
	{
		// we found it in the basic list, so
		return true;
	}

	// for every condition we actually have, check what is included in it
	for (int condIndex : conditions)
	{
		std::vector<int>& included = Includes[condIndex];
		if (std::find(included.begin(), included.end(), findVal) != included.end())
		{
			// we found it in the derived list, so
			return true;
		}
	}

	// didn't find it in the base or derived lists, so
	return false;
}

MortalWoundManager* MortalWoundManager::LoadMortalWoundData()
{
	gLog->Log("Mortal Loader", "Started");
	
	std::string mortalFilename = "RCK/scripts/mortal_wound_effects.json";
	std::ifstream is(mortalFilename);

	MortalWoundManager* mwm = new MortalWoundManager(jsoncons::decode_json<MortalWoundData>(is));

	gLog->Log("Mortal Loader", "Decoded " + mortalFilename);
	
	int nextMortalWoundIndex = 0;

	// feed out data into structures for use

	for (MortalEffect m : mwm->mwd.MortalEffects_NC())
	{
		mwm->Codes.push_back(m.Code());
		mwm->PlayerTexts.push_back(m.PlayerText());
		mwm->MonsterTexts.push_back(m.MonsterText());

		// now fill the special effect dictionary

		std::map<std::string, int> penalties;
		for(SpecialPenalty p : m.SpecialPenalties())
		{
			penalties[p.Name()] = p.Value();
		}
		mwm->SpecialPenalties.push_back(penalties);

		// all copied across, now fill up the lookups

		mwm->CodeLookup[m.Code()] = nextMortalWoundIndex++;
	}

	// now we've set all the the mortal wounds up we can translate the DoubleTo strings into indices.

	for (MortalEffect m : mwm->mwd.MortalEffects_NC())
	{
		std::string dt = m.DoubleTo();
		mwm->DoubleTo.push_back(mwm->CodeLookup[dt]);
	}

	// now load the mortal lookup CSV

	mwm->mortalstore.LoadMortalRollStore();

	gLog->Log("Mortal Loader", "Completed");

	return mwm;
}

void MortalWoundManager::DebugLog(std::string message)
{
	gLog->Log("Mortal Wound Manager", message);
}

void ConditionManager::DebugLog(std::string message)
{
	gLog->Log("Condition Manager", message);
}

void MortalRollStore::LoadMortalRollStore()
{
	std::vector<std::vector<std::string>> columnValues;

	std::string csv_name = "RCK/scripts/mortal_wounds.csv";
	std::ifstream csv(csv_name);

	jsoncons::csv::csv_options options;
	options.assume_header(false); // we have no header

	if (csv.is_open())
	{
		jsoncons::ojson jo = jsoncons::csv::decode_csv<jsoncons::ojson>(csv, options);
		int rowCount = 0;
		int size = jo.size();

		for (const auto& row : jo.array_range())
		{
			min.push_back(row[0].as_int());
			max.push_back(row[1].as_int());
			status.push_back(row[2].as_string());
			recovery.push_back(row[3].as_string());
			bedRest.push_back(row[4].as_int());
			std::vector<std::string> effects;
			for(int j=0; j<6; j++)
			{
				effects.push_back(row[5 + j].as_string());
			}
			results.push_back(effects);
			rowCount++;
		}

		csv.close();
	}
}

MortalRollResult* MortalRollStore::GetRoll(int severity, int d6)
{
	MortalRollResult* output = new MortalRollResult();

	int column = 6 - d6;

	// run from bottom to top of possible d20 (severity)

	for(int i=0;i<min.size();i++)
	{
		if(severity >= min[i])
		{
			if(severity <= max[i])
			{
				// this is our stage!
				output->status = status[i];
				output->recovery = recovery[i];
				output->bedRest = bedRest[i];

				std::string lookupVal = results[i][column];
				output->mortalIndex = gGame->mMortalManager->GetConditionIndex(lookupVal);
				output->effect = gGame->mMortalManager->GetMortalEffectFromIndex(output->mortalIndex);
			}
		}
	}

	return output;
}
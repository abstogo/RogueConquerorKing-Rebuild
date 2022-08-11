#include "Class.h"
#include "Game.h"
#include <string>
#include <locale>


std::string saveTypes[] = { "PetrificationParalysis","PoisonDeath","BlastBreath","StaffsWands","Spells" };

ClassManager* ClassManager::LoadClasses()
{
	gLog->Log("Class Loader", "Started");

	std::string classFilename = "RCK/scripts/classes.json";
	std::ifstream is(classFilename);

	ClassManager* output = new ClassManager(jsoncons::decode_json<ClassSet>(is));

	is.close();

	gLog->Log("Class Loader", "Decoded " + classFilename);
	
	// now for every defined class in the set, load the associated stats CSV

	const std::vector<ACKSClass>& classList = output->classes.Classes();

	for (int i = 0; i < classList.size(); i++)
	{
		std::vector<std::vector<int>> bonusMatrix;

		ACKSClass* cl = (ACKSClass*)&classList[i];

		std::string csv_name = "RCK/scripts/" + cl->Name() + "_chart.csv";
		std::transform(csv_name.begin(), csv_name.end(), csv_name.begin(), ::tolower);
		std::ifstream csv(csv_name);

		jsoncons::csv::csv_options options;
		options.assume_header(false);

		if (csv.is_open())
		{
			jsoncons::ojson jo = jsoncons::csv::decode_csv<jsoncons::ojson>(csv, options);
			int rowCount = 1;
			int size = jo.size() + 2;
			// quickly push out the size values
			cl->LevelXPValues.resize(size);
			cl->LevelTitles.resize(size);
			cl->LevelHitDie.resize(size);
			cl->LevelHitBonus.resize(size);
			
			for (const auto& row : jo.array_range())
			{
				// these have a fairly well-defined layout
				// The first column is the XP required for this level, followed by the level name
				// Then the hits - first in number of Hit Dice, then the bonus (non-cumulative)
				// Finally a succession of levelled bonuses as expressed in the class JSON
				// These are stored in a matrix before being filtered out into the tag list in the class object
				
				cl->LevelXPValues[rowCount] = row[0].as<int>();
				cl->LevelTitles[rowCount] = row[1].as<std::string>();
				cl->LevelHitDie[rowCount] = row[2].as<int>();
				cl->LevelHitBonus[rowCount] = row[3].as<int>();

				if (bonusMatrix.size() < row.size()) bonusMatrix.resize(row.size());
				
				for (int col = 4; col < row.size(); col++)
				{
					std::vector<int>& matrixCol = bonusMatrix[col];
					if (matrixCol.size() < rowCount + 1) matrixCol.resize(rowCount + 1);
					matrixCol[rowCount] = row[col].as<int>();
				}

				rowCount++;
			}

			csv.close();

			// once we've extracted the values, match them to tags and store them

			for (const LevelledChartColumn pair : cl->LevelledChartColumns())
			{
				std::string name = pair.Name();
				int column = pair.Column();

				std::vector<int>& col = bonusMatrix[column];

				cl->LevelTagBonuses[name] = col;
			}
		}

		// extract the indices for the class's prime reqs
		
		for(auto rq : cl->PrimeRequisites())
		{
			cl->PrimeReqIdx.push_back(gGame->mCharacterManager->GetStatisticIndex(rq));
		}

		// add this class to the index list
		output->classLookup[cl->Name()] = i;
	}

	output->advancementStore = new AdvancementStore();
	output->advancementStore->LoadAdvancementSets();

	gLog->Log("Class Loader", "Completed");
	
	return output;
}

int ClassManager::GetClassIndex(const std::string className)
{
	return classLookup[className];
}

void AdvancementStore::LoadAdvancementSets()
{
	// this function loads all the advancement charts for the four base classes (all the others are based on them)
	// they're all listed as headers in advancement.csv
	// we also need to load and ignore the attack advancement for monsters

	std::vector<std::string> columnNames;
	std::vector<std::vector<int>> columnValues;
	
	std::string csv_name = "RCK/scripts/advancement.csv";
	std::ifstream csv(csv_name);

	jsoncons::csv::csv_options options;
	options.assume_header(false); // we have a header - but we want to read it

	if (csv.is_open())
	{
		jsoncons::ojson jo = jsoncons::csv::decode_csv<jsoncons::ojson>(csv, options);
		int rowCount = -1;

		for (const auto& row : jo.array_range())
		{
			if (rowCount == -1)
			{
				// first row is the header, first title is "Level"
				for (int i = 1; i < row.size(); i++)
				{
					columnNames.push_back(row[i].as<std::string>());
				}
			}
			else
			{
				int level = row[0].as<int>();
				columnValues.resize(row.size());
				for (int i = 1; i < row.size(); i++)
				{
					if (!row[i].empty())
					{
						int value = row[i].as<int>();
						std::vector<int>& colArray = columnValues[i-1];
						columnValues[i-1].resize(rowCount + 1);
						colArray[rowCount] = value;
					}
				}
			}
			
			rowCount++;
		}
		csv.close();
	}

	// re-sort into the map
	// and open the save advancement charts for each
	//

	for (int i = 0; i < columnNames.size(); i++)
	{
		std::string name = columnNames[i];
		std::vector<int>& colArray = columnValues[i];

		AttackBonusLookup[name] = colArray;
		// skip Monsters (they have a save derived from a base class)
		if (i != 0)
		{
			std::string adv_csv_name = "RCK/scripts/" + name + "_advancement.csv";
			std::transform(adv_csv_name.begin(), adv_csv_name.end(), adv_csv_name.begin(), ::tolower);
			std::ifstream adv_csv(adv_csv_name);

			jsoncons::csv::csv_options options;
			options.assume_header(true);

			for (int j = 0; j < 5; j++)
			{
				SaveLookup[saveTypes[j]][name].resize(15);
			}

			if (adv_csv.is_open())
			{
				jsoncons::ojson jo = jsoncons::csv::decode_csv<jsoncons::ojson>(adv_csv, options);
				int rowCount = 0;
				int size = jo.size() + 1;

				for (const auto& row : jo.array_range())
				{
					for (int j = 0; j < 5; j++)
					{
						SaveLookup[saveTypes[j]][name][rowCount] = row[saveTypes[j]].as<int>();
					}
					rowCount++;
				}

				adv_csv.close();
			}
		}
	}
}

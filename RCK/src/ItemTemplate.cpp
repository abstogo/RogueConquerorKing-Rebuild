#include "ItemTemplate.h"
#include <cmath>
#include <numeric>
#include <cstdlib>

ItemManager* ItemManager::LoadItemTemplates()
{
	// read the characteristic bonuses from ability_bonus.csv

	gLog->Log("Item Loader", "Started");

	std::string itemRangeFilename = "RCK/scripts/ranges.csv";
	
	std::ifstream rs(itemRangeFilename);

	jsoncons::csv::csv_options options;
	options.assume_header(true);

	jsoncons::ojson j = jsoncons::csv::decode_csv<jsoncons::ojson>(rs, options);

	gLog->Log("Item Loader", "Decoded " + itemRangeFilename);

	std::map<std::string, std::vector<int>> rangeSet;

	std::vector<int> rangePenalties;
	rangePenalties.resize(RANGE_MAX);
	
	for (const auto& row : j.array_range())
	{
		std::vector<int> ranges;
		ranges.resize(RANGE_MAX);

		std::string tag = row["Tag"].as<std::string>();

		// we don't necessarily have to hardcode this, consider a more flexible approach
		if(tag=="Bonus")
		{
			// the bonus row does not correspond to a missile weapon tag
			rangePenalties[SHORT_RANGE] = row["Short"].as<int>();
			rangePenalties[MEDIUM_RANGE] = row["Medium"].as<int>();
			rangePenalties[LONG_RANGE] = row["Long"].as<int>();
		}
		else
		{
			ranges[SHORT_RANGE] = row["Short"].as<int>();
			ranges[MEDIUM_RANGE] = row["Medium"].as<int>();
			ranges[LONG_RANGE] = row["Long"].as<int>();
			rangeSet[tag] = ranges;
		}
	}

	rs.close();

	std::string equipmentFilename = "RCK/scripts/equipment.json";
	std::string decorationFilename = "RCK/scripts/decoration.json";
	
	std::ifstream is(equipmentFilename);
	std::ifstream ds(decorationFilename);

	ItemManager* output = new ItemManager(jsoncons::decode_json<TemplateSet>(is), jsoncons::decode_json<DecorationSet>(ds),rangeSet,rangePenalties);

	gLog->Log("Item Loader", "Decoded " + equipmentFilename);
	gLog->Log("Item Loader", "Decoded " + decorationFilename);
	
	is.close();
	ds.close();

	// fill in the reverse lookups

	for(int i=0; i<output->itemTemplates.ItemTemplates().size();i++)
	{
		const ItemTemplate& it = output->itemTemplates.ItemTemplates()[i];
		output->reverseTemplateDictionary[it.Name()] = i;
	}

	for (int i = 0; i < output->decorations.Decorations().size(); i++)
	{
		const Decoration& it = output->decorations.Decorations()[i];
		output->reverseTemplateDictionary[it.Tag()] = i;
	}

	gLog->Log("Item Loader", "Completed");
	
	return output;
}

int ItemManager::GenerateItemFromTemplate(std::string name)
{
	int templateID = reverseTemplateDictionary[name];
	return GenerateItemFromTemplate(templateID);
}

int ItemManager::GenerateItemFromTemplate(int templateID)
{
	// item generation!
	// start with base item, then go through material generation and generate decorations

	int output = items.nextID++;

	const ItemTemplate& it = itemTemplates.ItemTemplates().at(templateID);


	// copy base elements
	items.Name.push_back(it.Name());
	items.Visual.push_back(it.Visual());
	int baseValue = it.Value();
	std::vector<std::string> tags;
	auto& eTags = it.EquipmentTags();
	tags.insert(tags.end(), eTags.begin(), eTags.end());

	items.WeightDen.push_back(it.WeightDen());
	items.WeightNum.push_back(it.WeightNum());

	// generate material
	std::vector<int> probs;
	int maxProb = 0;
	for (MaterialType mt : it.MaterialTypes())
	{
		int chance = mt.Chance();
		probs.push_back(chance);
		maxProb += chance;
	}

	int select = rand() % maxProb;
	int result = probs.size() - 1;
	for (int i = 0; i < probs.size(); i++)
	{
		int prob = probs[i];
		select -= prob;
		if (select <= 0)
		{
			result = i;
		}
	}

	MaterialType material = it.MaterialTypes()[result];

	std::string shortDesc = "a " + material.Name() + " " + it.Name();
	std::string longDesc = "A " + it.Name() + ". It is made from " + material.Name() + ".";

	items.LongDescription.push_back(longDesc);
	items.ShortDescription.push_back(shortDesc);
	
	int undecoratedValue = material.ValueMultiplier() * baseValue;

	// collate tags
	auto mTags = material.MaterialTags();
	tags.insert(tags.end(), mTags.begin(), mTags.end());

	// generate decoration

	// no decorations in this version
	int decorationBaseValue = 0;
	int decorationMaterialValue = 0;

	
	int decorationValue = decorationBaseValue * decorationMaterialValue;
	// value = base value * material multiplier + decoration value * material multiplier

	items.Value.push_back(undecoratedValue + decorationValue);

	// add collated tags

	items.Tags.push_back(tags);


	return output;
}

int ItemManager::getRangePenalty(int id, int range)
{
	for (auto p : rangeDictionary) 
	{
		if(hasTag(id, p.first))
		{
			// we have the tag for this item, so return the penalty at this range (or -255 if no shot)
			return getRangePenalty(p.first, range);
		}
	}

	// item has no matching tag, so -255 for no shot
	return -255;
}

int ItemManager::getMaxRange(int id)
{
	for (auto p : rangeDictionary)
	{
		if (hasTag(id, p.first))
		{
			// we have the tag for this item, so return the penalty at this range (or -255 if no shot)
			std::vector<int> rangeList = rangeDictionary[p.first];
			return rangeList[LONG_RANGE];
		}
	}

	// item has no matching tag, so -255 for no shot
	return -255;
}

int ItemManager::getRangePenalty(std::string tag, int range)
{
	// this function is to be used when/if the relevant tag has already been located
	std::vector<int> rangeList = rangeDictionary[tag];
	for(int i = 0;i<RANGE_MAX;i++)
	{
		int r = rangeList[i];
		if(range<r)
		{
			return rangePenalties[i];
		}
	}
	// longer than long range for this tag, penalty is -255 (will be flagged as "no shot")
	return -255;
}

double ItemManager::getWeight(std::vector<int> items)
{
	double total = 0;

	for (int item : items)
	{
		total += getWeightNum(item) / getWeightDen(item);
	}

	return total;
}

double ItemManager::getWeight(std::list<int> items)
{
	double total = 0;

	for (int item : items)
	{
		total += (double)getWeightNum(item) / (double)getWeightDen(item);
	}

	return total;
}

bool ItemManager::hasTag(int id, std::string tag)
{
	return std::find(items.Tags[id].begin(), items.Tags[id].end(), tag) != items.Tags[id].end();
}

void ItemManager::DebugLog(std::string message)
{
	gLog->Log("ItemManager", message);
}

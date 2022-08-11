#pragma once

#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpath/json_query.hpp>
#include <jsoncons/json_type_traits_macros.hpp>
#include "Class.h"
#include "OutputLog.h"

// ACKS item manager
// Items in ACKS are largely divided into gear (magical or otherwise) and loot (with goods as a subgroup of loot)
// However, a lot of the time items in ACKS descriptions are quite fancy etc, with no changes to stats.
// This gives us an opportunity for a DF-style material/quality/carvings etc system!
// As a result, we can separate the following elements:
// Equipment Tags (string vector)
// Bonuses (Dictionary lookup)
// Object Name (string)
// Material (string)
// Decoration strings (Dictionary lookup)
// Value (int, in coppers)
// Weight Numerator (int)
// Weight Denominator (int)
//
// As an example "examine" for a generated loot item:
// A bronze figurine in the shape of a long-lost deity. It is worth 20gp. It weighs 1/16 stone.
// Equipment Tags & Bonuses: None. Object name: figurine. Material: bronze. Decoration string: shape|long-lost deity. Value: 2000. Weight Num: 1. Weight Denom: 16.
//
// An example "examine" for a generated usable item:
// An off-balance iron broadsword inlaid with gold. It is worth 13gp. It weighs 1/6 stone.
// Equipment Tag: Melee:HH. Bonus: Attack Bonus|-1. Material: iron. Object name: broadsword. Decoration string: inlay|gold. Value: 1300. Weight Num: 1. Weight Denom: 6.
// (This is a randomly generated scavenged weapon with the "off-balance" penalty from the scavenged equipment table, which is a value penalty of 20%.
// The gold inlay is also randomly generated, adding an additional value equal to 50% of base. 10 + 5 - 2 = 13gp.)
//
// We load nearly all of the base information for this generation system from the equipment.json file, with decoration coming from the decoration.json file

enum EQUIP_TYPES
{
	HAND_MAIN,
	HAND_OFF,
	ARMOUR,
	HELM,
	BOOTS,
	BRACERS,
	AMULET,			// incl. necklaces etc
	RING1,
	RING2,
	EQUIP_MAX
};

// most items can be made from a range of materials
class MaterialType
{
	std::string Name_;
	double Chance_;
	double ValueMultiplier_;
	std::vector<std::string> MaterialTags_;

public:
	MaterialType(const std::string& Name, const double& Chance, const double& ValueMultiplier,const std::vector<std::string>& MaterialTags) : Name_(Name), Chance_(Chance), ValueMultiplier_(ValueMultiplier), MaterialTags_(MaterialTags)
	{

	}

	const std::string& Name() const { return Name_; }
	const double Chance() const { return Chance_; }
	const double ValueMultiplier() const { return ValueMultiplier_; }
	const std::vector<std::string> MaterialTags() const { return MaterialTags_; }
};
JSONCONS_ALL_GETTER_CTOR_TRAITS_DECL(MaterialType, Name, Chance, ValueMultiplier, MaterialTags)


class ItemTemplate
{
	std::string Name_;
	std::string Visual_;
	std::vector<std::string> EquipmentTags_;
	std::vector<MaterialType> MaterialTypes_;
	std::vector<std::string> DecorationTypes_;
	int Value_;
	int WeightDen_;
	int WeightNum_;

public:

	ItemTemplate(const std::string& Name,  const std::string& Visual, const std::vector<std::string>& EquipmentTags, const std::vector<MaterialType>& MaterialTypes, const std::vector<std::string>& DecorationTypes, const int& Value, const int& WeightNum,const int& WeightDen) :
	Name_(Name), Visual_(Visual), EquipmentTags_(EquipmentTags), MaterialTypes_(MaterialTypes), DecorationTypes_(DecorationTypes), Value_(Value), WeightDen_(WeightDen), WeightNum_(WeightNum)
	{}

	const std::string Name() const { return Name_; }
	const std::string Visual() const { return Visual_; }
	const std::vector<std::string> EquipmentTags() const { return EquipmentTags_; }
	const std::vector<MaterialType> MaterialTypes() const { return MaterialTypes_; }
	const std::vector<std::string> DecorationTypes() const { return DecorationTypes_; }
	const int Value() const { return Value_; }
	const int WeightDen() const { return WeightDen_; }
	const int WeightNum() const { return WeightNum_; }
	
};
JSONCONS_ALL_GETTER_CTOR_TRAITS_DECL(ItemTemplate, Name, Visual, EquipmentTags, MaterialTypes, DecorationTypes, Value, WeightNum, WeightDen)

class TemplateSet
{
	std::vector<ItemTemplate> ItemTemplates_;

public:
	TemplateSet(const std::vector<ItemTemplate>& ItemTemplates) : ItemTemplates_(ItemTemplates)
	{}

	const std::vector<ItemTemplate>& ItemTemplates() const { return ItemTemplates_; }
};
JSONCONS_ALL_GETTER_CTOR_TRAITS_DECL(TemplateSet, ItemTemplates)


class DecorationMaterial
{
	std::string Name_;
	double Chance_;
	double ValueMultiplier_;

public:
	DecorationMaterial(const std::string& Name, const double& Chance, const double& ValueMultiplier) : Name_(Name), Chance_(Chance), ValueMultiplier_(ValueMultiplier)
	{

	}

	const std::string& Name() const { return Name_; }
	const double Chance() const { return Chance_; }
	const double ValueMultiplier() const { return ValueMultiplier_; }
};
JSONCONS_ALL_GETTER_CTOR_TRAITS_DECL(DecorationMaterial, Name, Chance, ValueMultiplier)

class Decoration
{
	std::string Tag_;
	std::string Text_;
	int ValueBase_;
	std::vector<DecorationMaterial> MaterialTypes_;

public:
	Decoration(const std::string& Tag, const std::string& Text, const int& ValueBase, const std::vector<DecorationMaterial>& MaterialTypes): Tag_(Tag), Text_(Text), ValueBase_(ValueBase), MaterialTypes_(MaterialTypes)
	{
		
	}

	const std::string& Tag() const { return Tag_; }
	const std::string& Text() const { return Text_; }
	const int ValueBase() const { return ValueBase_; }
	const std::vector<DecorationMaterial>& MaterialTypes() const { return MaterialTypes_; }
};
JSONCONS_ALL_GETTER_CTOR_TRAITS_DECL(Decoration, Tag, Text, ValueBase, MaterialTypes)

class DecorationSet
{
	std::vector<Decoration> Decorations_;

public:
	DecorationSet(const std::vector<Decoration>& Decorations) : Decorations_(Decorations)
	{}

	const std::vector<Decoration>& Decorations() const { return Decorations_; }
};
JSONCONS_ALL_GETTER_CTOR_TRAITS_DECL(DecorationSet, Decorations)

// generated item
struct ItemSet
{
	std::vector<std::string> Name;							// inventory name
	std::vector<std::string> Visual;						// visual character
	std::vector<std::string> ShortDescription;				// "look" text
	std::vector<std::string> LongDescription;				// "examine" text
	std::vector<int> Value;									// value in cp
	std::vector <std::vector<std::string>> Tags;			// compiled set of tags for the item (equipment tags, material tags etc)
	std::vector<int> WeightDen;								// weight denominator
	std::vector<int> WeightNum;								// weight numerator

	int nextID = 800;
	// int nextID = 0;
};

enum RANGES
{
	SHORT_RANGE = 0,
	MEDIUM_RANGE,
	LONG_RANGE,
	RANGE_MAX
};

class ItemManager
{
	ItemSet items;
	TemplateSet itemTemplates;
	DecorationSet decorations;

	std::map<std::string, std::vector<int>> rangeDictionary; // indexed by the item tag, then the range
	std::vector<int> rangePenalties; // indexed by range enum
	
	std::map<std::string, int> reverseTemplateDictionary;
	std::map<std::string, int> reverseDecorationDictionary;
	
	int getRangePenalty(std::string tag, int range);

public:
	ItemManager(TemplateSet& _items, DecorationSet& _decorations, std::map<std::string, std::vector<int>> _ranges, std::vector<int> _rangePenalties) : itemTemplates(_items), decorations(_decorations), rangeDictionary(_ranges), rangePenalties(_rangePenalties)
	{
		
		items.Visual.resize(items.nextID);
		items.Name.resize(items.nextID);
		items.ShortDescription.resize(items.nextID);
		items.LongDescription.resize(items.nextID);
		items.Value.resize(items.nextID);
		items.Tags.resize(items.nextID);
		items.WeightDen.resize(items.nextID);
		items.WeightNum.resize(items.nextID);
	}

	static ItemManager* LoadItemTemplates();

	TemplateSet& ItemTemplates() { return itemTemplates; }

	int GenerateItemFromTemplate(std::string name);
	int GenerateItemFromTemplate(int templateID);

	int getRangePenalty(int id, int range);
	int getMaxRange(int id);
	
	// accessors
	std::string getName(int id) { return items.Name[id]; }
	std::string getVisual(int id) { return items.Visual[id]; }
	std::string getShortDescription(int id) { return items.ShortDescription[id]; }
	std::string getLongDescription(int id) { return items.LongDescription[id]; }
	int getValue(int id) { return items.Value[id]; }
	int getWeightDen(int id) { return items.WeightDen[id]; }
	int getWeightNum(int id) { return items.WeightNum[id]; }
	
	bool hasTag(int id, std::string tag);

	std::vector<std::string>& getTags(int id) { return items.Tags[id]; }
	
	// utils
	double getWeight(std::vector<int> items);
	double getWeight(std::list<int> items);

	void DebugLog(std::string message);
};

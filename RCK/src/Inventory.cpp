#include "Inventory.h"

InventoryManager* InventoryManager::CreateInventoryManager()
{
    InventoryManager* im = new InventoryManager();

    return im;
}

int InventoryManager::RegisterInventory(int ownerManager, int ownerEntity)
{
    // create a new Inventory owned by the specified Entity and return the ID
    int newID = nextInventoryID++;
    
    itemIDs.push_back(std::vector<int>());
    itemCounts.push_back(std::vector<int>());
    ownerManagers.push_back(ownerManager);
    ownerEntities.push_back(ownerEntity);

    return newID;
}

int InventoryManager::AddItemToInventory(int inventoryID, int itemID, int count)
{
    int output = -1;
    if (std::find(itemIDs[inventoryID].begin(), itemIDs[inventoryID].end(), itemID) != itemIDs[inventoryID].end())
    {
        // we already have this item, so increase the count
        int index = std::find(itemIDs[inventoryID].begin(), itemIDs[inventoryID].end(), itemID) - itemIDs[inventoryID].begin();
        itemCounts[inventoryID][index] += count;
        output = index;
    }
    else
    {
        // we don't have this item, so add it to the inventory
        output = itemIDs[inventoryID].size();
        itemIDs[inventoryID].push_back(itemID);
        itemCounts[inventoryID].push_back(count);
    }
    return output;
}

std::vector<std::pair<int, int>> InventoryManager::RemoveItemFromInventory(int inventoryID, int itemID, int count)
{
    std::vector<std::pair<int, int>> removedItems;
    if (std::find(itemIDs[inventoryID].begin(), itemIDs[inventoryID].end(), itemID) != itemIDs[inventoryID].end())
    {
        // we already have this item, so decrease the count
        int index = std::find(itemIDs[inventoryID].begin(), itemIDs[inventoryID].end(), itemID) - itemIDs[inventoryID].begin();
        if (itemCounts[inventoryID][index] >= count)
        {
            itemCounts[inventoryID][index] -= count;
            removedItems.push_back(std::make_pair(itemID, count));
        }
        else
        {
            // we no longer have any of these items
            removedItems.push_back(std::make_pair(itemID, count - itemCounts[inventoryID][index]));
            // delete item from all vectors
            itemIDs[inventoryID].erase(itemIDs[inventoryID].begin() + index);
            itemCounts[inventoryID].erase(itemCounts[inventoryID].begin() + index);
        }
    }

    return removedItems;
}

void InventoryManager::TransferInventory(int sourceinventoryID, int destinationInventoryID)
{
    // transfer all items from source to destination
    for (int i = 0; i < itemIDs[sourceinventoryID].size(); i++)
    {
        AddItemToInventory(destinationInventoryID, itemIDs[sourceinventoryID][i], itemCounts[sourceinventoryID][i]);
    }
    // delete source inventory
    itemIDs.erase(itemIDs.begin() + sourceinventoryID);
    itemCounts.erase(itemCounts.begin() + sourceinventoryID);
    ownerManagers.erase(ownerManagers.begin() + sourceinventoryID);
    ownerEntities.erase(ownerEntities.begin() + sourceinventoryID);
}

int InventoryManager::GetItemCount(int inventoryID, int itemID)
{
    // find corresponding count for the item ID
    if (std::find(itemIDs[inventoryID].begin(), itemIDs[inventoryID].end(), itemID) != itemIDs[inventoryID].end())
    {
        int index = std::find(itemIDs[inventoryID].begin(), itemIDs[inventoryID].end(), itemID) - itemIDs[inventoryID].begin();
        return itemCounts[inventoryID][index];
    }
    else
    {
        return 0;
    }
}

int InventoryManager::GetInventorySize(int inventoryID)
{
    return itemIDs[inventoryID].size();
}

std::vector<std::pair<int, int>> InventoryManager::GetInventory(int inventoryID)
{
    // combine itemIDs and itemCounts into a single vector
    std::vector<std::pair<int, int>> output;
    for (int i = 0; i < itemIDs[inventoryID].size(); i++)
    {
        output.push_back(std::make_pair(itemIDs[inventoryID][i], itemCounts[inventoryID][i]));
    }
    return output;
}


void InventoryManager::OpenInventoryMenu(int sourceID, int destinationID)
{
    // if there is no destination, we pressed the inventory key, 
    // otherwise we're doing inventory transfer

    sourceMenuInventoryID = sourceID;
    targetMenuInventoryID = destinationID;
    
    sourceMenuPosition = 0;
    targetMenuPosition = 0;
    
    sourcePane = true;
}
void InventoryManager::CloseMenu()
{
    sourceMenuInventoryID = -1;
    targetMenuInventoryID = -1;
}

int InventoryManager::ControlMoveUp()
{
    int output = 0;
    if (sourcePane)
    {
        if (sourceMenuPosition > 0)
        {
            sourceMenuPosition--;
        }
        else
        {
            sourceMenuPosition = itemIDs[sourceMenuInventoryID].size() - 1;
        }
        output = sourceMenuPosition;
    }
    else
    {
        if (targetMenuPosition > 0)
        {
            targetMenuPosition--;
        }
        else
        {
            targetMenuPosition = itemIDs[targetMenuInventoryID].size() - 1;
        }
        output = targetMenuPosition;
    }
    return output;
}

int InventoryManager::ControlMoveDown()
{
    int output = 0;
    if (sourcePane)
    {
        if (sourceMenuPosition < itemIDs[sourceMenuInventoryID].size() - 1)
        {
            sourceMenuPosition++;
        }
        else
        {
            sourceMenuPosition = 0;
        }
        output = sourceMenuPosition;
    }
    else
    {
        if (targetMenuPosition < itemIDs[targetMenuInventoryID].size() - 1)
        {
            targetMenuPosition++;
        }
        else
        {
            targetMenuPosition = 0;
        }
        output = targetMenuPosition;
    }
    return output;
}

int InventoryManager::ControlMoveLeft()
{
    sourcePane = true;

    return sourceMenuPosition;
}

int InventoryManager::ControlMoveRight()
{
    sourcePane = false;

    return targetMenuPosition;
}

int InventoryManager::Select()
{
    return 0;
}

std::string InventoryManager::InvToText(int inventoryID, int slot)
{
    std::string output;
    
    int itemID = itemIDs[inventoryID][slot];
    int count = itemCounts[inventoryID][slot];
    
    return ItemToText(itemID, count);
}

std::string InventoryManager::ItemToText(int itemID, int count)
{
    std::string output;

    std::string itemName = gGame->mItemManager->getName(itemID);
    
    if (count > 1)
        output = std::to_string(count) + "x ";
    
    output = output + itemName;

    return output;
}

void InventoryManager::RenderInventory()
{
    g_console.clear();

    if (targetMenuInventoryID != -1)
    {
        // dual window setup
        tcod::print_frame(
            g_console,
            {
                0, 0, SAMPLE_SCREEN_WIDTH / 2, SAMPLE_SCREEN_HEIGHT
            },
            "",
            &TCOD_white,
            &TCOD_black,
            TCOD_BKGND_SET,
            true);

        
        tcod::print_frame(
            g_console,
            {
                0, 0, SAMPLE_SCREEN_WIDTH, SAMPLE_SCREEN_HEIGHT
            },
            "Inventory",
            &TCOD_white,
            &TCOD_black,
            TCOD_BKGND_SET,
            true);
    }
    else
    {
        // single window setup
        tcod::print_frame(
            g_console,
            {
                0, 0, SAMPLE_SCREEN_WIDTH, SAMPLE_SCREEN_HEIGHT
            },
            "Inventory",
            &TCOD_white,
            &TCOD_black,
            TCOD_BKGND_SET,
            true);
    }

    int start_ypos = 4;
    int ypos = start_ypos;

    if (sourceMenuInventoryID != -1)
    {
        for (int i = 0; i < itemIDs[sourceMenuInventoryID].size(); i++)
        {
            int selected = -1;
            if (sourcePane)
            {
                selected = sourceMenuPosition;
            }

            std::string s = InvToText(sourceMenuInventoryID,i);
            TCOD_ColorRGB backg = TCOD_black;
            TCOD_ColorRGB foreg = TCOD_white;
            if (i == selected)
            {
                backg = TCOD_white;
                foreg = TCOD_black;
            }

            tcod::print(g_console, { 3, ypos + i }, s, foreg, backg, TCOD_LEFT, TCOD_BKGND_SET);
        }
    }
    
    if (targetMenuInventoryID != -1)
    {
        for (int i = 0; i < itemIDs[targetMenuInventoryID].size(); i++)
        {
            int selected = -1;
            if (!sourcePane)
            {
                selected = targetMenuPosition;
            }

            std::string s = InvToText(targetMenuInventoryID, i);
            TCOD_ColorRGB backg = TCOD_black;
            TCOD_ColorRGB foreg = TCOD_white;
            if (i == selected)
            {
                backg = TCOD_white;
                foreg = TCOD_black;
            }

            tcod::print(g_console, { SAMPLE_SCREEN_WIDTH / 2 + 1, ypos + i }, s, foreg, backg, TCOD_LEFT, TCOD_BKGND_SET);
        }
    }
}


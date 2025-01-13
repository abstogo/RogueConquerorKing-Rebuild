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
    
    itemEntries.push_back(std::vector<std::pair<int, int>>());
    
    ownerManagers.push_back(ownerManager);
    ownerEntities.push_back(ownerEntity);

    return newID;
}

int InventoryManager::AddItemToInventory(int inventoryID, int itemID, int count)
{
    int output = -1;

    auto vec = itemEntries[inventoryID];
    auto loc = std::find_if(vec.begin(), vec.end(), [itemID](const std::pair<int, int>& p) { return p.second == itemID; });

    if( loc != vec.end())
    {
        // we already have this item, so increase the count
        int index = loc - vec.begin();
        itemEntries[inventoryID][index].second = count;
        output = index;
    }
    else
    {
        // we don't have this item, so add it to the inventory
        output = itemEntries[inventoryID].size();
        itemEntries[inventoryID].push_back(std::make_pair(itemID, count));
    }
    return output;
}

std::vector<std::pair<int, int>> InventoryManager::RemoveItemFromInventory(int inventoryID, int itemID, int count)
{
    std::vector<std::pair<int, int>> removedItems;

    auto vec = itemEntries[inventoryID];
    auto loc = std::find_if(vec.begin(), vec.end(), [itemID](const std::pair<int, int>& p) { return p.first == itemID; });

    if (loc != vec.end())
    {
        // we already have this item, so decrease the count
        int index = loc - vec.begin();
        if (itemEntries[inventoryID][index].second > count)
        {
            itemEntries[inventoryID][index].second -= count;
            removedItems.push_back(std::make_pair(itemID, count));
        }
        else
        {
            // we no longer have any of these items
            removedItems.push_back(std::make_pair(itemID, itemEntries[inventoryID][index].second));
            // delete item from all vectors
            itemEntries[inventoryID].erase(itemEntries[inventoryID].begin() + index);
        }
    }

    return removedItems;
}

void InventoryManager::TransferInventory(int sourceinventoryID, int destinationInventoryID)
{
    // transfer all items from source to destination
    for (int i = 0; i < itemEntries[sourceinventoryID].size(); i++)
    {
        AddItemToInventory(destinationInventoryID, itemEntries[sourceinventoryID][i].first, itemEntries[sourceinventoryID][i].second);
    }
    // we do not delete source inventory. Instead we zero it out
    itemEntries[sourceinventoryID].clear();
    //ownerManagers.erase(ownerManagers.begin() + sourceinventoryID);
    //ownerEntities.erase(ownerEntities.begin() + sourceinventoryID);
}

int InventoryManager::GetItemCount(int inventoryID, int itemID)
{
    auto vec = itemEntries[inventoryID];
    auto loc = std::find_if(vec.begin(), vec.end(), [itemID](const std::pair<int, int>& p) { return p.second == itemID; });

    // find corresponding count for the item ID
    if (loc != vec.end())
    {
        int index = loc - vec.begin();
        return itemEntries[inventoryID][index].second;
    }
    else
    {
        return 0;
    }
}

int InventoryManager::GetInventorySize(int inventoryID)
{
    return itemEntries[inventoryID].size();
}

std::vector<std::pair<int, int>>& InventoryManager::GetInventory(int inventoryID)
{
    return itemEntries[inventoryID];
}


void InventoryManager::OpenInventoryMenu(int sourceID, int destinationID, std::string title)
{
    // if there is no destination, we pressed the inventory key, 
    // otherwise we're doing inventory transfer

    menuTitle = title;
    
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
            sourceMenuPosition = itemEntries[sourceMenuInventoryID].size() - 1;
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
            targetMenuPosition = itemEntries[targetMenuInventoryID].size() - 1;
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
        if (sourceMenuPosition < itemEntries[sourceMenuInventoryID].size() - 1)
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
        if (targetMenuPosition < itemEntries[targetMenuInventoryID].size() - 1)
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

int InventoryManager::SelectPrimary()
{
    // Usually enter.
    // In the standard character inventory, this wields the selected item if possible.
    // In the inventory exchange screen, this transfers all of the selected items between invs
    
    int output = -1;

    if(targetMenuInventoryID == -1)
    {
        // we're in the character inventory, so wield/unwield the item
        
        if (ownerManagers[sourceMenuInventoryID] == MANAGER_CHARACTER)
        {
            if (gGame->mCharacterManager->GetEquipSlotForInventoryItem(ownerEntities[sourceMenuInventoryID], sourceMenuPosition) != -1)
            {
                gGame->mCharacterManager->UnequipItem(ownerEntities[sourceMenuInventoryID], sourceMenuPosition);
            }
            else
            {
                gGame->mCharacterManager->EquipItem(ownerEntities[sourceMenuInventoryID], sourceMenuPosition);
            }
        }
    }
    else
    {
        // we're in the inventory exchange screen, so transfer the selected stack of items
        if (sourcePane)
        {
            int itemID = itemEntries[sourceMenuInventoryID][sourceMenuPosition].first;
            int count = itemEntries[sourceMenuInventoryID][sourceMenuPosition].second;
            auto items = RemoveItemFromInventory(sourceMenuInventoryID, itemID, count);

            AddItemToInventory(targetMenuInventoryID, items[0].first, items[0].second);
        }
        else
        {
            int itemID = itemEntries[targetMenuInventoryID][targetMenuPosition].first;
            int count = itemEntries[targetMenuInventoryID][targetMenuPosition].second;
            auto items = RemoveItemFromInventory(targetMenuInventoryID, itemID, count);
            
            AddItemToInventory(sourceMenuInventoryID, items[0].first, items[0].second);
        }
    }
    
    return 0;
}

int InventoryManager::SelectSecondary()
{
    // Usually "D".
    // In the standard character inventory, this drops the item onto the map.
    // In the inventory exchange screen, this transfers all of one inventory to another.
    
    int output = -1;

    if (targetMenuInventoryID == -1)
    {
        // we're in the character inventory, so drop the item on the ground

        if (ownerManagers[sourceMenuInventoryID] == MANAGER_CHARACTER)
        {
            // this only works on the local maps
            int mapID = gGame->mCharacterManager->GetPlayerMap(ownerEntities[sourceMenuInventoryID]);
            if (mapID != -1)
            {
                int x = gGame->mCharacterManager->GetPlayerX(ownerEntities[sourceMenuInventoryID]);
                int y = gGame->mCharacterManager->GetPlayerY(ownerEntities[sourceMenuInventoryID]);
                Map* map = gGame->mMapManager->getMap(mapID);
                int itemID = itemEntries[sourceMenuInventoryID][sourceMenuPosition].first;
                int count = itemEntries[sourceMenuInventoryID][sourceMenuPosition].second;
                auto items = RemoveItemFromInventory(sourceMenuInventoryID, itemID, count);
                for(std::pair<int,int> g : items)
                {
                    for (int i = 0; i < g.second; i++)
                        map->addItem(x, y, g.first);
                }
            }
        }
    }
    else
    {
        // we're in the inventory exchange screen, so transfer all of the items
        TransferInventory(sourceMenuInventoryID, targetMenuInventoryID);
    }
    
    return output;
}

std::string InventoryManager::InvToText(int inventoryID, int slot)
{
    std::string output;
    
    int itemID = itemEntries[inventoryID][slot].first;
    int count = itemEntries[inventoryID][slot].second;
    
    return ItemToText(itemID, count);
}

std::string InventoryManager::ItemToText(int itemID, int count)
{
    std::string output = "";

    std::string itemName = gGame->mItemManager->getName(itemID);

    if (gGame->mItemManager->hasTag(itemID, "Goods") || (count > 1))
    {
            output = std::to_string(count) + "x ";
    }
    
    output = output + itemName;

    return output;
}

std::string InventoryManager::GetInventoryEntityName(int inventoryID)
{
    std::string output;

    if (ownerManagers[inventoryID] == MANAGER_CHARACTER)
    {
        output = gGame->mCharacterManager->getCharacterName(ownerEntities[inventoryID]);
    }
    else if (ownerManagers[inventoryID] == MANAGER_MAP)
    {
        // "map" inventories are for containers (including piles of items on the ground)
        // output = gGame->mMapManager->getMapName(ownerEntities[inventoryID]);
    }
    else if (ownerManagers[inventoryID] == MANAGER_MOB)
    {
        output = gGame->mMobManager->GetMonster(ownerEntities[inventoryID]).GetName();
    }
    else if (ownerManagers[inventoryID] == MANAGER_ITEM)
    {
        output = gGame->mItemManager->getName(ownerEntities[inventoryID]);
    }
    else if (ownerManagers[inventoryID] == MANAGER_PARTY)
    {
        output = "Party";
    }
    else if (ownerManagers[inventoryID] == MANAGER_BASE)
    {
        output = gGame->mBaseManager->GetBaseType(ownerEntities[inventoryID]);
    }

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
                0, 0, SAMPLE_SCREEN_WIDTH, SAMPLE_SCREEN_HEIGHT
            },
            menuTitle,
            &TCOD_white,
            &TCOD_black,
            TCOD_BKGND_SET,
            true);
        
        tcod::print_frame(
            g_console,
            {
                0, 1, SAMPLE_SCREEN_WIDTH / 2, SAMPLE_SCREEN_HEIGHT
            },
            GetInventoryEntityName(sourceMenuInventoryID),
            &TCOD_white,
            &TCOD_black,
            TCOD_BKGND_SET,
            true);

        tcod::print_frame(
            g_console,
            {
                SAMPLE_SCREEN_WIDTH / 2, 1, SAMPLE_SCREEN_WIDTH, SAMPLE_SCREEN_HEIGHT
            },
            GetInventoryEntityName(targetMenuInventoryID),
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
            menuTitle,
            &TCOD_white,
            &TCOD_black,
            TCOD_BKGND_SET,
            true);
        
        tcod::print_frame(
            g_console,
            {
                0, 1, SAMPLE_SCREEN_WIDTH, SAMPLE_SCREEN_HEIGHT
            },
            GetInventoryEntityName(sourceMenuInventoryID),
            &TCOD_white,
            &TCOD_black,
            TCOD_BKGND_SET,
            true);
    }

    int start_ypos = 4;
    int ypos = start_ypos;

    if (sourceMenuInventoryID != -1)
    {
        for (int i = 0; i < itemEntries[sourceMenuInventoryID].size(); i++)
        {
            int selected = -1;
            if (sourcePane)
            {
                selected = sourceMenuPosition;
            }

            std::string s = InvToText(sourceMenuInventoryID,i);
            
            // The entity manager modifies the displayed elements
            // The basic use for this is for equipment slots, but there are other possible uses
            if (ownerManagers[sourceMenuInventoryID] == MANAGER_CHARACTER)
            {
                int slot = gGame->mCharacterManager->GetEquipSlotForInventoryItem(ownerEntities[sourceMenuInventoryID], i);
                if(slot == HAND_MAIN)
                { 
                    s = s + " (in main hand)";
                }
                else if (slot == HAND_OFF)
                {
                    s = s + " (in off-hand)";
                }
                else if (slot > HAND_OFF)
                {
                    // this is a worn item such as armour or boots
                    s = s + " (worn)";
                }
            }
            
            
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
        for (int i = 0; i < itemEntries[targetMenuInventoryID].size(); i++)
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


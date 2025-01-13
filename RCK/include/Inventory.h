#pragma once
#include "Game.h"

class InventoryManager
{
    // The Inventory is a specialised form of Menu.
    // In essence, the InventoryManager produces two menus, side-by-side, with specialised content and controls.
    
    // Items in RCK are IDs into the ItemManager. Each entry in the ItemManager is a unique instance of an ItemTemplate.
    // With equipment and other discrete examples, this makes intuitive sense. However the Inventory also needs to manage 
    // currency and loot items.
    // To that end, an Inventory is a combination of Item IDs and "stack" counts. 
    // As a data element, we only stack identical items (note that similar items crafted apart from one another are *not* considered identical, 
    // unless something special like Forgery is happening).
    // However we can allow for similar items to be collapsed together for display purposes.
    
    // The Inventory screen is then used to manipulate and transfer items between inventories held by different things.
    // All these inventories are stored in this manager, since they are identical in functionality. 
    // You can also open an Inventory screen with no "destination" pane, eg the Inventory key, in order to manage the current player entity's inventory (character or party).

    // Once we have a unified inventory system, we can then add Inventories to anything - which makes containers easy to implement.

    int nextInventoryID = 0;
    
    std::vector<std::vector<std::pair<int, int>>> itemEntries;

    std::vector<int> ownerManagers;
    std::vector<int> ownerEntities;
    
    int sourceMenuInventoryID = -1;
    int sourceMenuPosition;
    
    int targetMenuInventoryID = -1;
    int targetMenuPosition;

    std::string menuTitle;

    bool sourcePane = true;
    
public:
    InventoryManager()
	{}
    ~InventoryManager()
	{}

	// Manager Factory
    static InventoryManager* CreateInventoryManager();

    int RegisterInventory(int ownerManager, int ownerEntity);

    // Inventory Management
    int AddItemToInventory(int inventoryID, int itemID, int count);
    std::vector<std::pair<int, int>> RemoveItemFromInventory(int inventoryID, int itemID, int count);
    void TransferInventory(int sourceinventoryID, int destinationInventoryID);
    
    int GetItemCount(int inventoryID, int itemID);
    int GetInventorySize(int inventoryID);
    std::vector<std::pair<int, int>>& GetInventory(int inventoryID);

    std::string GetInventoryEntityName(int inventoryID);

    // Inventory Menu   
    void OpenInventoryMenu(int sourceID, int destinationID=-1, std::string title="");
    void CloseMenu();

    // Inventory Controls
    int ControlMoveUp();
    int ControlMoveDown();
    int ControlMoveLeft();
    int ControlMoveRight();
    int SelectPrimary();
    int SelectSecondary();

    // Rendering
	void RenderInventory();
    std::string InvToText(int inventoryID, int slot);
    std::string ItemToText(int itemID, int count);
};

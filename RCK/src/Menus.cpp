#include "Menus.h"

MenuManager* MenuManager::CreateMenuManager()
{
    MenuManager* mm = new MenuManager();

    return mm;
}

int MenuManager::BuildMenu(int originManager, std::string name)
{
    int newID = currentMenuID++;

    menuNames.push_back(name);
    menuManagers.push_back((ManagerType)originManager);

    menuTexts.push_back(std::vector<std::string>());
    menuStates.push_back(std::vector<int>());
    menuTypes.push_back(std::vector<int>());

    menuPositions.push_back(0);

    return newID;
}

int MenuManager::AddMenuEntry(int menuID, MenuEntryTypes menuType, std::string menuText, int defaultState)
{
    int newID = menuTexts[menuID].size();
    
    menuTypes[menuID].push_back((int)menuType);
    menuTexts[menuID].push_back(menuText);
    menuStates[menuID].push_back(defaultState);

    return newID;
}

int MenuManager::GetCurrentMenu()
{
    return currentMenuID;
}

void MenuManager::OpenMenu(int menuID)
{
    currentMenuID = menuID;
}

int MenuManager::ControlMoveUp()
{
    int len = menuTexts[currentMenuID].size();

    int pos = menuPositions[currentMenuID] - 1;
    if (pos < 0)
    {
        pos = len - 1;
    }

    menuPositions[currentMenuID] = pos;

    return pos;
}

int MenuManager::ControlMoveDown()
{
    int len = menuTexts[currentMenuID].size();

    int pos = menuPositions[currentMenuID] + 1;

    if (pos >= len) {
        pos = 0;
    }

    menuPositions[currentMenuID] = pos;

    return pos;
}

int MenuManager::Select()
{
    ManagerType pos = (ManagerType)menuPositions[currentMenuID];

    switch(pos)
    {
            case MANAGER_GAME:
                if (gGame->MenuHandler(menuNames[currentMenuID], pos))
                {
                    // return true means we're done, close the menu
                    currentMenuID = -1;
                } else
                {
                    // return false means we're not done
                }
            return pos;
            break;
    }
}

bool MenuManager::MenuOpen()
{
    return currentMenuID != -1;
}

void MenuManager::RenderCurrentMenu()
{
    g_console.clear();

    tcod::print_frame(
        g_console,
        {
            0, 0, SAMPLE_SCREEN_WIDTH, SAMPLE_SCREEN_HEIGHT
        },
        menuNames[currentMenuID],
        &TCOD_white,
        &TCOD_black,
        TCOD_BKGND_SET,
        true);

    int start_ypos = 4;
    int ypos = start_ypos;

    int selected = menuPositions[currentMenuID];
    int p = 0;

    for (int i = 0; i < menuTexts[currentMenuID].size(); i++)
    {
        std::string s = menuTexts[currentMenuID][i];
        TCOD_ColorRGB backg = TCOD_white;
        TCOD_ColorRGB foreg = TCOD_black;
        if (i == selected)
        {
            TCOD_ColorRGB backg = TCOD_black;
            TCOD_ColorRGB foreg = TCOD_white;
        }

        tcod::print(g_console, {8, ypos+i}, s, foreg, backg, TCOD_LEFT, TCOD_BKGND_NONE);
    }
}

#pragma once
#include "Game.h"

enum MenuEntryTypes
{
    MT_SELECT,
    MT_OPTION,
};

class MenuManager
{
    // Data-driven menu manager
    //
    // The goal here is to store the status of the game's various menus in a single manager.
    // This will allow us to easily switch between menus and keep track of the state of the game.
    // Part of the First Big Refactor, since this stuff was cluttering up Game.cpp

    // This class handles selection and option menus depending on what line options you choose
    // Currently available: Selector, Radio Toggle
    // To Be Added: Slider, Modal Entry

    // Current menu (the one which renders on calling RenderMenu)
    int currentMenuID = -1;

    int nextMenuID = 0;
    std::vector<std::string> menuNames;
    std::vector<std::vector<std::string>> menuTexts;
    std::vector<std::vector<int>> menuTypes;
    std::vector<std::vector<int>> menuStates;
    std::vector<int> menuManagers;

    std::vector<int> menuPositions;

public:
    MenuManager()
	{}
    ~MenuManager()
	{}

	// Manager Factory
    static MenuManager* CreateMenuManager();

    int BuildMenu(int originManager, std::string name);
    int AddMenuEntry(int menuID, MenuEntryTypes type, std::string menuText, int defaultState = -1);

    int GetCurrentMenu();
    void OpenMenu(int menuID);
    void CloseMenu() { currentMenuID = -1; }
    bool MenuOpen();

    int ControlMoveUp();
    int ControlMoveDown();
    int Select();

	void RenderCurrentMenu();

};

#include <Vector.h>

#ifndef MENUS_h
#define MENUS_h

#define MAX_MENUS 10
#define MAX_ITEMS 100   // maximum items for all menus combined

typedef int Menu;

struct Item
{
    int id;
    int parentId;
    char *name;       // variable for determining type of storable value
    char *symbol;     // symbol to show along the numerical value
    int stepValue;    // how much to add
    int minNumValue;  // minimum numerical value for encoder
    int maxNumValue;  // maximum numerical value for encoder
    bool cycleValues; // for cycling through items or values in the menu
    bool isSelected;  // true if item is selected in menu
    bool changed;     // true if item changed (obviously)
    bool saveToNVS;   // set true to save this item's value to non volatile storage

    enum
    {
        INTEGER,
        FLOAT,
        STRING,
        BOOL,
        REDIRECT
    } valueType;

    union {             // storing main value for item
        int numValue;   // redirect, integer or boolean value in one type
        float fltValue; // float for value with more precision
        char *strValue; // String for status value
    };

    // used to run an attached function
    void (*valuePtr)(Item *);

    // used to run custom function processing display value
    String (*customDispValPtr)(int);
};

struct MenuItems
{
    int menuId;
    int idFirstDisplayedItem;
    int itemsAdded;
    int *itemIdArray;
    int prevSelectedItem;
    bool cycleItems;
    bool isItemSelected;
    char *menuTitle;
    const String *ptrStrings;
    bool changed;

    // used to run an attached function
    void (*customMenuFuncPtr)(MenuItems *);
};

class MenusClass
{
private:
    MenuItems menus_storage[MAX_MENUS];
    Item item_storage[MAX_ITEMS];
    int maxItemsToDisplay;
    int currentMenuId;

    // used to run an attached function
    void (*menuFuncPtr)(MenuItems *);

    // used to run function to save menu item value
    void (*saveItemFuncPtr)(Item *);

public:
    Vector<MenuItems> menusData;
    Vector<Item> itemsData;

    MenusClass();
    ~MenusClass();

    void init(int maxDisplayedItems, void (*funcPtr)(MenuItems *), void (*saveFuncPtr)(Item *));

    int createMenu(const char *name, const String *stringArray, int arraySize, bool cycle = false, void (*customFuncPtr)(MenuItems *) = NULL);

    void createItem(int parentId, int redirectValue);

    void createItem(int parentId, bool cycle, const char *symbolStr, void (*valuePtr)(Item *), int maxVal, int minVal = 0, int stepVal = 1, bool saveItem = false, String (*customDispFunc)(int) = NULL);

    void createItem(int indexItem, bool stateItem, void (*valuePtr)(Item *), bool saveItem = false);

    void createItem(int indexItem, const char *strVal, void (*valuePtr)(Item *), bool saveItem = false, bool numVal = false, String (*customDispFunc)(int) = NULL);

    void createItem(int indexItem, float fltVal, const char *symbolStr, void (*valuePtr)(Item *), bool saveItem = false);

    void updateItem(int menuId, int itemIndex, int value);

    void updateItem(int menuId, int itemIndex, float value);

    void updateItem(int menuId, int itemIndex, const char *value, int numVal = NULL);

    Item* getItem(int menuId, int itemIndex);

    void setCurrentMenu(int menuId);

    int getCurrentMenu();

    int getItemsCount(int menuId);

    int getFirstItemToDisplay();

    void setSelected(int itemIndex);

    String getItemString(int itemIndex);

    bool checkItemSelected(int itemIndex);

    void prepareMenu();

    void update();
};

extern MenusClass Menus;

#endif
#include "Menus.h"

MenusClass::MenusClass()
{
    menusData.setStorage(menus_storage);
    itemsData.setStorage(item_storage);
}

MenusClass::~MenusClass()
{
    // free heap memory before destructor
    for (int i = 0; i < menusData.size(); i++)
    {
        delete[] menusData[i].menuTitle;
        delete[] menusData[i].ptrStrings;
        delete[] menusData[i].itemIdArray;
        menusData[i].menuTitle = NULL;
        menusData[i].ptrStrings = NULL;
        menusData[i].itemIdArray = NULL;
    }

    for (int i = 0; i < itemsData.size(); i++)
    {
        delete[] itemsData[i].name;
        itemsData[i].name = NULL;
    }
}

void MenusClass::init(int maxDisplayedItems, void (*funcPtr)(MenuItems *), void (*saveFuncPtr)(Item *))
{
    maxItemsToDisplay = maxDisplayedItems;
    menuFuncPtr = funcPtr;
    saveItemFuncPtr = saveFuncPtr;
}

int MenusClass::createMenu(const char *name, const String *stringArray, int arraySize, bool cycle, void (*customFuncPtr)(MenuItems *))
{
    MenuItems menu;
    int size;

    menu.menuId = menusData.size();
    menu.idFirstDisplayedItem = 0;
    menu.itemsAdded = 0;
    menu.itemIdArray = new int[arraySize];
    menu.prevSelectedItem = 0;
    menu.cycleItems = cycle;
    menu.isItemSelected = false;
    menu.ptrStrings = stringArray;
    menu.changed = false;
    menu.customMenuFuncPtr = customFuncPtr;

    size = strlen(name) + 1;
    menu.menuTitle = new char[size];
    strncpy(menu.menuTitle, name, size);

    menusData.push_back(menu);

    return menusData.size() - 1;
}

void MenusClass::createItem(int parentId, int redirectVal)
{
    Item mItem;
    int size;

    mItem.id = menusData[parentId].itemsAdded++;
    mItem.parentId = parentId;
    mItem.valueType = Item::REDIRECT;
    mItem.numValue = redirectVal;
    mItem.changed = false;
    mItem.maxNumValue = NULL;
    mItem.cycleValues = NULL;
    mItem.symbol = NULL;
    mItem.stepValue = NULL;
    mItem.valuePtr = NULL;
    mItem.customDispValPtr = NULL;
    mItem.saveToNVS = false;

    if (mItem.id > 0)
        mItem.isSelected = false;
    else
        mItem.isSelected = true;

    size = strlen(menusData[parentId].ptrStrings[mItem.id].c_str()) + 1;
    mItem.name = new char[size];
    strncpy(mItem.name, menusData[parentId].ptrStrings[mItem.id].c_str(), size);

    menusData[parentId].itemIdArray[mItem.id] = itemsData.size();

    itemsData.push_back(mItem);
}

void MenusClass::createItem(int parentId, bool cycle, const char *symbolStr, void (*valuePtr)(Item *), int maxVal, int minVal, int stepVal, bool saveItem, String (*customDispFunc)(int))
{
    Item mItem;
    int size;

    mItem.id = menusData[parentId].itemsAdded++;
    mItem.parentId = parentId;
    mItem.valueType = Item::INTEGER;
    mItem.numValue = 0;
    mItem.changed = false;
    mItem.minNumValue = minVal;
    mItem.maxNumValue = maxVal;
    mItem.cycleValues = cycle;
    mItem.stepValue = stepVal;
    mItem.valuePtr = valuePtr;
    mItem.customDispValPtr = customDispFunc;
    mItem.saveToNVS = saveItem;

    if (mItem.id > 0)
        mItem.isSelected = false;
    else
        mItem.isSelected = true;

    size = strlen(menusData[parentId].ptrStrings[mItem.id].c_str()) + 1;
    mItem.name = new char[size];
    strncpy(mItem.name, menusData[parentId].ptrStrings[mItem.id].c_str(), size);

    size = strlen(symbolStr) + 1;
    mItem.symbol = new char[size];
    strncpy(mItem.symbol, symbolStr, size);

    menusData[parentId].itemIdArray[mItem.id] = itemsData.size();

    itemsData.push_back(mItem);
}

void MenusClass::createItem(int parentId, bool stateVal, void (*valuePtr)(Item *), bool saveItem)
{
    Item mItem;
    int size;
    String tempStr = stateVal ? "ON" : "OFF";

    mItem.id = menusData[parentId].itemsAdded++;
    mItem.parentId = parentId;
    mItem.valueType = Item::BOOL;
    mItem.numValue = stateVal;
    mItem.changed = false;
    mItem.minNumValue = NULL;
    mItem.maxNumValue = NULL;
    mItem.cycleValues = NULL;
    mItem.symbol = (char *)tempStr.c_str();
    mItem.stepValue = NULL;
    mItem.valuePtr = valuePtr;
    mItem.customDispValPtr = NULL;
    mItem.saveToNVS = saveItem;

    if (mItem.id > 0)
        mItem.isSelected = false;
    else
        mItem.isSelected = true;

    size = strlen(menusData[parentId].ptrStrings[mItem.id].c_str()) + 1;
    mItem.name = new char[size];
    strncpy(mItem.name, menusData[parentId].ptrStrings[mItem.id].c_str(), size);

    menusData[parentId].itemIdArray[mItem.id] = itemsData.size();

    itemsData.push_back(mItem);
}

void MenusClass::createItem(int parentId, const char *strVal, void (*valuePtr)(Item *), bool saveItem, bool numVal, String (*customDispFunc)(int))
{
    Item mItem;
    int size;

    mItem.id = menusData[parentId].itemsAdded++;
    mItem.parentId = parentId;
    mItem.valueType = Item::STRING;
    mItem.symbol = NULL;
    mItem.changed = false;
    mItem.minNumValue = NULL;
    mItem.maxNumValue = NULL;
    mItem.cycleValues = NULL;
    mItem.stepValue = NULL;
    mItem.valuePtr = valuePtr;
    mItem.customDispValPtr = customDispFunc;

    if (mItem.id > 0)
        mItem.isSelected = false;
    else
        mItem.isSelected = true;

    if(!numVal)
    {
        mItem.symbol = NULL;
        size = strlen(strVal) + 1;
        mItem.strValue = new char[size];
        strncpy(mItem.strValue, strVal, size);
        Serial.println(mItem.strValue);
    }
    else
    {
        size = strlen(strVal) + 1;
        mItem.symbol = new char[size];
        strncpy(mItem.symbol, strVal, size);
    }
    

    size = strlen(menusData[parentId].ptrStrings[mItem.id].c_str()) + 1;
    mItem.name = new char[size];
    strncpy(mItem.name, menusData[parentId].ptrStrings[mItem.id].c_str(), size);

    menusData[parentId].itemIdArray[mItem.id] = itemsData.size();

    itemsData.push_back(mItem);
}

void MenusClass::createItem(int parentId, float fltVal, const char *symbolStr, void (*valuePtr)(Item *), bool saveItem)
{
    Item mItem;
    int size;

    mItem.id = menusData[parentId].itemsAdded++;
    mItem.parentId = parentId;
    mItem.valueType = Item::FLOAT;
    mItem.changed = false;
    mItem.minNumValue = NULL;
    mItem.maxNumValue = NULL;
    mItem.cycleValues = NULL;
    mItem.stepValue = NULL;
    mItem.valuePtr = valuePtr;
    mItem.customDispValPtr = NULL;

    if (mItem.id > 0)
        mItem.isSelected = false;
    else
        mItem.isSelected = true;

    size = strlen(symbolStr) + 1;
    mItem.symbol = new char[size];
    strncpy(mItem.symbol, symbolStr, size);

    size = strlen(menusData[parentId].ptrStrings[mItem.id].c_str()) + 1;
    mItem.name = new char[size];
    strncpy(mItem.name, menusData[parentId].ptrStrings[mItem.id].c_str(), size);

    menusData[parentId].itemIdArray[mItem.id] = itemsData.size();

    itemsData.push_back(mItem);
}

void MenusClass::updateItem(int menuId, int itemIndex, int value)
{
    itemsData[menusData[menuId].itemIdArray[itemIndex]].numValue = value;

    if (currentMenuId == menuId && !menusData[menuId].isItemSelected)
    {
        menusData[menuId].changed = true;
        itemsData[menusData[menuId].itemIdArray[itemIndex]].changed = false;
    }
    else if (currentMenuId == menuId && menusData[menuId].isItemSelected && menusData[menuId].prevSelectedItem == itemIndex)
    {
        itemsData[menusData[menuId].itemIdArray[itemIndex]].changed = true;
        menusData[menuId].changed = false;
    }
}

void MenusClass::updateItem(int menuId, int itemIndex, float value)
{
    itemsData[menusData[menuId].itemIdArray[itemIndex]].fltValue = value;

    if (currentMenuId == menuId && !menusData[menuId].isItemSelected)
    {
        menusData[menuId].changed = true;
        itemsData[menusData[menuId].itemIdArray[itemIndex]].changed = false;
    }
    else if (currentMenuId == menuId && menusData[menuId].isItemSelected && menusData[menuId].prevSelectedItem == itemIndex)
    {
        itemsData[menusData[menuId].itemIdArray[itemIndex]].changed = true;
        menusData[menuId].changed = false;
    }
}

void MenusClass::updateItem(int menuId, int itemIndex, const char *value, int numVal)
{
    int size;
    size = strlen(value) + 1;

    if(numVal == NULL)
    {
        itemsData[menusData[menuId].itemIdArray[itemIndex]].strValue = new char[size];
        strncpy(itemsData[menusData[menuId].itemIdArray[itemIndex]].strValue, value, size);
    }
    else
    {
        itemsData[menusData[menuId].itemIdArray[itemIndex]].symbol = new char[size];
        strncpy(itemsData[menusData[menuId].itemIdArray[itemIndex]].symbol, value, size);
        itemsData[menusData[menuId].itemIdArray[itemIndex]].numValue = numVal;
    }
    

    if (currentMenuId == menuId)
        menusData[currentMenuId].changed = true;
}

Item *MenusClass::getItem(int menuId, int itemIndex)
{
    return &itemsData[menusData[menuId].itemIdArray[itemIndex]];
}

void MenusClass::setCurrentMenu(int menuId)
{
    menusData[menuId].changed = true;
    currentMenuId = menuId;
}

int MenusClass::getCurrentMenu()
{
    return currentMenuId;
}

int MenusClass::getItemsCount(int menuId)
{
    return menusData[menuId].itemsAdded;
}

int MenusClass::getFirstItemToDisplay()
{
    if (menusData[currentMenuId].itemsAdded > maxItemsToDisplay) // if there are more items in the menu than we can display
    {
        if (menusData[currentMenuId].idFirstDisplayedItem > menusData[currentMenuId].itemsAdded - maxItemsToDisplay)
        {
            menusData[currentMenuId].idFirstDisplayedItem = menusData[currentMenuId].itemsAdded - maxItemsToDisplay;
        }

        if (menusData[currentMenuId].prevSelectedItem < menusData[currentMenuId].idFirstDisplayedItem ||
            menusData[currentMenuId].prevSelectedItem >= (menusData[currentMenuId].idFirstDisplayedItem + maxItemsToDisplay))
        {
            if (menusData[currentMenuId].prevSelectedItem < menusData[currentMenuId].idFirstDisplayedItem)
            {
                menusData[currentMenuId].idFirstDisplayedItem = menusData[currentMenuId].prevSelectedItem;
            }
            else
            {
                menusData[currentMenuId].idFirstDisplayedItem += menusData[currentMenuId].prevSelectedItem - (menusData[currentMenuId].idFirstDisplayedItem + maxItemsToDisplay) + 1;
            }
        }
    }
    return menusData[currentMenuId].idFirstDisplayedItem;
}

void MenusClass::setSelected(int value)
{
    int itemId = menusData[currentMenuId].itemIdArray[menusData[currentMenuId].prevSelectedItem];

    if (menusData[currentMenuId].isItemSelected)
    {
        if (itemsData[itemId].isSelected)
        {
            switch (itemsData[itemId].valueType)
            {
            case Item::INTEGER:
                itemsData[itemId].numValue = value * itemsData[itemId].stepValue;
                itemsData[itemId].changed = true;
                break;
            case Item::BOOL:
                itemsData[itemId].numValue = value;
                itemsData[itemId].changed = true;
                break;
            default:
                break;
            }
        }
    }
    else
    {
        if (itemsData[itemId].isSelected)
            itemsData[itemId].isSelected = false;
        if (itemsData[menusData[currentMenuId].itemIdArray[value]].id == value)
            itemsData[menusData[currentMenuId].itemIdArray[value]].isSelected = true;

        menusData[currentMenuId].changed = true;
        menusData[currentMenuId].prevSelectedItem = value;
    }
}

String MenusClass::getItemString(int itemIndex)
{
    String str = "";
    int id = menusData[currentMenuId].itemIdArray[itemIndex];

    switch (itemsData[id].valueType)
    {
    case Item::INTEGER:
        if(itemsData[id].customDispValPtr != NULL)
            str = String(itemsData[id].name) + itemsData[id].customDispValPtr(itemsData[id].numValue);
        else
            str = String(itemsData[id].name) + String(itemsData[id].numValue) + String(itemsData[id].symbol);
        break;

    case Item::REDIRECT:
        str = String(itemsData[id].name);
        break;

    case Item::FLOAT:
        str = String(itemsData[id].name) + String(round(itemsData[id].fltValue * 10) / 10).substring(0, 4) + String(itemsData[id].symbol);
        break;

    case Item::STRING:

        if(itemsData[id].customDispValPtr != NULL)
            str = itemsData[id].customDispValPtr(itemsData[id].numValue);
        else
            str = itemsData[id].strValue;

        if (str.length() > 7)
            str = String(itemsData[id].name) + str.substring(0, 4) + "..."; // round values to one digit after point
        else
            str = String(itemsData[id].name) + str;
        break;

    case Item::BOOL:
        str = String(itemsData[id].name) + (itemsData[id].numValue ? "ON" : "OFF");
        break;

    default:
        break;
    }
    return str;
}

bool MenusClass::checkItemSelected(int itemIndex)
{
    return itemsData[menusData[currentMenuId].itemIdArray[itemIndex]].isSelected;
}

void MenusClass::prepareMenu()
{
    int itemId = menusData[currentMenuId].itemIdArray[menusData[currentMenuId].prevSelectedItem];

    if (menusData[currentMenuId].isItemSelected)
    {
        if(itemsData[itemId].saveToNVS) saveItemFuncPtr(&itemsData[itemId]);
        menusData[currentMenuId].isItemSelected = false;
        menusData[currentMenuId].changed = true;
    }
    else
    {
        if (itemsData[menusData[currentMenuId].itemIdArray[menusData[currentMenuId].prevSelectedItem]].valueType != Item::REDIRECT &&
            menusData[currentMenuId].customMenuFuncPtr == NULL)
        {
            menusData[currentMenuId].isItemSelected = true;
        }
        itemsData[menusData[currentMenuId].itemIdArray[menusData[currentMenuId].prevSelectedItem]].changed = true;
    }
}

void MenusClass::update()
{
    if (menusData[currentMenuId].changed)
    {
        menusData[currentMenuId].changed = false;
        if (menusData[currentMenuId].customMenuFuncPtr == NULL)
            menuFuncPtr(&menusData[currentMenuId]);
        else
            menusData[currentMenuId].customMenuFuncPtr(&menusData[currentMenuId]);
    }
    else
    {
        int itemId = menusData[currentMenuId].itemIdArray[menusData[currentMenuId].prevSelectedItem];

        if (itemsData[itemId].changed)
        {
            itemsData[itemId].changed = false;

            if (itemsData[itemId].valueType == Item::REDIRECT)
            {
                if (itemsData[itemId].id == (menusData[currentMenuId].itemsAdded - 1))
                    setSelected(0);

                menusData[itemsData[itemId].numValue].changed = true;
                currentMenuId = itemsData[itemId].numValue;

                if (menusData[currentMenuId].customMenuFuncPtr == NULL)
                    menuFuncPtr(&menusData[itemsData[itemId].numValue]);
                else
                    menusData[currentMenuId].customMenuFuncPtr(&menusData[currentMenuId]);
            }
            else
            {
                menusData[currentMenuId].changed = false;
                if (itemsData[itemId].valuePtr != NULL)
                    itemsData[itemId].valuePtr(&itemsData[itemId]);
            }
        }
    }
}

MenusClass Menus;
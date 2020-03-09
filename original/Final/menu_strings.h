// ---------------------------> Helper menu items <-------------------------------------//

enum menus    
{
    main,
    settings,
    buzzer,
    led,
    sensors,
    automation,
    ble,
    noreturn
};

struct MenuNames
{
    int id;
    const char *name;
    const String *itemStrings;
};

struct SaveItemNames
{
    int id;
    //int parentId;
    const char *saveName;
    int defaultValue;
};

// ---------------------------> Title menu items <-------------------------------------//

const String titleMenuItems[] = {"Main", "Settings", "Buzzer", "LED", "Sensors", "Automation", "Bluetooth"};

// ---------------------------> Main menu items <-------------------------------------//

const String mainMenuItems[] = {"Hello World!", "Press button"};

// ---------------------------> Settings menu <---------------------------------------//

const String settingsMenuItems[] = {"Buzzer", "RGB LED", "Sensors", "Automation", "Bluetooth", "Exit."};

// ---------------------------> Buzzer menu <-----------------------------------------//

const String buzzerMenuItems[] = {"Volume:", "Switch:", "Return"};

// ---------------------------> RGB Led menu <----------------------------------------//

const String ledMenuItems[] = {"R:", "G:", "B:", "Switch LEDS:", "Return"};

// ---------------------------> Sensors menu <------------------------------------------//

const String sensorsMenuItems[] = {"Temperature:", "Humidity:", "Heat Index:", "Dew Point:", "CO2:", "Soil Moisture:", "Light:", "Return"};

// ---------------------------> Pump and fans menu <------------------------------------------//

const String automationMenuItems[] = {"Watering Freq:", "Watering Time:", "Watering Dur:", "Ventilation Freq:", "Ventilation Time:", "Ventilation Dur:", "Opt Temp:", "Opt Hum:", "Opt SM:", "Thingspeak:", "Web sync:", "Set time:", "Return"};

// ---------------------------> Bluetooth menu <------------------------------------------//

const String bleMenuItems[] = {"Name:", "MAC:", "Return"};



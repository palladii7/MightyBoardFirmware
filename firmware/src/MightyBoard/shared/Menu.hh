#ifndef MENU_HH_
#define MENU_HH_

#include "Types.hh"
#include "ButtonArray.hh"
#include "LiquidCrystalSerial.hh"
#include "Configuration.hh"
#include "CircularBuffer.hh"
#include "Timeout.hh"
#include "Host.hh"

/// states for Welcome Menu
enum WeclomeStates{
    WELCOME_START,
    WELCOME_BUTTONS1,
    WELCOME_BUTTONS2,
    WELCOME_BUTTONS3,
    WELCOME_BUTTONS4,
    WELCOME_BUTTONS5,
    WELCOME_EXPLAIN,
    WELCOME_TOOL_SELECT,
    WELCOME_LEVEL,
    WELCOME_LEVEL_ACTION,
    WELCOME_LEVEL_OK,
    WELCOME_LOAD_PLASTIC,
    WELCOME_LOAD_ACTION,
    WELCOME_READY,
    WELCOME_LOAD_SD,
    WELCOME_PRINT_FROM_SD,
    WELCOME_DONE
};


/// The screen class defines a standard interface for anything that should
/// be displayed on the LCD.
class Screen {
public:
        /// Get the rate that this display should be updated. This is called
        /// after every screen display, so it can be used to dynamically
        /// adjust the update rate. This can be as fast as you like, however
        /// refreshing too fast during a build is certain to interfere with
        /// the serial and stepper processes, which will decrease build quality.
        /// \return refresh interval, in microseconds.
	virtual micros_t getUpdateRate();

        /// Update the screen display,
        /// \param[in] lcd LCD to write to
        /// \param[in] forceRedraw if true, redraw the entire screen. If false,
        ///                        only updated sections need to be redrawn.
	virtual void update(LiquidCrystalSerial& lcd, bool forceRedraw);

        /// Reset the screen to it's default state
	virtual void reset();

        /// Get a notification that a button was pressed down.
        /// This function is called for every button that is pressed. Screen
        /// logic can be updated, however the screen should not be redrawn
        /// until update() is called again.
        ///
        /// Note that the current implementation only supports one button
        /// press at a time, and will discard any other events.
        /// \param button Button that was pressed
        virtual void notifyButtonPressed(ButtonArray::ButtonName button);
        
        /// return true if this screen uses continuous button mode
        virtual bool continuousButtons(void){ return false;}
        
        /// set build percentage to be displayed in monitor mode
        virtual void setBuildPercentage(uint8_t percent){return;}
        
        /// poll if the screen is waiting on a timer
        virtual bool screenWaiting(void){ return false;}
};


/// The menu object can be used to display a list of options on the LCD
/// screen. It handles updating the display and responding to button presses
/// automatically.
class Menu: public Screen {
public:
	micros_t getUpdateRate() {return 500L * 1000L;}


	void update(LiquidCrystalSerial& lcd, bool forceRedraw);

	void reset();

	virtual void resetState();

    void notifyButtonPressed(ButtonArray::ButtonName button);

protected:

		bool lineUpdate;				///< flags the menu to update the current line
        uint8_t itemIndex;              ///< The currently selected item
        uint8_t lastDrawIndex;          ///< The index used to make the last draw
        uint8_t itemCount;              ///< Total number of items
        uint8_t firstItemIndex;         ///< The first selectable item. Set this
                                        ///< to greater than 0 if the first
                                        ///< item(s) are a title)

        /// Draw an item at the current cursor position.
        /// \param[in] index Index of the item to draw
        /// \param[in] LCD screen to draw onto
	virtual void drawItem(uint8_t index, LiquidCrystalSerial& lcd);

        /// Handle selection of a menu item
        /// \param[in] index Index of the menu item that was selected
	virtual void handleSelect(uint8_t index);

        /// Handle the menu being cancelled. This should either remove the
        /// menu from the stack, or pop up a confirmation dialog to make sure
        /// that the menu should be removed.
	virtual void handleCancel();
};

/// The Counter menu builds on menu to allow selecting number values .
class CounterMenu: public Menu {
public:
	micros_t getUpdateRate() {return 500L * 1000L;}
    
    
    void notifyButtonPressed(ButtonArray::ButtonName button);
    
protected:
    bool selectMode;        ///< true if in counter change state
    int selectIndex;    ///< The currently selected item, in a counter change state
    
    void reset();

    virtual void handleCounterUpdate(uint8_t index, bool up);
};

/// Display a welcome splash screen, that removes itself when updated.
class SplashScreen: public Screen {
public:
	micros_t getUpdateRate() {return 50L * 1000L;}


	void update(LiquidCrystalSerial& lcd, bool forceRedraw);

	void reset();

    void notifyButtonPressed(ButtonArray::ButtonName button);
};

class TestMenu: public Menu {
public:
	TestMenu();
    
    bool userResponse(void){ return responseEntered;}
    bool testOK(void) {return responseOK;}
    
	void resetState();
	
	void newTest(char * msg);
	
	uint8_t cursor;
    
protected:

	const static int TEST_SCREEN_MSG_SIZE = LCD_SCREEN_WIDTH*2 + 1;
	char message[TEST_SCREEN_MSG_SIZE];
	
	void drawItem(uint8_t index, LiquidCrystalSerial& lcd);
    
	void handleSelect(uint8_t index);
	
	bool responseOK;
	bool responseEntered;
};

class ToolSelectMenu: public Menu {
public:
	ToolSelectMenu();
    
	void resetState();
    
protected:
	void drawItem(uint8_t index, LiquidCrystalSerial& lcd);
    
	void handleSelect(uint8_t index);
};

class ReadyMenu: public Menu {
public:
	ReadyMenu();
    
	void resetState();
    
protected:
	void drawItem(uint8_t index, LiquidCrystalSerial& lcd);
    
	void handleSelect(uint8_t index);
};

class LevelOKMenu: public Menu {
public:
	LevelOKMenu();
    
	void resetState();
    
protected:
	void drawItem(uint8_t index, LiquidCrystalSerial& lcd);
    
	void handleSelect(uint8_t index);
};

/// test if heaters are plugged in correctly
class HeaterTestScreen: public Screen {
public:
	micros_t getUpdateRate() {return 50L * 1000L;}
	
	void update(LiquidCrystalSerial& lcd, bool forceRedraw);

	void reset();

    void notifyButtonPressed(ButtonArray::ButtonName button);
private:
    Timeout heater_timeout;
	bool heater_failed;
};



/// Display a message for the user, and provide support for
/// user-specified pauses.
class MessageScreen: public Screen {
private:
	uint8_t x, y;
	const static int BUF_SIZE = LCD_SCREEN_WIDTH*LCD_SCREEN_HEIGHT + 1;
	char message[BUF_SIZE];
	uint8_t cursor;
	bool needsRedraw;
	bool incomplete;
	bool lcdClear;
	bool popScreenOn;
	Timeout timeout;
public:
	MessageScreen() : needsRedraw(false) { message[0] = '\0'; }

	void setXY(uint8_t xpos, uint8_t ypos) { x = xpos; y = ypos; }

	void addMessage(CircularBuffer& buf, bool msgComplete);
	void addMessage(char * msg, bool msgComplete);
	void clearMessage();
	void setTimeout(uint8_t seconds, bool pop);

	micros_t getUpdateRate() {return 50L * 1000L;}
  
	void update(LiquidCrystalSerial& lcd, bool forceRedraw);
	
	void reset();

    void notifyButtonPressed(ButtonArray::ButtonName button);
    
    bool screenWaiting(void);
};


class JogMode: public Screen {
private:
	enum distance_t {
	  DISTANCE_SHORT,
	  DISTANCE_LONG,
	};
	enum jogmode_t {
		JOG_MODE_X,
		JOG_MODE_Y,
		JOG_MODE_Z
	};

	distance_t jogDistance;
	bool distanceChanged, modeChanged;
	jogmode_t  JogModeScreen;

    void jog(ButtonArray::ButtonName direction);

public:
	micros_t getUpdateRate() {return 50L * 1000L;}

	void update(LiquidCrystalSerial& lcd, bool forceRedraw);

	void reset();

    void notifyButtonPressed(ButtonArray::ButtonName button);
     
    bool continuousButtons(void) {return true;}
};

/// This is an easter egg.
class SnakeMode: public Screen {

#define MAX_SNAKE_SIZE 20      ///< Maximum length our snake can grow to
#define APPLES_BEFORE_GROW 4   ///< Number of apples the snake must eat before growing
#define START_SPEED  60        ///< Starting speed, in screen refresh times per turn


private:
	micros_t updateRate;

	struct coord_t {
		int8_t x;
		int8_t y;
	};

	enum direction_t {
	  DIR_NORTH,
	  DIR_EAST,
	  DIR_SOUTH,
	  DIR_WEST
	};

	int snakeLength;					// Length of our snake; this grows for every x 'apples' eaten
	coord_t snakeBody[MAX_SNAKE_SIZE];	// Table of each piece of the snakes body
	bool snakeAlive;					// The state of our snake
	direction_t snakeDirection;			// The direction the snake is heading
	coord_t applePosition;				// Location of the apple
	uint8_t applesEaten;				// Number of apples that have been eaten
//	int gameSpeed = START_SPEED;		// Speed of the game (in ms per turn)


public:
	micros_t getUpdateRate() {return updateRate;}


	// Refresh the display information
	void update(LiquidCrystalSerial& lcd, bool forceRedraw);

	void reset();

	// Get notified that a button was pressed
        void notifyButtonPressed(ButtonArray::ButtonName button);
};

class SDSpecialBuild: public Screen{
	
	protected:
		char buildType[host::MAX_FILE_LEN];
		bool buildFailed;
		

public:
	SDSpecialBuild();
	micros_t getUpdateRate() {return 50L * 1000L;}


	void update(LiquidCrystalSerial& lcd, bool forceRedraw);

	void reset();
	virtual void resetState();
	
	bool startBuild(void);
	void notifyButtonPressed(ButtonArray::ButtonName button);

};

class SDMenu: public Menu {
public:
	SDMenu();

	void resetState();

protected:
	bool cardNotFound;
	
	uint8_t countFiles();

    bool getFilename(uint8_t index,
                         char buffer[],
                         uint8_t buffer_size);

	void drawItem(uint8_t index, LiquidCrystalSerial& lcd);

	void handleSelect(uint8_t index);
};

/// Display a welcome splash screen on first user boot
class WelcomeScreen: public Screen {
    
private:
    int8_t welcomeState;
    
    SDMenu sdmenu;
    ToolSelectMenu tool_select;
    ReadyMenu ready;
    LevelOKMenu levelOK;
    
    bool needsRedraw;
public:
	micros_t getUpdateRate() {return 50L * 1000L;}
    
    
	void update(LiquidCrystalSerial& lcd, bool forceRedraw);
    
	void reset();
    
    void notifyButtonPressed(ButtonArray::ButtonName button);
    bool screenWaiting(void);
};

class PreheatSettingsMenu: public CounterMenu {
public:
	PreheatSettingsMenu();
    
protected:
    uint16_t counterRight;
    uint16_t counterLeft;
    uint16_t counterPlatform;
    
    void resetState();
    
	void drawItem(uint8_t index, LiquidCrystalSerial& lcd);
    
	void handleSelect(uint8_t index);
    
    void handleCounterUpdate(uint8_t index, bool up);
};

class ResetSettingsMenu: public Menu {
public:
	ResetSettingsMenu();
    
	void resetState();
    
protected:
	void drawItem(uint8_t index, LiquidCrystalSerial& lcd);
    
	void handleSelect(uint8_t index);
};


class CancelBuildMenu: public Menu {
public:
	CancelBuildMenu();

	void resetState();

protected:
	void drawItem(uint8_t index, LiquidCrystalSerial& lcd);

	void handleSelect(uint8_t index);
};


class MonitorMode: public Screen {
private:
	CancelBuildMenu cancelBuildMenu;

	uint8_t updatePhase;
	uint8_t buildPercentage;
	bool singleTool;

public:
	micros_t getUpdateRate() {return 500L * 1000L;}


	void update(LiquidCrystalSerial& lcd, bool forceRedraw);

	void reset();

    void notifyButtonPressed(ButtonArray::ButtonName button);
    
    void setBuildPercentage(uint8_t percent);
};

class HeaterPreheat: public Menu {
	
public:
	
	HeaterPreheat();
	
	void drawItem(uint8_t index, LiquidCrystalSerial& lcd);

	void handleSelect(uint8_t index);

private:
	MonitorMode monitorMode;
	bool _rightActive, _leftActive, _platformActive;
    
    void storeHeatByte();
    void resetState();
     
    bool singleTool;
	
};

class SettingsMenu: public CounterMenu {
public:
	SettingsMenu();
    
    
protected:
    void resetState();
    
	void drawItem(uint8_t index, LiquidCrystalSerial& lcd);
    
	void handleSelect(uint8_t index);
	
	void handleCounterUpdate(uint8_t index, bool up);
    
private:
    /// Static instances of our menus
    
    int8_t singleExtruder;
    int8_t soundOn;
    int8_t LEDColor;
    
};

class UtilitiesMenu: public Menu {
public:
	UtilitiesMenu();


protected:
	void drawItem(uint8_t index, LiquidCrystalSerial& lcd);

	void handleSelect(uint8_t index);
	
	void resetState();

private:
    /// Static instances of our menus
    MonitorMode monitorMode;
    JogMode jogger;
    WelcomeScreen welcome;
    HeaterTestScreen heater;
    SettingsMenu set;
    PreheatSettingsMenu preheat;
    ResetSettingsMenu reset_settings;
    
    bool stepperEnable;
    bool blinkLED;
    bool singleTool;
};

/// Diagnostics menu allows you to select
class DiagnosticsMenu: public Menu{  
public:
    DiagnosticsMenu();
    
    
protected:
    void drawItem(uint8_t index, LiquidCrystalSerial& lcd);
    
    void handleSelect(uint8_t index);
    
    void resetState();
    
};



class MainMenu: public Menu {
public:
	MainMenu();


protected:
	void drawItem(uint8_t index, LiquidCrystalSerial& lcd);

	void handleSelect(uint8_t index);
	
	void resetState();

private:
        /// Static instances of our menus
        SDMenu sdMenu;
        HeaterPreheat preheat;
        UtilitiesMenu utils;

};



#endif

/*
	Name:       GarageMonitoringSystem.ino
    
	Created:	11/14/2023 7:10:33 PM
	
	Author:     Garrett Miller

	Notes:		This code heavily uses Editor Outlining as used in Visual Studio, with
				frequent use of "#pragma region (Region Label/Name)\n ... \n ... \n #pragma endregion"
				to allow collapse of regions of code and comments into logical groupings.
				For those unfamiliar, Ctrl+M, Ctrl+L toggles all outlining open/closed
				while Ctrl+M, Ctrl+O collapses all outlining to closed. Handy for seeing related
				groups of code and temporarily "hiding" other code. This is ONE reason why I use
				Visual Studio instead of Arduino's editor.

	Design:		Monitor garage temperature (and humidity if needed) and give an alert when below the
				threshold temperature. Monitor garage door open status (by distance) and alert when
				open for than <some> amount of time. Monitor various MQ sensors and alert when PPM's
				become too high.
*/

#pragma region INCLUDES

///////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
#include "BlunoShield.h"
#include "OLEDMenu.h"
#include "PlainProtocolVariable.h"

//Local devices
#include "PulseSwitchDevice.h"
#include "UltrasonicSensor.h"
#include "MQ9Sensor.h"
#include "HeatingDevice.h"

#pragma endregion

//Bluno 1280 Mac 1: 0xC4BE8424C253

#pragma region Global Variables

#pragma region Bare Variables

///////////////////////////////////////////////////////////////////////////////////////////////////
//The actual garage door distance SHOULD be between the min and max below

//The MINIMUM distance that the garage door sensor can read and still chalk it up to the garage door
PPV_Float garageDoorDistanceMin = PPV_Float("Garage Door Distance Minimum");
//The MAXIMUM distance that the garage door sensor can read and still chalk it up to the garage door
PPV_Float garageDoorDistanceMax = PPV_Float("Garage Door Distance Maximum");

//How long the garage door can be open before triggering an alert
PPV_Float maxGarageDoorOpenTime = PPV_Float("Max Garage Door Open Time", (float) 10 TMINUTES);
ArduinoTimer garageDoorOpenTimer(maxGarageDoorOpenTime);

//The distance to the front of the car (from the car distance sensor)
//float carDistanceFront;

//The temperature that the garage needs to be below to send a "too cold" alert
PPV_Float tooColdAlarmTemp = PPV_Float("Garage Too Cold Alarm Temp", (float) 35);
PPV_Float garageWarmEnoughTemp = PPV_Float("Garage Warm Enough Temp", (float) 40);

#pragma endregion

#pragma region Devices

//MQ9Sensor mq9Sensor(A8);

//The device that controls the opening/closing of the garage door "remotely"
//(not the normal remote, but the Wifi-remote)
//PulseSwitchDevice garageDoorSwitch(40, "Garage Door");

//HeatingDevice spaceHeater(26, );
SwitchDevice spaceHeater(26, "Heater", SwitchDevice::eSDM_EventOnEventOff);

#pragma region Ultrasonic Sensors

UltrasonicSensor garageDoorSensor(22, 23);
//UltrasonicSensor carDistanceFront(24, 25);

#pragma endregion

#pragma endregion

#pragma region Trigger Variables
//These variables are 1-to-1 with any Arduino IOT Cloud triggers

bool garageTooCold;
bool garageOpenTooLong;

#pragma endregion

#pragma endregion

#pragma region Menu System

#pragma region Device Page

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// The menu options for both of the fans and the fogger
//MenuOption devicesPageOptions[] =
//{
//	MenuOption("Fogger", foggerRelayState),
//	MenuOption("Heat Lamp", heatLampRelayState),
//	MenuOption("UV Light", uvLightRelayState),
//};
//
//OLEDPage devicesPage("Local Devices", MenuOptionCount(devicesPageOptions), devicesPageOptions);

#pragma endregion

#pragma endregion

#pragma region Callbacks

///////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// The callback function for the Bluno shield OLED screen to draw what you want (the menu system).
/// This is necessary because drawing to the OLED can only be done at a very specific point in the
/// code, namely the oled.firstPage()/oled.nextPage() loop found in BlunoShield::UpdateOLED(), that
/// is called only ONCE PER LOOP()
/// </summary>
/// <param name="oled">A reference to the Bluno shield OLED screen</param>
void drawCallback(OLED& oled)
{
	//Just in case the menu system's pointer to the OLED is NULL, set it
	if (!menu.pOled)
		menu.pOled = &oled;

	//menu.Draw();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// The callback function for the Bluno Shield PlainProtocol (technically not the shield itself but
/// an actual DFRobot Bluno-board)
/// </summary>
/// <param name="input">The PlainProtocol object (reference)</param>
void processInputCallback(PlainProtocol& input)
{
	//if (input.available())
	{
		//Serial.println("HERE");
		//do your own BLE processing if needed
		//fanIntake.ProcessInput(input);
		//fanExhaust.ProcessInput(input);
	}
}

#pragma endregion

#pragma region Main Sub Routines

/// <summary>
/// Initializes all local devices
/// </summary>
void InitializeDevices()
{
	//Initialize ultrasonic sensors
	garageDoorSensor.Initialize();
	//carDistanceFront.Initialize();

	//Initialize MQ sensors
	//mq9Sensor.Initialize();

	//////////////////////////////////////
	//Initialize other sensors/devices
	//garageDoorSwitch.Initialize();
}

/// <summary>
/// Initializes the BlunoShield accessory completely
/// </summary>
void InitializeBlunoShield()
{
	//Initialize Bluno shield
	blunoShield.Initialize();

	//Set the Bluno shield to do internal (default) PlainProtocol processing,
	//but also our external, callback function too
	//(processInputCallback(PlainProtocol& input))
	blunoShield.SetInputProcessingMode(BlunoShield::eIPM_Both);

	//Set Bluno shield callbacks for drawing and input
	blunoShield.SetDrawCallback(drawCallback);
	blunoShield.SetInputCallback(processInputCallback);
}

/// <summary>
/// Initializes serial 0
/// </summary>
void BeginSerial0(unsigned long baud = 115200)
{
	static ArduinoTimer errorTimer(1 TSECONDS, true);
	
	//Begin serial
	Serial.begin(baud);
	
	while (!Serial && errorTimer.IsReady())
	{
		Serial.println("Can't initialize Serial 0");
	};//Wait for Serial to initialize
}

/// <summary>
/// Initializes serial 1
/// </summary>
void BeginSerial1(unsigned long baud = 9600)
{
	static ArduinoTimer errorTimer(1 TSECONDS, true);
	
	Serial1.begin(baud);

	while (!Serial1 && errorTimer.IsReady())
	{
		Serial.println("Can't initialize Serial 0");
	};//Wait for Serial1 to initialize
}

/// <summary>
/// Initializes serial 2
/// </summary>
void BeginSerial2(unsigned long baud = 9600)
{
	static ArduinoTimer errorTimer(1 TSECONDS, true);

	Serial2.begin(baud);

	while (!Serial2 && errorTimer.IsReady())
	{
		Serial.println("Can't initialize Serial 0");
	};//Wait for Serial1 to initialize
}

/// <summary>
/// Initializes serial 3
/// </summary>
void BeginSerial3(unsigned long baud = 9600)
{
	static ArduinoTimer errorTimer(1 TSECONDS, true);

	Serial3.begin(baud);

	while (!Serial3 && errorTimer.IsReady())
	{
		Serial.println("Can't initialize Serial 0");
	};//Wait for Serial1 to initialize
}

/// <summary>
/// Initializes all (necessary) Serial connections
/// </summary>
void InitializeAllSerial()
{
	//Initialize the main serial (USB<-->Arduino)
	BeginSerial0();

	//BeginSerial1();

	//BeginSerial2();

	//Initialize the 3rd serial (Arduino<-->ESP8266)
	BeginSerial3();

	Serial.println("All Serial initialized!");
}

/// <summary>
/// Set the heater (and other related things) on or off, based on temperature
/// </summary>
void ProcessTemperature()
{
	if (blunoShield.temperature <= tooColdAlarmTemp)
	{
		//Set the Arduino IOT Cloud trigger variable
		garageTooCold = true;

		spaceHeater.TurnOn();

		SendAlert("Garage temperature too cold! Current temperature: " + blunoShield.temperature);
	}
	else if (blunoShield.temperature >= garageWarmEnoughTemp)
	{
		garageTooCold = false;

		spaceHeater.TurnOn(false);

		SendAlert("Garage temperature warm enough! Current temperature: " + blunoShield.temperature);
	}
}

/// <summary>
/// Set the fogger (and other related things) on or off, based on humidity
/// </summary>
void ProcessHumidity()
{
}

/// <summary>
/// Do stuff based on whether or not the garage door is open
/// </summary>
void ProcessGarageDoorSensor()
{
	float garageDoorDistance = garageDoorSensor.GetDistance();

	//If the distance to the garage door is more than
	//the pre-set value maximum (meaning "open")
	if (garageDoorDistance > garageDoorDistanceMax)
	{
		//If the timer isn't already running, start it
		if (!garageDoorOpenTimer.enabled)
			garageDoorOpenTimer.Reset();
		else if (garageDoorOpenTimer.IsReady()) //otherwise, check if the door has been open too long
		{
			//Set the AIOTC trigger variable
			garageOpenTooLong = true;

			//Send garage door open alarm (whatever that means right now)
			SendAlert("Garage door seems to be open too long! Time open: " + garageDoorOpenTimer.GetTimeRunning());
			
			//DO NOT reset timer so we know how just long the door has been open for
			//garageDoorOpenTimer.Reset();
		}
	}
	else if (garageDoorDistance < garageDoorDistanceMin)
	{
		//Do we do anything if the sensor picks up distance less than the garage door?
		//What could that mean?? Obviously something between the door and the sensor that
		//is unexpected
		Serial.println("\n\nSomething between garage door and sensor!\n\n");
		
		//Probably reset timer??
		garageDoorOpenTimer.Reset(false);
	}
	else //otherwise, the door the door is closed, stop running the timer
	{
		Serial.println("Resetting door open timer...");
		garageDoorOpenTimer.Stop();
	}
}

/// <summary>
/// Set the Bluno shield's default message text, based on the status of both the fogger and heater
/// </summary>
void SetBlunoMessageText()
{

}

/// <summary>
/// Update the menu system
/// </summary>
void UpdateMenuSystem()
{
	//if the Bluno Accessory Shield screen draw mode is set to custom, indicating
	//WE (end-user, here, in the main Arduino sketch) tell it what to draw,
	//update the menu system (which essentially handles input and drawing the appropriate page).
	if (blunoShield.GetDrawMode() == BlunoShield::eDM_Custom)
		menu.Update();

	//otherwise, if we are in the default, initial (or "root") screen of the menu
	//system (defaulted to display temperature and humidity),
	//AND then the joystick is pushed, set the Bluno Shield draw mode to custom,
	//so WE can draw what we want, (which will be the actual menu system and pages)
	else if (blunoShield.GetJoystickValue() == eJoy_Push && blunoShield.GetDrawMode() == BlunoShield::eDM_TempAndHumidity)
		blunoShield.SetDrawMode(BlunoShield::eDM_Custom);
}

/// <summary>
/// Send data over the serial port (every second, by default) to any device that may be listening
/// </summary>
void SendData()
{
	//Create a local, static (persistent) timer, set to 1000 ms (or 1 second)
	static ArduinoTimer serialTimer(1 TSECONDS, true);
	//if (serialTimer.IsReady())	//The timer auto-resets by default if IsReady() is true
	{
		/*String message = garageDoorDistanceMin.GetFrame();
		message += garageDoorDistanceMax.GetFrame();*/
		
		String message = PlainProtocolVariableBase::GetAllFrames();
		Serial.print("Message: ");
		Serial.println(message);
		//Serial3.println(message);
		blunoShield.GetProtocol().write("blah");
		//blunoShield.GetProtocol().write();
	}
}

/// <summary>
/// Update the lights, the fogger, and the Bluno shield itself
/// </summary>
void UpdateObjects()
{
	//Update the bluno shield itself
	blunoShield.Update();

	ProcessLocalDevices();
}

/// <summary>
/// Sets the menu system up
/// </summary>
void SetupMenuSystem()
{
	//Init menu system with reference to the Bluno shield
	menu.Init(blunoShield);

	////////////////////////////////////////////////
	// ADD ALL MENU OPTIONS HERE
	//**********************************************

	//Add the menu pages containing the temp and humidity range options to the main page
	//menu.CurrentPage()->AddOption(MenuOption("Temperature Ranges", &temperatureRangesPage));
	//menu.CurrentPage()->AddOption(MenuOption("Humidity Ranges", &humidityRangesPage));

	//Add the devices page
	//menu.CurrentPage()->AddOption(MenuOption("Devices", &devicesPage));

	//**********************************************
	// NO MORE MENU OPTIONS ADDED AFTER HERE
	////////////////////////////////////////////////

	//Initialize Bluno shield's built-in menu pages (AFTER we have added all potential pages)
	blunoShield.InitMenuPages(menu);

	//Initialize local menu pages
	//devicesPage.Init(menu);
}

/// <summary>
/// Ensure that minimums are smaller than maximums
/// </summary>
void EnforceMinSmallerThanMax()
{
	//Enforce that the minimum temp is the smaller of the two between it and the maximum
	/*if (tempMinDay > tempMaxDay)
		Swap(RangedValueU8, tempMinDay, tempMaxDay);

	if (tempMinNight > tempMaxNight)
		Swap(RangedValueU8, tempMinNight, tempMaxNight);*/

	//Enforce that the minimum humidity is the smaller of the two between it and the maximum
	/*if (humMinDay > humMaxDay)
		Swap(RangedValueU8, humMinDay, humMaxDay);

	if (humMinNight > humMaxNight)
		Swap(RangedValueU8, humMinNight, humMaxNight);*/
}

/// <summary>
/// Update all "local" devices
/// </summary>
void ProcessLocalDevices()
{
	//Get a more accurate distance reading with air temperature
	//garageDoorSensor.SetCurrentTemperature(blunoShield.temperature);
	//garageDoorSensor.Update();

	//Get a more accurate distance reading with air temperature
	//carDistanceFront.SetCurrentTemperature(blunoShield.temperature);
	//carDistanceFront.Update();

	//Update MQ9 sensor
	//mq9Sensor.Update();

	//Check garage door "open" status
	//ProcessGarageDoorSensor();
}

/// <summary>
/// Eventually, this function will send a customizable phone notification when the user needs an alert
/// </summary>
/// <param name="alertText"></param>
void SendAlert(const char* alertText)
{
	Serial.print("AlertText: ");
	Serial.println(alertText);
}

#pragma endregion

#pragma region MAIN _ARDUINO_ FUNCTIONS

/// <summary>
/// THE MAIN SETUP function for Arduino programs
/// </summary>
void setup()
{
	//First, and foremost, initialize all necessary Serial connections
	InitializeAllSerial();

	//Set up the Bluno accessory shield
	InitializeBlunoShield();

	//Create and initialize the menu system
	SetupMenuSystem();

	//Set up all local devices
	InitializeDevices();
}

/// <summary>
/// THE MAIN ARDUINO LOOP. 'Nuff said
/// </summary>
void loop()
{
	//For sanity's sake, ensure that all minimums are
	//smaller than all maximums, and vice versa (you never know)
	//EnforceMinSmallerThanMax();

	//Update the menu system (handles drawing and input)
	//UpdateMenuSystem();

	//Check temperature values and adjust accordingly
	//ProcessTemperature();

	//Check humidity levels and adjust accordingly
	//ProcessHumidity();

	//Set the message text, if any
	//SetBlunoMessageText();

	//Send any data we need to
	//SendData();

	//Update all of our objects (BlunoShield, lights, sensors)
	UpdateObjects();

	//byte fanSpeed = map(blunoShield.GetKnobValue(), 0, 1023, 0, 255);

	//analogWrite(44, fanSpeed);
}

#pragma endregion

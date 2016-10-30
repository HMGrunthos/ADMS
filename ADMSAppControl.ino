#include "DisplayBehaviour.h"
#include "AlarmBehaviour.h"
#include "purchasingAPI.h"

const unsigned long int WAKEINTERVAL = 30000; // In milliseconds
const unsigned long int WAKEDEVIATION = 20000; // In milliseconds
const unsigned long int INETCONNECTIONTIMEOUT = 5000; // In milliseconds
const unsigned long int PURCHASETIMEOUT = 10000; // In milliseconds
const unsigned long int TAUNTTIMEOUT = 6000; // In milliseconds
const unsigned long int PURCHASECOMPLETETIMEOUT = 20000; // In milliseconds
const char FIXEDGOOD[] = "B000NM4OHK";

const int BUTTONPIN = D3;

enum ADMSSystemState {
	Sleeping,
	ConnectToInternetStart,
	ConnectToInternetInProgress,
	ConnectToInternetFailed,
	FindItem,
	FailedToFindItem,
	AlertUserToPurchase,
	CountdownToPurchase,
	PurchaseAborted,
	StartPurchasing,
	WaitForPurchaseComplete,
	TauntUser
};

struct {
	enum ADMSSystemState systemState;
	std::string itemToPurchase;
	struct {
		unsigned long int wake;
		unsigned long int connectToInternet;
		unsigned long int purchase;
		unsigned long int taunt;
		unsigned long int purchaseComplete;
	} timeouts;
} appState = {ConnectToInternetStart, "", {0, 0, 0, 0, 0}};

// Internet functions
void attemptToConnectToInternet();
bool hasConnectionToInternetSucceeded();

// Purchasing functions
bool findItemToBuy(std::string *itemToPurchase);

// Phone alert functions
void sendPhoneAlert();

// Button press functions
bool getButtonPressState();

// Alarm sounding function
void soundAlarm(enum AlarmSound alarm);
void powerDownAudio();

// Utils and hardware management
static unsigned long int setTimeout(unsigned long int timeout);
static bool hasTimedOut(unsigned long int timer);
static bool hasTimedOut(unsigned long int timer, unsigned long int currentTime);
static void initHardware();
static void powerDownPeripherals();

void setup()
{
	Serial.begin(9600);
	delay(10000);
	Serial.println("ADMS Starting...");
	initHardware();
	initPurchasingAPI();
}

void loop()
{
	switch(appState.systemState) {
		case Sleeping:
			//Serial.println("In Sleeping");
			if(hasTimedOut(appState.timeouts.wake)) {
				Serial.println("Attempting to connect to internet...");
				appState.systemState = ConnectToInternetStart;
			} else {
				powerDownPeripherals();
			}
			break;
		case ConnectToInternetStart:
			Serial.println("In ConnectToInternetStart");
			attemptToConnectToInternet();
			appState.timeouts.connectToInternet = setTimeout(INETCONNECTIONTIMEOUT);
			appState.systemState = ConnectToInternetInProgress;
			break;
		case ConnectToInternetInProgress:
			//Serial.println("In ConnectToInternetInProgress");
			if(hasTimedOut(appState.timeouts.connectToInternet)) { // Then we timed out
				Serial.println("Connection timed out");
				appState.systemState = ConnectToInternetFailed;
			} else { // Test if the connection succeed?
				bool connectionMade = hasConnectionToInternetSucceeded();
				if(connectionMade == true) { // Success?
					Serial.println("Connection success find item");
					appState.systemState = FindItem; // Then start searching for the item
				} else { // Not yet
					//Serial.println("Waiting for connection");
					appState.systemState = ConnectToInternetInProgress; // Keep waiting
				}
			}
			break;
		case ConnectToInternetFailed:
			//Serial.println("In ConnectToInternetFailed");
			appState.timeouts.wake = setTimeout(WAKEINTERVAL + random(WAKEDEVIATION) - WAKEDEVIATION/2);
			appState.systemState = Sleeping;
			break;
		case FindItem:
			Serial.println("In FindItem");
			if(findItemToBuy(&appState.itemToPurchase)) { // Success?
				Serial.println("Found item");
				appState.systemState = AlertUserToPurchase;
			} else { // Failed to find item
				Serial.println("Failed to find item");
				appState.systemState = FailedToFindItem; // Couldn't find an item, keep quiet about it
			}
			break;
		case FailedToFindItem:
			Serial.println("In FailedToFindItem");
			appState.timeouts.wake = setTimeout(WAKEINTERVAL + random(WAKEDEVIATION) - WAKEDEVIATION/2);
			appState.systemState = Sleeping;
			break;
		case AlertUserToPurchase:
			Serial.println("In AlertUserToPurchase");
			sendPhoneAlert();
			setDisplayMessage(PurchaseWarningMessage);
			soundAlarm(PurchaseWarningAlarm);
			appState.timeouts.purchase = setTimeout(PURCHASETIMEOUT);
			appState.systemState = CountdownToPurchase;
			break;
		case CountdownToPurchase:
			//Serial.println("In CountdownToPurchase");
			// Display countdown
			if(getButtonPressState()) {
				Serial.println("Button pressed");
				appState.systemState = PurchaseAborted;
			} else { // Button not pressed before timed out
				if(hasTimedOut(appState.timeouts.purchase)) { // Test the purchase timeout
					// No button pressed so start making a purchase
					Serial.println("Timed out purchasing...");
					appState.systemState = StartPurchasing;
				} else {
					//Serial.println("Counting down...");
					appState.systemState = CountdownToPurchase; // Stay in the same state
				}
			}
			break;
		case PurchaseAborted:
			Serial.println("In PurchaseAborted");
			setDisplayMessage(AbortedPuchaseMessage);
			soundAlarm(AbortedPuchaseAlarm);
			appState.timeouts.taunt = setTimeout(TAUNTTIMEOUT);
			appState.systemState = TauntUser;
			break;
		case StartPurchasing:
		  Serial.println("In StartPurchasing");
			setDisplayMessage(PuchasingNowMessage);
			soundAlarm(PuchasingNowAlarm);
			purchaseSelectedGoods(FIXEDGOOD);
			appState.timeouts.purchaseComplete = setTimeout(PURCHASECOMPLETETIMEOUT);
			appState.systemState = WaitForPurchaseComplete;
			break;
		case WaitForPurchaseComplete:
		  Serial.println("In WaitForPurchaseComplete");
			switch(getOrderState()) {
				case 0: // Success
					Serial.println("Got purchase success");
					setDisplayMessage(PuchasedMadeMessage);
					soundAlarm(PuchaseMadeAlarm);
					appState.systemState = TauntUser;
					break;
				case 2: // Waiting
					delay(5000);
					break;
				case 1: // Fail
				default:
					Serial.println("Got purchase failure");
					setDisplayMessage(PuchaseFailedMessage);
					soundAlarm(PuchaseFailedAlarm);
					appState.systemState = TauntUser;
					break;
			}
			if(hasTimedOut(appState.timeouts.purchaseComplete)) {
				Serial.println("Got purchase failure (timeout)");
				setDisplayMessage(PuchaseFailedMessage);
				soundAlarm(PuchaseFailedAlarm);
				appState.systemState = TauntUser;
			}
			if(appState.systemState == TauntUser) {
				appState.timeouts.taunt = setTimeout(TAUNTTIMEOUT);
			}
			break;
		case TauntUser:
			// Serial.println("In TauntUser");
			if(hasTimedOut(appState.timeouts.taunt)) { // Finished taunting the user
				Serial.println("Taunt over going to sleep");
				appState.timeouts.wake = setTimeout(WAKEINTERVAL + random(WAKEDEVIATION) - WAKEDEVIATION/2);
				appState.systemState = Sleeping;
			} else {
				// Serial.println("Taunting...");
				appState.systemState = TauntUser; // Keep taunting with the message
			}
			break;
	}
}

static unsigned long int setTimeout(unsigned long int timeout)
{
	Serial.printlnf("Timeout duration :%i:", timeout);
	return millis() + timeout;
}

static bool hasTimedOut(unsigned long int timer)
{
	return hasTimedOut(timer, millis());
}

static bool hasTimedOut(unsigned long int timer, unsigned long int currentTime)
{
	if(currentTime > timer) { // Then we timed out
		return true;
	} else { // Timer still running
		return false;
	}
}

static void initHardware()
{
	pinMode(BUTTONPIN, INPUT_PULLUP);
	initDisplay();
	initAlarm();
}

static void powerDownPeripherals()
{
	powerDownDisplay();
	powerDownAudio();
}

void powerDownAudio()
{
}

// Internet functions
void attemptToConnectToInternet()
{
}

bool hasConnectionToInternetSucceeded()
{
	return true;
}

// Purchasing functions
bool findItemToBuy(std::string *itemToPurchase)
{
	itemToPurchase = new std::string("Here is an item.");
	return true;
}

void purchaseSelectedGoods(std::string itemToPurchase)
{
}

bool hasPurchaseSucceeded()
{
	return true;
}

bool hasPurchaseFailed()
{
	return false;
}

// Phone alert functions
void sendPhoneAlert()
{
}

// Button press functions
bool getButtonPressState()
{
	if(digitalRead(BUTTONPIN) == LOW) {
		return true;
	} else {
		return false;
	}
}

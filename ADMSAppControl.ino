const unsigned long int INETCONNECTIONTIMEOUT = 5000; // In milliseconds
const unsigned long int PURCHASETIMEOUT = 30000; // In milliseconds
const unsigned long int TAUNTTIMEOUT = 1000; // In milliseconds
const unsigned long int PURCHASECOMPLETETIMEOUT = 30000; // In milliseconds

const int BUTTONPIN = D4;

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

enum AlarmSound {
	PurchaseWarningAlarm,
	AbortedPuchaseAlarm,
	PuchasingNowAlarm,
	PuchaseFailedAlarm,
	PuchaseMadeAlarm
};

enum DisplayMessage {
	PurchaseWarningMessage,
	AbortedPuchaseMessage,
	PuchasingNowMessage,
	PuchaseFailedMessage,
	PuchasedMadeMessage
};

struct {
	enum ADMSSystemState systemState;
	std::string itemToPurchase;
	struct {
		unsigned long int connectToInternet;
		unsigned long int purchase;
		unsigned long int taunt;
		unsigned long int purchaseComplete;
	} timeouts;
} appState = {Sleeping, {0, 0, 0, 0}};

// Internet functions
void attemptToConnectToInternet();
bool hasConnectionToInternetSucceeded();

// Purchasing functions
bool findItemToBuy(std::string *itemToPurchase);
void purchaseSelectedGoods(std::string itemToPurchase);
bool hasPurchaseSucceeded();
bool hasPurchaseFailed();

// Phone alert functions
void sendPhoneAlert();

// Button press functions
bool getButtonPressState();

// Alarm sounding function
void soundAlarm(enum AlarmSound alarm);
void powerDownAlarm();

// Display function
void setDisplayMessage(enum DisplayMessage message);
void clearDisplayMessage();

// Utils and hardware management
static unsigned long int setTimeout(unsigned long int timeout);
static bool hasTimedOut(unsigned long int timer);
static bool hasTimedOut(unsigned long int timer, unsigned long int currentTime);
static void initHardware();
static void powerDownPeripherals();
static bool timeToWakeUp();

void setup()
{
	initHardware();
}

void loop()
{
	unsigned long int nextSleep = 0;

	switch(appState.systemState) {
		case Sleeping:
			if(timeToWakeUp()) {
				appState.systemState = ConnectToInternetStart;
			}
			break;
		case ConnectToInternetStart:
			attemptToConnectToInternet();
			appState.timeouts.connectToInternet = setTimeout(INETCONNECTIONTIMEOUT);
			appState.systemState = ConnectToInternetInProgress;
			break;
		case ConnectToInternetInProgress:
			if(hasTimedOut(appState.timeouts.connectToInternet)) { // Then we timed out
				appState.systemState = ConnectToInternetFailed;
			} else { // Test if the connection succeed?
				bool connectionMade = hasConnectionToInternetSucceeded();
				if(connectionMade == true) { // Success?
					appState.systemState = FindItem; // Then start searching for the item
				} else { // Not yet
					appState.systemState = ConnectToInternetInProgress; // Keep waiting
				}
			}
			break;
		case ConnectToInternetFailed:
			appState.systemState = Sleeping;
			break;
		case FindItem:
			if(findItemToBuy(&appState.itemToPurchase)) { // Success?
				appState.systemState = AlertUserToPurchase;
			} else { // Failed to find item
				appState.systemState = FailedToFindItem; // Couldn't find an item, keep quiet about it
			}
			break;
		case FailedToFindItem:
			appState.systemState = Sleeping;
			break;
		case AlertUserToPurchase:
			sendPhoneAlert();
			soundAlarm(PurchaseWarningAlarm);
			setDisplayMessage(PurchaseWarningMessage);
			appState.timeouts.purchase = setTimeout(PURCHASETIMEOUT);
			appState.systemState = CountdownToPurchase;
			break;
		case CountdownToPurchase:
			// Display countdown
			if(getButtonPressState()) {
				appState.systemState = PurchaseAborted;
			} else { // Button not pressed before timed out
				if(hasTimedOut(appState.timeouts.purchase)) { // Test the purchase timeout
					// No button pressed so start making a purchase
					appState.systemState = StartPurchasing;
				} else {
					appState.systemState = CountdownToPurchase; // Stay in the same state
				}
			}
			break;
		case PurchaseAborted:
			soundAlarm(AbortedPuchaseAlarm);
			setDisplayMessage(AbortedPuchaseMessage);
			appState.timeouts.taunt = setTimeout(TAUNTTIMEOUT);
			appState.systemState = TauntUser;
			break;
		case StartPurchasing:
			soundAlarm(PuchasingNowAlarm);
			setDisplayMessage(PuchasingNowMessage);
			purchaseSelectedGoods(appState.itemToPurchase);
			appState.timeouts.purchaseComplete = setTimeout(PURCHASECOMPLETETIMEOUT);
			appState.systemState = WaitForPurchaseComplete;
			break;
		case WaitForPurchaseComplete:
			if(hasPurchaseSucceeded()) {
				soundAlarm(PuchaseMadeAlarm);
				setDisplayMessage(PuchasedMadeMessage);
				appState.systemState = TauntUser;
			} else if(hasPurchaseFailed()){
				soundAlarm(PuchaseFailedAlarm);
				setDisplayMessage(PuchaseFailedMessage);
				appState.systemState = TauntUser;
			} else if(hasTimedOut(appState.timeouts.purchaseComplete)) {
				soundAlarm(PuchaseFailedAlarm);
				setDisplayMessage(PuchaseFailedMessage);
				appState.systemState = TauntUser;
			}
			appState.timeouts.taunt = setTimeout(TAUNTTIMEOUT);
			break;
		case TauntUser:
			if(hasTimedOut(appState.timeouts.taunt)) { // Finished taunting the user
				powerDownPeripherals();
				appState.systemState = Sleeping;
			} else {
				appState.systemState = TauntUser; // Keep taunting with the message
			}
			break;
	}
}

static unsigned long int setTimeout(unsigned long int timeout)
{
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
	pinMode(BUTTONPIN, INPUT_PULLHIGH);
}

static void powerDownPeripherals()
{
	clearDisplayMessage();
}

static bool timeToWakeUp()
{
	return true;
}

// Alarm sounding function
void soundAlarm(enum AlarmSound alarm)
{
	switch(alarm) {
		case PurchaseWarningAlarm:
			break;
		case AbortedPuchaseAlarm:
			break;
		case PuchasingNowAlarm:
			break;
		case PuchaseFailedAlarm:
			break;
		case PuchaseMadeAlarm:
			break;
	}
}

void powerDownAlarm()
{
}

// Display function
void setDisplayMessage(enum DisplayMessage message)
{
	switch(message) {
		case PurchaseWarningMessage:
			break;
		case AbortedPuchaseMessage:
			break;
		case PuchasingNowMessage:
			break;
		case PuchaseFailedMessage:
			break;
		case PuchasedMadeMessage:
			break;
	}
}

void clearDisplayMessage()
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

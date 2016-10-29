const unsigned long int INETCONNECTIONTIMEOUT 5000; // In milliseconds
const unsigned long int PURCHASETIMEOUT 30000; // In milliseconds
const unsigned long int TAUNTTIMEOUT 1000; // In milliseconds

enum ADMSSystemState {
	Sleeping,
	WokenUp,
	ConnectToInternetStart,
	ConnectToInternetInProgress,
	ConnectToInternetFailed,
	FindItem,
	FailedToFindItem,
	AlertUserToPurchase,
	CountdownToPurchase,
	PurchaseAvoided,
	SoundPurchaseAlarm,
	MakePurchase
};

struct {
	enum ADMSSystemState systemState;
	std::string itemToBuy;
	struct {
		unsigned long int connectToInternet;
		unsigned long int purchase;
		unsigned long int taunt;
	} timeouts;
} appState = {Sleeping, {0, 0}};

void setup()
{
}

int loop()
{
	unsigned long int nextSleep = 0;

	switch(appState.systemState) {
		case Sleeping:
			appState.systemState = WokenUp;
			break;
		case WokenUp:
			appState.systemState = ConnectToInternetStart;
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
				bool connectionMade = hasConnectionToInternetSucceeded()
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
			bool foundItem = findItemToBuy(&appState.itemToBuy);
			if(foundItem == true) { // Success?
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
			soundPurchaseWarningAlarm();
			appState.timeouts.purchase = setTimeout(PURCHASETIMEOUT);
			appState.systemState = CountdownToPurchase;
			break;
		case CountdownToPurchase:
			void buttonPressed = getButtonPressState();
			if(buttonPressed) {
				appState.systemState = CountdownButtonPressed;
			} else { // Button not pressed before timed out
				if(hastimedOut(appState.timeouts.purchase)) { // Test the purchase timeout
					// No button pressed so start making a purchase
					appState.systemState = SoundPurchaseAlarm;
				} else {
					appState.systemState = CountdownToPurchase; // Stay in the same state
				}
			}
			break;
		case CountdownButtonPressed:
			setUserAbortedDisplayMessage();
			soundPuchaseAvoidedAlarm();
			appState.timeouts.taunt = setTimeout(TAUNTTIMEOUT);
			appState.systemState = TauntUser;
			break;
		case SoundPurchaseAlarm:
			setPurchasingDisplayMessage();
			soundPuchasingAlarm();
			appState.systemState = MakePurchase;
			break;
		case MakePurchase:
			purchaseSelectedGoods(appState.itemToBuy);
			setPuchasedDisplayMessage();
			soundPuchaseMadeAlarm();
			appState.timeouts.taunt = setTimeout(TAUNTTIMEOUT);
			appState.systemState = TauntUser;
			break;
		case TauntUser:
			if(hastimedOut(appState.timeouts.taunt)) { // Finished taunting the user
				powerDownPeripherals();
				appState.systemState = Sleep;
			} else {
				appState.systemState = TauntUser; // Keep taunting with the message
			}
			break;
	}
}

unsigned long int setTimeout(unsigned long int timeout)
{
	return millis() + timeout;
}

bool hasTimedOut(unsigned long int timer)
{
	return hasTimedOut(timer, millis());
}

bool hasTimedOut(unsigned long int timer, unsigned long int currentTime)
{
	if(currentTime > timer) { // Then we timed out
		return true;
	} else { // Timer still running
		return false;
	}
}

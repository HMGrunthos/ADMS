const unsigned long int INETCONNECTIONTIMEOUT 5000; // In milliseconds
const unsigned long int PURCHASETIMEOUT 30000; // In milliseconds

enum ADMSSystemState {
	Sleeping,
	WokenUp,
	ConnectToInternetStart,
	ConnectToInternetInProgress,
	ConnectToInternetFailed,
	FindItem,
	FailedToFindItem,
	AlertUser,
	Countdown,
	CountdownEndedTimedOut,
	CountdownButtonPressed,
	SoundPurchaseAlarms,
	MakePurchase
};

struct {
	enum ADMSSystemState systemState;
	std::string itemToBuy;
	struct {
		unsigned long int connectToInternet;
		unsigned long int purchase;
	} timeouts;
} appState = {Sleeping, {0}};

void setup()
{
}

int loop()
{
	unsigned long int nextSleep = 0;

	switch(appState.systemState) {
		case Sleeping:
			appState.systemState = WokenUp;
		case WokenUp:
			appState.systemState = ConnectToInternetStart;
		case ConnectToInternetStart:
			attemptConnectToInternet();
			appState.systemState = ConnectToInternetInProgress;
			appState.timeouts.connectToInternet = setTimeout(INETCONNECTIONTIMEOUT);
			break;
		case ConnectToInternetInProgress:
			if(hasTimedOut(appState.timeouts.connectToInternet)) { // Then we timed out
				appState.systemState = ConnectToInternetFailed;
			} else { // Test if the connection succeed?
				bool connectionMade = hadConnectionSucceeded()
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
				appState.systemState = AlertUser;
			} else { // Failed to find item
				appState.systemState = FailedToFindItem; // Couldn't find an item, keep quiet about it
			}
			break;
		case FailedToFindItem:
			appState.systemState = Sleeping;
			break;
		case AlertUser:
			sendPhoneAlert();
			soundAlarm();
			appState.timeouts.purchase = setTimeout(PURCHASETIMEOUT);
			appState.systemState = Countdown;
			break;
		case Countdown:
			void buttonPressed = getButtonPressState();
			if(buttonPressed) {
				appState.systemState = CountdownButtonPressed;
			} else {
				if(timedOut(appState.timeouts.purchase)) { // Test the purchase timeout
					// No button pressed so go to the purchase state
					appState.systemState = CountdownEndedTimedOut;
				}
			}
			break;
		case CountdownEndedTimedOut:
			break;
		case CountdownButtonPressed:
			break;
		case SoundPurchaseAlarms:
			break;
		case MakePurchase:
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

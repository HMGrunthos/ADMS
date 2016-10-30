
enum DisplayMessage {
	PurchaseWarningMessage,
	AbortedPuchaseMessage,
	PuchasingNowMessage,
	PuchaseFailedMessage,
	PuchasedMadeMessage
};

// Display function
void initDisplay();
void setDisplayMessage(enum DisplayMessage message);
void clearDisplayMessage();
void powerDownDisplay();

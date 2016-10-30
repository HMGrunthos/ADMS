
enum AlarmSound {
	PurchaseWarningAlarm,
	AbortedPuchaseAlarm,
	PuchasingNowAlarm,
	PuchaseFailedAlarm,
	PuchaseMadeAlarm
};

// Alarm function
void initAlarm();
void soundAlarm(enum AlarmSound alarm);

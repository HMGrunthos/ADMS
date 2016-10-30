#include "AlarmBehaviour.h"
// This #include statement was automatically added by the Particle IDE.
#include "SparkIntervalTimer.h"
#include "math.h"

// connect a 8Ohm speaker with a 220Ohm resistor to pin DAC and GND
const int AUDIO = DAC1;
const int SPEAKERUPDATE = 50; // uSecs
IntervalTimer audio_clock;
int wave_ix = 0;
int currentFreq = 0;

static void play_wave();
static void start_play(int freq);
static void stop_play();

void initAlarm()
{
	pinMode(AUDIO, OUTPUT);
}

// Alarm sounding function
void soundAlarm(enum AlarmSound alarm)
{
	switch(alarm) {
		case PurchaseWarningAlarm:
			start_play(256);
			delay(5000);
      stop_play();
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

void play_wave(void) {
    wave_ix++;
    analogWrite(AUDIO, (2046*sin(currentFreq*2*M_PI*wave_ix*SPEAKERUPDATE*1e-6)+2046));
}

void start_play(int freq) {
    wave_ix = 0;
		currentFreq = freq;
    audio_clock.begin(play_wave, SPEAKERUPDATE, uSec);
}

void stop_play(void) {
    audio_clock.end();
    analogWrite(AUDIO, 0);
    wave_ix = 0;
}

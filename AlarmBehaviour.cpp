#include "AlarmBehaviour.h"
// This #include statement was automatically added by the Particle IDE.
#include "SparkIntervalTimer/SparkIntervalTimer.h"
#include "math.h"

// connect a 8Ohm speaker with a 220Ohm resistor to pin DAC and GND
const int audio = DAC1;
const int speakerUpdate = 50; // uSecs
IntervalTimer audio_clock;

static void play_wave();
static void start_play();
static void stop_play();

void initAlarm()
{
	pinMode(audio, OUTPUT);
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

void play_wave(int freq) {
    // wave_ix++;
    analogWrite(audio, (2046*sin(freq*2*M_PI*wave_ix*speakerUpdate*1e-6)+2046));
}

void start_play(int freq) {
    // wave_ix = 0;
    audio_clock.begin(play_wave(freq)), speakerUpdate, uSec);
}

void stop_play(void) {
    audio_clock.end();
    analogWrite(audio, 2048);
    // wave_ix = 0;
}

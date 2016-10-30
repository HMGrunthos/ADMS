#include "dogm_7036.h"
#include "DisplayBehaviour.h"

static dogm_7036 DOG;

//the following port definitions are used by our demo board "EA PCBARDDOG7036"
const int LEDRED = WKP;
const int LEDGREEN = RX;
const int LEDBLUE  = TX;

static void initBacklight();
static void rgbBacklight(byte red, byte green, byte blue);

void initDisplay()
{
	initBacklight();
	DOG.initialize(A2, 0, 0, D0, D1, 0, DOGM163);   // SS = A2, 0, 0= use Hardware SPI, D0 = RS, D1 = RESET, 0 = 3.3V, EA DOGM163-A (=3 lines)
	DOG.cursor_onoff(false); // No cursor
}

void setDisplayMessage(enum DisplayMessage message)
{
	DOG.displ_onoff(true);
	DOG.clear_display();
   switch(message) {
      case PurchaseWarningMessage:
			DOG.position(1,1);
			DOG.string("PurchaseWarningMessage");
			rgbBacklight(255, 0, 0);
         break;
      case AbortedPuchaseMessage:
			DOG.position(1,1);
			DOG.string("AbortedPuchaseMessage");
			rgbBacklight(0, 255, 0);
         break;
      case PuchasingNowMessage:
			DOG.position(1,1);
			DOG.string("PuchasingNowMessage");
			rgbBacklight(0, 0, 255);
         break;
      case PuchaseFailedMessage:
			DOG.position(1,1);
			DOG.string("PuchaseFailedMessage");
			rgbBacklight(0, 255, 255);
         break;
      case PuchasedMadeMessage:
			DOG.position(1,1);
			DOG.string("PuchasedMadeMessage");
			rgbBacklight(255, 0, 255);
         break;
   }
}

void clearDisplayMessage()
{
	DOG.clear_display();
}

void powerDownDisplay()
{
	clearDisplayMessage();
	DOG.displ_onoff(false);
	rgbBacklight(0, 0, 0);
}

//The following functions controll the backlight with a PWM. Not needed for the display content
static void initBacklight()
{
	pinMode(LEDRED, OUTPUT);
	pinMode(LEDGREEN, OUTPUT);
	pinMode(LEDBLUE, OUTPUT);
	rgbBacklight(0, 0, 0);
}

//Use this function for RGB backlight
static void rgbBacklight(byte red, byte green, byte blue)
{
  analogWrite(LEDRED, red);
  analogWrite(LEDGREEN, green);
  analogWrite(LEDBLUE, blue);
}

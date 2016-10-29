#include <dogm_7036/dogm_7036.h>
/*Available functions in dogm_7036 Libraray:
void initialize       (byte p_cs, byte p_si, byte p_clk, byte p_rs, byte p_res, boolean sup_5V, byte lines);
void string           (const char *str);
void ascii            (char character);
void position         (byte column, byte line);
void displ_onoff      (boolean on);
void cursor_onoff     (boolean on);
void define_char      (byte mem_adress, byte *dat);
void clear_display    (void);
void contrast         (byte contr);
*/

dogm_7036 DOG;
byte arrow_down[] = {0x04, 0x04, 0x04, 0x04, 0x15, 0x0E, 0x04, 0x00}; //pattern for own defined character

//the following port definitions are used by our demo board "EA PCBARDDOG7036"
int led_red   = WKP;
int led_green = RX;
int led_blue  = TX;
int btn = D2;
void init_backlight();
void rgb_backlight(byte red, byte green, byte blue);


void setup()
{
  pinMode(btn, INPUT_PULLDOWN);
  init_backlight(); //use rgb backlight
  DOG.initialize(A2,0,0,D0,D1,0,DOGM163);   //SS = A2, 0,0= use Hardware SPI, D0 = RS, D1 = RESET, 0 = 3.3V, EA DOGM163-A (=3 lines)
  DOG.displ_onoff(true);          //turn Display on
  DOG.cursor_onoff(true);         //turn Curosor blinking on
  DOG.define_char(0, arrow_down); //define own char on memory adress 0
  DOG.position(3,1);              //set Position: first line, third character
  DOG.string("EA DOGM TEXT");     //show String
  DOG.position(1,2);
  DOG.string("Special char: ");
  DOG.ascii(0);                   //show own defined char (adress 0, see DOG.define_char())
  DOG.position(0,3);
  DOG.string("Photon lib");
}


void loop()
{
  if (digitalRead(btn) == HIGH) {
  }
  DOG.clear_display();            //Clear the whole content
  DOG.position(1,1);              //set Position: first line, first character
  DOG.string("DOGM163-A");        //show String
  DOG.position(1,2);              //set Position: second line, first character
  DOG.string("ST7036 COG");
  DOG.position(1,3);
  DOG.string("3 lines 16 char");
  rgb_backlight(255, 0, 0); //red backlight
  delay(2000);
  DOG.clear_display();
  DOG.position(1,1);
  DOG.string("ELECTRONIC");
  DOG.position(9,2);
  DOG.string("ASSEMBLY");
  DOG.position(1,3);
  DOG.string("lcd-module.com");
  rgb_backlight(0, 255, 0); //green backlight
  delay(2000);
  DOG.clear_display();
  DOG.position(1,1);
  DOG.string("Arduino Library");
  DOG.position(1,2);
  DOG.string("0wn Char ");
  DOG.ascii(0);                   //show own defined char (adress 0, see DOG.define_char())
  DOG.position(1,3);
  DOG.string("low power");
  rgb_backlight(0, 0, 255); //blue backlight
  delay(2000);
  DOG.clear_display();
  DOG.position(1,1);
  DOG.string("LED-Backlights:");
  DOG.position(1,2);
  DOG.string("5 monochrome");
  DOG.position(1,3);
  DOG.string("full color RGB");
  rgb_backlight(255, 255, 0); //purple backlight
  delay(2000);
}

//The following functions controll the backlight with a PWM. Not needed for the display content
void init_backlight()
{
    pinMode(led_blue, OUTPUT);
    pinMode(led_green, OUTPUT);
    pinMode(led_red, OUTPUT);
    rgb_backlight(127,127,127);
}

//Use this function for RGB backlight
void rgb_backlight(byte red, byte green, byte blue)
{
  analogWrite(led_red,  red);
  analogWrite(led_green, green);
  analogWrite(led_blue,   blue);
}

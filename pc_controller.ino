#include <FastLED.h>

#define BUTTON_PIN  2
#define LED_PIN     3
#define FAN_PIN     5


#define NUM_LEDS      16
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];

int fanSpeed          =  9;
int ledSpeed          =  5;
int brightness        =  65;
int ledProgram        =  8;
int r                 =  255;
int g                 =  0;
int b                 =  0;
int ledPower          =  1;
int fanPower          =  0;

const int SHORT_PRESS_TIME = 500; // milliseconds
const int LONG_PRESS_TIME  = 500; 

int lastState = LOW;  
int currentState;     
unsigned long pressedTime  = 0;
unsigned long releasedTime = 0;

CRGBPalette16 currentPalette;
TBlendType    currentBlending;

extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;

void setup() {
    delay( 2000 ); 
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
    FastLED.setBrightness(  brightness );
    Serial.begin(115200);

    currentPalette = RainbowColors_p;
    currentBlending = LINEARBLEND;

    pinMode(BUTTON_PIN, INPUT);
    pinMode(FAN_PIN, OUTPUT);

    analogWrite(FAN_PIN, 255);
    delay( 300 );
    analogWrite(FAN_PIN, fanSpeed);

    Serial.println("power_fan:1");
    Serial.println("led_power:1");

    Serial.print("fan_speed:");
    Serial.println(fanSpeed);

    Serial.print("led_speed:");
    Serial.println(ledSpeed);

    Serial.print("brightness:");
    Serial.println(brightness);

    Serial.print("led_program:");
    Serial.println(ledProgram);

    ChangePalettePeriodically(ledProgram);
}


void loop()
{
  readSerial();
  ledControll();
  buttonControll();
}

void ledControll(){
   static uint8_t startIndex = 0;
    startIndex = startIndex + 1; /* motion speed */

    if(ledPower == 1){
      FillLEDsFromPaletteColors( startIndex);
      FastLED.show();
      FastLED.delay(1000 / ledSpeed);
    } else {
      FastLED.clearData();
      FastLED.show();
    }
}

void buttonControll(){
  currentState = digitalRead(BUTTON_PIN);

  if(lastState == LOW && currentState == HIGH){        // button is pressed
    pressedTime = millis();
  } else if(lastState == HIGH && currentState == LOW) { // button is released
    releasedTime = millis();

    long pressDuration = releasedTime - pressedTime;

    if( pressDuration < SHORT_PRESS_TIME ){
      if(ledPower == 1) {
        ledPower = 0;
        Serial.println("led_power:0");
      } else {
        ledPower = 1;
        Serial.println("led_power:1");
      }

     
    }

    if( pressDuration > LONG_PRESS_TIME ){
      if(fanSpeed > 0){
        Serial.println("fan_power:0");
        Serial.println("fan_speed:0");
        fanSpeed = 0;
      } else {
        Serial.println("fan_power:1");
        Serial.println("fan_speed:255");
        fanSpeed = 255;
      }
      analogWrite(FAN_PIN, fanSpeed);
    }
  }
  lastState = currentState;
}

void FillLEDsFromPaletteColors( uint8_t colorIndex)
{
    uint8_t brightness = 255;

    for( int i = 0; i < NUM_LEDS; ++i) {
        leds[i] = ColorFromPalette( currentPalette, colorIndex, brightness, currentBlending);
        colorIndex += 3;
    }
}

void readSerial(){
  if(Serial.available() > 0){
    String data = Serial.readString();

    String topic = data.substring(0,data.indexOf(':'));
    String value = data.substring(data.indexOf(':') + 1, data.length());

    Serial.println("debug: topic: " + topic);
    Serial.println("debug: value: " + value);

    if(topic == "led_speed"){
      ledSpeed = value.toInt(); 
    
    } else if(topic == "brightness"){
      ledPower = 1;
      brightness = value.toInt();
      FastLED.setBrightness(brightness);

    } else if(topic == "led_power") {
      ledPower = value.toInt(); 

    } else if(topic == "fan_power") {
        fanPower = value.toInt();

        if(fanPower == 1){
          analogWrite(FAN_PIN, 255);
          delay(200);
          analogWrite(FAN_PIN, fanSpeed);
        }else{
           analogWrite(FAN_PIN, 0);
        }
      
    } else if(topic == "led_program") {
      ledProgram = value.toInt();
      ChangePalettePeriodically(ledProgram);
      Serial.println("led_power:1");
      
    } else if(topic == "rgb") {
      ledPower = 1;
      int x;
      char *endptr;
      r = strtol(value.substring(0,2).c_str(), &endptr, 16);
      g = strtol(value.substring(2,4).c_str(), &endptr, 16);
      b = strtol(value.substring(4,6).c_str(), &endptr, 16);
      ChangePalettePeriodically(9);
      Serial.println("led_program:9");
      Serial.println("led_power:1");

    } else if(topic == "fan_speed"){
        fanPower = 1;
        fanSpeed = value.toInt();
         
        if(fanSpeed != 0 && fanSpeed < 30){
          analogWrite(FAN_PIN, 255);
          delay(200);
        }
        analogWrite(FAN_PIN, fanSpeed);
     } 

  }
}

void ChangePalettePeriodically(int program){
    uint8_t secondHand = (millis() / 1000) % 60;
    static uint8_t lastSecond = 99;
    
    if( lastSecond != secondHand) {
        if( program == 1)  { currentPalette = RainbowStripeColors_p;   currentBlending = LINEARBLEND; }
        if( program == 2)  { SetupPurpleAndGreenPalette();             currentBlending = LINEARBLEND; }
        if( program == 3)  { SetupTotallyRandomPalette();              currentBlending = LINEARBLEND; }
        if( program == 4)  { SetupBlackAndWhiteStripedPalette();       currentBlending = NOBLEND; }
        if( program == 5)  { currentPalette = CloudColors_p;           currentBlending = LINEARBLEND; }
        if( program == 6)  { currentPalette = PartyColors_p;           currentBlending = LINEARBLEND; }
        if( program == 7)  { currentPalette = myRedWhiteBluePalette_p; currentBlending = LINEARBLEND; }
        if( program == 8)  { currentPalette = RainbowColors_p;         currentBlending = LINEARBLEND; }
        if( program == 9)  { rgb();         currentBlending = LINEARBLEND; }
    }
}

void SetupTotallyRandomPalette()
{
    for( int i = 0; i < 16; ++i) {
        currentPalette[i] = CHSV( random8(), 255, random8());
    }
}

void rgb(){
for( int i = 0; i < 16; ++i) {
   //     currentPalette[i] = CHSV( 255, 1, random8());
        currentPalette[i].r=r;
        currentPalette[i].g=g;
        currentPalette[i].b=b;
    
    }                               
}

void SetupBlackAndWhiteStripedPalette()
{
    // 'black out' all 16 palette entries...

    fill_solid( currentPalette, 16, CHSV( 255, 0, 0));
    // and set every fourth one to white.
    currentPalette[0] = CRGB::White;
    currentPalette[4] = CRGB::White;
    currentPalette[8] = CRGB::White;
    currentPalette[12] = CRGB::White;
    
}

// This function sets up a palette of purple and green stripes.
void SetupPurpleAndGreenPalette()
{
    CRGB purple = CHSV( HUE_PURPLE, 255, 255);
    CRGB green  = CHSV( HUE_GREEN, 255, 255);
    CRGB black  = CRGB::Black;
    
    currentPalette = CRGBPalette16(
                                   green,  green,  black,  black,
                                   purple, purple, black,  black,
                                   green,  green,  black,  black,
                                   purple, purple, black,  black );
}


// This example shows how to set up a static color palette
// which is stored in PROGMEM (flash), which is almost always more
// plentiful than RAM.  A static PROGMEM palette like this
// takes up 64 bytes of flash.
const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM =
{
    CRGB::Red,
    CRGB::Gray, // 'white' is too bright compared to red and blue
    CRGB::Blue,
    CRGB::Black,
    
    CRGB::Red,
    CRGB::Gray,
    CRGB::Blue,
    CRGB::Black,
    
    CRGB::Red,
    CRGB::Red,
    CRGB::Gray,
    CRGB::Gray,
    CRGB::Blue,
    CRGB::Blue,
    CRGB::Black,
    CRGB::Black
};



// Additional notes on FastLED compact palettes:
//
// Normally, in computer graphics, the palette (or "color lookup table")
// has 256 entries, each containing a specific 24-bit RGB color.  You can then
// index into the color palette using a simple 8-bit (one byte) value.
// A 256-entry color palette takes up 768 bytes of RAM, which on Arduino
// is quite possibly "too many" bytes.
//
// FastLED does offer traditional 256-element palettes, for setups that
// can afford the 768-byte cost in RAM.
//
// However, FastLED also offers a compact alternative.  FastLED offers
// palettes that store 16 distinct entries, but can be accessed AS IF
// they actually have 256 entries; this is accomplished by interpolating
// between the 16 explicit entries to create fifteen intermediate palette
// entries between each pair.
//
// So for example, if you set the first two explicit entries of a compact 
// palette to Green (0,255,0) and Blue (0,0,255), and then retrieved 
// the first sixteen entries from the virtual palette (of 256), you'd get
// Green, followed by a smooth gradient from green-to-blue, and then Blue.

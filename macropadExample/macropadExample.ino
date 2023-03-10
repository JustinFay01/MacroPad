#include <Adafruit_SH110X.h>
#include <Adafruit_NeoPixel.h>
#include <RotaryEncoder.h>
#include <Wire.h>
#include <Keyboard.h>

//https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json     //MacroPad Link
//https://raw.githubusercontent.com/NicoHood/HoodLoader2/master/package_NicoHood_HoodLoader2_index.json // Enhanced HID link



// Create the neopixel strip with the built in definitions NUM_NEOPIXEL and PIN_NEOPIXEL
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_NEOPIXEL, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);

// Create the OLED display
Adafruit_SH1106G display = Adafruit_SH1106G(128, 64, &SPI1, OLED_DC, OLED_RST, OLED_CS);

// Create the rotary encoder
RotaryEncoder encoder(PIN_ROTA, PIN_ROTB, RotaryEncoder::LatchMode::FOUR3);
void checkPosition() {  encoder.tick(); } // just call tick() to check the state.
// our encoder position state
int encoder_pos = 0;

void setup() {
  Serial.begin(115200);
  //while (!Serial) { delay(10); }     // wait till serial port is opened
  delay(100);  // RP2040 delay is not a bad idea

  Serial.println("Adafruit Macropad with RP2040");

  // start pixels!
  pixels.begin();
  pixels.setBrightness(255);
  pixels.show(); // Initialize all pixels to 'off'

  // Start OLED
  display.begin(0, true); // we dont use the i2c address but we will reset!
  display.display();
  
  // set all mechanical keys to inputs
  for (uint8_t i=0; i<=12; i++) {
    pinMode(i, INPUT_PULLUP);
  }

  // set rotary encoder inputs and interrupts
  pinMode(PIN_ROTA, INPUT_PULLUP);
  pinMode(PIN_ROTB, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIN_ROTA), checkPosition, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_ROTB), checkPosition, CHANGE);  

  // We will use I2C for scanning the Stemma QT port
  Wire.begin();

  // text display tests
  display.setTextSize(1);
  display.setTextWrap(false);
  display.setTextColor(SH110X_WHITE, SH110X_BLACK); // white text, black background

}

void speakerInit(){
 // Enable speaker
  pinMode(PIN_SPEAKER_ENABLE, OUTPUT);
  digitalWrite(PIN_SPEAKER_ENABLE, HIGH);
  // Play some tones
  pinMode(PIN_SPEAKER, OUTPUT);
  digitalWrite(PIN_SPEAKER, LOW);
  tone(PIN_SPEAKER, 988, 100);  // tone1 - B5
  delay(100);
  tone(PIN_SPEAKER, 1319, 200); // tone2 - E6
  delay(200);
}


//Display Key pressed and message 
void switchDisplay(int i){
  Serial.print("Switch "); Serial.println(i);
      pixels.setPixelColor(i-1, 0xFFFFFF);  // make white
      // move the text into a 3x4 grid
      display.setCursor(((i-1) % 3)*48, 32 + ((i-1)/3)*8);
      display.print("tay :) ");
      display.print(i);
}

//Basic numpad functions
void useNumPad(int i){
  char key = i + '0';
  Keyboard.begin();
  Keyboard.press(key);
  delay(150);
  Keyboard.release(key);
}

void changeVol(int x){
  Keyboard.begin();
  if(x == 0)      display.print("Not yet :(");
  else if (x == 1) display.print("Not yet :(");            
  else 

  delay(150);                             
  Keyboard.releaseAll();
}


// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
   return pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  } else {
   WheelPos -= 170;
   return pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
}
uint8_t j = 0;
bool i2c_found[128] = {false};

void loop() {
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("* Adafruit Macropad *");
  
  encoder.tick();          // check the encoder
  int newPos = encoder.getPosition();
  // if (encoder_pos != newPos) {
  //   Serial.print("Encoder:");
  //   Serial.print(newPos);
  //   Serial.print(" Direction:");
  //   Serial.println((int)(encoder.getDirection()));
  //   encoder_pos = newPos;
  // }

  if (encoder_pos > newPos){
    display.setCursor(0, 24);
    display.print("Volume up");
    pixels.setBrightness(255);

    changeVol(0);
  } 
  else if(encoder_pos < newPos){
    display.setCursor(0, 24);
    display.print("Volume down");
    pixels.setBrightness(255);

    changeVol(1);
  }
  encoder_pos = newPos;
  
  display.setCursor(0, 8);
  display.print("Rotary encoder: ");
  display.print(encoder_pos);

  // Scanning takes a while so we don't do it all the time
  if ((j & 0x3F) == 0) {
    Serial.println("Scanning I2C: ");
    Serial.print("Found I2C address 0x");
    for (uint8_t address = 0; address <= 0x7F; address++) {
      Wire.beginTransmission(address);
      i2c_found[address] = (Wire.endTransmission () == 0);
      if (i2c_found[address]) {
        Serial.print("0x");
        Serial.print(address, HEX);
        Serial.print(", ");
      }
    }
    Serial.println();
  }
  
  display.setCursor(0, 16);
  display.print("I2C Scan: ");
  for (uint8_t address=0; address <= 0x7F; address++) {
    if (!i2c_found[address]) continue;
    display.print("0x");
    display.print(address, HEX);
    display.print(" ");
  }
  
  // check encoder press
  display.setCursor(0, 24);
  if (!digitalRead(PIN_SWITCH)) {
    Serial.println("Encoder button");
    display.print("Encoder pressed ");
    pixels.setBrightness(255);     // bright!
  } else {
    pixels.setBrightness(80);
  }

  //Rainbow Colors
  for(int i=0; i< pixels.numPixels(); i++) {
    pixels.setPixelColor(i, Wheel(((i * 256 / pixels.numPixels()) + j) & 255));
  }
  
  for (int i=1; i<=12; i++) {
    if (!digitalRead(i)) { // switch pressed!
      switchDisplay(i);
      //useNumPad(i);
      pixels.setPixelColor(i-1, 0x0);
    }
  }

  // show neopixels, incredment swirl
  pixels.show();
  j++;

  // display oled
  display.display();
}


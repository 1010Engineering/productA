/***********************************************************************
 *                        1010 INNOVATION LABS                         *
 * AUTHOR:   Alec Selfridge                                            *
 * PROJECT:  Solar-Powered Armband                                     *
 * VERSION:  1.1                                                       *
 * REVISION: 02/25/2016                                                *
 * NOTES:    Target device: ATTINY85 (Trinket 8MHZ/USBTinyISP)         *
 *           Two NeoPixels, connected to the same pin, may be          *
 *           configured by a push-button connected to another pin.     *
 *           Said button is software debounced by comparing the time   *
 *           of the first low-to-high transition with a later reading. *
 *           If the input was HIGH in both cases, then we accept the   *
 *           input and cycle modes.                                    *
 *           V1.1: added CDS cell and non-blocking delays              *
 *                                                                     *
 * No part of this document may be used, distributed, or reproduced    *
 * without written permission from its author.                         *
 ***********************************************************************/
// serial RGB LED controller interface
#include <Adafruit_NeoPixel.h>
// accompanying color definitions
#include "NeoColors.h"

void cyclePattern(void);
void setLights(void);
void debounceRead(int pin);
int findMax(int array[]);

// defaults
#define ON     true
#define OFF    false
#define DEFAULT_STATE {RED[0], RED[1], RED[2]}
#define T_rf       500
#define CDS_CUTOFF 250
#define CDS_DELAY  15

// brightness options
#define MAXED    255
#define STANDARD 127
#define DIMMED    64
#define PWR_SV    16

// maximum amount of supported modes (including 0)
#define MAX_MODES  6

// pin assignments
#define LED_PIN    0
#define BTN_PIN    1
#define CDS_PIN    1 //analog1 - pin #2

// state machine variables
unsigned char currentMode  = 0;
unsigned char lightState[] = DEFAULT_STATE;
int           btnState;
bool          blinkStage   = OFF;
bool          lightEN      = OFF;
int           lastBtnState = LOW;
long          lastDbTime   = 0;
long          dbDelay      = 20;
unsigned long prevTime     = millis(); // initialize the refresh clock

// CDS sensor variables
int sensorReads[15]   = {};
unsigned long timeOld = millis();
char count            = 0;

// objects
Adafruit_NeoPixel lights = Adafruit_NeoPixel(2, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  pinMode(BTN_PIN, INPUT);
  pinMode(CDS_PIN, INPUT);
  lights.begin();
  lights.setBrightness(PWR_SV);
  lights.show();
}

// non-blocking scheme for updating LEDs
// it's comprised of an ON cycle and an OFF cycle
// during the ON portion, the color determined by the button is applied
void loop() {
  // take care of button input
  debounceRead(BTN_PIN);
  // evaluate surrounding light conditions
  float maxVal = 0;
  if((millis() - timeOld) > CDS_DELAY) {
    if(count < 15) {
      timeOld = millis();
      sensorReads[count] = analogRead(CDS_PIN);
      count++;
    }
    else {
      timeOld = millis();
      maxVal = findMax(sensorReads);
      if(maxVal < CDS_CUTOFF) 
        lightEN = ON;
      else
        lightEN = OFF; 
      count = 0;
    }
  }
  // light refresh handling
  if((millis() - prevTime) > T_rf) {
    prevTime = millis(); blinkStage = !blinkStage;
    // light enable based on the ambient light...like an auto switch
    if(lightEN == OFF) {
      //lights.clear(); lights.show();
      digitalWrite(LED_PIN, LOW);
    }
    else {
      if(blinkStage) { // on cycle
        /*setLights();
        lights.setPixelColor(0, lightState[0], lightState[1], lightState[2]);
        lights.setPixelColor(1, lightState[0], lightState[1], lightState[2]);
        lights.show();*/
        digitalWrite(LED_PIN, HIGH);
      }
      else { // off cycle
        /*lightState[0] = 0; lightState[1] = 0; lightState[2] = 0;
        lights.setPixelColor(0, lightState[0], lightState[1], lightState[2]);
        lights.setPixelColor(1, lightState[0], lightState[1], lightState[2]);
        lights.show();*/
        digitalWrite(LED_PIN, LOW);
      }
    }
  }
}

void cyclePattern(void) {
  currentMode++;
  if(currentMode >= MAX_MODES)
    currentMode = 0;
}

// sets the colors for each mode along with the blink period
void setLights(void) {
  switch(currentMode) {
    case 0: lightState[0] = RED[0]; lightState[1] = RED[1]; lightState[2] = RED[2]; 
    break;
    case 1: lightState[0] = PINK[0]; lightState[1] = PINK[1]; lightState[2] = PINK[2];
    break;
    case 2: lightState[0] = BLUE[0]; lightState[1] = BLUE[1]; lightState[2] = BLUE[2];
    break;
    case 3: lightState[0] = CYAN[0]; lightState[1] = CYAN[1]; lightState[2] = CYAN[2];
    break;
    case 4: lightState[0] = GREEN[0]; lightState[1] = GREEN[1]; lightState[2] = GREEN[2];
    break;
    case 5: lightState[0] = YELLOW[0]; lightState[1] = YELLOW[1]; lightState[2] = YELLOW[2];
    break;
    case 255: lightState[0] = 0; lightState[1] = 0; lightState[2] = 0;

    break;
    default: lightState[0] = PURPLE[0]; lightState[1] = PURPLE[1]; lightState[2] = PURPLE[2];
    break;
  }
}

// variable-rate debouncer on any given pin. the rate is determined by dbDelay
void debounceRead(int pin) {
  int reading = digitalRead(pin);
  // if the button changed in any way, be it noise or a press...
  if (reading != lastBtnState) {
    // start the timer
    lastDbTime = millis();
  }
  // if the button has stayed HIGH longer than a threshold...
  if ((millis() - lastDbTime) > dbDelay) {
    // update the state of the button
    if (reading != btnState) {
      btnState = reading;
      // if the button stayed HIGH, then we assume it's a valid press
      if (btnState == HIGH) {
        cyclePattern();
      }
    }
  }
  // this prevents one button press from being perceived as multiple presses
  lastBtnState = reading;
}

// finds the maximum of an arbitrary-length array
int findMax(int array[]){
  char i;
  int big = array[0];
  for(i = 0; i < (sizeof(array)/sizeof(char)); i++){
    if(array[i] > big)
      big = array[i]; 
  }
  return big;
}

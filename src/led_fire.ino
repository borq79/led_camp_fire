#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define EMBERS_WS2812B_PIN 5
#define SPARKS_LED_PWM_PIN 6
#define NUM_OF_PIXELS      25
#define SECTIONS           4
#define RAIL_MAX           5
#define COMM_SPEED         115200
#define TIME_QUANTA        10
#define MAX_DURATION_STEP  500

struct COLOR {
  byte r;
  byte g;
  byte b;
};

COLOR rail[] = {  {0xFF, 0x00, 0x00},
                  {0xFF, 0x4D, 0x00},
                  {0xFF, 0x5A, 0x14},
                  {0xFF, 0x6D, 0x31},
                  {0xFF, 0x79, 0x43},
                  {0xFF, 0x91, 0x67},
                  {0xFF, 0xA4, 0x81},
                  {0xFF, 0xB6, 0x99},
                  {0xFF, 0xC8, 0xB5},
                  {0xFF, 0xFF, 0xFF} };

const int numberPerSection = NUM_OF_PIXELS / SECTIONS;

struct HEAT_SLIDE_PER_SECTION {
  byte railIndex;
  byte railMax;
  long durationAtEachStep;
  long durationElapsed;
};
HEAT_SLIDE_PER_SECTION heatSlides[SECTIONS];


// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_OF_PIXELS, EMBERS_WS2812B_PIN, NEO_GRB + NEO_KHZ800);

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

void initHeatState(int i) {
  heatSlides[i].railIndex  = 0;
  heatSlides[i].railMax = random(0, RAIL_MAX - 1);
  heatSlides[i].durationAtEachStep = random(TIME_QUANTA, MAX_DURATION_STEP);
  heatSlides[i].durationElapsed = 0;
}

int sparkFlare = 0;
int sparkFlareDuration = 0;
int sparkFlareElapsed = 0;

void setup() {
  Serial.begin(COMM_SPEED);

  // This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
  #if defined (__AVR_ATtiny85__)
    if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
  #endif

  // Clear out the pixel array
  strip.begin();
  for(int i = 0; i < NUM_OF_PIXELS; i++) {
    strip.setPixelColor(i, 0xFF, 0x00, 0x00);
  }
  strip.show();


  pinMode(SPARKS_LED_PWM_PIN, OUTPUT);
  randomSeed(analogRead(0));

  for(int i = 0; i < SECTIONS; i++) {
    initHeatState(i);
  }

  sparkFlareElapsed = 0;
  sparkFlareDuration = random(1000, 5000);
  sparkFlare = random(64, 255);
  analogWrite(SPARKS_LED_PWM_PIN, sparkFlare);
}



void loop() {
  // Reset any items that need to be reset
  for(int i = 0; i < SECTIONS; i++) {
    int durationExpected = (heatSlides[i].railMax - heatSlides[i].railIndex) * heatSlides[i].durationAtEachStep;

    if (heatSlides[i].durationElapsed > durationExpected) {
      initHeatState(i);
    } else if (heatSlides[i].durationElapsed % heatSlides[i].durationAtEachStep == 0){
      int pixelIndex = i * numberPerSection; //Serial.print("Pixel base: "); Serial.println(pixelIndex);
      COLOR c = rail[heatSlides[i].railIndex++];
      for(int j = 0; j < numberPerSection; j++, pixelIndex++) {
          //Serial.println(pixelIndex); Serial.print("COLOR: "); Serial.print(i); Serial.print(" : " ); Serial.print(c.r); Serial.print(" : "); Serial.print(c.g); Serial.print(" : "); Serial.println(c.b);
          strip.setPixelColor(pixelIndex, c.r, c.g, c.b);
      }
    } else {
      heatSlides[i].durationElapsed += TIME_QUANTA;
    }
  }

  strip.show();

  if (sparkFlareElapsed > sparkFlareDuration) {
    sparkFlareElapsed = 0;
    sparkFlareDuration = random(100, 1500);
    sparkFlare = random(0, 255);
    analogWrite(SPARKS_LED_PWM_PIN, sparkFlare);
    //Serial.print("SPARK: "); Serial.println(sparkFlare);
  } else {
    sparkFlareElapsed += TIME_QUANTA;
  }
  
  delay(TIME_QUANTA);
}

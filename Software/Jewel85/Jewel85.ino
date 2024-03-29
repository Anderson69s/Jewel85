//Thank To Adafruit for NeoPixel Library and Jeff Saltzman for his awsome code !!!
//https://learn.adafruit.com/adafruit-neopixel-uberguide
// One press on push button = Change Led color/animation
// Hold the button = increase brightness
// Hold more for more brightness..
// Double press to decrease the brightness.

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

// Pattern types supported:
enum  pattern { NONE, RAINBOW_CYCLE, THEATER_CHASE, COLOR_WIPE, SCANNER, FADE };
// Patern directions supported:
enum  direction { FORWARD, REVERSE };
float potard;// declare float to store potientiometer value

// NeoPattern Class - derived from the Adafruit_NeoPixel class
class NeoPatterns : public Adafruit_NeoPixel
{
  public:

    // Member Variables:
    pattern  ActivePattern;  // which pattern is running
    direction Direction;     // direction to run the pattern

    unsigned long Interval;   // milliseconds between updates
    unsigned long lastUpdate; // last update of position

    uint32_t Color1, Color2;  // What colors are in use
    uint16_t TotalSteps;  // total number of steps in the pattern
    uint16_t Index;  // current step within the pattern

    void (*OnComplete)();  // Callback on completion of pattern

    // Constructor - calls base-class constructor to initialize strip
    NeoPatterns(uint16_t pixels, uint8_t pin, uint8_t type, void (*callback)())
      : Adafruit_NeoPixel(pixels, pin, type)
    {
      OnComplete = callback;
    }

    // Update the pattern
    void Update()
    {
      if ((millis() - lastUpdate) > Interval) // time to update
      {
        lastUpdate = millis();
        switch (ActivePattern)
        {
          case RAINBOW_CYCLE:
            RainbowCycleUpdate();
            break;
          case THEATER_CHASE:
            TheaterChaseUpdate();
            break;
          case COLOR_WIPE:
            ColorWipeUpdate();
            break;
          case SCANNER:
            ScannerUpdate();
            break;
          case FADE:
            FadeUpdate();
            break;
          default:
            break;
        }
      }
    }

    // Increment the Index and reset at the end
    void Increment()
    {
      if (Direction == FORWARD)
      {
        Index++;
        if (Index >= TotalSteps)
        {
          Index = 0;
          if (OnComplete != NULL)
          {
            OnComplete(); // call the comlpetion callback
          }
        }
      }
      else // Direction == REVERSE
      {
        --Index;
        if (Index <= 0)
        {
          Index = TotalSteps - 1;
          if (OnComplete != NULL)
          {
            OnComplete(); // call the comlpetion callback
          }
        }
      }
    }

    // Reverse pattern direction
    void Reverse()
    {
      if (Direction == FORWARD)
      {
        Direction = REVERSE;
        Index = TotalSteps - 1;
      }
      else
      {
        Direction = FORWARD;
        Index = 0;
      }
    }

    // Initialize for a RainbowCycle
    void RainbowCycle(uint8_t interval, direction dir = FORWARD)
    {
      ActivePattern = RAINBOW_CYCLE;
      Interval = interval;
      TotalSteps = 255;
      Index = 0;
      Direction = dir;
    }

    // Update the Rainbow Cycle Pattern
    void RainbowCycleUpdate()
    {
      for (int i = 0; i < numPixels(); i++)
      {
        setPixelColor(i, Wheel(((i * 256 / numPixels()) + Index) & 255));
      }
      show();
      Increment();
    }

    // Initialize for a Theater Chase
    void TheaterChase(uint32_t color1, uint32_t color2, uint8_t interval, direction dir = FORWARD)
    {
      ActivePattern = THEATER_CHASE;
      Interval = interval;
      TotalSteps = numPixels();
      Color1 = color1;
      Color2 = color2;
      Index = 0;
      Direction = dir;
    }

    // Update the Theater Chase Pattern
    void TheaterChaseUpdate()
    {
      for (int i = 0; i < numPixels(); i++)
      {
        if ((i + Index) % 3 == 0)
        {
          setPixelColor(i, Color1);
        }
        else
        {
          setPixelColor(i, Color2);
        }
      }
      show();
      Increment();
    }

    // Initialize for a ColorWipe
    void ColorWipe(uint32_t color, uint8_t interval, direction dir = FORWARD)
    {
      ActivePattern = COLOR_WIPE;
      Interval = interval;
      TotalSteps = numPixels();
      Color1 = color;
      Index = 0;
      Direction = dir;
    }

    // Update the Color Wipe Pattern
    void ColorWipeUpdate()
    {
      setPixelColor(Index, Color1);
      show();
      Increment();
    }

    // Initialize for a SCANNNER
    void Scanner(uint32_t color1, uint8_t interval)
    {
      ActivePattern = SCANNER;
      Interval = interval;
      TotalSteps = (numPixels() - 1) * 2;
      Color1 = color1;
      Index = 0;
    }

    // Update the Scanner Pattern
    void ScannerUpdate()
    {
      for (int i = 0; i < numPixels(); i++)
      {
        if (i == Index)  // Scan Pixel to the right
        {
          setPixelColor(i, Color1);
        }
        else if (i == TotalSteps - Index) // Scan Pixel to the left
        {
          setPixelColor(i, Color1);
        }
        else // Fading tail
        {
          setPixelColor(i, DimColor(getPixelColor(i)));
        }
      }
      show();
      Increment();
    }

    // Initialize for a Fade
    void Fade(uint32_t color1, uint32_t color2, uint16_t steps, uint8_t interval, direction dir = FORWARD)
    {
      ActivePattern = FADE;
      Interval = interval;
      TotalSteps = steps;
      Color1 = color1;
      Color2 = color2;
      Index = 0;
      Direction = dir;
    }

    // Update the Fade Pattern
    void FadeUpdate()
    {
      // Calculate linear interpolation between Color1 and Color2
      // Optimise order of operations to minimize truncation error
      uint8_t red = ((Red(Color1) * (TotalSteps - Index)) + (Red(Color2) * Index)) / TotalSteps;
      uint8_t green = ((Green(Color1) * (TotalSteps - Index)) + (Green(Color2) * Index)) / TotalSteps;
      uint8_t blue = ((Blue(Color1) * (TotalSteps - Index)) + (Blue(Color2) * Index)) / TotalSteps;

      ColorSet(Color(red, green, blue));
      show();
      Increment();
    }

    // Calculate 50% dimmed version of a color (used by ScannerUpdate)
    uint32_t DimColor(uint32_t color)
    {
      // Shift R, G and B components one bit to the right
      uint32_t dimColor = Color(Red(color) >> 1, Green(color) >> 1, Blue(color) >> 1);
      return dimColor;
    }

    // Set all pixels to a color (synchronously)
    void ColorSet(uint32_t color)
    {
      for (int i = 0; i < numPixels(); i++)
      {
        setPixelColor(i, color);
      }
      show();
    }

    // Returns the Red component of a 32-bit color
    uint8_t Red(uint32_t color)
    {
      return (color >> 16) & 0xFF;
    }

    // Returns the Green component of a 32-bit color
    uint8_t Green(uint32_t color)
    {
      return (color >> 8) & 0xFF;
    }

    // Returns the Blue component of a 32-bit color
    uint8_t Blue(uint32_t color)
    {
      return color & 0xFF;
    }

    // Input a value 0 to 255 to get a color value.
    // The colours are a transition r - g - b - back to r
    // We add Potard to control the brightness
    uint32_t Wheel(byte WheelPos)
    {
      WheelPos = 255 - WheelPos;
      if (WheelPos < 85)
      {
        return Color((255 - WheelPos * 3 ) * potard, 0, (WheelPos * 3 ) * potard);
      }
      else if (WheelPos < 170)
      {
        WheelPos -= 85;
        return Color(0, (WheelPos * 3) * potard, (255 - WheelPos * 3) * potard);
      }
      else
      {
        WheelPos -= 170;
        return Color((WheelPos * 3) * potard, (255 - WheelPos * 3) * potard, 0);
      }
    }
};

void Ring1Complete();

// Define some NeoPatterns for the two rings and the stick
//  as well as some completion routines
NeoPatterns Ring1(6, 4, NEO_GRBW + NEO_KHZ800, &Ring1Complete);
// Initialize everything and prepare to start
int showType = 0;//initialize to the first animation

void setup()
{
#if defined (__AVR_ATtiny85__)
  if (F_CPU == 8000000) clock_prescale_set(clock_div_1);
#endif
  pinMode(3, INPUT);
  digitalWrite(3, HIGH );//set D3 to HIGH
  potard =  0.05;// value brightness ( between 0 - 1)
  // Initialize all the pixelStri1ps
  Ring1.begin();
  Ring1.ActivePattern = NONE;
  Ring1.ColorSet(Ring1.Color(0, 0, 0, 255 * potard));
}

// Main loop
void loop()
{
  // Update the rings.
  Ring1.Update();
  int b = checkButton();// Check Button for event and go to the event
  if (b == 1) clickEvent();
  if (b == 2) doubleClickEvent();
  if (b == 3) holdEvent();
  if (b == 4) longHoldEvent();
}
void clickEvent() {// Change animation
  showType++;
  if (showType > 9)
    showType = 0;
  startShow(showType);
}
void doubleClickEvent() { //Change to Mininum Luminosity
  potard =  0.05;
  startShow(showType);
}
void holdEvent() { //Change to Medium Luminosity
  potard =  0.35;
  startShow(showType);
}
void longHoldEvent() { //Change to Maximum Luminosity
  potard =  1;
  startShow(showType);
}
void startShow(int i) { //Chose between the animation
  switch (i) {
    case 0:
      //WHITE
      Ring1.ActivePattern = NONE;
      Ring1.ColorSet(Ring1.Color(0, 0, 0, 255 * potard));
      break;
    case 1:
      Ring1.TotalSteps = 255;
      Ring1.Interval = 1;
      Ring1.ActivePattern = RAINBOW_CYCLE;
      break;
    case 2:
      Ring1.TotalSteps = 10;
      Ring1.Color1 = Ring1.Color(255 * potard, 0, 0, 0);
      Ring1.Interval = 25;
      Ring1.ActivePattern = FADE;
      break;
    case 3:
      Ring1.TotalSteps = 10;
      Ring1.Color1 = Ring1.Color(0, 255 * potard, 0);
      Ring1.Interval = 250;
      Ring1.ActivePattern = FADE;
      break;
    case 4:
      Ring1.TotalSteps = 5;
      Ring1.Color1 = Ring1.Color(0, 0, 255 * potard, 0);
      Ring1.Interval = 25;
      Ring1.ActivePattern = SCANNER;
      Ring1.Update();
      break;
    case 5:
      Ring1.TotalSteps = 2;
      Ring1.Color1 = Ring1.Color(0, 0, 0, 255 * potard );
      Ring1.Interval = 250;
      Ring1.ActivePattern = SCANNER;
      Ring1.Update();
      break;
    case 6:
      Ring1.TotalSteps = 5;
      Ring1.Interval = 25;
      Ring1.Color2 = Ring1.Color(0, 0, 0, 0);
      Ring1.Color1 = Ring1.Color(0, 255 * potard, 255 * potard, 0);
      Ring1.ActivePattern = THEATER_CHASE;
      break;
    case 7:
      Ring1.TotalSteps = 5;
      Ring1.Interval = 250;
      Ring1.Color2 = Ring1.Color(0, 0, 0, 0);
      Ring1.Color1 = Ring1.Color(255 * potard, 0, 255 * potard, 0);
      Ring1.ActivePattern = THEATER_CHASE;
      break;
    case 8:
      Ring1.TotalSteps = 5;
      Ring1.Interval = 25;
      Ring1.Color2 = Ring1.Color(0, 0, 0, 0);
      Ring1.Color1 = Ring1.Color(255 * potard, 255 * potard, 0, 0);
      Ring1.ActivePattern = THEATER_CHASE;
      break;
    case 9:
      Ring1.TotalSteps = 5;
      Ring1.Interval = 250;
      Ring1.Color2 = Ring1.Color(0, 0, 0, 0);
      Ring1.Color1 = Ring1.Color(0, 0, 0, 255 * potard);
      Ring1.ActivePattern = THEATER_CHASE;
      break;
  }
}


//-----------------------------------------------------------
//Completion Routines - get called on completion of a pattern
//------------------------------------------------------------

// Ring1 Completion Callback
void Ring1Complete()
{
  //Ring1.Reverse();
}

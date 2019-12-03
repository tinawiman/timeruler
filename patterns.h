// add SUNRISE -- maybe add "don't interrupt" to  sunrise
//  add FADE_TO (fade every pixel from it's current color (incl brightness settings) to target color
//  add default completion tasks to class, that can be called from onComplete-void
// rainbow_wipe
//rainbow_scanner
// random_Fade
// add colorway variable? for rainbow effects. or use sequence

#include "colornames.h" // in it's own tab


// Pattern types supported:
enum  pattern {NONE, PAUSE, RAINBOW_CYCLE, THEATER_CHASE, COLOR_WIPE, SCANNER, FADE, FLICKER, FLICKER_FADE, FADE_TO, OMBRE, OMBRE_CYCLE, BREATH, SUNSET, SUNRISE, COUNTDOWN};

//Combine patterns in specific sequences xxx implement -- or color sequences?
enum sequence {S_NONE, S_SUNRISE, S_SUNSET, S_RAINBOW};

// Pattern directions supported:
enum  direction { FORWARD, REVERSE };


// NeoPattern Class - derived from the Adafruit_NeoPixel class
class NeoPatterns : public Adafruit_NeoPixel
{
  public:
    // Member Variables:
    pattern  ActivePattern;  // which pattern is running
    direction Direction;     // direction to run the pattern
    sequence ActiveSequence; // which sequence is running xxx

    unsigned long Interval;   // milliseconds between updates
    unsigned long lastUpdate; // last update of position
    unsigned long TotalTime;   // for calculating Interval in some fade patterns etc

    uint32_t Color1, Color2;  // What colors are in use
    uint16_t TotalSteps;  // total number of steps in the pattern
    uint16_t Index;  // current step within the pattern

    uint8_t Flavor; // some kind of choice, for example color scheme

    void (*OnComplete)();  // Callback on completion of pattern // that in turn can and should call default DefaultComplete in class

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
          // no case for PAUSE - just stop right in the middle
          // no case for NONE - initializer clears all pixels
          case SUNSET:
            SunsetUpdate();
            break;
          case SUNRISE:
            SunriseUpdate();
            break;
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
          case FLICKER_FADE:
            FlickerFadeUpdate();
            break;
          case FLICKER:
            FlickerUpdate();
            break;
          case OMBRE:
            OmbreUpdate();
            break;
          case OMBRE_CYCLE:
            OmbreCycleUpdate();
            break;
          case BREATH:
            BreathUpdate();
            break;
          case COUNTDOWN:
            CountdownUpdate();
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

    // on complete i e Index >= TotalSteps
    void DefaultComplete() {
      // default completion can be called from complete-void in main code
      // no case for NONE, STOP, PAUSE
      if (ActivePattern == RAINBOW_CYCLE) Index = 0; //start over again
      else if (ActivePattern == THEATER_CHASE) Index = 0; //start over again
      else if (ActivePattern == COLOR_WIPE) Color1 = Wheel(random(255));  // start again with random color
      else if (ActivePattern == SCANNER) Color1 = Wheel(random(255)); // start again with random color
      else if (ActivePattern == FADE) ActivePattern = PAUSE; //just stay
      else if (ActivePattern == OMBRE) ActivePattern = PAUSE; //just stay
      else if (ActivePattern == OMBRE_CYCLE) Reverse(); //change direction
      else if (ActivePattern == BREATH) Reverse(); //change direction
      else if (ActivePattern == FLICKER) Index = 0; //start over again
      else if (ActivePattern == FLICKER_FADE) { //go to flicker
        if (Direction == FORWARD) Flicker(Color2, Interval * 2);
        else Flicker(Color1, Interval * 2);
      }
      else if (ActivePattern == SUNRISE) ActivePattern = PAUSE; //just stay
      else if (ActivePattern == SUNSET) NoPattern(); //all lights out
      else if (ActivePattern == FADE_TO) ActivePattern = PAUSE; //just stay
      else if (ActivePattern == COUNTDOWN) {
        if (Direction == FORWARD) {
          ColorAll(0);
          Serial.println("Countdown finished");
        }
        else {
          CountdownFill();
          Serial.println("Countup finished");
        }
        show();
        ActivePattern = PAUSE; //just stay, not none bc countdown reversed is a countup
      }
    }


    // Initialize for no pattern - clears strip. No update void necessary
    void NoPattern() {
      ActivePattern = NONE;
      TotalSteps = numPixels();
      ColorAll(0);
    }



    // Initialize for a RainbowCycle
    void RainbowCycle(unsigned long interval, direction dir = FORWARD)
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
    void TheaterChase(uint32_t color1, uint32_t color2, unsigned long interval, direction dir = FORWARD)
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
    void ColorWipe(uint32_t color, unsigned long interval, direction dir = FORWARD)
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
    void Scanner(uint32_t color1, unsigned long interval)
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

      for (uint8_t i = 0; i < numPixels(); i++)
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



    // Initialize for Fade
    void Fade(uint32_t color1, uint32_t color2, uint8_t totalMinutes, direction dir = FORWARD)
    {
      ActivePattern = FADE;
      TotalTime = totalMinutes * 1000 * 60; // totalTime in millis
      TotalSteps = 255;
      Interval = TotalTime / TotalSteps; // depends on pattern code
      Index = 0;
      Direction = dir;
      Color1 = color1;
      Color2 = color2;
    }

    // Update the Fade Pattern
    void FadeUpdate()
    {
      uint32_t stepColor = StepColor(Index, TotalSteps);
      uint8_t r = Red(stepColor);
      uint8_t g = Green(stepColor);
      uint8_t b = Blue(stepColor);

      ColorAll(Color(r, g, b));
      show();
      Increment();
    }



    // Initialize for Flicker pattern
    void Flicker(uint32_t color1, unsigned long interval, direction dir = FORWARD)
    {
      ActivePattern = FLICKER;
      TotalSteps = numPixels();
      Interval = interval / 2; // random flickering 1 time out of 4 => mean = interval*2
      Color1 = color1;
      Index = 0;
      Direction = dir;
      ColorAll(Color1);
    }

    // Update the Flicker Pattern
    void FlickerUpdate()
    {
      if (random(0, 3) == 0) { // flickers unevenly, at time = (random+1)*Interval
        uint16_t p = random(0, numPixels());
        uint32_t colorHue = HuedColor(Color1, 25, 0);
        setPixelColor(p, colorHue);
        show();
        Increment();
      }
    }



    // Initialize for a FlickerFade
    void FlickerFade(uint32_t color1, uint32_t color2, uint8_t totalMinutes, direction dir = FORWARD)
    {
      ActivePattern = FLICKER_FADE;
      TotalTime = totalMinutes * 1000 * 60; // totalTime in millis
      TotalSteps = 255 * numPixels(); // fade in 255 steps and randomly FlickerFade number of pixels for each step
      Interval = totalMinutes * 118 / numPixels(); // mean time because of random flicker function 1000*60/255/2 = 117,6
      Color1 = color1; // start with color
      Color2 = color2; // move towards color
      Direction = dir;
      Index = 0;
      ColorAll(Color1);
    }

    // Update the FlickerFade Pattern
    void FlickerFadeUpdate() {
      if (random(0, 3) == 0) { // flickers unevenly, at time = (random+1)*Interval
        uint16_t p = random(0, numPixels());
        uint32_t stepColor = StepColor(Index, TotalSteps);
        uint32_t colorHue = HuedColor(stepColor, 50, 0);
        setPixelColor(p, colorHue);
        show();
        Increment();
      }
    }



    // Initialize for an ombre
    void Ombre(uint32_t color1, uint32_t color2, unsigned long interval, direction dir = FORWARD)
    {
      ActivePattern = OMBRE;
      Interval = interval;
      TotalSteps = numPixels();
      Color1 = color1;
      Color2 = color2;
      Index = 0;
      Direction = dir;
      OmbreUpdate(); // fill at once
    }

    // Update the Ombre Pattern
    void OmbreUpdate()
    {
      for (int i = 0; i < numPixels(); i++)
      {
        setPixelColor(i, StepColor(i, TotalSteps));
      }
      show();
      Increment();
    }



    // Initialize for an ombre Cycle
    void OmbreCycle(uint32_t color1, uint32_t color2, unsigned long interval, direction dir = FORWARD)
    {
      ActivePattern = OMBRE_CYCLE;
      Interval = interval;
      TotalSteps = numPixels();
      Color1 = color1;
      Color2 = color2;
      Index = 0;
      Direction = dir;
      OmbreCycleUpdate(); // fill at once
    }

    // Update the Ombre cycle Pattern
    void OmbreCycleUpdate()
    {
      for (int i = 0; i < numPixels(); i++)
      {
        setPixelColor(i, StepColor((i + numPixels()), TotalSteps)); //   xxx not finished
      }
      show();
      Increment();
    }



    // Initialize for a breath pattern
    void Breath(uint32_t color1, uint32_t color2, unsigned long interval, direction dir = FORWARD)
    {
      ActivePattern = BREATH;
      Interval = interval;
      TotalSteps = numPixels();
      Color1 = color1;
      Color2 = color2;
      Index = 0;
      Direction = dir;
      BreathUpdate(); // fill at once
    }

    // Update the breath Pattern
    void BreathUpdate()
    {
      for (int i = 0; i < numPixels(); i++)
      {
        setPixelColor(i, StepColor((((i / numPixels()) + Index) & 255), TotalSteps)); // xxx simplify!
      }
      show();
      Increment();
    }



    // Initialize for a flickering Sunset
    void Sunset(uint8_t totalMinutes, direction dir = FORWARD)
    {
      ActivePattern = SUNSET;
      TotalTime = totalMinutes * 1000 * 60; // totalTime in millis
      TotalSteps = 255 * numPixels(); // fade in 255 steps and randomly FlickerFade number of pixels for each step
      Interval = totalMinutes * 118 / numPixels(); // mean time because of random flicker function 1000*60/255/2 = 117,6
      Color1 = Color(255, 204, 32);
      Color2 = BLACK;
      Direction = dir;
      Index = 0;
      ColorAll(Color1);
    }

    // Update the FlickerFade Pattern
    void SunsetUpdate() {
      if (random(0, 3) == 0) { // flickers unevenly, at time = (random+1)*Interval
        uint16_t p = random(0, numPixels());
        uint32_t stepColor = SunsetColor(Index, TotalSteps);
        uint32_t colorHue = HuedColor(stepColor, 50, 1);
        setPixelColor(p, colorHue);
        show();
        Increment();
      }
    }


    // Initialize for Fade sunrise
    void Sunrise(uint8_t totalMinutes, direction dir = FORWARD)
    {
      ActivePattern = SUNRISE;
      TotalTime = totalMinutes * 1000 * 60; // totalTime in millis
      TotalSteps = 255;
      Interval = TotalTime / TotalSteps; // depends on pattern code
      Index = 0;
      Direction = dir;
    }

    // Update the Fade sunrise Pattern
    void SunriseUpdate()
    {
      uint32_t stepColor = SunriseColor(Index, TotalSteps);
      uint8_t r = Red(stepColor);
      uint8_t g = Green(stepColor);
      uint8_t b = Blue(stepColor);

      ColorAll(Color(r, g, b));
      show();
      Increment();
    }



    // Initialize for Countdown
    void Countdown(uint8_t totalMinutes, uint16_t totalPixels, uint8_t flavorChoice, direction dir) //   Countdown(uint8_t totalMinutes, uint16_t totalPixels, uint8_t flavorChoice, direction dir)

    {
      ActivePattern = COUNTDOWN;
      TotalTime = totalMinutes * 1000 * 60; // totalTime in millis
      if (totalPixels == 0) {
        TotalSteps = numPixels();
      }
      else
      {
        TotalSteps = totalPixels;
      }
      Interval = TotalTime / TotalSteps; // depends on pattern code
      Flavor = flavorChoice; // 0 = rainbow wheel, 1-19 = different single colors, 20-- different ombre colors
      Direction = dir;
      // clear all pixels
      setColors();

      ColorAll(0);

      if (Direction == FORWARD) {
        Index = 0;
        CountdownFill(); // show filled
        show();
        Serial.println("Countdown initialized");
      }
      else
      {
        Index = TotalSteps;
        Serial.println("Countup initialized");
      }
    }

    // Update for Countdown
    void CountdownUpdate() {
      // color all with chosen flavor/color scheme
      CountdownFill();
      // turn step pixels off
      for (uint16_t p = 0; p < Index; p++) {
        setPixelColor(TotalSteps - p - 1, 0);
      }
      show();
      Serial.print("Countdown in progress: "); Serial.print(Index); Serial.print(" of "); Serial.println(TotalSteps);

      Increment();

    }

    void CountdownFill() {
      // color all with chosen flavor/color scheme
      for (uint16_t i = 0; i < TotalSteps; i++) {
        if (Flavor == 0)
        {
          setPixelColor(i, Wheel(((i * 256 / TotalSteps)) & 255)); // rainbow wheel
        }
        else if (Flavor > 0) { // 1 - 19 = single colors color1=color 2, 20-- ombre colors color1 - color2
          setPixelColor(i, StepColor(i, TotalSteps));
        }
      }
    }

    // change flavor i e colors without changeing pattern
    void setFlavor(uint8_t flavorChoice) {
      Flavor = flavorChoice;
      setColors();
    }
    // set colors from flavor variable
    void setColors() {
      // Flavor 0 = rainbow wheel
      if (Flavor < 100) {
        switch (Flavor)
        {
          case 19:
            Color2 = RED;
            Color1 = GREEN;
            break;
          case 20:
            Color2 = GREEN;
            Color1 = RED;
            break;

          case 21:
            Color2 = PURPLE;
            Color1 = PINK;
            break;
          case 22:
            Color2 = COLDRED;
            Color1 = BLUE;
            break;
          case 23:
            Color2 = BROWN;
            Color1 = YELLOW;
            break;
          case 24:
            Color2 = MAGENTA;
            Color1 = SEAGREEN;
            break;
          case 25:
            Color2 = VIOLET;
            Color1 = SAGE;
            break;
          case 26:
            Color2 = ORANGE;
            Color1 = VIOLET;
            break;
          case 27:
            Color2 = MAGENTA;
            Color1 = YELLOW;
            break;
          case 28:
            Color2 = LAWNGREEN;
            Color1 = LIGHTCYAN;
            break;
          case 29:
            Color2 = GREEN;
            Color1 = CYAN;
            break;
          case 30:
            Color2 = BROWN;
            Color1 = BLUE;
            break;
          default: // none of the above
            Flavor = 0;
            break;
        }
      }
      else if (Flavor > 100) {
        switch (Flavor)
        {
          // SAGE,LIGHTGREEN, LAWNGREEN, GREEN, SEAGREEN
          case 101:
            Color1 = SAGE;
            break;
          case 102:
            Color1 = LIGHTGREEN;
            break;
          case 103:
            Color1 = LAWNGREEN;
            break;
          case 104:
            Color1 = GREEN;
            break;
          case 105:
            Color1 = SEAGREEN;
            break;

          // TURQUOISE, TURQUOISE2, LIGHTCYAN, CYAN, BLUE
          case 106:
            Color1 = TURQUOISE;
            break;
          case 107:
            Color1 = TURQUOISE2;
            break;
          case 108:
            Color1 = LIGHTCYAN;
            break;
          case 109:
            Color1 = CYAN;
            break;
          case 110:
            Color1 = BLUE;
            break;

          // BLUEVIOLET, VIOLET, PURPLE
          case 111:
            Color1 = BLUEVIOLET;
            break;
          case 112:
            Color1 = VIOLET;
            break;
          case 113:
            Color1 = PURPLE;
            break;

          // MAGENTA, PINK, COLDRED, RED, LIGHTPINK, WARMPINK
          case 114:
            Color1 = MAGENTA;
            break;
          case 115:
            Color1 = PINK;
            break;
          case 116:
            Color1 = COLDRED;
            break;
          case 117:
            Color1 = RED;
            break;
          case 118:
            Color1 = LIGHTPINK;
            break;
          case 119:
            Color1 = WARMPINK;
            break;

          // APRICOT, LIGHTORANGE, ORANGE, YELLOW, BROWN
          case 120:
            Color1 = APRICOT;
            break;
          case 121:
            Color1 = LIGHTORANGE;
            break;
          case 122:
            Color1 = ORANGE;
            break;
          case 123:
            Color1 = YELLOW;
            break;
          case 124:
            Color1 = BROWN;
            break;
          case 125:
            Color1 = WHITE;
            break;

          // default case
          default:
            Color1 = PINK;
            break;
        }
        Color2 = Color1;
      }
      Serial.print("Colors set via flavor "); Serial.print(Flavor); Serial.print(" to 1: "); Serial.print(Color1, HEX); Serial.print(" & 2: ");  Serial.println(Color2, HEX);
    }

    // Send RGBs to serial for debugging
    void PrintRGB(int r, int g, int b) {
      //  Serial.print(r); Serial.print("\t"); Serial.print(g); Serial.print("\t"); Serial.print(b); Serial.print("\t");
    }



    // Set all pixels to a color (synchronously)
    void ColorAll(uint32_t color)
    {
      for (int i = 0; i < numPixels(); i++)
      {
        setPixelColor(i, color);
      }
      show();
    }



    // For a flicker/color hue effect
    uint32_t HuedColor(uint32_t color, uint8_t randomMax, bool trueZero) {
      int flickerNum = random(0, randomMax) - randomMax / 2;
      int r = Red(color);
      int g = Green(color);
      int b = Blue(color);

      int r1 = r - flickerNum; //  + 15; // keep mostly in red hues
      int g1 = g - flickerNum;
      int b1 = b - flickerNum;

      if (trueZero) {
        if (r == 0) r1 = 0;
        if (g == 0) g1 = 0;
        if (b == 0) b1 = 0;
      }

      r1 = ValidColorValue(r1);
      g1 = ValidColorValue(g1);
      b1 = ValidColorValue(b1);

      //PrintRGB(r, g, b); Serial.print("255"); Serial.print("\t"); PrintRGB(r1, g1, b1); Serial.println(); // xxx

      uint32_t colorHue = Color(r1, g1, b1);
      return colorHue;
    }



    uint32_t SunriseColor(uint32_t stepIndex, uint32_t noOfSteps)
    {
      // use local variables like stepIndex and totalColorSteps instead of index and totalsteps to be able to light up consecutive pixels in a fade pattern
      int r = (255 * stepIndex / noOfSteps);
      int g = (225 * stepIndex / (noOfSteps * 2 / 3)) - 225 / 2; // y=225/20x-227/2
      int b = (128 * stepIndex / (noOfSteps / 2)) - 128; // y=128/50x-128 xxx

      uint8_t red = ValidColorValue(r);
      uint8_t green = ValidColorValue(g);
      uint8_t blue = ValidColorValue(b);

      // PrintRGB(r, g, b); Serial.print("\t255\t"); PrintRGB(red, green, blue); Serial.println(); // xxx
      uint32_t colorStep = Color(red, green, blue);
      return colorStep;
    }



    uint32_t SunsetColor(uint32_t stepIndex, uint32_t noOfSteps)
    {
      // use local variables like stepIndex and totalColorSteps instead of index and totalsteps to be able to light up consecutive pixels in a fade pattern
      int r = 255 - (255 * stepIndex / noOfSteps);
      int g = 204 - (204 * 2 * stepIndex / noOfSteps);
      int b = 32 - (32 * 4 * stepIndex / noOfSteps);
      uint8_t red = ValidColorValue(r);
      uint8_t green = ValidColorValue(g);
      uint8_t blue = ValidColorValue(b);

      uint32_t colorStep = Color(red, green, blue);
      return colorStep;
    }



    uint32_t StepColor(uint32_t stepIndex, uint32_t noOfSteps)
    {
      // use local variables like stepIndex and totalColorSteps instead of index and totalsteps to be able to light up consecutive pixels in an ombre fashion
      uint8_t red = ((Red(Color1) * (noOfSteps - stepIndex)) + (Red(Color2) * stepIndex)) / noOfSteps;
      uint8_t green = ((Green(Color1) * (noOfSteps - stepIndex)) + (Green(Color2) * stepIndex)) / noOfSteps;
      uint8_t blue = ((Blue(Color1) * (noOfSteps - stepIndex)) + (Blue(Color2) * stepIndex)) / noOfSteps;
      uint32_t colorStep = Color(red, green, blue);
      // PrintRGB(red, green, blue); Serial.println();
      return colorStep;
    }



    uint8_t ValidColorValue(int colorValue)
    {
      if (colorValue < 0) colorValue = 0;
      if (colorValue > 255) colorValue = 255;
      return colorValue;
    }

    // Calculate 50% dimmed version of a color (used by ScannerUpdate)
    uint32_t DimColor(uint32_t color)
    {
      // Shift R, G and B components one bit to the right
      uint32_t dimColor = Color(Red(color) >> 1, Green(color) >> 1, Blue(color) >> 1);
      return dimColor;
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
    // The colours are a transition r - g - b - back to r.
    uint32_t Wheel(byte WheelPos)
    {
      WheelPos = 255 - WheelPos;
      if (WheelPos < 85)
      {
        return Color(255 - WheelPos * 3, 0, WheelPos * 3);
      }
      else if (WheelPos < 170)
      {
        WheelPos -= 85;
        return Color(0, WheelPos * 3, 255 - WheelPos * 3);
      }
      else
      {
        WheelPos -= 170;
        return Color(WheelPos * 3, 255 - WheelPos * 3, 0);
      }
    }
};

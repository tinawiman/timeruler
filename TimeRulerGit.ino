// a lot of this is Adafruit code, threwn together by tin@w aka Tina Wiman 2019
// For Wemos/Lolin D1 mini or other ESP8266 microcontroller
// and RGB LEDs like Adafruit Neopixels, ie WS2812
//
// Main controller is via URL, find local IP via router and open web page, for example "192.168.1.215" 
//
// ideas and notes:
// xxx make button functions (double clicks, long etc?)
// add serial monitor commands? url? xxx
// add some kind of animation showing countdown/up is still on xxx
// add buzzer and play sounds

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Adafruit_NeoPixel.h>
#include <DHT.h>
#include <OneButton.h>

#include "patterns.h" // in it's own tab
#include "sensors.h" // in it's own tab

////// pins, input, output //////

// Sensors
#define LDR_PIN A0
#define REED_PIN D0
#define PIR1_PIN D1

Sensors ldr(LDR_PIN, 5000, LDR, ANALOG); //     Sensors(uint8_t sensorPin, unsigned long readInterval, sType sensorType, aodType readType)
Sensors reed(REED_PIN, 2000, REED, DIGITAL); //     Sensors(uint8_t sensorPin, unsigned long readInterval, sType sensorType, aodType readType)
Sensors pir(PIR1_PIN, 5000, PIR, DIGITAL); //     Sensors(uint8_t sensorPin, unsigned long readInterval, sType sensorType, aodType readType)

// button xxx add library etc
#define BTN_PIN D2
OneButton button(BTN_PIN, true);
// * pushbutton attached to pin D2 from +3V3
// * 10K resistor attached to pin D2 from ground


// Neopixels
#define NEO_PIN D3
#define PIXELS 34 //36 // 30 // 49
#define BRIGHTNESS 15   // 200
void stripComplete();
//void Ring1Complete();
//void Ring2Complete();
NeoPatterns strip(PIXELS, NEO_PIN, NEO_RGB + NEO_KHZ800, &stripComplete);
int NeoRoutine = 0;
int NeoRoutinePart = 0;

//I2C: DHT sensor
#define DHT_PIN D4
DHT dht(DHT_PIN, DHT11);
long lastDHTtime = 0;
long dhtInterval = 60000; // 60000 = 1 hour. DHT11 takes about 250ms for each measurement + builtin LED will flash every time since shield uses pin D4


// Stepper motor xxx add library etc
#define SM_PIN_1 D5
#define SM_PIN_2 D6
#define SM_PIN_3 D7
#define SM_PIN_4 D8


// internal LED
const int LED_PIN = 14; // inverted to pin D4


////// wifi 'n server stuff //////

const char* ssid = "wifinetworkname";
const char* password = "wifipassword";
ESP8266WebServer server(80);

void handleRoot() {
  // digitalWrite(LED_PIN, 1);
  String message = handleMessage("");
  server.send(200, "text/html", message);
  // digitalWrite(LED_PIN, 0);
}


String handleMessage(String firstRowMessage) {
  String message = "<html><head></head><body style='font-family: sans-serif; font-size: 12px'>";
  message += (firstRowMessage);

  message += "<a href='https://icon-library.net/icon/flat-clock-icon-20.html' title='Flat Clock Icon #69099'><img src='https://icon-library.net//images/flat-clock-icon/flat-clock-icon-20.jpg' width='350' /></a><br><br>";
  message += "Functions:<br><br>";

  message += "<a href = '/countdown?t=60&p=0&f=0'>/countdown</a> Countdown for 1h (rainbow), use all pixels<br>";
  message += "<a href = '/countdown?t=45&p=0&f=0'>/countdown</a> Countdown for 45 minutes (rainbow), use all pixels<br>";
  message += "<a href = '/countdown?t=30&p=24&f=0'>/countdown</a> Countdown for 30 minutes (rainbow), use 24 pixels<br>";
  message += "<a href = '/countdown?t=20&p=20&f=0'>/countdown</a> Countdown for 20 minutes (rainbow), use 20 pixels<br>";
  message += "<a href = '/countdown?t=15&p=20&f=0'>/countdown</a> Countdown for 15 minutes (rainbow), use 20 pixels<br>";
  message += "<a href = '/countdown?t=10&p=12&f=0'>/countdown</a> Countdown for 10 minutes (rainbow), use 12 pixels<br>";
  message += "<a href = '/countdown?t=5&p=10&f=0'>/countdown</a> Countdown for 5 minutes (rainbow), use 10 pixels<br>";
  message += "<br><br>";

  message += "<a href = '/countdown?t=10&p=10&f=106'>/countdown</a> Countdown for 10 minutes (turquoise), only use 10 pixels <br>";
  message += "<a href = '/countup?t=1&p=0&f=20'>/Countup</a> Countup for 1 minute (red - greed) <br>";
  message += "<a href = '/countup?t=5&p=0&f=3'>/countup</a> Countup for 5 minutes (pink)<br>";

  message += "<a href = '/night?t=15&f=20'>/night</a> Start night routine<br>";

  message += "<br><br>";


  message += "<a href = '/sunset?t=15'>/sunset</a> a flickering sunset<br>";
  message += "<a href = '/sunrise?t=30'>/sunrise</a> a slow sunrise fade<br>";
  message += "<br><br>";

  message += "<a href = '/rainbow?i=50'>/rainbow</a> rainbow cycle<br>";
  message += "<a href = '/wipe?i=100&r=200&g=150&b=25'>/wipe</a> color wipe<br>";
  message += "<a href = '/scan?i=50&r=127&g=0&b=125'>/scan</a> color changing scanner wipe<br>";
  message += "<a href = '/chase?i=100&r1=0&g1=20&b1=30&r2=200&g2=35&b2=155'>/chase</a> theatre chase<br>";

  message += "<br><br>";

  message += "<a href = '/ombre?i=100&r1=50&g1=15&b1=0&r2=50&g2=0&b2=50'>/ombre</a> ombre colors over strip length<br>";
  message += "<a href = '/ombrecycle?i=100&r1=0&g1=40&b1=60&r2=200&g2=35&b2=155'>/ombrecycle</a> ombre colors cycling<br>";

  message += "<br><br>";

  message += "<a href = '/fade?t=1&r1=255&g1=255&b1=75&r2=255&g2=0&b2=0'>/fade</a> fade<br>";
  message += "<a href = '/breath?i=75&r1=0&g1=0&b1=0&r2=50&g2=0&b2=50'>/breath</a> breathing between colors<br>";

  message += "<a href = '/flickerfade?t=5&r1=255&g1=150&b1=0&r2=0&g2=0&b2=0'>/flickerfade</a> a color fade with flicker<br>";
  message += "<a href = '/flicker?i=75&r1=50&g1=25&b1=0'>/flicker</a> flickering lights<br>";

  message += "<br><br>";

  message += "<a href = '/slower'>/slower</a> slower animation<br>";
  message += "<a href = '/faster'>/faster</a> faster animation<br>";
  message += "<a href = '/reverse'>/reverse</a> reverse animation<br>";
  // xxx add "set" message when debugged

  message += "<br><br>";

  message += "<a href = '/off'>/off</a> LEDs off<br>";


  message += "<a href = '/flavor?f=0'>/flavor</a> Change color to rainbow<br>";

  message += "<br><br><br>";
  // add change color-clickable stuff xxx
  // ie more/less red / green / blue

  message += "<br>Syntax: http://&IP>/&command>?&arg1>=&value1>&arg2>=&value2>&...<br>";
  message += "<br><br><br>";
  
  message += "Flavors (colors) to choose from<br><br>";
  
  message += "<a href = '/flavor?f=101'>/flavor?f=101 SAGE</a><br>";
  message += "<a href = '/flavor?f=102'>/flavor?f=102 LIGHTGREEN</a><br>";
  message += "<a href = '/flavor?f=103'>/flavor?f=103 LAWNGREEN</a><br>";
  message += "<a href = '/flavor?f=104'>/flavor?f=104 GREEN</a><br>";
  message += "<a href = '/flavor?f=105'>/flavor?f=105 SEAGREEN</a><br><br>";

  message += "<a href = '/flavor?f=106'>/flavor?f=106 TURQUOISE</a><br>";
  message += "<a href = '/flavor?f=107'>/flavor?f=107 TURQUOISE2</a><br>";
  message += "<a href = '/flavor?f=108'>/flavor?f=108 LIGHTCYAN</a><br>";
  message += "<a href = '/flavor?f=109'>/flavor?f=109 CYAN</a><br>";
  message += "<a href = '/flavor?f=110'>/flavor?f=110 BLUE</a><br><br>";

  message += "<a href = '/flavor?f=111'>/flavor?f=111 BLUEVIOLET</a><br>";
  message += "<a href = '/flavor?f=112'>/flavor?f=112 VIOLET</a><br>";
  message += "<a href = '/flavor?f=113'>/flavor?f=113 PURPLE</a><br><br>";

  message += "<a href = '/flavor?f=114'>/flavor?f=114 MAGENTA</a><br>";
  message += "<a href = '/flavor?f=115'>/flavor?f=115 PINK</a><br>";
  message += "<a href = '/flavor?f=116'>/flavor?f=116 COLDRED</a><br>";
  message += "<a href = '/flavor?f=117'>/flavor?f=117 RED</a><br>";
  message += "<a href = '/flavor?f=118'>/flavor?f=118 LIGHTPINK</a><br>";
  message += "<a href = '/flavor?f=119'>/flavor?f=119 WARMPINK</a><br><br>";

  message += "<a href = '/flavor?f=120'>/flavor?f=120 APRICOT</a><br>";
  message += "<a href = '/flavor?f=121'>/flavor?f=121 LIGHTORANGE</a><br>";
  message += "<a href = '/flavor?f=122'>/flavor?f=122 ORANGE</a><br>";
  message += "<a href = '/flavor?f=123'>/flavor?f=123 YELLOW</a><br>";
  message += "<a href = '/flavor?f=124'>/flavor?f=124 BROWN</a><br>";
  message += "<a href = '/flavor?f=125'>/flavor?f=125 WHITE</a><br><br>";

  message += "<a href = '/flavor?f=152'> (f > 125 PINK)</a><br><br>";
  
  message += "Gradient (ombre) colors <br><br>";
  
  message += "<a href = '/flavor?f=0'>/flavor?f=0 Rainbow</a><br>";
  message += "<a href = '/flavor?f=19'>/flavor?f=19  RED - GREEN</a><br>";
  message += "<a href = '/flavor?f=20'>/flavor?f=20  GREEN - RED</a><br>";
  message += "<a href = '/flavor?f=21'>/flavor?f=21  PURPLE - PINK</a><br>";
  message += "<a href = '/flavor?f=22'>/flavor?f=22  COLDRED - BLUE</a><br><br>";

  message += "<a href = '/flavor?f=23'>/flavor?f=23  BROWN - YELLOW</a><br>";
  message += "<a href = '/flavor?f=24'>/flavor?f=24  MAGENTA - SEAGREEN</a><br>";
  message += "<a href = '/flavor?f=25'>/flavor?f=25  VIOLET - SAGE</a><br>";
  message += "<a href = '/flavor?f=26'>/flavor?f=26  ORANGE - VIOLET</a><br><br>";

  message += "<a href = '/flavor?f=27'>/flavor?f=27  MAGENTA - YELLOW</a><br>";
  message += "<a href = '/flavor?f=28'>/flavor?f=28  LAWNGREEN - LIGHTCYAN</a><br>";
  message += "<a href = '/flavor?f=29'>/flavor?f=29  GREEN - CYAN</a><br>";
  message += "<a href = '/flavor?f=30'>/flavor?f=30  BROWN - BLUE</a><br><br>";


  message += "<br><br><br>";
  return message;
}

void handleNotFound() {
  // digitalWrite(LED_PIN, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/html", message);
  // digitalWrite(LED_PIN, 0);
}

int getArgValue(String name)
{
  for (uint8_t i = 0; i < server.args(); i++)
    if (server.argName(i) == name)
      return server.arg(i).toInt();
  return -1;
}

uint32_t getArgColor() {
  uint8_t red = getArgValue("r"); // get argument
  uint8_t green = getArgValue("g"); // get argument
  uint8_t blue = getArgValue("b"); // get argument
  uint32_t color = strip.Color(red, green, blue);
  return color;
}

uint32_t getArgColor1() {
  uint8_t red = getArgValue("r1"); // get argument
  uint8_t green = getArgValue("g1"); // get argument
  uint8_t blue = getArgValue("b1"); // get argument
  uint32_t color = strip.Color(red, green, blue);
  return color;
}

uint32_t getArgColor2() {
  uint8_t red = getArgValue("r2"); // get argument
  uint8_t green = getArgValue("g2"); // get argument
  uint8_t blue = getArgValue("b2"); // get argument
  uint32_t color = strip.Color(red, green, blue);
  return color;
}

//////////////////////////////////////////

// Initialize everything and prepare to start
void setup()
{

  // Initialize sensors
  ldr.begin();
  pir.begin();
  reed.begin();
  dht.begin();
  // pinMode(REED_PIN, INPUT_PULLUP); // done in sensors.h now!


  // Initialize all the pixelStrips
  strip.begin();
  strip.setBrightness(BRIGHTNESS);
  strip.ColorAll(0);
  strip.show(); // Initialize all pixels to 'off'
  //Ring1.begin();
  //Ring2.begin(); etc

  // Initialize builtin led
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, 0);

  //initialize server
  Serial.begin(9600);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);

  server.on("/clear", []() {
    String message = handleMessage("LEDs off<br><br>");
    server.send(200, "text/html", message);
    strip.NoPattern();
  });

  server.on("/off", []() {
    String message = handleMessage("LEDs off<br><br>");
    server.send(200, "text/html", message);
    strip.NoPattern();
  });

  server.on("/reverse", []() {
    String message = handleMessage("Reversing strip<br><br>");
    server.send(200, "text/html", message);
    strip.Reverse();
  });

  server.on("/pause", []() {
    String message = handleMessage("Pausing<br><br>");
    server.send(200, "text/html", message);
    strip.ActivePattern == PAUSE;
  });

  server.on("/rainbow", []() {
    String message = handleMessage("Rainbow cycle<br><br>");
    server.send(200, "text/html", message);
    unsigned long interval = getArgValue("i"); // get argument
    strip.RainbowCycle(interval);  //RainbowCycle(unsigned long interval, direction dir = FORWARD)
  });

  server.on("/wipe", []() {
    String message = handleMessage("Color wipe (random colors)<br><br>");
    server.send(200, "text/html", message);    uint32_t color = getArgColor() ;
    unsigned long interval = getArgValue("i"); // get argument
    strip.ColorWipe(color, interval);  //ColorWipe(uint32_t color, unsigned long interval, direction dir = FORWARD)
  });

  server.on("/scan", []() {
    String message = handleMessage("Scanner wipe (random colors)<br><br>");
    server.send(200, "text/html", message);
    uint32_t color = getArgColor() ;
    unsigned long interval = getArgValue("i"); // get argument
    strip.Scanner(color, interval);  //Scanner(uint32_t color1, unsigned long interval)
  });

  server.on("/fade", []() {
    String message = handleMessage("Fade<br><br>");
    server.send(200, "text/html", message);
    uint32_t color = getArgColor() ;
    uint8_t totalMinutes = getArgValue("t"); // get argument
    uint8_t steps = getArgValue("steps"); // get argument
    uint32_t color1 = getArgColor1();
    uint32_t color2 = getArgColor2();
    strip.Fade(color1, color2, totalMinutes);  //Fade(uint32_t color1, uint32_t color2, uint8_t totalMinutes, direction dir = FORWARD)
  });

  server.on("/flickerfade", []() {
    String message = handleMessage("Flicker fade<br><br>");
    server.send(200, "text/html", message);
    uint8_t totalMinutes = getArgValue("t"); // get argument
    uint32_t color1 = getArgColor1();
    uint32_t color2 = getArgColor2();
    strip.FlickerFade(color1, color2, totalMinutes);  //FlickerFade(uint32_t color1, uint32_t color2, unsigned long interval)
  });


  server.on("/flicker", []() {
    String message = handleMessage("Flicker<br><br>");
    server.send(200, "text/html", message);
    uint8_t interval = getArgValue("i"); // get argument
    uint32_t color1 = getArgColor1();
    strip.Flicker(color1, interval);  //Flicker(uint32_t color1, unsigned long interval)
  });

  server.on("/chase", []() {
    String message = handleMessage("Theatre chase<br><br>");
    server.send(200, "text/html", message);
    unsigned long interval = getArgValue("i"); // get argument
    uint32_t color1 = getArgColor1();
    uint32_t color2 = getArgColor2();
    strip.TheaterChase(color1, color2, interval);  //TheaterChase(uint32_t color1, uint32_t color2, unsigned long interval, direction dir = FORWARD)
  });

  server.on("/ombre", []() {
    String message = handleMessage("Ombre<br><br>");
    server.send(200, "text/html", message);
    unsigned long interval = getArgValue("i"); // get argument
    uint32_t color1 = getArgColor1();
    uint32_t color2 = getArgColor2();
    strip.Ombre(color1, color2, interval);  //Ombre(uint32_t color1, uint32_t color2, unsigned long interval, direction dir = FORWARD)
  });

  server.on("/ombrecycle", []() {
    String message = handleMessage("Ombre cycle<br><br>");
    server.send(200, "text/html", message);
    unsigned long interval = getArgValue("i"); // get argument
    uint32_t color1 = getArgColor1();
    uint32_t color2 = getArgColor2();
    strip.OmbreCycle(color1, color2, interval);  //OmbreCycle(uint32_t color1, uint32_t color2, unsigned long interval, direction dir = FORWARD)
  });

  server.on("/breath", []() {
    String message = handleMessage("Breath<br><br>");
    server.send(200, "text/html", message);
    unsigned long interval = getArgValue("i"); // get argument
    uint32_t color1 = getArgColor1();
    uint32_t color2 = getArgColor2();
    strip.Breath(color1, color2, interval);  //Breath(uint32_t color1, uint32_t color2, unsigned long interval, direction dir = FORWARD)
  });

  server.on("/sunset", []() {
    String message = handleMessage("Sunset (flickering)<br><br>");
    server.send(200, "text/html", message);
    uint8_t totalMinutes = getArgValue("t");
    strip.Sunset(totalMinutes);  //Sunset(uint8_t totalMinutes, direction dir = FORWARD)
  });


  server.on("/sunrise", []() {
    String message = handleMessage("Sunrise (simple)<br><br>");
    server.send(200, "text/html", message);
    uint8_t totalMinutes = getArgValue("t");
    strip.Sunrise(totalMinutes);  //Sunrise(uint8_t totalMinutes, direction dir = FORWARD)
  });

  server.on("/countdown", []() {
    String message = handleMessage("Countdown<br><br>");
    server.send(200, "text/html", message);
    uint8_t totalMinutes = getArgValue("t"); // get argument
    uint16_t totalPixels = getArgValue("p"); // get argument
    uint8_t colorType = getArgValue("f"); // get argument
    direction dir = FORWARD;
    strip.Countdown(totalMinutes, totalPixels, colorType, dir);  //Countdown(uint8_t totalMinutes, uint8_t colorType, direction dir = FORWARD)
  });

  server.on("/countup", []() {
    String message = handleMessage("Countdown<br><br>");
    server.send(200, "text/html", message);
    uint8_t totalMinutes = getArgValue("t"); // get argument
    uint16_t totalPixels = getArgValue("p"); // get argument
    uint8_t flavor = getArgValue("f"); // get argument
    direction dir = REVERSE;
    strip.Countdown(totalMinutes, totalPixels, flavor, dir);  //Countdown(uint8_t totalMinutes, uint8_t colorType, direction dir = FORWARD)
  });

  server.on("/night", []() {
    String message = handleMessage("Night<br><br>");
    server.send(200, "text/html", message);
    uint8_t totalMinutes = getArgValue("t"); // get argument
    uint16_t totalPixels = getArgValue("p"); // get argument
    uint8_t flavor = getArgValue("f"); // get argument

    direction dir = REVERSE;
    NeoRoutine = 1;
    NeoRoutinePart = 0;
    strip.Countdown(totalMinutes, totalPixels, flavor, dir);  //Countdown(uint8_t totalMinutes, uint16_t totalPixels, uint8_t colorType, direction dir = FORWARD)
  });


  server.on("/flavor", []() {
    String message = handleMessage("Night<br><br>");
    server.send(200, "text/html", message);
    uint8_t flavor = getArgValue("f"); // get argument

    strip.setFlavor(flavor);    //setFlavor(uint8_t flavorChoice)
  });



  server.on("/faster", []() {
    int32_t currentInterval = strip.Interval;
    Serial.print("Current Interval: "); Serial.println(currentInterval);
    if (currentInterval <= 0) strip.Interval = 1;
    if (currentInterval <= 100) strip.Interval = currentInterval - 10;
    else if (currentInterval <= 1000) strip.Interval = currentInterval - 100;
    else if (currentInterval <= 2000) strip.Interval = currentInterval - 500;
    else if (currentInterval > 2000) strip.Interval = currentInterval - 1000;
    String firstRowMessage = "Faster (";
    firstRowMessage += "Interval:  ";
    firstRowMessage += (strip.Interval);
    firstRowMessage += ")<br><br>";
    String message = handleMessage(firstRowMessage);
    server.send(200, "text/html", message);
  });

  server.on("/slower", []() {
    int8_t currentInterval = strip.Interval;
    Serial.print("Current Interval: "); Serial.println(currentInterval);
    if (currentInterval <= 0) strip.Interval = 1;
    if (currentInterval <= 100) strip.Interval = currentInterval + 10;
    else if (currentInterval <= 1000) strip.Interval = currentInterval + 100;
    else if (currentInterval <= 2000) strip.Interval = currentInterval + 500;
    else if (currentInterval > 2000) strip.Interval = currentInterval + 1000;
    String firstRowMessage = "Slower (";
    firstRowMessage += "Interval:  ";
    firstRowMessage += (strip.Interval);
    firstRowMessage += ")<br><br>";
    String message = handleMessage(firstRowMessage);
    server.send(200, "text/html", message);
  });




  server.on("/set", []() { // xxx not finished, not debugged
    server.send(200, "text / plain", "this works as well");

    // totaltimexxx

    // set interval, syntax: i=200
    unsigned long currentInterval = strip.Interval;
    unsigned long interval = getArgValue("i"); // get argument
    if (interval >= 0) strip.Interval = interval;

    // set color, syntax: set?r1=200&g1=200&b1=200&r2=100&g2=100&b2=100 or set?r=100&g=200&b=150
    // xxx not working
    uint32_t color = getArgColor(); // get argument

    if (color >= 0) strip.Color1 = color;
    // if (color1 >= 0) strip.Color1 = color1;
    // if (color2 >= 0) strip.Color2 = color2;
  });


  /*  add more handles here

    // FADE_TO
  */


  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

///////////////////////////////////////////////

// Main loop
void loop()
{

  server.handleClient(); // watch for server messages
  button.tick();     // watch push button


  strip.Update();

  //ldr.sensorRead(); // read ldr light depentdant resistor
  // pir.sensorRead(); // read pir motion detector
  //reed.sensorRead(); // read magnetic reed switch
  //read_dht(); // read DHT-module ie temperature and humidity

}


// button callback functions here xxx

// this function will be called when the button was pressed 2 times in a short timeframe. from onebButton examle
void doubleclick() {
  static int m = LOW;
  // reverse the LED
  m = !m;
  digitalWrite(LED_PIN, m);
} // doubleclick


void read_dht() {
  // Wait at least 2 seconds seconds between measurements.
  // If the difference between the current time and last time you read
  // the sensor is bigger than the interval you set, read the sensor.
  // Works better than delay for things happening elsewhere also.
  if ((millis() - lastDHTtime) > dhtInterval) // time to update
  {
    // Save the last time you read the sensor
    lastDHTtime = millis();

    // Reading temperature and humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (it's a very slow sensor)
    float hum = dht.readHumidity();        // Read humidity as a percent
    float temp = dht.readTemperature();  // Read temperature as Celsius

    // Check if any reads failed and exit early (to try again).
    if (isnan(hum) || isnan(temp)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }
    Serial.print("Humidity: ");
    Serial.print(hum);
    Serial.print(" %\t");
    Serial.print("Temperature: ");
    Serial.print(temp);
    Serial.println(" °C");
  }

}
////////////////////////////////////////////////////

//------------------------------------------------------------
//Completion Routines - get called on completion of a pattern
//------------------------------------------------------------


// strip Completion Callback
void stripComplete()
{
  //Serial.println("stripComplete"); // xxx
  if (strip.ActivePattern == SCANNER) {
    strip.DefaultComplete(); // just an example, anything could happen here really
  }
  else if (NeoRoutine == 1) {
    // NeoRoutine 1: starts w countup, calling this callback, then countdown 45 mins, then countdown 45 mins, then countup 10 mins.
    NeoRoutinePart = NeoRoutinePart + 1; // increase by one
    // (part 0 is the calling countdown initiator, something like
    // strip.Countdown(totalMinutes, colorType, dir);  //Countdown(uint8_t totalMinutes, uint8_t colorType, direction dir = FORWARD)
    if (NeoRoutinePart == 1 ) // part 1
    {
      Serial.println("Routine part 2");

      strip.DefaultComplete();
      // do a flash and sound thing, maybe not as millis? or as separate menuPart?
      direction dir = FORWARD;
      strip.Countdown(45, 0, 0, dir);  //45 Countdown(uint8_t totalMinutes, uint8_t colorType, direction dir = FORWARD)
    }
    else if (NeoRoutinePart == 2 ) // part 2
    {
      Serial.println("Routine part 2");
      strip.DefaultComplete();
      direction dir = FORWARD;
      strip.Countdown(45, 0, 20, dir);  //45 Countdown(uint8_t totalMinutes, uint8_t colorType, direction dir = FORWARD)
    }
    else if (NeoRoutinePart == 3 ) // part 3
    {
      Serial.println("Routine part 3");
      strip.DefaultComplete();
      direction dir = FORWARD;
      strip.Countdown(10, 10, 1, dir);  //10 Countdown(uint8_t totalMinutes, uint8_t colorType, direction dir = FORWARD)
    }
    else {
      strip.DefaultComplete();
      NeoRoutine = 0;
      NeoRoutinePart = 0;
      Serial.println("Routine finished");
    }
  }
  else // if none of the above, use default
  {
    strip.DefaultComplete();
  }
}

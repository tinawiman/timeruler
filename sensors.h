// simple sensor class for millis functionality etc
// Tina Wiman, 2017




// sensor types
enum sType { LDR, REED, PIR, DHTP }; // add more as needed
enum aodType { ANALOG, DIGITAL }; // use analog or digital read

// sensor Class
class Sensors
{
  public:
    // Member Variables:
    aodType readType;
    sType sensorType;

    uint8_t sensorPin;
    uint32_t value;
    uint32_t lastValue;
    String sensorName;

    unsigned long readInterval;   // milliseconds between reads
    unsigned long lastReadTime; // last read in milliseconds


    // Constructor - calls base-class constructor to initialize strip
    Sensors(uint8_t pin, unsigned long interval, sType sensor, aodType dOrA)
    {
      sensorPin = pin;
      readInterval = interval;
      sensorType = sensor;
      readType = dOrA;
      setSensorName();
    }

    // initialize called from setup()
    void begin() {

      // pinmode
      switch (sensorType)
      {
        case REED:
          pinMode(sensorPin, INPUT_PULLUP);
          Serial.println("Reed sensor as input pullup");
          break;
        default:
          pinMode(sensorPin, INPUT);
          break;
      }


      Serial.print(sensorName); Serial.print(" on pin: "); Serial.print(sensorPin); Serial.print("\t");
      Serial.print("Interval: "); Serial.println(readInterval);


      lastValue = sensorRead();
      lastReadTime = 0;
    }


    // class methods

    int sensorRead() {
      if ((millis() - lastReadTime) > readInterval) // time to update
      {
        lastReadTime = millis();
        switch (readType)
        {
          // digital or analog read
          case DIGITAL:
            value = digitalRead(sensorPin);
            break;
          case ANALOG:
            value = analogRead(sensorPin);
            break;
          default:
            value = digitalRead(sensorPin);
            break;
        }

        Serial.print(sensorName); Serial.print(": "); Serial.println(value); Serial.println();


      }
    }


    void setSensorName() {
      switch (sensorType)
      {
        case LDR:
          sensorName = "LDR";
          break;
        case REED:
          sensorName = "reed";
          break;
        case PIR:
          sensorName = "PIR";
          break;
        case 3:
          sensorName = "DHT";
          break;
        default:
          sensorName = "Unspecified";
          break;
      }
    }

};

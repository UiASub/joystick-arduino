#include <math.h>
#include <ArduinoJson.h>

float mapping_function(
    int measure_val,
    int min_val,
    int max_val,
    int zero,
    int percent_dzone_size,
    float curve_coefficient)
{
    int range = max_val - min_val;
    int dzone_range = range * percent_dzone_size / 100;
    int dzone_min = zero - dzone_range / 2;
    int dzone_max = zero + dzone_range / 2;

    if (measure_val >= dzone_min && measure_val <= dzone_max)
    {
        return 0.0f;
    }

    float fraction;
    if (measure_val < dzone_min)
    {
        fraction = float(dzone_min - measure_val) / float(dzone_min - min_val);
        fraction = constrain(fraction, 0.0f, 1.0f);
        return -pow(fraction, curve_coefficient) * 100.0f;
    }
    else
    {
        fraction = float(measure_val - dzone_max) / float(max_val - dzone_max);
        fraction = constrain(fraction, 0.0f, 1.0f);
        return pow(fraction, curve_coefficient) * 100.0f;
    }
}

void debug_print(int measure_val, float mapped_val)
{ 
  Serial.print("Raw value: ");
  Serial.print(measure_val);
  Serial.print(" | Mapped value: ");
  Serial.print(mapped_val);
  Serial.println("%");
  Serial.println("");
}

const int xHallPin = A1;
const int yHallPin = A0;
const int zHallPin = A3;
const int pitchHallPin = A4;
const int rollHallPin = A5;
const int yawHallPin = A2;

const int pinUp = 3;
const int pinDown = 5;
int gain = 0;
const int step = 10;
const int maxGain = 100;
const int minGain = 0;

const int button_openclose_pin = 4;
int button_openclose = 0;
bool last_button_openclose_state = HIGH;

const int button_surface_pin = 2;
int button_surface = 0;
bool last_button_surface_state = HIGH;

const float curveCoefficient = 2.0;
const int deadZoneSize = 8;

// Global mapping values for easier configuration
const int xRawMin = 220, xRawMax = 445, xZero = 340;
const int yRawMin = 545, yRawMax = 740, yZero = 640;
const int zRawMin = 0, zRawMax = 1020, zZero = 515;
const int pitchRawMin = 0, pitchRawMax = 1023, pitchZero = 503;
const int rollRawMin = 0, rollRawMax = 1023, rollZero = 518;
const int yawRawMin = 435, yawRawMax = 573, yawZero = 503;

void setup()
{
    Serial.begin(115200);
    pinMode(pinUp, INPUT_PULLUP);
    pinMode(pinDown, INPUT_PULLUP);
    pinMode(button_surface_pin, INPUT_PULLUP);
    pinMode(button_openclose_pin, INPUT_PULLUP);
}

void loop()
{
  int button_surface_read = digitalRead(button_surface_pin);
  int button_openclose_read = digitalRead(button_openclose_pin);

  if (button_surface_read == LOW && last_button_surface_state == HIGH)
  {
      button_surface = !button_surface;
      delay(50);
  }
  last_button_surface_state = button_surface_read;

  if (button_openclose_read == LOW && last_button_openclose_state == HIGH)
  {
      button_openclose = !button_openclose;
      delay(50);
  }
  last_button_openclose_state = button_openclose_read;

  int xHallValue = analogRead(xHallPin);
  int yHallValue = analogRead(yHallPin);
  int zHallValue = analogRead(zHallPin);

  int pitchHallValue = analogRead(pitchHallPin);
  int rollHallValue = analogRead(rollHallPin);
  int yawHallValue = analogRead(yawHallPin);

  float x = mapping_function(xHallValue, xRawMin, xRawMax, xZero, deadZoneSize, curveCoefficient);
  float y = mapping_function(yHallValue, yRawMin, yRawMax, yZero, deadZoneSize, curveCoefficient);
  float z = mapping_function(zHallValue, zRawMin, zRawMax, zZero, deadZoneSize, curveCoefficient);

  float pitch = mapping_function(pitchHallValue, pitchRawMin, pitchRawMax, pitchZero, deadZoneSize, curveCoefficient);
  float roll = mapping_function(rollHallValue, rollRawMin, rollRawMax, rollZero, deadZoneSize, curveCoefficient);
  float yaw = mapping_function(yawHallValue, yawRawMin, yawRawMax, yawZero, deadZoneSize, curveCoefficient);

  float thrust[6] = {x, y, z, pitch, roll, yaw};

  StaticJsonDocument<200> doc;
  JsonArray thrustArray = doc.createNestedArray("Thrust");

  for (int i = 0; i < 6; i++)
  {
      thrustArray.add(thrust[i]);
  }

  doc["Gain"] = gain;
  JsonObject buttons = doc.createNestedObject("Buttons");
  buttons["button_openclose_arm"] = button_openclose;
  buttons["button_surface"] = button_surface;

  serializeJson(doc, Serial);
  Serial.println();

  delay(200);
}

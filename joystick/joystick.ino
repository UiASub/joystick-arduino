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
    // 1) Compute dead zone boundaries
    int range = max_val - min_val;
    int dzone_range = range * percent_dzone_size / 100; // raw size in ADC units
    int dzone_min = zero - dzone_range / 2;
    int dzone_max = zero + dzone_range / 2;

    // 2) If within dead zone => 0%
    if (measure_val >= dzone_min && measure_val <= dzone_max)
    {
        return 0.0f;
    }

    float fraction;
    // 3) Negative side
    if (measure_val < dzone_min)
    {
        // fraction = how far measure_val is from dzone_min to min_val
        // so measure_val == dzone_min => fraction = 0
        // measure_val == min_val (or 0) => fraction = 1
        fraction = float(dzone_min - measure_val) / float(dzone_min - min_val);

        // clamp to [0,1]
        if (fraction < 0.0f)
            fraction = 0.0f;
        if (fraction > 1.0f)
            fraction = 1.0f;

        // apply curve
        float curved = pow(fraction, curve_coefficient);

        // scale to % and return negative
        return -curved * 100.0f;
    }
    // 4) Positive side (measure_val > dzone_max)
    else
    {
        // fraction = how far measure_val is from dzone_max to max_val
        // measure_val == dzone_max => fraction = 0
        // measure_val == max_val => fraction = 1
        fraction = float(measure_val - dzone_max) / float(max_val - dzone_max);

        // clamp to [0,1]
        if (fraction < 0.0f)
            fraction = 0.0f;
        if (fraction > 1.0f)
            fraction = 1.0f;

        // apply curve
        float curved = pow(fraction, curve_coefficient);

        // scale to % and return positive
        return curved * 100.0f;
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

const int xHallPin = A0;
const int yHallPin = A1;
const int zHallPin = A2;
const int pitchHallPin = A4;
const int rollHallPin = A5 const int yawHallPin = A3;

// Raw sensor value ranges for X, Y movement and Z rotation
// X - range
const int xRawMin = 545;
const int xRawMax = 740;
const int xZero = 0;

// Y - range
const int yRawMin = 240;
const int yRawMax = 820;
const int yZero = 445;

// Z - range
const int zRawMin = 125;
const int zRawMax = 945;
const int zZero = 0;

// pitch - range
const int pitchRawMin = 0;
const int pitchRawMax = 0;
const int pitchZero = 0;

// roll - range
const int rollRawMin = 0;
const int rollRawMax = 0;
const int rollZero = 0;

// yaw - range
const int yawRawMin = 475;
const int yawRawMax = 520;
const int yawZero = 0;

// Curve coefficient, Dead zone size
const float curveCoefficient = 2.0;
const int deadZoneSize = 5;

void setup()
{
    Serial.begin(115200);
}

void loop()
{
    // X, Y and Z movement
    // Read the Hall effect sensor value
    int xHallValue = analogRead(xHallPin);
    int yHallValue = analogRead(yHallPin);
    int zHallValue = analogRead(zHallPin);

    // Pitch, Roll and Yaw rotation
    int pitchHallValue = analogRead(pitchHallPin);
    int rollHallValue = analogRead(rollHallPin);
    int yawHallValue = analogRead(yawHallPin);

    float x = mapping_funciton(xHallValue, xRawMin, xRawMax, xZero, deadZoneSize, curveCoefficient);
    float y = mapping_function(yHallValue, yRawMin, yRawMax, yZero, deadZoneSize, curveCoefficient);
    float z = mapping_function(zHallValue, zRawMin, zRawMax, zZero, deadZoneSize, curveCoefficient);

    float pitch = mapping_function(pitchHallValue, pitchRawMin, pitchRawMax, pitchZero, deadZoneSize, curveCoefficient);
    float roll = mapping_function(rollHallValue, rollRawMin, rollRawMax, rollZero, deadZoneSize, curveCoefficient);
    float yaw = mapping_function(yawHallValue, yawRawMin, yawRawMax, yawZero, deadZoneSize, curveCoefficient);

    debug_print(xHallValue, x);

    // [x, y, z, pitch, roll, yaw]
    float thrust[6] = {x, y, z, pitch, roll, yaw};

    // Create a JSON object
    StaticJsonDocument<200> doc;
    JsonArray thrustArray = doc.createNestedArray("Thrust");

    // Add elements to the array
    for (int i = 0; i < 6; i++)
    {
        thrustArray.add(thrust[i]);
    }

    // Serialize JSON to a string and send it
    serializeJson(doc, Serial);
    Serial.println(); // Ensure newline for Python to read properly

    delay(200);
}
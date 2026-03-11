#include <Arduino.h>
#include <wcx.h>

static float averageStorage[8];
static wcx_moving_average_t averageFilter;
static wcx_ema_filter_t emaFilter;
static wcx_pid_t controller;

void setup()
{
    Serial.begin(115200);
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(9, OUTPUT);

    wcx_moving_average_init(&averageFilter, averageStorage, WCX_ARRAY_SIZE(averageStorage));
    wcx_ema_init(&emaFilter, 0.15f, 0.0f, false);
    wcx_pid_init(&controller, 0.7f, 0.4f, 0.05f, 0.0f, 255.0f);
}

void loop()
{
    static uint32_t lastTick = 0;
    const float dt = 0.02f;
    const float setpoint = 650.0f;
    const float sensorRaw = (float)analogRead(A0);
    const float filtered = wcx_ema_update(&emaFilter, wcx_moving_average_update(&averageFilter, sensorRaw));
    const float output = wcx_pid_compute(&controller, setpoint, filtered, dt);

    if (!wcx_time_reached(millis(), lastTick + 20))
    {
        return;
    }

    analogWrite(9, (int)output);
    digitalWrite(LED_BUILTIN, output > 127.0f ? HIGH : LOW);

    Serial.print("raw=");
    Serial.print(sensorRaw, 2);
    Serial.print(" filtered=");
    Serial.print(filtered, 2);
    Serial.print(" output=");
    Serial.println(output, 2);

    lastTick = millis();
}

#include <Arduino.h>
#include <string.h>
#include <wcx.h>

static uint8_t serialStorage[64];
static wcx_ring_buffer_t serialRing;
static wcx_stats_t stats;
static wcx_debounce_t button;

static void enqueueMessage(const char *text)
{
    wcx_ring_buffer_write(&serialRing, (const uint8_t *)text, strlen(text));
}

void setup()
{
    Serial.begin(115200);
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(2, INPUT_PULLUP);

    wcx_ring_buffer_init(&serialRing, serialStorage, sizeof(serialStorage));
    wcx_stats_init(&stats);
    wcx_debounce_init(&button, false, 25);

    enqueueMessage("WCX diagnostics ready\n");
}

void loop()
{
    static uint32_t lastReport = 0;

    const bool pressed = !digitalRead(2);
    const int sample = analogRead(A0);
    uint8_t value;

    wcx_stats_update(&stats, (float)sample);
    wcx_debounce_update(&button, pressed, millis());

    if (wcx_debounce_rose(&button))
    {
        enqueueMessage("Button pressed\n");
        digitalWrite(LED_BUILTIN, HIGH);
    }

    if (wcx_debounce_fell(&button))
    {
        enqueueMessage("Button released\n");
        digitalWrite(LED_BUILTIN, LOW);
    }

    if (wcx_time_reached(millis(), lastReport + 1000))
    {
        const uint16_t crc = wcx_crc16_ccitt((const uint8_t *)&sample, sizeof(sample), 0xFFFF);
        Serial.print("samples=");
        Serial.print(wcx_stats_count(&stats));
        Serial.print(" mean=");
        Serial.print(wcx_stats_mean(&stats), 2);
        Serial.print(" min=");
        Serial.print(wcx_stats_min(&stats), 2);
        Serial.print(" max=");
        Serial.print(wcx_stats_max(&stats), 2);
        Serial.print(" crc16=0x");
        Serial.println(crc, HEX);
        lastReport = millis();
    }

    while (wcx_ring_buffer_pop(&serialRing, &value))
    {
        Serial.write(value);
    }
}

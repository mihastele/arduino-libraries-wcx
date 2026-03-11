#include <Arduino.h>
#include <wcx.h>

static bool ledState = false;
static wcx_task_t tasks[2];
static wcx_scheduler_t scheduler;

static void toggleLed(void *context)
{
    (void)context;
    ledState = !ledState;
    digitalWrite(LED_BUILTIN, ledState ? HIGH : LOW);
}

static void printHeartbeat(void *context)
{
    (void)context;
    Serial.println("heartbeat");
}

void setup()
{
    Serial.begin(115200);
    pinMode(LED_BUILTIN, OUTPUT);

    wcx_task_init(&tasks[0], millis() + 250, 250, true, toggleLed, NULL);
    wcx_task_init(&tasks[1], millis() + 1000, 1000, true, printHeartbeat, NULL);
    wcx_scheduler_init(&scheduler, tasks, WCX_ARRAY_SIZE(tasks));
}

void loop()
{
    wcx_scheduler_tick(&scheduler, millis());
}

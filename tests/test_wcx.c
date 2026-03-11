#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wcx.h"

static int g_failures = 0;
static int g_checks = 0;

typedef struct
{
    int calls;
} callback_counter_t;

static void assert_true(bool condition, const char *message)
{
    g_checks++;

    if (!condition)
    {
        g_failures++;
        fprintf(stderr, "FAIL: %s\n", message);
    }
}

static void assert_size(size_t actual, size_t expected, const char *message)
{
    g_checks++;

    if (actual != expected)
    {
        g_failures++;
        fprintf(stderr, "FAIL: %s (expected=%lu actual=%lu)\n",
                message,
                (unsigned long)expected,
                (unsigned long)actual);
    }
}

static void assert_u8(uint8_t actual, uint8_t expected, const char *message)
{
    g_checks++;

    if (actual != expected)
    {
        g_failures++;
        fprintf(stderr, "FAIL: %s (expected=0x%02X actual=0x%02X)\n",
                message,
                expected,
                actual);
    }
}

static void assert_u16(uint16_t actual, uint16_t expected, const char *message)
{
    g_checks++;

    if (actual != expected)
    {
        g_failures++;
        fprintf(stderr, "FAIL: %s (expected=0x%04X actual=0x%04X)\n",
                message,
                expected,
                actual);
    }
}

static void assert_u32(uint32_t actual, uint32_t expected, const char *message)
{
    g_checks++;

    if (actual != expected)
    {
        g_failures++;
        fprintf(stderr, "FAIL: %s (expected=%lu actual=%lu)\n",
                message,
                (unsigned long)expected,
                (unsigned long)actual);
    }
}

static void assert_float_close(float actual, float expected, float tolerance, const char *message)
{
    g_checks++;

    if (fabsf(actual - expected) > tolerance)
    {
        g_failures++;
        fprintf(stderr, "FAIL: %s (expected=%.6f actual=%.6f tolerance=%.6f)\n",
                message,
                expected,
                actual,
                tolerance);
    }
}

static void increment_counter(void *context)
{
    callback_counter_t *counter = (callback_counter_t *)context;

    if (counter != NULL)
    {
        counter->calls++;
    }
}

static void test_common(void)
{
    assert_true(wcx_time_reached(100U, 100U), "time reached accepts equal timestamps");
    assert_true(wcx_time_reached(5U, 0xFFFFFFF0U), "time reached handles millis wraparound");
    assert_float_close(wcx_mapf(5.0f, 0.0f, 10.0f, 0.0f, 100.0f), 50.0f, 0.0001f,
                       "mapf maps midpoint");
    assert_float_close(wcx_mapf(3.0f, 1.0f, 1.0f, 7.0f, 9.0f), 7.0f, 0.0001f,
                       "mapf returns out_min on zero input span");
    assert_float_close(wcx_apply_deadband(0.04f, 0.05f), 0.0f, 0.0001f,
                       "deadband zeros values inside threshold");
    assert_float_close(wcx_apply_deadband(-0.3f, -0.1f), -0.3f, 0.0001f,
                       "deadband normalizes negative threshold");
}

static void test_ring_buffer(void)
{
    uint8_t storage[4];
    uint8_t output[3] = {0U, 0U, 0U};
    uint8_t value = 0U;
    uint8_t payload[3] = {0x11U, 0x22U, 0x33U};
    wcx_ring_buffer_t ring;

    assert_true(!wcx_ring_buffer_init(NULL, storage, sizeof(storage)),
                "ring buffer init rejects null struct");
    assert_true(wcx_ring_buffer_init(&ring, storage, sizeof(storage)),
                "ring buffer init accepts valid storage");
    assert_true(wcx_ring_buffer_is_empty(&ring), "ring buffer starts empty");
    assert_size(wcx_ring_buffer_capacity(&ring), 4U, "ring buffer capacity is reported");

    assert_true(wcx_ring_buffer_push(&ring, 0xAAU), "ring buffer push succeeds");
    assert_true(wcx_ring_buffer_push(&ring, 0xBBU), "ring buffer second push succeeds");
    assert_size(wcx_ring_buffer_size(&ring), 2U, "ring buffer size tracks pushes");
    assert_true(wcx_ring_buffer_peek(&ring, &value), "ring buffer peek succeeds");
    assert_u8(value, 0xAAU, "ring buffer peek returns oldest item");

    assert_size(wcx_ring_buffer_write(&ring, payload, sizeof(payload)), 2U,
                "ring buffer write stops when full");
    assert_true(wcx_ring_buffer_is_full(&ring), "ring buffer detects full state");
    assert_true(!wcx_ring_buffer_push(&ring, 0x44U), "ring buffer push fails when full");

    assert_true(wcx_ring_buffer_pop(&ring, &value), "ring buffer pop succeeds");
    assert_u8(value, 0xAAU, "ring buffer pop preserves FIFO order");
    assert_size(wcx_ring_buffer_read(&ring, output, sizeof(output)), 3U,
                "ring buffer read drains remaining bytes");
    assert_u8(output[0], 0xBBU, "ring buffer read preserves second byte");
    assert_u8(output[1], 0x11U, "ring buffer read preserves third byte");
    assert_u8(output[2], 0x22U, "ring buffer read preserves fourth byte");
    assert_true(wcx_ring_buffer_is_empty(&ring), "ring buffer becomes empty after read");

    wcx_ring_buffer_clear(&ring);
    assert_size(wcx_ring_buffer_size(&ring), 0U, "ring buffer clear resets size");
}

static void test_debounce(void)
{
    wcx_debounce_t debounce;

    wcx_debounce_init(&debounce, false, 10U);
    assert_true(!wcx_debounce_state(&debounce), "debounce initializes stable state");

    assert_true(!wcx_debounce_update(&debounce, true, 5U),
                "debounce does not switch immediately on candidate change");
    assert_true(!wcx_debounce_changed(&debounce), "debounce changed stays false before interval");
    assert_true(!wcx_debounce_update(&debounce, true, 14U),
                "debounce still waits before interval expires");
    assert_true(wcx_debounce_update(&debounce, true, 15U),
                "debounce switches when interval expires");
    assert_true(wcx_debounce_changed(&debounce), "debounce changed is set on accepted edge");
    assert_true(wcx_debounce_rose(&debounce), "debounce rose is set on rising edge");

    assert_true(wcx_debounce_update(&debounce, false, 20U),
                "debounce keeps prior stable state while falling edge is pending");
    assert_true(wcx_debounce_update(&debounce, false, 30U) == false,
                "debounce accepts falling edge after interval");
    assert_true(wcx_debounce_fell(&debounce), "debounce fell is set on falling edge");
}

static void test_scheduler(void)
{
    callback_counter_t periodic_counter = {0};
    callback_counter_t oneshot_counter = {0};
    wcx_task_t tasks[2];
    wcx_scheduler_t scheduler;

    wcx_task_init(&tasks[0], 10U, 5U, true, increment_counter, &periodic_counter);
    wcx_task_init(&tasks[1], 8U, 100U, false, increment_counter, &oneshot_counter);

    assert_true(!wcx_task_run(&tasks[0], 9U), "task does not run before deadline");
    assert_true(wcx_task_run(&tasks[0], 10U), "task runs at deadline");
    assert_true(wcx_task_run(&tasks[0], 15U), "repeating task runs again at next period");
    assert_u32((uint32_t)periodic_counter.calls, 2U, "periodic task callback count increments");

    assert_true(wcx_task_run(&tasks[1], 8U), "one-shot task runs once");
    assert_true(!tasks[1].enabled, "one-shot task disables itself");

    wcx_task_enable(&tasks[1], true);
    wcx_task_schedule_from_now(&tasks[1], 40U);
    assert_true(!wcx_task_run(&tasks[1], 100U), "rescheduled task waits for new deadline");
    assert_true(wcx_task_run(&tasks[1], 140U), "rescheduled one-shot runs at new deadline");

    wcx_scheduler_init(&scheduler, tasks, WCX_ARRAY_SIZE(tasks));
    assert_size(wcx_scheduler_tick(&scheduler, 20U), 1U,
                "scheduler reports number of tasks run in a tick");
}

static void test_filter(void)
{
    float storage[4];
    wcx_moving_average_t average;
    wcx_ema_filter_t ema;

    assert_true(wcx_moving_average_init(&average, storage, WCX_ARRAY_SIZE(storage)),
                "moving average init succeeds");
    assert_float_close(wcx_moving_average_update(&average, 2.0f), 2.0f, 0.0001f,
                       "moving average returns first sample");
    assert_float_close(wcx_moving_average_update(&average, 4.0f), 3.0f, 0.0001f,
                       "moving average averages partial window");

    wcx_moving_average_reset(&average, 10.0f);
    assert_float_close(wcx_moving_average_value(&average), 10.0f, 0.0001f,
                       "moving average reset seeds the whole window");
    assert_float_close(wcx_moving_average_update(&average, 16.0f), 11.5f, 0.0001f,
                       "moving average rolls window after reset");

    wcx_ema_init(&ema, 0.25f, 0.0f, false);
    assert_float_close(wcx_ema_update(&ema, 8.0f), 8.0f, 0.0001f,
                       "EMA uses first sample as initial state");
    assert_float_close(wcx_ema_update(&ema, 12.0f), 9.0f, 0.0001f,
                       "EMA applies smoothing factor");
}

static void test_pid(void)
{
    wcx_pid_t pid;
    wcx_pid_t saturated;

    wcx_pid_init(&pid, 2.0f, 1.0f, 0.5f, 0.0f, 100.0f);
    assert_float_close(wcx_pid_compute(&pid, 10.0f, 8.0f, 0.5f), 5.0f, 0.0001f,
                       "PID computes proportional and integral output on first run");
    assert_float_close(wcx_pid_compute(&pid, 10.0f, 9.0f, 0.5f), 2.5f, 0.0001f,
                       "PID applies derivative on measurement and accumulates integral");

    wcx_pid_init(&saturated, 100.0f, 0.0f, 0.0f, 0.0f, 50.0f);
    assert_float_close(wcx_pid_compute(&saturated, 10.0f, 0.0f, 1.0f), 50.0f, 0.0001f,
                       "PID clamps output to configured maximum");
    assert_float_close(wcx_pid_compute(NULL, 0.0f, 0.0f, 1.0f), 0.0f, 0.0001f,
                       "PID returns zero for null controller");
}

static void test_crc(void)
{
    static const uint8_t text[] = "123456789";
    static const uint8_t payload[] = {0x01U, 0x02U, 0x03U};

    assert_u8(wcx_crc8(text, 9U, 0x00U), 0xF4U, "CRC-8 matches standard reference vector");
    assert_u16(wcx_crc16_ccitt(text, 9U, 0xFFFFU), 0x29B1U,
               "CRC-16/CCITT matches standard reference vector");
    assert_u8(wcx_xor_checksum(payload, sizeof(payload)), 0x00U,
              "XOR checksum folds bytes with xor");
}

static void test_stats(void)
{
    wcx_stats_t stats;

    wcx_stats_init(&stats);
    wcx_stats_update(&stats, 1.0f);
    wcx_stats_update(&stats, 2.0f);
    wcx_stats_update(&stats, 3.0f);

    assert_u32(wcx_stats_count(&stats), 3U, "stats count tracks samples");
    assert_float_close(wcx_stats_min(&stats), 1.0f, 0.0001f, "stats min tracks minimum");
    assert_float_close(wcx_stats_max(&stats), 3.0f, 0.0001f, "stats max tracks maximum");
    assert_float_close(wcx_stats_mean(&stats), 2.0f, 0.0001f, "stats mean is computed");
    assert_float_close(wcx_stats_variance(&stats), 0.6666667f, 0.0002f,
                       "stats variance uses population formula");
}

int main(void)
{
    test_common();
    test_ring_buffer();
    test_debounce();
    test_scheduler();
    test_filter();
    test_pid();
    test_crc();
    test_stats();

    if (g_failures != 0)
    {
        fprintf(stderr, "\n%d of %d checks failed.\n", g_failures, g_checks);
        return EXIT_FAILURE;
    }

    printf("All %d checks passed.\n", g_checks);
    return EXIT_SUCCESS;
}

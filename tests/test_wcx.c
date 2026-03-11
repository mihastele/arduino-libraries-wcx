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

typedef struct
{
    int enters;
    int exits;
    int actions;
} fsm_counter_t;

static void fsm_on_enter(void *context, wcx_fsm_state_id_t state)
{
    fsm_counter_t *counter = (fsm_counter_t *)context;

    (void)state;

    if (counter != NULL)
    {
        counter->enters++;
    }
}

static void fsm_on_exit(void *context, wcx_fsm_state_id_t state)
{
    fsm_counter_t *counter = (fsm_counter_t *)context;

    (void)state;

    if (counter != NULL)
    {
        counter->exits++;
    }
}

static void fsm_on_transition(void *context,
                              wcx_fsm_state_id_t from_state,
                              wcx_fsm_event_id_t event,
                              wcx_fsm_state_id_t to_state)
{
    fsm_counter_t *counter = (fsm_counter_t *)context;

    (void)from_state;
    (void)event;
    (void)to_state;

    if (counter != NULL)
    {
        counter->actions++;
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

static void test_timer(void)
{
    wcx_timer_t timer;

    wcx_timer_init(&timer, 10U, false);
    assert_true(!wcx_timer_running(&timer), "timer starts stopped");

    wcx_timer_start(&timer, 100U);
    assert_true(wcx_timer_running(&timer), "timer start enables timer");
    assert_u32(wcx_timer_remaining(&timer, 105U), 5U, "timer reports remaining time");
    assert_true(!wcx_timer_expired(&timer, 109U), "timer does not expire early");
    assert_true(wcx_timer_expired(&timer, 110U), "timer expires at deadline");
    assert_true(!wcx_timer_running(&timer), "one-shot timer stops after expiry");

    wcx_timer_init(&timer, 4U, true);
    wcx_timer_start(&timer, 20U);
    assert_true(wcx_timer_expired(&timer, 24U), "periodic timer expires at first interval");
    assert_true(wcx_timer_running(&timer), "periodic timer remains running");
    assert_u32(wcx_timer_remaining(&timer, 25U), 3U, "periodic timer advances deadline");
    wcx_timer_stop(&timer);
    assert_true(!wcx_timer_running(&timer), "timer stop disables timer");
}

static void test_fsm(void)
{
    fsm_counter_t counters = {0};
    wcx_fsm_t fsm;
    static const wcx_fsm_state_t states[] = {
        {1U, fsm_on_enter, fsm_on_exit},
        {2U, fsm_on_enter, fsm_on_exit},
    };
    static const wcx_fsm_transition_t transitions[] = {
        {1U, 10U, 2U, fsm_on_transition},
        {2U, 11U, 2U, fsm_on_transition},
        {2U, 12U, 1U, fsm_on_transition},
    };

    wcx_fsm_init(&fsm,
                 1U,
                 states,
                 WCX_ARRAY_SIZE(states),
                 transitions,
                 WCX_ARRAY_SIZE(transitions),
                 &counters);

    assert_u32((uint32_t)wcx_fsm_state(&fsm), 1U, "FSM initializes with selected state");
    assert_true(wcx_fsm_dispatch(&fsm, 10U), "FSM handles matching transition");
    assert_u32((uint32_t)wcx_fsm_state(&fsm), 2U, "FSM updates current state");
    assert_u32((uint32_t)counters.exits, 1U, "FSM invokes exit callback on state change");
    assert_u32((uint32_t)counters.enters, 1U, "FSM invokes enter callback on state change");
    assert_u32((uint32_t)counters.actions, 1U, "FSM invokes transition action");

    assert_true(wcx_fsm_dispatch(&fsm, 11U), "FSM handles self transition");
    assert_u32((uint32_t)counters.exits, 1U, "FSM skips exit callback on self transition");
    assert_u32((uint32_t)counters.enters, 1U, "FSM skips enter callback on self transition");
    assert_u32((uint32_t)counters.actions, 2U, "FSM still invokes action on self transition");

    assert_true(!wcx_fsm_dispatch(&fsm, 99U), "FSM rejects unknown event");
}

static void test_protocol(void)
{
    static const uint8_t payload[] = {0x01U, 0x7EU, 0x7DU, 0x20U};
    uint8_t encoded[16] = {0U};
    uint8_t decoded[8] = {0U};
    wcx_frame_decoder_t decoder;
    wcx_frame_decoder_result_t result = WCX_FRAME_DECODER_IDLE;
    size_t index;

    assert_size(wcx_frame_encoded_capacity(sizeof(payload)), 10U,
                "frame capacity accounts for worst-case escaping");

    assert_true(wcx_frame_decoder_init(&decoder, decoded, sizeof(decoded), 0x7EU, 0x7DU),
                "frame decoder init succeeds");
    assert_size(wcx_frame_encode(payload,
                                 sizeof(payload),
                                 0x7EU,
                                 0x7DU,
                                 encoded,
                                 sizeof(encoded)),
                8U,
                "frame encode escapes delimiter and escape bytes");

    for (index = 0U; index < 8U; ++index)
    {
        result = wcx_frame_decoder_push(&decoder, encoded[index]);
    }

    assert_u32((uint32_t)result, (uint32_t)WCX_FRAME_DECODER_COMPLETE,
               "frame decoder reports complete frame");
    assert_size(wcx_frame_decoder_length(&decoder), sizeof(payload),
                "frame decoder restores payload length");
    assert_u8(wcx_frame_decoder_data(&decoder)[1], 0x7EU,
              "frame decoder unescapes delimiter");
    assert_u8(wcx_frame_decoder_data(&decoder)[2], 0x7DU,
              "frame decoder unescapes escape byte");

    wcx_frame_decoder_init(&decoder, decoded, 2U, 0x7EU, 0x7DU);
    for (index = 0U; index < 8U; ++index)
    {
        result = wcx_frame_decoder_push(&decoder, encoded[index]);
    }
    assert_u32((uint32_t)result, (uint32_t)WCX_FRAME_DECODER_OVERFLOW,
               "frame decoder reports overflow when payload exceeds buffer");
}

static void test_calibration(void)
{
    wcx_linear_calibration_t linear;
    wcx_affine_calibration_t affine;

    wcx_linear_calibration_init(&linear, 0.0f, 1023.0f, 0.0f, 5.0f, true);
    assert_float_close(wcx_linear_calibration_apply(&linear, 511.5f), 2.5f, 0.001f,
                       "linear calibration maps midpoint");
    assert_float_close(wcx_linear_calibration_apply(&linear, 1200.0f), 5.0f, 0.0001f,
                       "linear calibration clamps output when configured");
    assert_float_close(wcx_linear_calibration_inverse(&linear, 2.5f), 511.5f, 0.01f,
                       "linear calibration inverts scaled value");

    wcx_affine_calibration_init(&affine, 2.0f, -1.0f);
    assert_float_close(wcx_affine_calibration_apply(&affine, 3.0f), 5.0f, 0.0001f,
                       "affine calibration applies scale and offset");
    assert_float_close(wcx_affine_calibration_inverse(&affine, 5.0f), 3.0f, 0.0001f,
                       "affine calibration inverse restores value");
}

static void test_fixed_point(void)
{
    wcx_q16_16_t one_and_half = wcx_q16_16_from_float(1.5f);
    wcx_q16_16_t two = wcx_q16_16_from_int(2);
    wcx_q16_16_t product = wcx_q16_16_mul(one_and_half, two);
    wcx_q16_16_t quotient = wcx_q16_16_div(wcx_q16_16_from_int(3), two);

    assert_float_close(wcx_q16_16_to_float(one_and_half), 1.5f, 0.0001f,
                       "Q16.16 round-trips float conversion");
    assert_u32((uint32_t)wcx_q16_16_to_int(wcx_q16_16_from_int(7)), 7U,
               "Q16.16 converts integer values");
    assert_float_close(wcx_q16_16_to_float(product), 3.0f, 0.0001f,
                       "Q16.16 multiply preserves fractional values");
    assert_float_close(wcx_q16_16_to_float(quotient), 1.5f, 0.0001f,
                       "Q16.16 divide preserves fractional values");
    assert_float_close(wcx_q16_16_to_float(wcx_q16_16_clamp(product,
                                                             wcx_q16_16_from_int(0),
                                                             wcx_q16_16_from_int(2))),
                       2.0f,
                       0.0001f,
                       "Q16.16 clamp bounds values");
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
    test_timer();
    test_fsm();
    test_protocol();
    test_calibration();
    test_fixed_point();

    if (g_failures != 0)
    {
        fprintf(stderr, "\n%d of %d checks failed.\n", g_failures, g_checks);
        return EXIT_FAILURE;
    }

    printf("All %d checks passed.\n", g_checks);
    return EXIT_SUCCESS;
}

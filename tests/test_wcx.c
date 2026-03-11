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

static void assert_i32(int32_t actual, int32_t expected, const char *message)
{
    g_checks++;

    if (actual != expected)
    {
        g_failures++;
        fprintf(stderr, "FAIL: %s (expected=%ld actual=%ld)\n",
                message,
                (long)expected,
                (long)actual);
    }
}

/* ---- Hysteresis tests ---- */

static void test_hysteresis(void)
{
    /* NULL guard */
    assert_true(!wcx_hysteresis_update(NULL, 0.0f), "hysteresis NULL guard");
    assert_true(!wcx_hysteresis_state(NULL), "hysteresis state NULL guard");

    wcx_hysteresis_t h;
    wcx_hysteresis_init(&h, 20.0f, 80.0f, false);
    assert_true(!wcx_hysteresis_state(&h), "hysteresis starts low");

    /* Value between thresholds should not change state */
    wcx_hysteresis_update(&h, 50.0f);
    assert_true(!wcx_hysteresis_state(&h), "hysteresis stays low in band");

    /* Rise above high threshold triggers */
    wcx_hysteresis_update(&h, 81.0f);
    assert_true(wcx_hysteresis_state(&h), "hysteresis goes high above upper");

    /* Drop into band should stay high */
    wcx_hysteresis_update(&h, 50.0f);
    assert_true(wcx_hysteresis_state(&h), "hysteresis stays high in band");

    /* Drop below low threshold resets */
    wcx_hysteresis_update(&h, 19.0f);
    assert_true(!wcx_hysteresis_state(&h), "hysteresis goes low below lower");

    /* Threshold detector */
    wcx_threshold_detector_t det;
    wcx_threshold_detector_init(&det, 10.0f, 80.0f, 95.0f, 5.0f);
    assert_true(wcx_threshold_detector_zone(&det) == WCX_ZONE_NORMAL,
                "threshold det starts normal");

    wcx_threshold_detector_update(&det, 5.0f);
    assert_true(wcx_threshold_detector_zone(&det) == WCX_ZONE_LOW,
                "threshold det goes low");

    /* Must exceed threshold + hysteresis to return to normal */
    wcx_threshold_detector_update(&det, 12.0f);
    assert_true(wcx_threshold_detector_zone(&det) == WCX_ZONE_LOW,
                "threshold det stays low inside hysteresis");

    wcx_threshold_detector_update(&det, 16.0f);
    assert_true(wcx_threshold_detector_zone(&det) == WCX_ZONE_NORMAL,
                "threshold det returns to normal above hyst");

    wcx_threshold_detector_update(&det, 85.0f);
    assert_true(wcx_threshold_detector_zone(&det) == WCX_ZONE_HIGH,
                "threshold det goes high");

    wcx_threshold_detector_update(&det, 96.0f);
    assert_true(wcx_threshold_detector_zone(&det) == WCX_ZONE_CRITICAL,
                "threshold det goes critical");

    wcx_threshold_detector_update(&det, 92.0f);
    assert_true(wcx_threshold_detector_zone(&det) == WCX_ZONE_CRITICAL,
                "threshold det stays critical inside hyst");

    wcx_threshold_detector_update(&det, 89.0f);
    assert_true(wcx_threshold_detector_zone(&det) == WCX_ZONE_HIGH,
                "threshold det back to high below critical-hyst");
}

/* ---- Lookup-table interpolation tests ---- */

static void test_lut(void)
{
    /* NULL guards */
    assert_true(!wcx_lut_init(NULL, NULL, 0), "lut init NULL guard");
    assert_float_close(wcx_lut_lookup(NULL, 0.0f), 0.0f, 0.001f,
                       "lut lookup NULL guard");

    static const wcx_lut_point_t pts[] = {
        {0.0f, 0.0f},
        {10.0f, 100.0f},
        {20.0f, 200.0f},
        {30.0f, 400.0f}};

    wcx_lut_t lut;
    assert_true(wcx_lut_init(&lut, pts, 4), "lut init succeeds");

    /* Exact match */
    assert_float_close(wcx_lut_lookup(&lut, 10.0f), 100.0f, 0.001f,
                       "lut exact match");

    /* Interpolation */
    assert_float_close(wcx_lut_lookup(&lut, 5.0f), 50.0f, 0.001f,
                       "lut linear interpolation midpoint");
    assert_float_close(wcx_lut_lookup(&lut, 25.0f), 300.0f, 0.001f,
                       "lut linear interpolation upper");

    /* Extrapolation clamped */
    assert_float_close(wcx_lut_lookup(&lut, -5.0f), 0.0f, 0.001f,
                       "lut clamps below table");
    assert_float_close(wcx_lut_lookup(&lut, 50.0f), 400.0f, 0.001f,
                       "lut clamps above table");

    /* Integer lookup table */
    static const wcx_lut_ipoint_t ipts[] = {
        {0, 0},
        {100, 1000},
        {200, 3000}};

    wcx_ilut_t ilut;
    assert_true(wcx_ilut_init(&ilut, ipts, 3), "ilut init succeeds");
    assert_i32(wcx_ilut_lookup(&ilut, 50), 500, "ilut midpoint");
    assert_i32(wcx_ilut_lookup(&ilut, 150), 2000, "ilut second segment");
    assert_i32(wcx_ilut_lookup(&ilut, -10), 0, "ilut clamp low");
    assert_i32(wcx_ilut_lookup(&ilut, 300), 3000, "ilut clamp high");
}

/* ---- Event queue / pub-sub tests ---- */

typedef struct
{
    int count;
    uint8_t last_data;
} event_test_ctx_t;

static void event_handler_a(wcx_event_t event, void *context)
{
    event_test_ctx_t *ctx = (event_test_ctx_t *)context;
    ctx->count++;
    ctx->last_data = event.data;
}

static void event_handler_b(wcx_event_t event, void *context)
{
    (void)event;
    event_test_ctx_t *ctx = (event_test_ctx_t *)context;
    ctx->count++;
}

static void test_event(void)
{
    /* NULL guards */
    assert_true(!wcx_event_bus_init(NULL, NULL, 0, NULL, 0),
                "event bus init NULL guard");
    assert_true(!wcx_event_publish(NULL, 0, 0), "event publish NULL guard");
    assert_size(wcx_event_dispatch(NULL), 0U, "event dispatch NULL guard");

    wcx_event_t queue_buf[8];
    wcx_subscription_t sub_buf[4];
    wcx_event_bus_t bus;

    assert_true(wcx_event_bus_init(&bus, queue_buf, 8, sub_buf, 4),
                "event bus init OK");

    event_test_ctx_t ctx_a = {0, 0};
    event_test_ctx_t ctx_b = {0, 0};

    assert_true(wcx_event_subscribe(&bus, 1, event_handler_a, &ctx_a),
                "subscribe handler A for event 1");
    assert_true(wcx_event_subscribe(&bus, 2, event_handler_b, &ctx_b),
                "subscribe handler B for event 2");

    /* Publish and dispatch */
    assert_true(wcx_event_publish(&bus, 1, 42), "publish event 1");
    assert_true(wcx_event_publish(&bus, 2, 0), "publish event 2");
    assert_size(wcx_event_pending(&bus), 2U, "2 events pending");

    size_t dispatched = wcx_event_dispatch(&bus);
    assert_size(dispatched, 2U, "dispatched 2 events");
    assert_size(wcx_event_pending(&bus), 0U, "0 events after dispatch");
    assert_true(ctx_a.count == 1, "handler A called once");
    assert_u8(ctx_a.last_data, 42, "handler A received data");
    assert_true(ctx_b.count == 1, "handler B called once");

    /* Queue overflow */
    for (int i = 0; i < 8; i++)
    {
        wcx_event_publish(&bus, 1, (uint8_t)i);
    }
    assert_true(!wcx_event_publish(&bus, 1, 99), "publish fails when full");
    wcx_event_dispatch(&bus);
}

/* ---- Serialization tests ---- */

static void test_serialize(void)
{
    uint8_t buf[8];

    /* NULL guards */
    assert_size(wcx_pack_u8(NULL, 0, 0), 0U, "pack_u8 NULL guard");
    assert_size(wcx_pack_u16_be(NULL, 0, 0), 0U, "pack_u16_be NULL guard");
    assert_u8(wcx_unpack_u8(NULL, 0), 0U, "unpack_u8 NULL guard");

    /* u8 */
    assert_size(wcx_pack_u8(buf, 8, 0xAB), 1U, "pack_u8 returns 1");
    assert_u8(wcx_unpack_u8(buf, 1), 0xAB, "unpack_u8 correct");

    /* u16 big-endian */
    assert_size(wcx_pack_u16_be(buf, 8, 0x1234), 2U, "pack_u16_be returns 2");
    assert_u8(buf[0], 0x12, "pack_u16_be MSB first");
    assert_u8(buf[1], 0x34, "pack_u16_be LSB second");
    assert_u16(wcx_unpack_u16_be(buf, 2), 0x1234, "unpack_u16_be correct");

    /* u32 big-endian */
    assert_size(wcx_pack_u32_be(buf, 8, 0xDEADBEEF), 4U,
                "pack_u32_be returns 4");
    assert_u32(wcx_unpack_u32_be(buf, 4), 0xDEADBEEF,
               "unpack_u32_be correct");

    /* u16 little-endian */
    assert_size(wcx_pack_u16_le(buf, 8, 0x1234), 2U, "pack_u16_le returns 2");
    assert_u8(buf[0], 0x34, "pack_u16_le LSB first");
    assert_u8(buf[1], 0x12, "pack_u16_le MSB second");
    assert_u16(wcx_unpack_u16_le(buf, 2), 0x1234, "unpack_u16_le correct");

    /* u32 little-endian */
    assert_size(wcx_pack_u32_le(buf, 8, 0xCAFEBABE), 4U,
                "pack_u32_le returns 4");
    assert_u32(wcx_unpack_u32_le(buf, 4), 0xCAFEBABE,
               "unpack_u32_le correct");

    /* Signed round-trip */
    wcx_pack_i16_be(buf, 8, -1234);
    assert_i32(wcx_unpack_i16_be(buf, 2), -1234, "i16 be round-trip");

    wcx_pack_i32_be(buf, 8, -99999);
    assert_i32(wcx_unpack_i32_be(buf, 4), -99999, "i32 be round-trip");

    wcx_pack_i16_le(buf, 8, -5678);
    assert_i32(wcx_unpack_i16_le(buf, 2), -5678, "i16 le round-trip");

    wcx_pack_i32_le(buf, 8, -12345678);
    assert_i32(wcx_unpack_i32_le(buf, 4), -12345678, "i32 le round-trip");

    /* Buffer too small */
    assert_size(wcx_pack_u32_be(buf, 3, 0), 0U, "pack_u32_be rejects small buf");

    /* Packer/unpacker cursors */
    uint8_t pkt[16];
    wcx_packer_t pk;
    wcx_packer_init(&pk, pkt, sizeof(pkt));
    assert_true(wcx_packer_ok(&pk), "packer starts ok");
    wcx_packer_u8(&pk, 0x01);
    wcx_packer_u16_be(&pk, 0x0203);
    wcx_packer_u32_be(&pk, 0x04050607);
    wcx_packer_i16_be(&pk, -100);
    wcx_packer_i32_be(&pk, -200000);
    assert_size(wcx_packer_length(&pk), 13U, "packer wrote 13 bytes");
    assert_true(wcx_packer_ok(&pk), "packer still ok");

    wcx_unpacker_t up;
    wcx_unpacker_init(&up, pkt, wcx_packer_length(&pk));
    assert_u8(wcx_unpacker_u8(&up), 0x01, "unpacker u8");
    assert_u16(wcx_unpacker_u16_be(&up), 0x0203, "unpacker u16");
    assert_u32(wcx_unpacker_u32_be(&up), 0x04050607, "unpacker u32");
    assert_i32(wcx_unpacker_i16_be(&up), -100, "unpacker i16");
    assert_i32(wcx_unpacker_i32_be(&up), -200000, "unpacker i32");
    assert_size(wcx_unpacker_remaining(&up), 0U, "unpacker consumed all");
    assert_true(wcx_unpacker_ok(&up), "unpacker ok");

    /* Overflow detection */
    wcx_unpacker_u8(&up);
    assert_true(!wcx_unpacker_ok(&up), "unpacker overflowed");
}

/* ---- Integer filter tests ---- */

static void test_ifilter(void)
{
    /* NULL guards */
    assert_true(!wcx_imoving_average_init(NULL, NULL, 0),
                "imoving avg init NULL guard");
    assert_i32(wcx_imoving_average_value(NULL), 0,
               "imoving avg value NULL guard");
    assert_i32(wcx_iema_value(NULL), 0, "iema value NULL guard");

    /* Integer moving average */
    int32_t storage[4];
    wcx_imoving_average_t imf;
    assert_true(wcx_imoving_average_init(&imf, storage, 4),
                "imoving avg init OK");

    assert_i32(wcx_imoving_average_update(&imf, 100), 100,
               "imoving avg first sample");
    assert_i32(wcx_imoving_average_update(&imf, 200), 150,
               "imoving avg two samples");
    assert_i32(wcx_imoving_average_update(&imf, 300), 200,
               "imoving avg three samples");
    assert_i32(wcx_imoving_average_update(&imf, 400), 250,
               "imoving avg four samples (full)");
    /* Window shifts: oldest (100) removed */
    assert_i32(wcx_imoving_average_update(&imf, 400), 325,
               "imoving avg after window shift");
    assert_size(wcx_imoving_average_count(&imf), 4U,
                "imoving avg count stays at capacity");

    wcx_imoving_average_reset(&imf);
    assert_size(wcx_imoving_average_count(&imf), 0U,
                "imoving avg count after reset");

    /* Integer EMA */
    wcx_iema_t iema;
    wcx_iema_init(&iema, 2); /* shift=2 -> alpha ~= 0.25 */

    assert_i32(wcx_iema_update(&iema, 1000), 1000,
               "iema first sample is pass-through");
    /* (1000 - 1000) >> 2 = 0 -> 1000 unchanged if same value */
    assert_i32(wcx_iema_update(&iema, 1000), 1000,
               "iema steady input stays same");
    /* Step up: (2000 - 1000) >> 2 = 250 -> 1250 */
    assert_i32(wcx_iema_update(&iema, 2000), 1250,
               "iema step response first");

    wcx_iema_reset(&iema);
    assert_i32(wcx_iema_update(&iema, 500), 500,
               "iema after reset takes first sample");
}

/* ---- Watchdog tests ---- */

static bool g_watchdog_fired = false;
static void watchdog_cb(void *context)
{
    (void)context;
    g_watchdog_fired = true;
}

static void test_watchdog(void)
{
    assert_true(!wcx_watchdog_check(NULL, 0), "watchdog NULL guard");
    assert_true(!wcx_watchdog_is_expired(NULL), "watchdog expired NULL guard");

    wcx_watchdog_t wd;
    wcx_watchdog_init(&wd, 100, watchdog_cb, NULL);

    /* Not started yet — check should return false */
    assert_true(!wcx_watchdog_check(&wd, 200), "watchdog not started");

    wcx_watchdog_start(&wd, 1000);
    assert_true(!wcx_watchdog_check(&wd, 1050), "watchdog within deadline");
    assert_true(!wcx_watchdog_is_expired(&wd), "watchdog not expired");

    g_watchdog_fired = false;
    assert_true(wcx_watchdog_check(&wd, 1200), "watchdog expired after deadline");
    assert_true(g_watchdog_fired, "watchdog callback fired");
    assert_true(wcx_watchdog_is_expired(&wd), "watchdog remains expired");

    /* Kick resets it */
    wcx_watchdog_kick(&wd, 1200);
    assert_true(!wcx_watchdog_is_expired(&wd), "watchdog reset after kick");
    assert_true(!wcx_watchdog_check(&wd, 1250), "watchdog ok after kick");
    assert_true(wcx_watchdog_check(&wd, 1400), "watchdog expires again");
}

/* ---- Pulse counter tests ---- */

static void test_pulse(void)
{
    assert_u32(wcx_pulse_count(NULL), 0U, "pulse count NULL guard");
    assert_u32(wcx_pulse_period_ms(NULL), 0U, "pulse period NULL guard");

    wcx_pulse_t p;
    wcx_pulse_init(&p);

    /* No edges yet */
    assert_u32(wcx_pulse_count(&p), 0U, "pulse starts at 0");

    /* Rising edge */
    wcx_pulse_update(&p, false, 0);
    wcx_pulse_update(&p, true, 100);
    assert_u32(wcx_pulse_count(&p), 1U, "pulse counted first rise");
    assert_u32(wcx_pulse_period_ms(&p), 0U, "no period after first edge");

    /* Second rising edge */
    wcx_pulse_update(&p, false, 200);
    wcx_pulse_update(&p, true, 300);
    assert_u32(wcx_pulse_count(&p), 2U, "pulse counted second rise");
    assert_u32(wcx_pulse_period_ms(&p), 200U, "period = 200ms");
    assert_float_close(wcx_pulse_frequency_hz(&p), 5.0f, 0.001f,
                       "frequency = 5 Hz");

    /* Staying high does not count */
    wcx_pulse_update(&p, true, 400);
    assert_u32(wcx_pulse_count(&p), 2U, "no count while staying high");

    wcx_pulse_reset(&p);
    assert_u32(wcx_pulse_count(&p), 0U, "pulse count after reset");
}

/* ---- Rate limiter tests ---- */

static void test_rate_limiter(void)
{
    assert_float_close(wcx_rate_limiter_value(NULL), 0.0f, 0.001f,
                       "rate limiter NULL guard");
    assert_i32(wcx_irate_limiter_value(NULL), 0,
               "irate limiter NULL guard");

    /* Float rate limiter */
    wcx_rate_limiter_t rl;
    wcx_rate_limiter_init(&rl, 10.0f, 5.0f);

    /* First sample taken immediately */
    assert_float_close(wcx_rate_limiter_update(&rl, 100.0f), 100.0f, 0.001f,
                       "rate limiter first sample");

    /* Large step up: clamp to +10 */
    assert_float_close(wcx_rate_limiter_update(&rl, 200.0f), 110.0f, 0.001f,
                       "rate limiter clamps rise");

    /* Large step down: clamp to -5 */
    assert_float_close(wcx_rate_limiter_update(&rl, 0.0f), 105.0f, 0.001f,
                       "rate limiter clamps fall");

    /* Small change passes through */
    assert_float_close(wcx_rate_limiter_update(&rl, 107.0f), 107.0f, 0.001f,
                       "rate limiter passes small change");

    wcx_rate_limiter_reset(&rl);
    assert_float_close(wcx_rate_limiter_update(&rl, 50.0f), 50.0f, 0.001f,
                       "rate limiter reset takes new first sample");

    /* Integer rate limiter */
    wcx_irate_limiter_t irl;
    wcx_irate_limiter_init(&irl, 100, 50);

    assert_i32(wcx_irate_limiter_update(&irl, 1000), 1000,
               "irate limiter first sample");
    assert_i32(wcx_irate_limiter_update(&irl, 2000), 1100,
               "irate limiter clamps rise");
    assert_i32(wcx_irate_limiter_update(&irl, 0), 1050,
               "irate limiter clamps fall");
    assert_i32(wcx_irate_limiter_update(&irl, 1080), 1080,
               "irate limiter passes small change");
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
    test_hysteresis();
    test_lut();
    test_event();
    test_serialize();
    test_ifilter();
    test_watchdog();
    test_pulse();
    test_rate_limiter();

    if (g_failures != 0)
    {
        fprintf(stderr, "\n%d of %d checks failed.\n", g_failures, g_checks);
        return EXIT_FAILURE;
    }

    printf("All %d checks passed.\n", g_checks);
    return EXIT_SUCCESS;
}

# WCX

[![Tests](https://github.com/mihastele/arduino-libraries-wcx/actions/workflows/test.yml/badge.svg)](https://github.com/mihastele/arduino-libraries-wcx/actions/workflows/test.yml)

WCX is a reusable Arduino library implemented in portable C for small embedded systems. It is organized as a set of small, allocation-free modules that are easy to combine inside Arduino sketches, but the code is written to stay broadly useful outside the Arduino ecosystem as well.

## What is included

The first version of the library provides these modules:

- `wcx_timer`: lightweight non-blocking timers for polling-style state machines and loop logic.
- `wcx_ring_buffer`: fixed-capacity byte FIFO for serial queues, sensor packets, and producer-consumer pipelines.
- `wcx_debounce`: software debouncing for buttons, switches, and digital sensor edges.
- `wcx_scheduler`: cooperative periodic and one-shot task execution on top of `millis()`.
- `wcx_filter`: moving average and exponential moving average filters for noisy measurements.
- `wcx_pid`: a practical PID controller with bounded output and derivative-on-measurement.
- `wcx_crc`: CRC-8, CRC-16/CCITT, and XOR checksum helpers.
- `wcx_stats`: running min, max, average, and variance without storing every sample.
- `wcx_fsm`: table-driven finite state machines with optional enter, exit, and transition callbacks.
- `wcx_protocol`: framed byte-stream encoding and decoding with delimiter escaping.
- `wcx_calibration`: linear and affine calibration helpers for raw sensor values.
- `wcx_fixed_point`: Q16.16 fixed-point conversions and arithmetic helpers.
- `wcx_hysteresis`: hysteresis comparator and multi-zone threshold detector.
- `wcx_lut`: float and integer lookup-table interpolation with binary search.
- `wcx_event`: allocation-free event queue with publish/subscribe dispatch.
- `wcx_serialize`: byte-order-safe pack/unpack helpers and cursor-based packer/unpacker.
- `wcx_ifilter`: integer-only moving average and exponential moving average filters.
- `wcx_watchdog`: software watchdog / heartbeat monitor with expiry callback.
- `wcx_pulse`: pulse counter and frequency meter for digital edges.
- `wcx_rate_limiter`: float and integer slew-rate limiters for smooth setpoint changes.
- `wcx_common`: small timing and scaling helpers shared by the other modules.

## Design goals

- Plain C implementation for predictable compilation on resource-constrained boards.
- No dynamic allocation.
- No hidden hardware dependencies in the `.c` files.
- Arduino-friendly usage from sketches through a single `#include <wcx.h>`.
- APIs that accept caller-owned storage instead of allocating memory internally.

## Layout

- `src/wcx.h`: umbrella include for the full library.
- `src/*.h`: module APIs with brief Doxygen-style comments.
- `src/*.c`: implementations.
- `examples/`: ready-to-run sketches demonstrating typical usage.

## Installation

### Arduino IDE

1. Clone or copy this repository into your Arduino libraries directory.
2. Restart the Arduino IDE.
3. Open one of the examples from the library examples menu.

### PlatformIO

Add the repository as a local or git dependency, then include `wcx.h` from your sketch or source file.

## Testing

The core modules are covered by a native host-side test suite so they can be validated in CI without needing a physical Arduino board.

Run the tests locally with:

```sh
make test
```

The GitHub Actions workflow in `.github/workflows/test.yml` runs the same command on every push to `main` and on pull requests. The badge at the top of this README reflects the latest workflow status.

## Quick start

```c
#include <Arduino.h>
#include <wcx.h>

static uint8_t queueStorage[32];
static wcx_ring_buffer_t queue;
static wcx_debounce_t button;

void setup(void)
{
	pinMode(2, INPUT_PULLUP);
	wcx_ring_buffer_init(&queue, queueStorage, sizeof(queueStorage));
	wcx_debounce_init(&button, false, 20);
}

void loop(void)
{
	bool pressed = !digitalRead(2);
	wcx_debounce_update(&button, pressed, millis());

	if (wcx_debounce_rose(&button)) {
		wcx_ring_buffer_push(&queue, 0x42);
	}
}
```

## Module notes

### Ring buffer

The ring buffer stores bytes only. This keeps the implementation compact and makes it straightforward to use for serial transport, small packets, and framed protocols. For larger data structures, store serialized bytes or wrap the buffer with your own encoding layer.

### Debounce

The debounce module tracks a candidate state and only commits a new stable state after the configured interval has elapsed. This makes it suitable for polling in the main loop without interrupts.

### Scheduler

The scheduler is cooperative, not preemptive. Each task callback should return quickly and avoid blocking. This is a good fit for LED blinking, periodic sensor reads, time-based state transitions, and heartbeat messages.

### Timers and FSMs

The timer and FSM modules are meant to work together. Timers handle elapsed-time checks without blocking the main loop, while the FSM module keeps state transitions explicit and testable.

### Filters and PID

The filtering and control modules are intentionally independent. A common pattern is:

1. Read a noisy sensor.
2. Filter the measurement with a moving average or EMA.
3. Feed the filtered value into the PID controller.
4. Clamp the control output to the actuator range.

### CRC and checksums

These helpers are useful for validating messages sent over serial links, radios, and custom protocols.

### Framing, calibration, and fixed-point math

`wcx_protocol` implements a compact framed transport format for byte streams that need delimiter-safe encoding. `wcx_calibration` helps turn raw ADC or engineering-unit readings into usable values, and `wcx_fixed_point` is useful when you want deterministic fractional math without paying for floating-point everywhere.

### Hysteresis and threshold detection

`wcx_hysteresis` provides a simple comparator with separate high and low thresholds to prevent noisy signals from causing rapid toggling. The threshold detector extends this idea to four configurable zones (low, normal, high, critical) with a user-defined hysteresis band.

### Lookup-table interpolation

`wcx_lut` stores a sorted set of (x, y) points and performs linear interpolation between them using binary search. Values outside the table are clamped to the nearest endpoint. An integer-only variant (`wcx_ilut`) avoids floating-point entirely.

### Event queue and publish/subscribe

`wcx_event` is a lightweight event bus with caller-owned queue and subscription storage. Events carry a one-byte payload. Subscribers register interest in a specific event ID; `wcx_event_dispatch` drains the queue and invokes matching handlers.

### Serialization

`wcx_serialize` provides standalone pack/unpack functions for 8, 16, and 32-bit integers in both big-endian and little-endian formats. Cursor-based `wcx_packer_t` and `wcx_unpacker_t` wrappers track the current position and detect buffer overflows.

### Integer-only filters

`wcx_ifilter` provides `wcx_imoving_average_t` and `wcx_iema_t` — integer analogues of the floating-point filters in `wcx_filter`. The EMA uses a shift-based weighting so no division or floating-point is needed.

### Software watchdog

`wcx_watchdog` monitors whether a subsystem has checked in within its deadline. If the deadline expires, an optional callback fires. Call `wcx_watchdog_kick` periodically to reset the timer.

### Pulse counter

`wcx_pulse` counts rising edges and measures inter-edge period. It computes frequency in Hz from the most recent period. Useful for rotary encoders, flow meters, and RPM sensing.

### Rate limiter

`wcx_rate_limiter` constrains how fast a value can change per update step (slew-rate limiting). Separate rise and fall limits allow asymmetric ramping. An integer variant (`wcx_irate_limiter_t`) is included for MCUs without FPU.

## Examples

- `BasicDiagnostics`: shows debouncing, ring-buffered serial messages, running statistics, and CRC computation.
- `TaskBlink`: shows cooperative periodic task scheduling.
- `ControlLoopDemo`: shows a simple filtered PID loop driving a PWM output.

## Extending the library

All previously planned follow-on modules are now implemented. Reasonable next additions beyond this baseline would be:

- GPIO pin abstraction layer for portable digital I/O,
- multi-channel ADC multiplexing helpers,
- non-volatile storage wrappers for EEPROM / flash settings,
- MODBUS or CANopen protocol helpers,
- battery voltage monitoring with coulomb-counting estimates.

## Status

This repository now contains a broader baseline Arduino C utility library covering scheduling, control, framing, calibration, fixed-point math, and explicit state-machine building blocks. It should still be validated against your target board set in the Arduino IDE or PlatformIO, especially if you want stricter AVR-specific footprint constraints.

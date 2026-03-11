# WCX

[![Tests](https://github.com/mihastele/arduino-libraries-wcx/actions/workflows/test.yml/badge.svg)](https://github.com/mihastele/arduino-libraries-wcx/actions/workflows/test.yml)

WCX is a reusable Arduino library implemented in portable C for small embedded systems. It is organized as a set of small, allocation-free modules that are easy to combine inside Arduino sketches, but the code is written to stay broadly useful outside the Arduino ecosystem as well.

## What is included

The first version of the library provides these modules:

- `wcx_ring_buffer`: fixed-capacity byte FIFO for serial queues, sensor packets, and producer-consumer pipelines.
- `wcx_debounce`: software debouncing for buttons, switches, and digital sensor edges.
- `wcx_scheduler`: cooperative periodic and one-shot task execution on top of `millis()`.
- `wcx_filter`: moving average and exponential moving average filters for noisy measurements.
- `wcx_pid`: a practical PID controller with bounded output and derivative-on-measurement.
- `wcx_crc`: CRC-8, CRC-16/CCITT, and XOR checksum helpers.
- `wcx_stats`: running min, max, average, and variance without storing every sample.
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
bash tests/run-tests.sh
```

If you have `make` installed, `make test` is also available as a convenience wrapper.

The GitHub Actions workflow in `.github/workflows/test.yml` runs the same shell command on every push to `main` and on pull requests. The badge at the top of this README reflects the latest workflow status.

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

### Filters and PID

The filtering and control modules are intentionally independent. A common pattern is:

1. Read a noisy sensor.
2. Filter the measurement with a moving average or EMA.
3. Feed the filtered value into the PID controller.
4. Clamp the control output to the actuator range.

### CRC and checksums

These helpers are useful for validating messages sent over serial links, radios, and custom protocols.

## Examples

- `BasicDiagnostics`: shows debouncing, ring-buffered serial messages, running statistics, and CRC computation.
- `TaskBlink`: shows cooperative periodic task scheduling.
- `ControlLoopDemo`: shows a simple filtered PID loop driving a PWM output.

## Extending the library

Reasonable next modules for this library would be:

- finite state machines,
- protocol framing,
- sensor calibration helpers,
- fixed-point math utilities,
- non-blocking software timers.

## Status

This repository now contains a functional baseline Arduino C utility library. It should still be validated against your target board set in the Arduino IDE or PlatformIO, especially if you want stricter AVR-specific footprint constraints.

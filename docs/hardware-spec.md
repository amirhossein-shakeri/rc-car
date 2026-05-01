# RC Car Hardware Spec

This project targets the following hardware and wiring layout.

## Incident Log

- Previous motor driver: L9110S dual H-bridge.
- Incident: the L9110S module was burnt after being powered from a 3S Li-ion pack measuring about 12.08V.
- Root cause: 12.08V is at or beyond the practical safe edge for common L9110S breakout boards, and motor back-EMF/current spikes can exceed the chip/module limits.
- Decision: retire L9110S for this build and use TB6612FNG instead.
- Code status: L9110S support is isolated in `include/l9110s_motor_driver.h` and `src/l9110s_motor_driver.cpp`; the active build uses `include/tb6612fng_motor_driver.h` and `src/tb6612fng_motor_driver.cpp`.

## Power

- 3x Li-ion cells, 3.7V nominal each, measured about 12.08V when charged.
- Battery `V+` splits to TB6612FNG `VM` through a power switch/fuse and to buck converter `IN+`.
- Battery `V-` splits to TB6612FNG `GND` and to buck converter `IN-`.
- Buck converter `OUT+` -> ESP32 `VIN`.
- Buck converter `OUT-` -> ESP32 `GND`.
- All grounds are tied together (common ground).
- Add a bulk capacitor close to the motor driver, for example 100uF-470uF electrolytic across `VM` and `GND`, plus a 0.1uF ceramic if available.
- Do not connect the 12V battery rail to TB6612FNG `VCC`; `VCC` is logic power only.

## Motor Driver

- Active driver: TB6612FNG dual H-bridge.
- Two armature DC motors (tank-drive left/right).
- TB6612FNG typical limits to check against your exact board/datasheet:
  - `VM` motor supply operating range: about 2.5V-13.5V.
  - `VCC` logic supply range: about 2.7V-5.5V; use ESP32 `3V3`.
  - Current: commonly rated around 1.2A average per channel with higher short peaks; motor stall current must stay under the driver/module limit.
- If either motor spins the wrong way, swap that motor's two output wires or swap that side's two direction pins in code.

## TB6612FNG Wiring

Suggested active pin map in `src/main.cpp`:

| TB6612FNG pin | Connect to | Purpose |
| --- | --- | --- |
| `VM` | Battery `V+` motor rail, through switch/fuse | Motor power input |
| `VCC` | ESP32 `3V3` | Logic power input |
| `GND` | Common ground | Battery, buck, ESP32, and driver ground |
| `AO1` | Left motor lead 1 | Left motor output |
| `AO2` | Left motor lead 2 | Left motor output |
| `BO1` | Right motor lead 1 | Right motor output |
| `BO2` | Right motor lead 2 | Right motor output |
| `GND` | Common ground | Use all available ground pins if practical |
| `GND` | Common ground | Use all available ground pins if practical |
| `PWMB` | ESP32 GPIO16 | Right motor speed PWM, LEDC channel 1 |
| `BIN1` | ESP32 GPIO18 | Right motor direction 1 |
| `BIN2` | ESP32 GPIO19 | Right motor direction 2 |
| `STBY` | ESP32 GPIO17 | Driver enable/standby |
| `AIN1` | ESP32 GPIO23 | Left motor direction 1 |
| `AIN2` | ESP32 GPIO22 | Left motor direction 2 |
| `PWMA` | ESP32 GPIO21 | Left motor speed PWM, LEDC channel 0 |

Wiring notes:

- `STBY` must be driven high for movement. Firmware drives it high only while moving and low on stop.
- Keep motor wires physically away from ESP32 antenna and signal wires when possible.
- Put the buck converter, ESP32, and TB6612FNG grounds on the same ground rail before testing motors.
- Test first with the car raised off the ground and with the motor battery switch within reach.
- The ESP32 pins above avoid input-only pins and avoid the main boot strap pins used by many NodeMCU-32S boards.

## Legacy L9110S Wiring

This is retained only for reference and for the isolated L9110S driver module. Do not use it with the current 12.08V 3S pack.

- Driver: L9110S dual H-bridge.
- Previous ESP32 control pins:
  - GPIO23: left A
  - GPIO22: left B
  - GPIO18: right A
  - GPIO19: right B

## Feedback (Optional)

- Buzzer on GPIO15.
- NodeMCU-32S built-in LED available (commonly GPIO2).

## PWM Compatibility Baseline

Motor PWM behavior should remain compatible with `src/main.cpp.basic`:

- Active TB6612FNG motor channels: 0 for `PWMA`, 1 for `PWMB`
- Legacy L9110S motor channels: 0, 1, 2, 3
- Frequency: 20000 Hz
- Resolution: 10-bit
- Duty range: -1023..1023 mapped by direction

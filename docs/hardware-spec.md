# RC Car Hardware Spec

This project targets the following hardware and wiring layout.

## Power

- 3x Li-ion cells, 3.7V nominal each, ~12.08V total when fully charged.
- Battery `V+` -> L9110S `VCC` and buck converter `IN+`.
- Battery `V-` -> L9110S `GND` and buck converter `IN-`.
- Buck converter `OUT+` -> ESP32 `VIN`.
- Buck converter `OUT-` -> ESP32 `GND`.
- All grounds are tied together (common ground).

## Motor Driver

- Driver: L9110S dual H-bridge.
- Two armature DC motors (tank-drive left/right).
- ESP32 control pins:
  - GPIO18
  - GPIO19
  - GPIO22
  - GPIO23

## Feedback (Optional)

- Buzzer on GPIO15.
- NodeMCU-32S built-in LED available (commonly GPIO2).

## PWM Compatibility Baseline

Motor PWM behavior should remain compatible with `src/main.cpp.basic`:

- Motor channels: 0, 1, 2, 3
- Frequency: 20000 Hz
- Resolution: 10-bit
- Duty range: -1023..1023 mapped by direction

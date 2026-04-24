# AGENTS

## Project Goal
- Build a tank-drive RC car powered by ESP32 and controlled from a phone over Wi-Fi.

## Quality Priorities
- Highest quality parameters:
  - clean-code
  - readability
  - simplicity
  - robustness

## Architecture Rules
- Keep logic and implementation modular.
- Keep each subsystem in separate files.
- Motor/driving control logic must stay isolated from networking and HTTP concerns.
- Wi-Fi hotspot setup and networking logic must live in dedicated networking files.
- Web/HTTP server logic for serving the phone control page and handling drive commands must live in dedicated server files.

#include <Arduino.h>

#include "control_server.h"
#include "motor_controller.h"
#include "network_manager.h"

namespace
{
  constexpr MotorPins kPins{
      23,
      22,
      18,
      19,
  };

  constexpr MotorChannels kChannels{
      0,
      1,
      2,
      3,
  };

  constexpr uint32_t kPwmFrequency = 20000;
  constexpr uint8_t kPwmResolution = 10;
  constexpr char kApSsid[] = "RC-Car-ESP32";
  constexpr char kApPassword[] = "rc-car-2026";
} // namespace

MotorController motorController(kPins, kChannels);
NetworkManager networkManager;
ControlServer controlServer(80);

void setup()
{
  Serial.begin(115200);
  delay(500);
  Serial.println("[boot] Tank-drive RC car booting...");

  motorController.begin(kPwmFrequency, kPwmResolution);

  if (!networkManager.beginAccessPoint(kApSsid, kApPassword))
  {
    Serial.println("[boot] Hotspot failed to start. Retrying in loop...");
  }

  controlServer.begin(
      [](int leftPercent, int rightPercent, float speedMultiplier, float leftToRightRatio)
      {
        motorController.setSpeedMultiplier(speedMultiplier);
        motorController.setLeftToRightRatio(leftToRightRatio);
        motorController.driveTankPercent(leftPercent, rightPercent);
      },
      []()
      {
        motorController.stop();
      });

  Serial.println("[boot] Setup complete.");
}

void loop()
{
  controlServer.loop();
}

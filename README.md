# ESP32 Knock Sensor and MQTT Client

This project uses an ESP32 to detect knocks (like on a door or surface) and send events via MQTT to a home automation system such as Home Assistant. It also provides visual feedback using a NeoPixel LED.

## Features

- Detects knocks on a sensor connected to pin `D4`.
- Counts knocks and publishes an MQTT message after a short delay.
- Visual feedback using a NeoPixel LED:
    - **Green**: Idle / ready.
    - **Red**: Knock detected.
- Supports OTA (Over-the-Air) updates for easy firmware upgrades.

## Hardware

- **ESP32**
- **Piezo knock sensor** connected to pin `4` (analog input).
- **NeoPixel LED** connected to pin `10` (can be adjusted in code).
    - This is just the built in LED on WaveShare C3-Zero

## Software / Libraries

- [Arduino Core for ESP32](https://github.com/espressif/arduino-esp32)
- [PubSubClient](https://github.com/knolleary/pubsubclient) (for MQTT)
- [Adafruit NeoPixel](https://github.com/adafruit/Adafruit_NeoPixel)
- Arduino OTA for firmware updates

## Wiring

| Component     | ESP32 Pin                         |
| -             | -                                 |
| Knock Sensor  | GPIO 4 (1M Resistor to Ground)    |

## Configuration

Make a secrets.h file inside knock-sensor like:

```cpp
#pragma once

#define SSID "YourWiFiSSID"
#define PASSWORD "YourWiFiPassword"

#define MQTT_SERVER "MQTTBrokerIPAddress"
#define MQTT_USER   "MQTTUserName"
#define MQTT_PASS   "MQTTPassword"

```

My Home Assistant YAML Configuration:

```yaml
alias: Door Knock Door Bell
description: "Triggers doorbell on door knock"
triggers:
  - trigger: mqtt
    topic: homeassistant/event/knock-alert
conditions:
  - condition: template
    value_template: |
      {{ trigger.payload_json.knocks >= 2 }}
actions:
  - action: media_player.play_media
    metadata: {}
    data:
      media:
        media_content_id: media-source://media_source/local/doorbell.wav
        media_content_type: audio/x-wav
        metadata:
          title: doorbell.wav
          thumbnail: null
          media_class: music
          children_media_class: null
          navigateIds:
            - {}
            - media_content_type: app
              media_content_id: media-source://media_source
          browse_entity_id: media_player.butter_bot
    target:
      entity_id: media_player.butter_bot
mode: single

```
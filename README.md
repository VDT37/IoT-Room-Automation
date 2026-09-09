# IoT Room Automation

An occupancy-aware room automation system built on a PIC16F887 microcontroller and an ESP8266 (NodeMCU) IoT gateway. The room decides for itself when lights and cooling should be on, based on how many people are actually inside, and the same appliances can be overridden remotely over WiFi (MQTT via Adafruit IO) or locally over Bluetooth.

B.Tech Minor Project, Electronics and Communication Engineering.

## What it does

Two IR sensor pairs are mounted at the main door, one outside and one inside. The order in which they break tells the firmware whether a person entered or exited, so the system maintains a live headcount rather than a simple motion trigger. When the count goes from zero to one, the room light and the cooling system come on. When the count returns to zero, both are switched off. The count is written to the PIC's internal EEPROM, so it survives a power cycle.

A second IR pair on the bathroom door runs the same entry/exit logic for the bathroom light, and a PIR sensor covers the bathroom for presence confirmation.

A DS3231 real-time clock over I2C drives a 16x2 LCD showing time, date, weekday and ambient temperature, with custom CGRAM glyphs for the degree and clock symbols. The LCD doubles as a settings interface: three push buttons (Mode, Next, Select) walk an 11-state finite state machine that lets the user set the hour, minute, AM/PM, date, month, year and weekday, and read back the current occupancy count. Buttons are debounced in software with press-and-release event handling, and a long press auto-increments.

The DS3231's on-chip temperature register also drives a proportional cooling output. The reading is mapped through `CCPR2L = 60*T - 1100` onto the CCP2 PWM channel, so duty cycle scales linearly from roughly 10 percent at 20 C to full at 35 C. On the prototype this drives an LED standing in for a fan or cooler.

## Remote control

The PIC and the NodeMCU talk over UART. The NodeMCU has its hardware UART tied up by the USB serial monitor, so a SoftwareSerial port is used for the link to the PIC.

Over WiFi, the NodeMCU subscribes to two Adafruit IO MQTT feeds, `Room Light` and `Cooling System`. An ON or OFF payload on either feed is translated into a single command character (`A`, `B`, `C`, `D`) forwarded to the PIC, which the PIC receives on its USART and acts on. Connection loss on either WiFi or MQTT is retried automatically.

Over Bluetooth, an HC-05 module connected to the NodeMCU accepts characters `a` through `d` from a phone app and forwards them the same way.

Manual override interacts with the automatic occupancy logic rather than fighting it. Separate flags track whether an appliance was last switched by a person walking through the door or by a remote command, so a light turned on from the phone is not immediately switched off by the occupancy rules, and vice versa.

## Hardware

- PIC16F887 microcontroller, 20 MHz external crystal, programmed in MPLAB X with the XC8 compiler
- ESP8266 ESP-12E NodeMCU development board, Arduino framework
- DS3231 RTC module on I2C, used for both timekeeping and temperature
- 16x2 HD44780 character LCD in 4-bit mode
- 5 IR sensor modules (two pairs on doors, plus spares) and one PIR motion sensor
- HC-05 Bluetooth module
- Relay module for mains appliance switching
- 3 push buttons for the menu interface, 2 switches for WiFi/Bluetooth mode selection

## Repository layout

```
IoT_Room_Automation.X/        MPLAB X project: PIC16F887 firmware
  Room_Automation.c           Main application, ISR, FSM, sensor and appliance logic
  Config_Hardware.h           Configuration bits and clock frequency
  I2C.h, DS3231_I2C.h         I2C bus driver and RTC read/write
  Usart_Config.h              USART init, transmit and receive
  Custom_Characters.h         CGRAM glyph definitions
NodeMCU Arduino Code/
  Room_Automation/            The ESP8266 sketch and its secrets header
  Adafruit_MQTT*              Vendored Adafruit MQTT library and examples
Proteus Simulation/           Proteus circuit simulation of the PIC subsystem
Report/                       LaTeX source, figures and compiled project report
Home Automation IEEE Papers/  Reference papers surveyed during the literature review
Project Test Cases.xlsx       Test matrix used for verification
```

## Building and running

PIC firmware:

1. Open `IoT_Room_Automation.X` in MPLAB X IDE.
2. Select the XC8 compiler toolchain and PIC16F887 as the device.
3. Build, then flash with a PICkit or compatible programmer.

NodeMCU sketch:

1. Copy `NodeMCU Arduino Code/Room_Automation/secrets_example.h` to `secrets.h` in the same folder and fill in your WiFi SSID, WiFi password, Adafruit IO username and Adafruit IO key. `secrets.h` is gitignored and must never be committed.
2. Install the ESP8266 board package in the Arduino IDE and select the NodeMCU 1.0 (ESP-12E) board.
3. Add the vendored `Adafruit_MQTT` library to your Arduino libraries folder, or install Adafruit MQTT Library from the Library Manager.
4. Upload at 115200 baud.

On Adafruit IO, create two feeds named exactly `Room Light` and `Cooling System`, then add ON/OFF toggle blocks to a dashboard to control them.

## Known limitations

- The IR entry/exit logic assumes one person crosses the door at a time. Two people walking through together are counted once.
- The occupancy count is stored as separate units and tens digits in EEPROM, which caps the display at 99.
- The cooling output is a PWM-driven LED on the prototype, not a real driven fan or compressor.
- MQTT runs on port 1883 without TLS, which is acceptable for a lab prototype but not for deployment.
- Bluetooth and WiFi modes are selected by hardware switches and are not intended to be used at the same time.

## Media not in this repository

The project demo video and the packaged `.rar` archive of the full working directory exceed GitHub's 100 MB per-file limit and are excluded by `.gitignore`.

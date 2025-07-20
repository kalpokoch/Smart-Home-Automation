# Arduino Smart Home Controller

An Arduino-based system for gas leak detection, automation of fan/light, SMS control via GSM, and LCD status display.

## Features
- Gas leak detection with alerts and auto-fan activation.
- Remote control of devices via SMS (e.g., "ONL" for light on).
- Real-time LCD updates.
- Safety lock: Can't turn off fan during gas leak.

## Hardware Requirements
- Arduino board (e.g., Uno).
- Gas sensor (MQ series).
- LDR sensor.
- 2x Relays (for fan and light).
- Buzzer.
- Servo motor.
- GSM module (SIM900 or similar).
- I2C LCD display (16x2).
- Jumper wires and breadboard.

## Software Requirements
- Arduino IDE.
- Libraries: SoftwareSerial, LiquidCrystal_I2C, Servo.

## Setup Instructions
1. Connect hardware as per the circuit diagram.
2. Install required libraries in Arduino IDE.
3. Upload the code from `src/smart_home_controller.ino`.
4. Insert a SIM card in GSM module and test SMS.

## Diagrams
- [Circuit Diagram](diagrams/circuit_diagram.png)
- [Flowchart](diagrams/flowchart.png)
- [Block Diagram](diagrams/block_diagram.png)

## Usage
- Send SMS commands like "ONL" to control light.
- Monitor LCD for status.

## License
MIT License

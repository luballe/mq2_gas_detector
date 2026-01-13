# MQ-2 Gas Detector

An Arduino-based gas detection system using the MQ-2 sensor with LCD display, audible alarm, and automatic exhaust fan control.

## Features

- **Gas Detection**: Monitors combustible gases (LPG, propane, methane, hydrogen, smoke) using the MQ-2 sensor
- **Real-time Display**: 16x2 I2C LCD shows gas levels, voltage readings, and system status
- **Audible Alarm**: Intermittent buzzer alerts when gas levels exceed threshold
- **Automatic Ventilation**: Relay-controlled exhaust fan activates during gas detection
- **Purge Cycle**: Fan continues running after gas clears to ensure complete ventilation
- **Signal Smoothing**: Moving average filter reduces sensor noise for stable readings
- **Serial Monitoring**: Real-time data output for debugging and logging

## Hardware Components

| Component | Specification |
|-----------|---------------|
| Microcontroller | Arduino Uno (or compatible) |
| Gas Sensor | MQ-2 |
| Display | 16x2 LCD with I2C adapter (address 0x27) |
| Buzzer | Active buzzer, 5V |
| Relay Module | 5V single-channel (for fan control) |
| Exhaust Fan | 12V DC (powered separately) |

## Wiring

| Component | Arduino Pin |
|-----------|-------------|
| MQ-2 (AOUT) | A0 |
| Relay (IN) | D4 |
| Buzzer (+) | D8 |
| LCD (SDA) | A4 |
| LCD (SCL) | A5 |

**Note**: The relay module uses active LOW logic. Connect the exhaust fan to the relay's normally open (NO) terminals.

## Installation

1. Install the required library in Arduino IDE:
   - **LiquidCrystal_I2C** by Frank de Brabander

2. Clone and upload:
   ```bash
   git clone https://github.com/luballe/mq2_gas_detector.git
   cd mq2_gas_detector
   arduino-cli compile --fqbn arduino:avr:uno mq2_gas_detector.ino
   arduino-cli upload -p /dev/cu.usbmodem* --fqbn arduino:avr:uno mq2_gas_detector.ino
   ```

3. Open Serial Monitor at 9600 baud to view readings.

## Configuration

Adjust these constants at the top of `mq2_gas_detector.ino`:

| Parameter | Default | Description |
|-----------|---------|-------------|
| `INITIALIZATION_TIME` | 60000 ms | Sensor warm-up period before detection activates |
| `GAS_THRESHOLD` | 375 | Analog value (0-1023) that triggers alarm |
| `PURGE_TIME` | 60000 ms | Duration fan runs after gas clears |
| `NUM_READINGS` | 30 | Moving average window size for smoothing |
| `BUZZER_INTERVAL` | 1000 ms | Buzzer on/off toggle interval |

## Operation

1. **Warm-up Phase**: On power-up, the sensor requires 60 seconds to stabilize. The LCD displays countdown and current readings.

2. **Normal Monitoring**: Once ready, the system continuously monitors gas levels. LCD shows the smoothed analog value and voltage.

3. **Alert Mode**: When gas exceeds the threshold:
   - Buzzer sounds intermittently
   - Relay activates the exhaust fan
   - LCD displays voltage and gas value

4. **Purge Mode**: After gas drops below threshold:
   - Buzzer stops
   - Fan continues running for the configured purge time
   - Ensures complete gas evacuation before returning to standby

## Serial Output

Connect via serial monitor (9600 baud) to see:
```
Raw value: 245 | Smoothed: 238 | Voltage: 1.16V | Status: READY - Normal
Raw value: 412 | Smoothed: 389 | Voltage: 1.90V | Status: ALERT! - Buzzer ACTIVE and Fan RUNNING
```

## License

MIT License

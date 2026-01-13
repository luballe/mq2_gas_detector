# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Arduino-based MQ-2 gas detector with LCD display, buzzer alarm, and exhaust fan control via relay.

## Hardware Configuration

- **MQ-2 Sensor**: Analog pin A0
- **16x2 I2C LCD**: Address 0x27
- **Buzzer**: Digital pin 8
- **Relay (fan control)**: Digital pin 4 (active LOW)

## Build and Upload

This is an Arduino sketch. Use Arduino IDE or arduino-cli:

```bash
# Compile
arduino-cli compile --fqbn arduino:avr:uno mq2_gas_detector.ino

# Upload (adjust port as needed)
arduino-cli upload -p /dev/cu.usbmodem* --fqbn arduino:avr:uno mq2_gas_detector.ino

# Monitor serial output
arduino-cli monitor -p /dev/cu.usbmodem* -c baudrate=9600
```

## Dependencies

- Wire.h (built-in)
- LiquidCrystal_I2C library

## Key Parameters

All configurable constants are at the top of `mq2_gas_detector.ino`:
- `INITIALIZATION_TIME`: Sensor warm-up period (default 60s)
- `GAS_THRESHOLD`: Analog value triggering alarm (default 375)
- `PURGE_TIME`: Fan continues running after gas clears (default 60s)
- `NUM_READINGS`: Moving average window size (default 30)

## System States

1. **INIT**: Sensor warming up, no alarms triggered
2. **READY**: Normal operation, monitoring gas levels
3. **ALERT**: Gas above threshold, buzzer sounds intermittently, fan runs
4. **PURGE**: Gas cleared but fan continues for configured duration

# ESP32_Adafruit_STEMMA_Soil_Sensor_Example

[![Platform: ESP-IDF](https://img.shields.io/badge/ESP--IDF-v5.0.2-blue.svg)](https://docs.espressif.com/projects/esp-idf/en/stable/get-started/)
[![license](https://img.shields.io/github/license/mashape/apistatus.svg)]()

## Introduction

This is an example application for the Adafruit STEMMA Soil Sensor, an I2C-capable sensor that measures both moisture and temperature.

It is written and tested for v3.3 of the [ESP-IDF](https://github.com/espressif/esp-idf) environment, using the xtensa-esp32-elf toolchain (gcc version 5.2.0).

Ensure that submodules are cloned:

    $ git clone --recursive https://github.com/YourUserName/ESP32_Adafruit_STEMMA_Soil_Sensor_Example.git

Build the application with:

    $ cd ESP32_Adafruit_STEMMA_Soil_Sensor_Example.git
    $ idf.py menuconfig
    $ idf.py build
    $ idf.py -p (PORT) flash monitor

The program should detect your connected sensor and display readings on the console.

## Dependencies

This application makes use of the following components (included as submodules):

 * components/[ESP32_Adafruit_STEMMA_Soil_Sensor](https://github.com/Krzyshio/ESP32_I2C_Adafruit_STEMMA_Soil_Sensor)

## Hardware

To run this example, connect the STEMMA Soil Sensor to two GPIOs on the ESP32 (I2C SDA and SCL).

## Features

This example demonstrates the use of the ESP32_Adafruit_STEMMA_Soil_Sensor component. It demonstrates:

 * Sensor initialization.
 * Reading of moisture and temperature.
 * Error handling and reporting.

## License

The code in this project is licensed under the MIT license - see LICENSE for details.

## Links

 * [Adafruit STEMMA Soil Sensor documentation](https://learn.adafruit.com/adafruit-stemma-soil-sensor-i2c-capacitive-moisture-sensor)
 * [Espressif IoT Development Framework for ESP32](https://github.com/espressif/esp-idf)
 * [ESP32_Adafruit_STEMMA_Soil_Sensor](https://github.com/Krzyshio/ESP32_I2C_Adafruit_STEMMA_Soil_Sensor)

## Acknowledgements

 * "I2C" is a registered trademark of Phillips Corporation.
 * "Adafruit" and "STEMMA" are trademarks of Adafruit Industries.

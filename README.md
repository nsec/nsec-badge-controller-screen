NorthSec 2023 Badge Controller Screen
===================

## Overview

This device is part of a challenge for NorthSec 2023. The device could be used as an interface for controlling the LEDs on this year's badge over a bluetooth low energy mesh network.

[scene file](sd-card-data/partition-1/User%20Manual/Picture%201.png)

## Hardware

The device is built around the `ESP32-2432S028R` development board: https://www.aliexpress.com/item/1005004502250619.html?spm=a2g0o.order_list.order_list_main.144.76881802IyLAn1. It has a `ESP32-D0WDQ6` microcontroller with wifi and bluetooth connectivity.

## Building the firmware

The badge firmware is based on
[ESP-IDF](https://www.espressif.com/en/products/sdks/esp-idf), the exact version used is commited to this repository since it has local changes. In particular `#define ALARM_CBS_NUM` needs to be increased to `100` for BLE mesh connectivity and `FF_USE_LABEL` needs to be enabled for one of the challenge.

```bash
git clone https://github.com/nsec/nsec-badge-controller-screen.git
cd nsec-badge-controller-screen/
./esp-idf/install.sh

# You need to manually install the Pillow package.
pip install Pillow
```

The installation procedure for your OS may differ a little, please consult the
[documentation website](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/index.html#installation-step-by-step)
if you have any difficulties.

Once the installation is complete you can build the firmware and flash it to
the badge:

```bash
source esp-idf/export.sh
cd esp32/
idf.py build
```

If for some reason `idf.py` is not able to complete the operation, refer to the
[ESP-IDF documentation](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/index.html#step-9-flash-onto-the-device).

## Flashing

Devices used during the competition were using ESP's secure boot v2 to prevent solving challenges by simply reading the flash data. The flash encryption key and the secure boot key are included in this repository.

If you are flashing a brand new, non secure-boot board:

```bash
idf.py build flash monitor
```

If you want to update the firmware on a board that has already been flashed with secure boot, for instance one of those we gave to participants during the competition:

```bash
./esp32/secure_reflash.sh /dev/ttyUSB0
```

If, for some reason, you want to flash a board with secure boot for the first time (this is the script that was used to prepare the boards before the competition):

```bash
./esp32/secure_flash.sh /dev/ttyUSB0
```

## Using the boards

You can use `idf.py menuconfig` to enable the `BADGE_MESH_ADMIN_COMMANDS` option, this adds a menu to the serial console on the badge with additional BLE mesh commands. Equiped with the hardware badge and the controller screen from NorthSec 2023, you can connect to the console on the serial port and use these commands to test the various bluetooth mesh commands.

Some of the BLE mesh commands that were implemented are:
* Setting the time on the controller screen
* Setting the controller screen's team name
* Performing a census of all devices connected to the BLE mesh
* Querying basic info on a specific mesh device
* Controlling the LEDs on the hardware badge
* Sending messages to other controller screens
* Sending UI popup notifications to other controller screens

There is a `pin.py` script included in this repository which can help solve the initial challenge and put the screen in "debug mode". This enables the main UI on the controller screen, which allows for changing the LED colors and patterns on the hardware badge via the BLE mesh.

The `broadcast_time.py` script can be used to send the current time to all connected devices on the BLE mesh. This was only useful during the competition, as there is no way to save and restore the time after power loss.

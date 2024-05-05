# Caliper Keyboard - Digital Caliper Data Over Bluetooth (HOG)

## Overview
This poject's goal is to provide a remote interface to the various cheap Digital Calipers.  
This project's firmware is being developed on Zephyr Version 3.6.99.  Zephyr V3.5.99 is supported too.

The Nordic nRF52 series Developemt Kits are the target hardware platform: PCA10056 (nRF52840dk_nrf52840) and a new custom board (**"nrf52840_caliper"**).
The nordic PCA10056 board was use for early development; there will be new board, nrf52840_caliper, specifically designed to be affixed to the back of a caliper.

Go to this project's [**Wiki**](https://github.com/foldedtoad/caliper_keyboard/wiki) page for more details.  

The image below shows size of the  **nrf52840_caliper** board, which is sized to fit on the back of calipers.  

![here](https://github.com/foldedtoad/caliper_keyboard/blob/master/images/Caliper_board.jpg)

![here](https://github.com/foldedtoad/caliper_keyboard/blob/master/images/caliper_board_w_case.JPEG)

## Video of Caliper_Keyboard
There is a youtube video showing the caliper_keyboard attached to a cheap caliper: https://youtu.be/K_FeKUWlJKM

## How to Build
There are two easy methods to build the firmware.

### CMake Method
This build method use cmake directly, and doesn't use west or ninja.
1) cd to your caliper_keyboard root directory
2) run "./configure.sh"
3) cd to build directory
4) make

### West Method
1) cd to your caliper_keyboard root directory
2) rm -rf build
3) west build -b nrf52840_caliper

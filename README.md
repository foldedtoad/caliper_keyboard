# Caliper Keyboard - Digital Caliper Data Over Bluetooth (HOG)

This poject's goal is to provide a remote interface to the various cheap Digital Calipers.  
This project's firmware is being developed on Zephyr Version 3.4.99.  

The Nordic nRF52 series Developemt Kits are the target hardware platform: PCA10040 (nRF52832) and PCA10056 (nRF52840).
These two boards are use for early development; there will be new board specifically designed to be affixed to the back of a caliper.

Go to this project's [Wiki](https://github.com/foldedtoad/caliper_keyboard/wiki) page for more details.

To build, use the following flow.
1) cd to your caliper_keyboard root directory
2) run "./configure.sh"
3) cd to build diretory
4) make

This build method use cmake directly, and doesn't use west or ninja.

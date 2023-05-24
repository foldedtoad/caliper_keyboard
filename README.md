# Caliper Keyboard - Digital Caliper Data Over Bluetooth

This poject's goal is to provide a remote interface to the various cheap Digital Calipers: below is a photo of one such product.  
This project's firmware is being developed on Zephyr Version 3.3.99.  
The Nordic nRF52 series Developemt Kits are the target hardware platform: PCA10040 (nRF52832) and PCA10056 (nRF52840).
These two boards are use for early development; there will be new board specifically designed to be affixed to the back of a caliper.

![here](https://github.com/foldedtoad/caliper_keyboard/blob/master/images/caliper_product_package.jpg)

## Theory of Operation
The Caliper Keyboard architecture can be broken into several subsystems.
* Basic Bluetooth Low-Energy (BLE) functions: Advertising, Connection Establishment and Security.
* Keyboard Emulation over Bluetooth functionality: Use of HID Over GATT (HOG) to provide keyboard emulation via BLE.
* Caliper Interface: Responding to and decoding the Caliper incoming frames.
* Button Eventing: Handle button presses and trigger snapshot events.
* Event Management: Receive "snapshot" button events, gather one-shot caliper value, build character string and send string via keyboard emulstion.
* Watchdog: Insures any critical errors are detected and reset/restart system.

## Various Caliper Interface Protocols
There are three veriations of caliper interface protocols;
* Cheap calipers, mostly from China, which support the lastest protocol: one "Relative" measurement in a 24+1-bit frames.
* "Absolute + Relative" measurement calipers, two 24-bit measurements in one frame. 
* Mitutoyo-type calipers use a completely differenct protocol, which is beyond the scope of this project.

## Caliper [BOM and Tool List](../master/CALIPER_BOM_TOOLS.md).

## Some Factoids

* While it may seem strange that calipers would be made from carbon fiber (plastic), there is a reasonable reason for this: Plastic doesn't stratch delicate surfaces. These plastic calipers are use by jewlery makers, musuem curators, and medical personal.

# Caliper Keyboard - Digital Caliper Data Over Bluetooth (HOG)

## Overview
This poject's goal is to provide a remote interface to the various cheap Digital Calipers.  
This project's firmware is being developed on Zephyr Version 4.4.99.  
This is a major Zephyr upgrade and breaks backwards compatibility.

The Nordic nRF52 series Developemt Kits are the target hardware platform: PCA10056 (nRF52840dk_nrf52840) and a new custom board (**"nrf52840_caliper"**).
The nordic PCA10056 board was use for early development; there will be new board, nrf52840_caliper, specifically designed to be affixed to the back of a caliper.

Go to this project's [**Wiki**](https://github.com/foldedtoad/caliper_keyboard/wiki) page for more details.  

The image below shows size of the  **nrf52840_caliper** board, which is sized to fit on the back of calipers.  

![here](https://github.com/foldedtoad/caliper_keyboard/blob/master/images/Caliper_board.jpg)

![here](https://github.com/foldedtoad/caliper_keyboard/blob/master/images/caliper_board_w_case.JPEG)

## Video of Caliper_Keyboard
There is a youtube video showing the caliper_keyboard attached to a cheap caliper: https://youtu.be/K_FeKUWlJKM

## How to Build

This firmware is built against **Zephyr v4.4** (tested with v4.4.2) using
`west`. The primary target is the **XIAO nRF52840** based caliper board
(`xiao_caliper`); the nRF52840 DK and the older `nrf52840_caliper` board
also still build.

```
cd <your zephyr workspace>
west build -b xiao_caliper/nrf52840 <path>/caliper_keyboard -p always
```

The build produces `zephyr.uf2`. To flash: double-tap the XIAO's reset
button so it appears as a USB drive, then copy `build/zephyr/zephyr.uf2`
onto it. (The Seeed/Adafruit UF2 bootloader is not touched.)

Other targets:

```
west build -b nrf52840dk/nrf52840 <path>/caliper_keyboard
west build -b nrf52840_caliper/nrf52840 <path>/caliper_keyboard
```

See [docs/PORTING_zephyr_4.4_xiao.md](docs/PORTING_zephyr_4.4_xiao.md) for
what changed from Zephyr 3.5 and the XIAO hardware notes.


# Porting notes: Zephyr 3.5 -> 4.4, nrf52840 -> XIAO nRF52840

Target board: `xiao_caliper/nrf52840` (schematic:
`hardware/Caliper_XIAO_v2./Caliper_XIAO-NRF52840_SCH.pdf`).
Verified against Zephyr v4.4.2. **Not yet run on hardware.**

## Pin map (from the schematic)

| Signal      | Pin   | XIAO | Notes                                          |
|-------------|-------|------|------------------------------------------------|
| A_CLOCK     | P1.14 | D9   | via SN74AXC2T45 level shifter (DIR low: B->A)  |
| A_DATA      | P1.13 | D8   | via level shifter                              |
| BUTTON      | P0.04 | D4   | SW2 to 3V3, no external pull-down: internal pull-down, active-high |
| BUZZ        | P0.03 | D1   | gate of Q1 (R1 pull-down); PWM, idles low      |
| DEBUG       | P1.15 | D10  | logic-analyzer test point (`logic_analyzer_testing`) |
| USART_TX/RX | P1.11 / P1.12 | D6 / D7 | `uart0`, J1 pins 6/7                  |

## What changed

**Board (`boards/arm/xiao_caliper`)**
- `pwm0` now outputs on P0.03 and is *not* inverted. The copied stock XIAO
  definition used P0.17 inverted, which does not match Q1.
- Removed the stock `pwm-leds` node: it claimed `pwm0` channel 0, the same
  channel the buzzer uses.
- Added the button, `caliper` (clock/data/debug) and buzzer nodes plus the
  aliases the application expects (`sw0`, `caliper-clock`, `caliper-data`,
  `caliper-debug`, `caliper-buzzer`).

**Application**
- `buzzer.c`: rewritten on the standard PWM API. Zephyr 4.4.2 has no in-tree
  buzzer driver (`<zephyr/drivers/buzzer.h>`), so the earlier code could not
  build. New binding: `dts/bindings/caliper/caliper-pwm-buzzer.yaml`.
  Playlist steps now run on the system workqueue instead of a timer callback
  (timer callbacks run in ISR context; PWM drivers may sleep).
- `app_uicr.c`: line-end / standard settings are stored with Zephyr settings
  (NVS, `storage_partition`) instead of erasing and rewriting the UICR page.
  The UF2 bootloader keeps its start address in UICR, so a power loss during
  that erase/rewrite could leave the board unable to boot. API unchanged.
- `buttons.c` takes the GPIO controller from the `sw0` alias (was hard-coded
  `gpio0`); `shell.c` reports the real shell device (`zephyr,shell-uart`).
- `prj.conf`: added `CONFIG_PWM`; removed `CONFIG_BUZZER` (symbol gone),
  `CONFIG_WDT_DISABLE_AT_BOOT` (not supported on nRF), the UICR flash options,
  `CONFIG_UART_0_INTERRUPT_DRIVEN` and a duplicate `BT_MAX_CONN`.
- `CMakeLists.txt`: no longer forces `BOARD` (it overrode `west build -b`
  and breaks board qualifiers); `find_package(Zephyr HINTS $ENV{ZEPHYR_BASE})`.
- `nrf52840dk_nrf52840.overlay` and `nrf52840_caliper.dts` moved to the new
  buzzer binding so those targets keep building.

The caliper decode path (`caliper.c`, `framer.c`) needed no change: it only
uses the portable GPIO API, and the `1 << CLOCK` interrupt mask is
port-relative, so P1.14 works.

## Behavior to know about

- **Console and shell are on the UART header (J1, 115200 8N1)** for
  `xiao_caliper`, set by two app-level files:
  `boards/xiao_caliper.overlay` (chosen `zephyr,console` / `zephyr,shell-uart`
  = `&uart0`) and `boards/xiao_caliper.conf` (turns off the board's default
  USB CDC ACM console, and routes `LOG_INF` to the shell). Zephyr merges
  them automatically for `west build -b xiao_caliper`; no extra build
  options are needed. Connect a 3.3 V USB-serial adapter to USART_TX
  (P1.11) / USART_RX (P1.12) and GND. Delete those two files to go back to
  the USB CDC console.
- `prj.conf` only enables the RTT log backend and disables the shell log
  backend, so without `xiao_caliper.conf` `LOG_INF` output is visible only
  over RTT, whichever console is chosen.
- **Battery level is not meaningful yet.** `battery.c` measures VDD against a
  2.0-3.2 V coin-cell range. On the XIAO, VDD is a regulated 3.3 V, so it will
  read "high / 100%". Fixing it needs a decision about the battery chemistry
  and reading VBAT through the XIAO's divider.
- BLE bonds are still not persisted (`CONFIG_BT_SETTINGS=n`), as before.

## Hardware note

The schematic says "Q1 PCB footprint is not correct. Rework Q1 to wire
correctly." The buzzer will not sound until that is fixed on the board.

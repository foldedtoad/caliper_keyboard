/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

const struct {} battery_def_include_once;

#define BATTERY_MEAS_ADC_INPUT              NRF_SAADC_VDD  //HDIV5
#define BATTERY_MEAS_ADC_GAIN               ADC_GAIN_1_6
#define BATTERY_MEAS_VOLTAGE_GAIN           6

#define BATTERY_MEAS_POLL_INTERVAL_MS       60000
#define BATTERY_MEAS_MAX_LEVEL              3200
#define BATTERY_MEAS_MIN_LEVEL              2000

#define VOLTAGE_TO_SOC_DELTA                10

/* 
 * Converting measured battery voltage[mV] to State of Charge[%].
 * First element corresponds to BATTERY_MIN_LEVEL.
 * Each element is VOLTAGE_TO_SOC_DELTA higher than previous.
 * Defined separately for every configuration.
 */
static const uint8_t battery_voltage_to_soc[] = 
{
  // 0   1   2   3   4   5   6   7   8   9 
  //======================================== 
     0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  // 00
     1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  // 10
     2,  2,  2,  2,  2,  2,  2,  2,  3,  3,  // 20
     3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  // 30
     4,  5,  5,  5,  6,  6,  7,  7,  8,  8,  // 40
     9,  9, 10, 11, 12, 13, 13, 14, 15, 16,  // 50
    18, 19, 22, 25, 28, 32, 36, 40, 44, 47,  // 60
    51, 53, 56, 58, 60, 62, 64, 66, 67, 69,  // 70
    71, 72, 74, 76, 77, 79, 81, 82, 84, 85,  // 80
    85, 86, 86, 86, 87, 88, 88, 89, 90, 91,  // 90
    91, 92, 93, 94, 95, 96, 97, 98, 99, 100, // 100
    100
};

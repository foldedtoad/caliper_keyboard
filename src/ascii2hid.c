/*
 *  ascii2hid.c  -- HOG key code encoder
 *
 *  Copyright (c) 2023   Callender-Consulting
 *
 *  SPDX-License-Identifier: Apache-2.0
 */
#include "keyboard.h"

int ascii_to_hid(uint8_t ascii)
{
	if (ascii < 32) {
		switch (ascii) {
		case 0x09:                // Horizontal Tab
		case 0x0A:                // New Line
		case 0x0B:                // Vertical Tab
		case 0x0C:                // Form Feed
		case 0x0D:                // Carriage Return
			return HID_KEY_ENTER;
		default:
			return -1;
		}
	} else if (ascii < 48) {
		/* Special characters */  //TBD support sending of BELL
		switch (ascii) {
		case 32:
			return HID_KEY_SPACE;
		case 33:
			return HID_KEY_1;
		case 34:
			return HID_KEY_APOSTROPHE;
		case 35:
			return HID_KEY_3;
		case 36:
			return HID_KEY_4;
		case 37:
			return HID_KEY_5;
		case 38:
			return HID_KEY_7;
		case 39:
			return HID_KEY_APOSTROPHE;
		case 40:
			return HID_KEY_9;
		case 41:
			return HID_KEY_0;
		case 42:
			return HID_KEY_8;
		case 43:
			return HID_KEY_EQUAL;
		case 44:
			return HID_KEY_COMMA;
		case 45:
			return HID_KEY_MINUS;
		case 46:
			return HID_KEY_DOT;
		case 47:
			return HID_KEY_SLASH;
		default:
			return -1;
		}
	} else if (ascii < 58) {
		/* Numbers */
		if (ascii == 48U) {
			return HID_KEY_0;
		} else {
			return ascii - 19;
		}
	} else if (ascii < 65) {
		/* Special characters #2 */
		switch (ascii) {
		case 58:
			return HID_KEY_SEMICOLON;
		case 59:
			return HID_KEY_SEMICOLON;
		case 60:
			return HID_KEY_COMMA;
		case 61:
			return HID_KEY_EQUAL;
		case 62:
			return HID_KEY_DOT;
		case 63:
			return HID_KEY_SLASH;
		case 64:
			return HID_KEY_2;
		default:
			return -1;
		}
	} else if (ascii < 91) {
		/* Uppercase characters */
		return ascii - 61U;
	} else if (ascii < 97) {
		/* Special characters #3 */
		switch (ascii) {
		case 91:
			return HID_KEY_LEFTBRACE;
		case 92:
			return HID_KEY_BACKSLASH;
		case 93:
			return HID_KEY_RIGHTBRACE;
		case 94:
			return HID_KEY_6;
		case 95:
			return HID_KEY_MINUS;
		case 96:
			return HID_KEY_GRAVE;
		default:
			return -1;
		}
	} else if (ascii < 123) {
		/* Lowercase letters */
		return ascii - 93;
	} else if (ascii < 128) {
		/* Special characters #4 */
		switch (ascii) {
		case 123:
			return HID_KEY_LEFTBRACE;
		case 124:
			return HID_KEY_BACKSLASH;
		case 125:
			return HID_KEY_RIGHTBRACE;
		case 126:
			return HID_KEY_GRAVE;
		case 127:
			return HID_KEY_DELETE;
		default:
			return -1;
		}
	}

	return -1;
}

bool needs_shift(uint8_t ascii)
{
	if ((ascii < 33) || (ascii == 39U)) {
		return false;
	} else if ((ascii >= 33U) && (ascii < 44)) {
		return true;
	} else if ((ascii >= 44U) && (ascii < 58)) {
		return false;
	} else if ((ascii == 59U) || (ascii == 61U)) {
		return false;
	} else if ((ascii >= 58U) && (ascii < 91)) {
		return true;
	} else if ((ascii >= 91U) && (ascii < 94)) {
		return false;
	} else if ((ascii == 94U) || (ascii == 95U)) {
		return true;
	} else if ((ascii > 95) && (ascii < 123)) {
		return false;
	} else if ((ascii > 122) && (ascii < 127)) {
		return true;
	} else {
		return false;
	}
}

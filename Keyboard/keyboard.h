/*
 * keyboard.h
 *
 *  Created on: May 21, 2023
 *      Author: xq123
 */

#ifndef KEYBOARD_H_
#define KEYBOARD_H_

#include "lefl.h"
#include "usb_hid_keys.h"
#include "usbd_conf.h"

#define ADVANCED_KEY_NUM        64
#define KEY_NUM                 8

typedef struct
{
    uint8_t keycode;
    uint8_t modifier;
} key_binding_t;

extern uint8_t Keyboard_ReportBuffer[USBD_CUSTOMHID_OUTREPORT_BUF_SIZE];
extern lefl_bit_array_t Keyboard_KeyArray;

extern lefl_key_t Keyboard_Keys[KEY_NUM];
extern lefl_advanced_key_t Keyboard_AdvancedKeys[ADVANCED_KEY_NUM];
extern key_binding_t keymap[5][64];
extern const key_binding_t default_keymap[5][64];



void Keyboard_Init();
void Keyboard_FactoryReset();
void Keyboard_SystemReset();
void Keyboard_Scan();
void Keyboard_Recovery();
void Keyboard_Save();
void Keyboard_SendReport();

#endif /* KEYBOARD_H_ */

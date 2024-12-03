/*
 * keyboard.c
 *
 *  Created on: May 21, 2023
 *      Author: xq123
 */
#include "keyboard.h"
#include "analog.h"
#include "rgb.h"
#include "keyboard_conf.h"
#include "lfs.h"
#include "main.h"
#include "action.h"

uint8_t g_keyboard_report_buffer[HID_BUFFER_LENGTH];
bool Keybaord_SendReport_Enable;
uint8_t g_keyboard_current_layer;
uint16_t g_keymap[LAYER_NUM][ADVANCED_KEY_NUM + KEY_NUM];
Keyboard_6KROBuffer g_keyboard_6kro_buffer;

Action *g_keyboard_actions[LAYER_NUM][ADVANCED_KEY_NUM + KEY_NUM];

uint8_t g_keyboard_knob_flag;
volatile bool g_keybaord_send_report_enable = true;
volatile bool g_keybaord_alpha_flag;
volatile bool g_keybaord_shift_flag;

int keyboard_6KRObuffer_add(Keyboard_6KROBuffer* buf, uint16_t key)
{
    buf->buffer[0] |= KEY_MODIFIER(key);
    if (KEY_KEYCODE(key) != KEY_NO_EVENT && buf->keynum < 6)
    {
        buf->buffer[2 + buf->keynum] = KEY_KEYCODE(key);
        buf->keynum++;
        return 0;
    }
    else
    {
        return 1;
    }
}

void keyboard_6KRObuffer_send(Keyboard_6KROBuffer* buf)
{
    keyboard_hid_send(buf->buffer, sizeof(buf->buffer));
}

void keyboard_6KRObuffer_clear(Keyboard_6KROBuffer* buf)
{
    memset(buf, 0, sizeof(Keyboard_6KROBuffer));
}

void keyboard_init()
{
    memcpy(g_keymap, g_default_keymap, sizeof(g_keymap));
}

void keyboard_factory_reset()
{
    memcpy(g_keymap,g_default_keymap,sizeof(g_keymap));
    for (uint8_t i = 0; i < ADVANCED_KEY_NUM; i++)
    {
        g_keyboard_advanced_keys[i].mode = DEFAULT_ADVANCED_KEY_MODE;
        g_keyboard_advanced_keys[i].trigger_distance = DEFAULT_TRIGGER_DISTANCE;
        g_keyboard_advanced_keys[i].release_distance = DEFAULT_RELEASE_DISTANCE;
        g_keyboard_advanced_keys[i].schmitt_parameter = DEFAULT_SCHMITT_PARAMETER;
        g_keyboard_advanced_keys[i].activation_value = 0.5;
        // Keyboard_AdvancedKeys[i].lower_deadzone = 0.32;
        advanced_key_set_deadzone(g_keyboard_advanced_keys + i, DEFAULT_UPPER_DEADZONE, DEFAULT_LOWER_DEADZONE);
        // Keyboard_AdvancedKeys[i].phantom_lower_deadzone = 0.32;
        // Keyboard_AdvancedKeys[i].key.keycode = default_keymap[0][Keyboard_AdvancedKeys[i].key.id];
    }
    rgb_factory_reset();
    keyboard_save();
    //keyboard_system_reset();
}
void keyboard_system_reset()
{
    __set_FAULTMASK(1);
    NVIC_SystemReset();
}

__WEAK void keyboard_scan()
{
}
void keyboard_recovery()
{
    // mount the filesystem
    int err = lfs_mount(&lfs_w25qxx, &cfg);
    // reformat if we can't mount the filesystem
    // this should only happen on the first boot
    if (err)
    {
        lfs_format(&lfs_w25qxx, &cfg);
        lfs_mount(&lfs_w25qxx, &cfg);
    }
    lfs_file_open(&lfs_w25qxx, &lfs_file_w25qxx, "config1.dat", LFS_O_RDWR | LFS_O_CREAT);
    lfs_file_rewind(&lfs_w25qxx, &lfs_file_w25qxx);
    for (uint8_t i = 0; i < ADVANCED_KEY_NUM; i++)
    {
        //lfs_file_read(&lfs_w25qxx, &lfs_file_w25qxx, &(g_keyboard_advanced_keys[i].key.id),
        //              sizeof(g_keyboard_advanced_keys[i].key.id));
        lfs_file_read(&lfs_w25qxx, &lfs_file_w25qxx, ((void *)(&g_keyboard_advanced_keys[i])) + sizeof(Key),
                      sizeof(AdvancedKey) - sizeof(Key));
    }
    lfs_file_read(&lfs_w25qxx, &lfs_file_w25qxx, g_keymap, sizeof(g_keymap));
    lfs_file_read(&lfs_w25qxx, &lfs_file_w25qxx, &g_rgb_switch, sizeof(g_rgb_switch));
    lfs_file_read(&lfs_w25qxx, &lfs_file_w25qxx, &g_rgb_configs, sizeof(g_rgb_configs));
    // remember the storage is not updated until the file is closed successfully
    lfs_file_close(&lfs_w25qxx, &lfs_file_w25qxx);
    printf("recovery = %d", err);
    // release any resources we were using
    lfs_unmount(&lfs_w25qxx);
    // print the boot count
}


void keyboard_save()
{
    // mount the filesystem
    int err = lfs_mount(&lfs_w25qxx, &cfg);
    // reformat if we can't mount the filesystem
    // this should only happen on the first boot
    if (err)
    {
        lfs_format(&lfs_w25qxx, &cfg);
        lfs_mount(&lfs_w25qxx, &cfg);
    }
    // read current count
    lfs_file_open(&lfs_w25qxx, &lfs_file_w25qxx, "config1.dat", LFS_O_RDWR | LFS_O_CREAT);
    lfs_file_rewind(&lfs_w25qxx, &lfs_file_w25qxx);
    for (uint8_t i = 0; i < ADVANCED_KEY_NUM; i++)
    {
        //lfs_file_write(&lfs_w25qxx, &lfs_file_w25qxx, &(g_keyboard_advanced_keys[i].key.id),
        //               sizeof(g_keyboard_advanced_keys[i].key.id));
        lfs_file_write(&lfs_w25qxx, &lfs_file_w25qxx, ((void *)(&g_keyboard_advanced_keys[i])) + sizeof(Key),
                       sizeof(AdvancedKey) - sizeof(Key));
    }
    lfs_file_write(&lfs_w25qxx, &lfs_file_w25qxx, g_keymap, sizeof(g_keymap));
    lfs_file_write(&lfs_w25qxx, &lfs_file_w25qxx, &g_rgb_switch, sizeof(g_rgb_switch));
    lfs_file_write(&lfs_w25qxx, &lfs_file_w25qxx, &g_rgb_configs, sizeof(g_rgb_configs));
    // remember the storage is not updated until the file is closed successfully
    err = lfs_file_close(&lfs_w25qxx, &lfs_file_w25qxx);
    printf("save = %d", err);
    // release any resources we were using
    lfs_unmount(&lfs_w25qxx);
    // print the boot count
}


void keyboard_key_add_buffer(Key *k)
{
    if (g_keyboard_actions[g_keyboard_current_layer][k->id])
    {
        action_execute(k, g_keyboard_actions[g_keyboard_current_layer][k->id]);
    }
    else if (k->state)
    {
        uint8_t keycode = g_keymap[g_keyboard_current_layer][k->id] & 0xFF;
    	uint8_t modifier = (g_keymap[g_keyboard_current_layer][k->id] >> 8) & 0xFF;
        if (keycode <= KEY_EXSEL)
        {
            //KEYBOARD_REPORT_BUFFER_ADD(g_keymap[g_keyboard_current_layer][k->id]);
    	    uint8_t index = keycode/8;// +1 for modifier
    	    uint8_t bitIndex = keycode%8;
            g_keyboard_report_buffer[index + 2] |= (1 << (bitIndex));
            g_keyboard_report_buffer[1] |= modifier;
        }
        else
        {
            /*
            switch (g_keymap[g_keyboard_current_layer][k->id])
            {
            case MOUSE_LBUTTON:
                g_mouse.buttons |= 0x01;
                break;
            case MOUSE_RBUTTON:
                g_mouse.buttons |= 0x02;
                break;
            case MOUSE_MBUTTON:
                g_mouse.buttons |= 0x04;
                break;
            case MOUSE_FORWARD:
                g_mouse.buttons |= 0x08;
                break;
            case MOUSE_BACK:
                g_mouse.buttons |= 0x10;
                break;
            case MOUSE_WHEEL_UP:
                g_mouse.wheel = 1;
                break;
            case MOUSE_WHEEL_DOWN:
                g_mouse.wheel = -1;
                break;
            default:
                break;
            }
            */
        }
    }
}

void keyboard_send_report()
{
    //keyboard_6KRObuffer_clear(&g_keyboard_6kro_buffer);
    memset(g_keyboard_report_buffer, 0, HID_BUFFER_LENGTH);
    g_keyboard_report_buffer[0] = 1;
    g_keyboard_current_layer = g_keyboard_advanced_keys[49].key.state?1:0; //Fn key
    // keyboard_6KRObuffer_add(&Keyboard_ReportBuffer,(KeyBinding){KEY_E,KEY_NO_MODIFIER});
    for (int i = 0; i < ADVANCED_KEY_NUM; i++)
    {
        keyboard_key_add_buffer(&g_keyboard_advanced_keys[i].key);
    }
    for (int i = 0; i < KEY_NUM; i++)
    {
        keyboard_key_add_buffer(&g_keyboard_keys[i]);
    }
    if (g_keybaord_send_report_enable)
    {
        keyboard_hid_send(g_keyboard_report_buffer,16+1);
        //keyboard_6KRObuffer_send(&g_keyboard_6kro_buffer);
        //if ((*(uint32_t*)&g_mouse)!=mouse_value)
        //{
        //    mouse_buffer_send(&g_mouse);
        //}
    }

    /*
    int16_t index, bitIndex, keycode, layer;
    layer = g_keyboard_advanced_keys[49].key.state?1:0; //Fn key

    memset(g_keyboard_report_buffer, 0, HID_BUFFER_LENGTH);
    g_keyboard_report_buffer[0] = 1;
    for (int i = 0; i < ADVANCED_KEY_NUM; i++)
    {
    	keycode = g_keymap[layer][g_keyboard_advanced_keys[i].key.id];
    	index = (int16_t)(keycode/8 + 1);// +1 for modifier
    	bitIndex = (int16_t)(keycode%8);
        if (bitIndex < 0)
        {
            index -= 1;
            bitIndex += 8;
        } else if (keycode > 100)
            continue;
        if(g_keyboard_advanced_keys[i].key.state)g_keyboard_report_buffer[index + 1] |= 1 << (bitIndex); // +1 for Report-ID
    }
    //debug
    //memset(Keyboard_ReportBuffer, 0, USBD_CUSTOMHID_OUTREPORT_BUF_SIZE);
    keyboard_hid_send(g_keyboard_report_buffer,16+1);
    */
}

__WEAK void keyboard_task()
{
    keyboard_scan();
    analog_average();
    analog_check();
    keyboard_post_process();
    keyboard_send_report();
}

__WEAK void keyboard_hid_send(uint8_t *report, uint16_t len)
{
}
__WEAK void keyboard_delay(uint32_t ms)
{
}
__WEAK void keyboard_post_process()
{
}

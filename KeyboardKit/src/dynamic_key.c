/*
 * Copyright (c) 2024 Zhangqi Li (@zhangqili)
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */


#include "dynamic_key.h"
#include "command.h"
#include "keyboard.h"

#define DK_TAP_DURATION 5

//static uint8_t g_keyboard_dynamic_keys_length;

void dynamic_key_update(DynamicKey*dynamic_key,AdvancedKey*advanced_key, bool state)
{
    switch (dynamic_key->type)
    {
    case DYNAMIC_KEY_STROKE:
        dynamic_key_s_update(dynamic_key, advanced_key, state);
        break;
    case DYNAMIC_KEY_MOD_TAP:
        dynamic_key_mt_update(dynamic_key, advanced_key, state);
        break;
    case DYNAMIC_KEY_TOGGLE_KEY:
        dynamic_key_tk_update(dynamic_key, advanced_key, state);
        break;
    case DYNAMIC_KEY_RAPPY_SNAPPY:
        dynamic_key_rs_update(dynamic_key, advanced_key, state);
        break;
    case DYNAMIC_KEY_MUTEX:
        dynamic_key_m_update(dynamic_key, advanced_key, state);
        break;
    default:
        break;
    }
}


void dynamic_key_add_buffer(DynamicKey*dynamic_key)
{
    switch (dynamic_key->type)
    {
    case DYNAMIC_KEY_STROKE:
        DynamicKeyStroke4x4*dynamic_key_s=(DynamicKeyStroke4x4*)dynamic_key;
        for (int i = 0; i < 4; i++)
        {
            if (BIT_GET(dynamic_key_s->key_state,i))
                keyboard_event_handler(MK_EVENT(dynamic_key_s->key_binding[i], KEYBOARD_EVENT_KEY_TRUE));
        }
        break;
    case DYNAMIC_KEY_MOD_TAP:
        DynamicKeyModTap*dynamic_key_mt=(DynamicKeyModTap*)dynamic_key;
        keyboard_event_handler(MK_EVENT(dynamic_key_mt->key_binding[dynamic_key_mt->state], KEYBOARD_EVENT_KEY_TRUE));
        break;
    case DYNAMIC_KEY_TOGGLE_KEY:
        DynamicKeyToggleKey*dynamic_key_tk=(DynamicKeyToggleKey*)dynamic_key;
        keyboard_event_handler(MK_EVENT(dynamic_key_tk->key_binding, KEYBOARD_EVENT_KEY_TRUE));
        break;
    case DYNAMIC_KEY_RAPPY_SNAPPY:
        {
            DynamicKeyRappySnappy*dynamic_key_rs=(DynamicKeyRappySnappy*)dynamic_key;
            if (dynamic_key_rs->key1_state)
                keyboard_event_handler(MK_EVENT(dynamic_key_rs->key1_binding, KEYBOARD_EVENT_KEY_TRUE));
            if (dynamic_key_rs->key2_state)
                keyboard_event_handler(MK_EVENT(dynamic_key_rs->key2_binding, KEYBOARD_EVENT_KEY_TRUE));
        }
        break;
    case DYNAMIC_KEY_MUTEX:
        {
            DynamicKeyMutex*dynamic_key_m=(DynamicKeyMutex*)dynamic_key;
            AdvancedKey*key0 = &g_keyboard_advanced_keys[command_advanced_key_mapping[dynamic_key_m->key[0].id]];
            AdvancedKey*key1 = &g_keyboard_advanced_keys[command_advanced_key_mapping[dynamic_key_m->key[1].id]];
            if (key0->key.report_state)
                keyboard_event_handler(MK_EVENT(dynamic_key_m->key[0].binding, KEYBOARD_EVENT_KEY_TRUE));
            if (key1->key.report_state)
                keyboard_event_handler(MK_EVENT(dynamic_key_m->key[1].binding, KEYBOARD_EVENT_KEY_TRUE));
        }
        break;
    default:
        break;
    }
}

#define DKS_PRESS_BEGIN 0
#define DKS_PRESS_FULLY 4
#define DKS_RELEASE_BEGIN 8
#define DKS_RELEASE_FULLY 12
void dynamic_key_s_update(DynamicKey*dynamic_key, AdvancedKey*key, bool state)
{
    UNUSED(state);
    DynamicKeyStroke4x4*dynamic_key_s=(DynamicKeyStroke4x4*)dynamic_key;
    AnalogValue last_value = dynamic_key_s->value;
    AnalogValue current_value = key->value;
    uint8_t last_key_state = dynamic_key_s->key_state;
    if (current_value > last_value)
    {
        if (current_value >= dynamic_key_s->press_begin_distance &&
            last_value < dynamic_key_s->press_begin_distance)
        {
            for (int i = 0; i < 4; i++)
            {
                if (BIT_GET(dynamic_key_s->key_control[i], DKS_PRESS_BEGIN))
                {
                    if (BIT_GET(dynamic_key_s->key_control[i],DKS_PRESS_BEGIN+1))
                    {
                        dynamic_key_s->key_end_time[i] = 0xFFFFFFFF;
                    }
                    else
                    {
                        dynamic_key_s->key_end_time[i] = g_keyboard_tick + DK_TAP_DURATION;
                    }
                    BIT_SET(dynamic_key_s->key_state, i);
                }
                else
                {
                    BIT_RESET(dynamic_key_s->key_state, i);
                }
            }
        }
        if (current_value >= dynamic_key_s->press_fully_distance &&
            last_value < dynamic_key_s->press_fully_distance)
        {
            for (int i = 0; i < 4; i++)
            {
                if (BIT_GET(dynamic_key_s->key_control[i], DKS_PRESS_FULLY))
                {
                    if (BIT_GET(dynamic_key_s->key_control[i],DKS_PRESS_FULLY+1))
                    {
                        dynamic_key_s->key_end_time[i] = 0xFFFFFFFF;
                    }
                    else
                    {
                        dynamic_key_s->key_end_time[i] = g_keyboard_tick + DK_TAP_DURATION;
                    }
                    BIT_SET(dynamic_key_s->key_state, i);
                }
                else
                {
                    BIT_RESET(dynamic_key_s->key_state, i);
                }
            }
        }
    }
    if (current_value < last_value)
    {
        if (current_value <= dynamic_key_s->release_begin_distance &&
            last_value > dynamic_key_s->release_begin_distance)
        {
            for (int i = 0; i < 4; i++)
            {
                if (BIT_GET(dynamic_key_s->key_control[i], DKS_RELEASE_BEGIN))
                {
                    if (BIT_GET(dynamic_key_s->key_control[i],DKS_RELEASE_BEGIN+1))
                    {
                        dynamic_key_s->key_end_time[i] = 0xFFFFFFFF;
                    }
                    else
                    {
                        dynamic_key_s->key_end_time[i] = g_keyboard_tick + DK_TAP_DURATION;
                    }
                    BIT_SET(dynamic_key_s->key_state, i);
                }
                else
                {
                    BIT_RESET(dynamic_key_s->key_state, i);
                }
            }
        }
        if (current_value <= dynamic_key_s->release_fully_distance &&
            last_value > dynamic_key_s->release_fully_distance)
        {
            for (int i = 0; i < 4; i++)
            {
                if (BIT_GET(dynamic_key_s->key_control[i], DKS_RELEASE_FULLY))
                {
                    if (BIT_GET(dynamic_key_s->key_control[i],DKS_RELEASE_FULLY+1))
                    {
                        dynamic_key_s->key_end_time[i] = 0xFFFFFFFF;
                    }
                    else
                    {
                        dynamic_key_s->key_end_time[i] = g_keyboard_tick + DK_TAP_DURATION;
                    }
                    BIT_SET(dynamic_key_s->key_state, i);
                }
                else
                {
                    BIT_RESET(dynamic_key_s->key_state, i);
                }
            }
        }
    }

    for (int i = 0; i < 4; i++)
    {
        if (g_keyboard_tick > dynamic_key_s->key_end_time[i])
        {
            BIT_RESET(dynamic_key_s->key_state, i);
        }
        if (BIT_GET(dynamic_key_s->key_state, i) && !BIT_GET(last_key_state, i))
        {
            layer_cache_set(key->key.id, g_current_layer);
            keyboard_event_handler(MK_EVENT(dynamic_key_s->key_binding[i], KEYBOARD_EVENT_KEY_DOWN));
            keyboard_advanced_key_event_handler(key, KEYBOARD_EVENT_KEY_DOWN);
        }
        if (!BIT_GET(dynamic_key_s->key_state, i) && BIT_GET(last_key_state, i))
        {
            keyboard_event_handler(MK_EVENT(dynamic_key_s->key_binding[i], KEYBOARD_EVENT_KEY_UP));
            keyboard_advanced_key_event_handler(key, KEYBOARD_EVENT_KEY_UP);
        }
    }
    advanced_key_update_state(key, dynamic_key_s->key_state > 0);
    key->key.report_state = key->key.state;
    dynamic_key_s->value = current_value;
}

void dynamic_key_mt_update(DynamicKey*dynamic_key, AdvancedKey*key, bool state)
{
    DynamicKeyModTap*dynamic_key_mt=(DynamicKeyModTap*)dynamic_key;
    if (!(key->key.state) && state)
    {
        dynamic_key_mt->begin_time = g_keyboard_tick;
    }
    if ((key->key.state) && !state)
    {
        if (g_keyboard_tick - dynamic_key_mt->begin_time < dynamic_key_mt->duration)
        {
            dynamic_key_mt->end_time = g_keyboard_tick+DK_TAP_DURATION;
            dynamic_key_mt->state = DYNAMIC_KEY_ACTION_TAP;
            layer_cache_set(key->key.id, g_current_layer);
            keyboard_event_handler(MK_EVENT(dynamic_key_mt->key_binding[1], KEYBOARD_EVENT_KEY_DOWN));
            keyboard_advanced_key_event_handler(key, KEYBOARD_EVENT_KEY_DOWN);
            key->key.report_state = true;
        }
        else
        {
            keyboard_event_handler(MK_EVENT(dynamic_key_mt->key_binding[1], KEYBOARD_EVENT_KEY_UP));
            keyboard_advanced_key_event_handler(key, KEYBOARD_EVENT_KEY_UP);
            key->key.report_state = false;
        }
        dynamic_key_mt->begin_time = g_keyboard_tick;
    }
    if (g_keyboard_tick - dynamic_key_mt->begin_time > DK_TAP_DURATION)
    {
        dynamic_key_mt->end_time = 0xFFFFFFFF;
        dynamic_key_mt->state = DYNAMIC_KEY_ACTION_HOLD;
        layer_cache_set(key->key.id, g_current_layer);
        keyboard_event_handler(MK_EVENT(dynamic_key_mt->key_binding[1], KEYBOARD_EVENT_KEY_DOWN));
            keyboard_advanced_key_event_handler(key, KEYBOARD_EVENT_KEY_DOWN);
        key->key.report_state = true;
    }
    if (g_keyboard_tick > dynamic_key_mt->end_time)
    {
        keyboard_event_handler(MK_EVENT(dynamic_key_mt->key_binding[1], KEYBOARD_EVENT_KEY_UP));
        keyboard_advanced_key_event_handler(key, KEYBOARD_EVENT_KEY_UP);
        key->key.report_state = false;
    }
    advanced_key_update_state(key, state);
}

void dynamic_key_tk_update(DynamicKey*dynamic_key, AdvancedKey*key, bool state)
{
    DynamicKeyToggleKey*dynamic_key_tk=(DynamicKeyToggleKey*)dynamic_key;
    if (!(key->key.state) && state)
    {
        dynamic_key_tk->state = !dynamic_key_tk->state;
        key->key.report_state = dynamic_key_tk->state;
        if (dynamic_key_tk->state)
        {
            layer_cache_set(key->key.id, g_current_layer);
            keyboard_event_handler(MK_EVENT(dynamic_key_tk->key_binding, KEYBOARD_EVENT_KEY_DOWN));
            keyboard_advanced_key_event_handler(key, KEYBOARD_EVENT_KEY_DOWN);
        }
        else
        {
            keyboard_event_handler(MK_EVENT(dynamic_key_tk->key_binding, KEYBOARD_EVENT_KEY_UP));
            keyboard_advanced_key_event_handler(key, KEYBOARD_EVENT_KEY_UP);
        }
    }
    advanced_key_update_state(key, state);
}

void dynamic_key_rs_update(DynamicKey*dynamic_key, AdvancedKey*key, bool state)
{
    UNUSED(key);
    UNUSED(state);
    DynamicKeyRappySnappy*dynamic_key_rs=(DynamicKeyRappySnappy*)dynamic_key;

    if (!dynamic_key_rs->trigger_state)
    {
        dynamic_key_rs->trigger_state = !dynamic_key_rs->trigger_state;
        return;
    }
    AdvancedKey*key1 = &g_keyboard_advanced_keys[command_advanced_key_mapping[dynamic_key_rs->key1_id]];
    AdvancedKey*key2 = &g_keyboard_advanced_keys[command_advanced_key_mapping[dynamic_key_rs->key2_id]];

    bool last_key1_state = dynamic_key_rs->key1_state;
    if ((key1->value > key2->value) ||
    ((key1->value>= (ANALOG_VALUE_MAX - key1->lower_deadzone))&&
    (key2->value>= (ANALOG_VALUE_MAX - key2->lower_deadzone))))
    {
        dynamic_key_rs->key1_state = true;
    }
    else if (key1->value != key2->value || (key1->value < key1->upper_deadzone))
    {
        dynamic_key_rs->key1_state = false;
    }
    if (dynamic_key_rs->key1_state && !last_key1_state)
    {
        layer_cache_set(key1->key.id, g_current_layer);
        keyboard_event_handler(MK_EVENT(dynamic_key_rs->key1_binding, KEYBOARD_EVENT_KEY_DOWN));
        keyboard_advanced_key_event_handler(key, KEYBOARD_EVENT_KEY_DOWN);
    }
    if (!dynamic_key_rs->key1_state && last_key1_state)
    {
        keyboard_event_handler(MK_EVENT(dynamic_key_rs->key1_binding, KEYBOARD_EVENT_KEY_UP));
        keyboard_advanced_key_event_handler(key, KEYBOARD_EVENT_KEY_UP);
    }
    advanced_key_update_state(key1, dynamic_key_rs->key1_state);
    key1->key.report_state = dynamic_key_rs->key1_state;

    bool last_key2_state = dynamic_key_rs->key2_state;
    if ((key1->value < key2->value) ||
    ((key1->value>= (ANALOG_VALUE_MAX - key1->lower_deadzone))&&
    (key2->value>= (ANALOG_VALUE_MAX - key2->lower_deadzone))))
    {
        dynamic_key_rs->key2_state = true;
    }
    else if (key1->value != key2->value || (key2->value > key2->upper_deadzone))
    {
        dynamic_key_rs->key2_state = false;
    }
    if (dynamic_key_rs->key2_state && !last_key2_state)
    {
        layer_cache_set(key2->key.id, g_current_layer);
        keyboard_event_handler(MK_EVENT(dynamic_key_rs->key1_binding, KEYBOARD_EVENT_KEY_DOWN));
        keyboard_advanced_key_event_handler(key, KEYBOARD_EVENT_KEY_DOWN);
    }
    if (!dynamic_key_rs->key2_state && last_key2_state)
    {
        keyboard_event_handler(MK_EVENT(dynamic_key_rs->key1_binding, KEYBOARD_EVENT_KEY_UP));
        keyboard_advanced_key_event_handler(key, KEYBOARD_EVENT_KEY_UP);
    }
    advanced_key_update_state(key2, dynamic_key_rs->key2_state);
    key2->key.report_state = dynamic_key_rs->key2_state;
}

void dynamic_key_m_update(DynamicKey*dynamic_key, AdvancedKey*key, bool state)
{
    DynamicKeyMutex*dynamic_key_m=(DynamicKeyMutex*)dynamic_key;
    AdvancedKey*key0 = &g_keyboard_advanced_keys[command_advanced_key_mapping[dynamic_key_m->key[0].id]];
    AdvancedKey*key1 = &g_keyboard_advanced_keys[command_advanced_key_mapping[dynamic_key_m->key[1].id]];

    if ((dynamic_key_m->mode & 0x0F) == DK_MUTEX_DISTANCE_PRIORITY)
    {
        if (!dynamic_key_m->trigger_state)
        {
            dynamic_key_m->trigger_state = !dynamic_key_m->trigger_state;
            return;
        }

        bool last_key0_state = key0->key.report_state;
        if ((key0->value > key1->value) ||
        ((key0->value>= (ANALOG_VALUE_MAX - key0->lower_deadzone))&&
        (key1->value>= (ANALOG_VALUE_MAX - key1->lower_deadzone))))
        {
            key0->key.report_state = true;
        }
        else if (key0->value != key1->value || (key0->value < key0->upper_deadzone))
        {
            key0->key.report_state = false;
        }
        if (key0->key.report_state && !last_key0_state)
        {
            layer_cache_set(key0->key.id, g_current_layer);
            keyboard_event_handler(MK_EVENT(dynamic_key_m->key[0].binding, KEYBOARD_EVENT_KEY_DOWN));
            keyboard_advanced_key_event_handler(key0, KEYBOARD_EVENT_KEY_DOWN);
        }
        if (key0->key.report_state && last_key0_state)
        {
            keyboard_event_handler(MK_EVENT(dynamic_key_m->key[0].binding, KEYBOARD_EVENT_KEY_UP));
            keyboard_advanced_key_event_handler(key0, KEYBOARD_EVENT_KEY_UP);
        }
        advanced_key_update_state(key0, key0->key.report_state);
        key0->key.report_state = key0->key.report_state;

        bool last_key1_state = key1->key.report_state;
        if ((key0->value < key1->value) ||
        ((key0->value>= (ANALOG_VALUE_MAX - key0->lower_deadzone))&&
        (key1->value>= (ANALOG_VALUE_MAX - key1->lower_deadzone))))
        {
            key1->key.report_state = true;
        }
        else if (key0->value != key1->value || (key1->value > key1->upper_deadzone))
        {
            key1->key.report_state = false;
        }
        if (key1->key.report_state && !last_key1_state)
        {
            layer_cache_set(key1->key.id, g_current_layer);
            keyboard_event_handler(MK_EVENT(dynamic_key_m->key[1].binding, KEYBOARD_EVENT_KEY_DOWN));
            keyboard_advanced_key_event_handler(key1, KEYBOARD_EVENT_KEY_DOWN);
        }
        if (!key1->key.report_state && last_key1_state)
        {
            keyboard_event_handler(MK_EVENT(dynamic_key_m->key[1].binding, KEYBOARD_EVENT_KEY_UP));
            keyboard_advanced_key_event_handler(key1, KEYBOARD_EVENT_KEY_UP);
        }
        advanced_key_update_state(key1, key1->key.report_state);
        key1->key.report_state = key1->key.report_state;
        return;
    }
    bool last_key0_state = key0->key.report_state;
    bool last_key1_state = key1->key.report_state;
    switch (dynamic_key_m->mode & 0x0F)
    {
    case DK_MUTEX_LAST_PRIORITY:
        if (state && !key->key.state)
        {
            if (key->key.id == dynamic_key_m->key[0].id)
            {
                if (key1->key.report_state)
                {
                    key1->key.report_state = false;
                }
                key0->key.report_state = true;
            }
            else if (key->key.id == dynamic_key_m->key[1].id)
            {
                if (key0->key.report_state)
                {
                    key0->key.report_state = false;
                }
                key1->key.report_state = true;
            }
            key0->key.report_state &= key0->key.state;
            key1->key.report_state &= key1->key.state;
        }
        advanced_key_update_state(key, state);
        break;
    case DK_MUTEX_KEY1_PRIORITY:
        advanced_key_update_state(key, state);
        key0->key.report_state = key0->key.state;
        key1->key.report_state = key0->key.state ? false : key1->key.state;
        break;
    case DK_MUTEX_KEY2_PRIORITY:
        advanced_key_update_state(key, state);
        key0->key.report_state = key1->key.state ? false : key0->key.state;
        key1->key.report_state = key1->key.state;
        break;
    case DK_MUTEX_NEUTRAL:
        advanced_key_update_state(key, state);
        key0->key.report_state = key0->key.state;
        key1->key.report_state = key1->key.state;
        if (key0->key.state && key1->key.state)
        {
            key0->key.report_state = false;
            key1->key.report_state = false;
        }
        break;
    default:
        break;
    }
    if (dynamic_key_m->mode & 0xF0)
    {
        if ((key0->value>= (ANALOG_VALUE_MAX - key0->lower_deadzone))&&
        (key1->value>= (ANALOG_VALUE_MAX - key1->lower_deadzone)))
        {
            key0->key.report_state = true;
            key1->key.report_state = true;
        }
    }
    if (key0->key.report_state && !last_key0_state)
    {
        layer_cache_set(key1->key.id, g_current_layer);
        keyboard_event_handler(MK_EVENT(dynamic_key_m->key[0].binding, KEYBOARD_EVENT_KEY_DOWN));
        keyboard_advanced_key_event_handler(key0, KEYBOARD_EVENT_KEY_DOWN);
    }
    if (!key0->key.report_state && last_key0_state)
    {
        keyboard_event_handler(MK_EVENT(dynamic_key_m->key[0].binding, KEYBOARD_EVENT_KEY_UP));
        keyboard_advanced_key_event_handler(key0, KEYBOARD_EVENT_KEY_UP);
    }

    if (key1->key.report_state && !last_key1_state)
    {
        layer_cache_set(key1->key.id, g_current_layer);
        keyboard_event_handler(MK_EVENT(dynamic_key_m->key[1].binding, KEYBOARD_EVENT_KEY_DOWN));
        keyboard_advanced_key_event_handler(key1, KEYBOARD_EVENT_KEY_DOWN);
    }
    if (!key1->key.report_state && last_key1_state)
    {
        keyboard_event_handler(MK_EVENT(dynamic_key_m->key[1].binding, KEYBOARD_EVENT_KEY_UP));
        keyboard_advanced_key_event_handler(key1, KEYBOARD_EVENT_KEY_UP);
    }
}
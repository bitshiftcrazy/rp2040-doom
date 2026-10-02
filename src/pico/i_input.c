//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2021-2022 Graham Sanderson
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//     SDL implementation of system-specific input interface.
//

#include "pico/stdlib.h"
#include "hardware/i2c.h"

#include "doomkeys.h"
#include "doomtype.h"
#include "d_event.h"
#include "i_input.h"
extern boolean menuactive;
#define BTN_LEFT   8
#define BTN_FIRE   9
#define BTN_RIGHT 10

#define TOUCH_I2C       i2c0
#define TOUCH_SDA       20
#define TOUCH_SCL       21
#define CAP1203_ADDR    0x28

#define CAP1203_MAIN_CONTROL       0x00
#define CAP1203_SENSOR_STATUS      0x03

#define TOUCH_FORWARD   (1u << 0)
#define TOUCH_USE       (1u << 1)

typedef struct
{
    uint gpio;
    bool pressed;
} button_t;

static button_t buttons[] =
{
    { BTN_LEFT,  false },
    { BTN_FIRE,  false },
    { BTN_RIGHT, false },
};

static bool touch_forward_pressed;
static bool touch_use_pressed;


static void post_key(int key, bool pressed)
{
    event_t event;

    event.type  = pressed ? ev_keydown : ev_keyup;
    event.data1 = key;
    event.data2 = 0;
    event.data3 = 0;

    D_PostEvent(&event);
}


static bool cap1203_read(uint8_t reg, uint8_t *value)
{
    if (i2c_write_blocking(
            TOUCH_I2C,
            CAP1203_ADDR,
            &reg,
            1,
            true) != 1)
    {
        return false;
    }

    return i2c_read_blocking(
               TOUCH_I2C,
               CAP1203_ADDR,
               value,
               1,
               false) == 1;
}


static void cap1203_write(uint8_t reg, uint8_t value)
{
    uint8_t data[2] = { reg, value };

    i2c_write_blocking(
        TOUCH_I2C,
        CAP1203_ADDR,
        data,
        sizeof(data),
        false
    );
}


void I_InputInit(void)
{
    for (unsigned i = 0;
         i < sizeof(buttons) / sizeof(buttons[0]);
         ++i)
    {
        gpio_init(buttons[i].gpio);
        gpio_set_dir(buttons[i].gpio, GPIO_IN);
        gpio_pull_up(buttons[i].gpio);

        buttons[i].pressed = !gpio_get(buttons[i].gpio);
    }

    i2c_init(TOUCH_I2C, 400 * 1000);

    gpio_set_function(TOUCH_SDA, GPIO_FUNC_I2C);
    gpio_set_function(TOUCH_SCL, GPIO_FUNC_I2C);

    gpio_pull_up(TOUCH_SDA);
    gpio_pull_up(TOUCH_SCL);

    touch_forward_pressed = false;
    touch_use_pressed = false;
}


static void poll_buttons(void)
{
    static boolean last_menuactive = false;

    if (menuactive != last_menuactive)
    {
        post_key(KEY_LEFTARROW, false);
        post_key(KEY_RCTRL, false);
        post_key(KEY_RIGHTARROW, false);

        post_key(KEY_UPARROW, false);
        post_key(KEY_ENTER, false);
        post_key(KEY_DOWNARROW, false);

        for (unsigned int i = 0;
             i < sizeof(buttons) / sizeof(buttons[0]);
             ++i)
        {
            buttons[i].pressed = false;
        }

        last_menuactive = menuactive;
    }

    for (unsigned int i = 0;
         i < sizeof(buttons) / sizeof(buttons[0]);
         ++i)
    {
        bool pressed = !gpio_get(buttons[i].gpio);

        if (pressed != buttons[i].pressed)
        {
            buttons[i].pressed = pressed;

            int key;

            if (menuactive)
            {
                static const int menu_keys[] = {
                    KEY_UPARROW,
                    KEY_ENTER,
                    KEY_DOWNARROW
                };

                key = menu_keys[i];
            }
            else
            {
                static const int game_keys[] = {
                    KEY_LEFTARROW,
                    KEY_RCTRL,
                    KEY_RIGHTARROW
                };

                key = game_keys[i];
            }

            post_key(key, pressed);
        }
    }
}

static void poll_touch(void)
{
    uint8_t status;

    if (!cap1203_read(CAP1203_SENSOR_STATUS, &status))
        return;

    bool forward = (status & TOUCH_FORWARD) != 0;
    bool use     = (status & TOUCH_USE) != 0;

    if (forward != touch_forward_pressed)
    {
        touch_forward_pressed = forward;
        post_key(KEY_UPARROW, forward);
    }

    if (use != touch_use_pressed)
    {
        touch_use_pressed = use;
        post_key(' ', use);
    }

    uint8_t main_control;

    if (cap1203_read(CAP1203_MAIN_CONTROL, &main_control))
    {
        main_control &= ~1u;
        cap1203_write(CAP1203_MAIN_CONTROL, main_control);
    }
}


void I_GetEventTimeout(int key_timeout)
{
    (void) key_timeout;

    poll_buttons();
    poll_touch();
}


void I_GetEvent(void)
{
    I_GetEventTimeout(0);
}


void I_StartTextInput(int x1, int y1, int x2, int y2)
{
    (void) x1;
    (void) y1;
    (void) x2;
    (void) y2;
}


void I_StopTextInput(void)
{
}


void I_BindInputVariables(void)
{
}

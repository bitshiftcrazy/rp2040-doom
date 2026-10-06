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
#include <string.h>

#include "doomkeys.h"
#include "doomtype.h"
#include "d_event.h"
#include "i_input.h"

extern boolean menuactive;

#define BTN_SELECT 26
#define BTN_BACK   27
#define BTN_UP     28
#define BTN_DOWN   29

typedef struct
{
    bool up;
    bool down;
    bool left;
    bool right;
    bool fire;
    bool use;
    bool enter;
    bool escape;
} tome_key_state_t;

static tome_key_state_t posted_keys;

static void post_key(int key, bool pressed)
{
    event_t event;

    event.type  = pressed ? ev_keydown : ev_keyup;
    event.data1 = key;
    event.data2 = 0;
    event.data3 = 0;

    D_PostEvent(&event);
}

static inline void sync_key(int key, bool wanted, bool *posted)
{
    if (wanted == *posted)
        return;

    *posted = wanted;
    post_key(key, wanted);
}

void I_InputInit(void)
{
    const uint buttons[] = { BTN_UP, BTN_DOWN, BTN_SELECT, BTN_BACK };

    for (unsigned i = 0; i < sizeof(buttons) / sizeof(buttons[0]); ++i) {
        gpio_init(buttons[i]);
        gpio_set_dir(buttons[i], GPIO_IN);
        gpio_pull_up(buttons[i]);
    }

    memset(&posted_keys, 0, sizeof(posted_keys));
}

static void poll_buttons(void)
{
    bool up     = !gpio_get(BTN_UP);
    bool down   = !gpio_get(BTN_DOWN);
    bool select = !gpio_get(BTN_SELECT);
    bool back   = !gpio_get(BTN_BACK);

    tome_key_state_t wanted = {0};

    if (menuactive) {
        wanted.up     = up;
        wanted.down   = down;
        wanted.enter  = select;
        wanted.escape = back;
    } else if (back) {
        wanted.left  = up;
        wanted.right = down;
        wanted.use   = select;
    } else {
        wanted.up   = up;
        wanted.down = down;
        wanted.fire = select;
    }

    sync_key(KEY_UPARROW,    wanted.up,     &posted_keys.up);
    sync_key(KEY_DOWNARROW,  wanted.down,   &posted_keys.down);
    sync_key(KEY_LEFTARROW,  wanted.left,   &posted_keys.left);
    sync_key(KEY_RIGHTARROW, wanted.right,  &posted_keys.right);
    sync_key(KEY_RCTRL,      wanted.fire,   &posted_keys.fire);
    sync_key(' ',            wanted.use,    &posted_keys.use);
    sync_key(KEY_ENTER,      wanted.enter,  &posted_keys.enter);
    sync_key(KEY_ESCAPE,     wanted.escape, &posted_keys.escape);
}

void I_GetEventTimeout(int key_timeout)
{
    (void) key_timeout;
    poll_buttons();
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

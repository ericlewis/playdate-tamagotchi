/*
 * Playdate-Tamagotchi - A Tamagotchi P1 emulator for Playdate
 *
 * Copyright (C) 2022 Eric Lewis <ericlewis777@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#include "stdbool.h"
#include "stdio.h"

#include "tamalib/tamalib.h"
#include "pd_api.h"

#include "rom.h"
#include "state.h"
#include "preferences.h"

PlaydateAPI *pd;

static const int top_y = 16;
static const int bottom_y = 189;

static PDMenuItem *audioMenuItem;

static bool_t lcd_buffer[LCD_HEIGHT][LCD_WIDTH] = {};
static bool_t icon_buffer[ICON_NUM] = {};

static PDSynth* beeper = NULL;
static LCDBitmap* frame = NULL;
static LCDBitmap* background = NULL;

static LCDBitmap* icon0 = NULL;
static LCDBitmap* icon1 = NULL;
static LCDBitmap* icon2 = NULL;
static LCDBitmap* icon3 = NULL;
static LCDBitmap* icon4 = NULL;
static LCDBitmap* icon5 = NULL;
static LCDBitmap* icon6 = NULL;
static LCDBitmap* icon7 = NULL;

static LCDBitmap* icon0_off = NULL;
static LCDBitmap* icon1_off = NULL;
static LCDBitmap* icon2_off = NULL;
static LCDBitmap* icon3_off = NULL;
static LCDBitmap* icon4_off = NULL;
static LCDBitmap* icon5_off = NULL;
static LCDBitmap* icon6_off = NULL;
static LCDBitmap* icon7_off = NULL;

static bool_t lcd_changed = false;
static bool_t icon_changed = false;

static float frequency = -1;

static void * hal_malloc(u32_t size) 
{
	return malloc(size);
}

static void hal_free(void *ptr) 
{
	free(ptr);
}

static void hal_halt(void) 
{
	pd->system->error("halted execution");
}

static bool_t hal_is_log_enabled(log_level_t level) 
{
	return false;
}

static timestamp_t hal_get_timestamp(void) 
{
	return pd->system->getCurrentTimeMilliseconds();
}

static void hal_log(log_level_t level, char *buff, ...) {}
static void hal_sleep_until(timestamp_t ts) {}

static void hal_update_screen(void) 
{			
	if (icon_changed) 
	{
		pd->graphics->drawBitmap(icon_buffer[0] ? icon0 : icon0_off, 102, top_y, 0);
		pd->graphics->drawBitmap(icon_buffer[1] ? icon1 : icon1_off, 158, top_y-2, 0);
		pd->graphics->drawBitmap(icon_buffer[2] ? icon2 : icon2_off, 200, top_y, 0);
		pd->graphics->drawBitmap(icon_buffer[3] ? icon3 : icon3_off, 258, top_y-1, 0);
		pd->graphics->drawBitmap(icon_buffer[4] ? icon4 : icon4_off, 100, bottom_y, 0);
		pd->graphics->drawBitmap(icon_buffer[5] ? icon5 : icon5_off, 151, bottom_y + 5, 0);
		pd->graphics->drawBitmap(icon_buffer[6] ? icon6 : icon6_off, 199, bottom_y + 5, 0);
		pd->graphics->drawBitmap(icon_buffer[7] ? icon7 : icon7_off, 258, bottom_y+4, 0);
		icon_changed = false;
	}
	if (lcd_changed) 
	{
		pd->graphics->pushContext(frame);
		for (int x = 0; x < LCD_WIDTH; x++) 
		{
			for (int y = 0; y < LCD_HEIGHT; y++) 
			{
				pd->graphics->drawRect(x, y, 1, 1, lcd_buffer[y][x] ? kColorBlack : kColorWhite);
			}
		}
		pd->graphics->popContext();
		pd->graphics->drawScaledBitmap(frame, 102, 74, 6, 6);
		lcd_changed = false;
	}
}

static void hal_set_lcd_matrix(u8_t x, u8_t y, bool_t val) {
	if (!lcd_changed && lcd_buffer[y][x] != val) 
	{
		lcd_changed = true;
	}
	
	lcd_buffer[y][x] = val;
}

static void hal_set_lcd_icon(u8_t icon, bool_t val) {
	if (!icon_changed && icon_buffer[icon] != val) 
	{
		icon_changed = true;
	}
	
	icon_buffer[icon] = val;
}

static void hal_set_frequency(u32_t freq) 
{
	frequency = freq;
}

static void hal_play_frequency(bool_t play) 
{
	if (play && preferences_sound_enabled) 
	{
		pd->sound->synth->playNote(beeper, frequency, 1, 0.1, 0);
	} 
	else if (pd->sound->synth->isPlaying(beeper)) 
	{
		pd->sound->synth->noteOff(beeper, 0);
	}
}

static int hal_handler(void) 
{
	PDButtons released_buttons;
	PDButtons pressed_buttons;
	pd->system->getButtonState(NULL, &pressed_buttons, &released_buttons);
	
	if (pressed_buttons & kButtonLeft || pressed_buttons & kButtonRight || pressed_buttons & kButtonUp || pressed_buttons & kButtonDown) 
	{
		tamalib_set_button(BTN_LEFT, BTN_STATE_PRESSED);
	} 
	else if (released_buttons & kButtonLeft || released_buttons & kButtonRight || released_buttons & kButtonUp || released_buttons & kButtonDown) 
	{
		tamalib_set_button(BTN_LEFT, BTN_STATE_RELEASED);
	} 
	else if (pressed_buttons & kButtonA) 
	{
		tamalib_set_button(BTN_RIGHT, BTN_STATE_PRESSED);
	} 
	else if (released_buttons & kButtonA) 
	{
		tamalib_set_button(BTN_RIGHT, BTN_STATE_RELEASED);
	} 
	else if (pressed_buttons & kButtonB) 
	{
		tamalib_set_button(BTN_MIDDLE, BTN_STATE_PRESSED);
	} 
	else if (released_buttons & kButtonB) 
	{
		tamalib_set_button(BTN_MIDDLE, BTN_STATE_RELEASED);
	}
	
	return 0;
}

static hal_t hal = {
	.malloc = &hal_malloc,
	.free = &hal_free,
	.halt = &hal_halt,
	.is_log_enabled = &hal_is_log_enabled,
	.log = &hal_log,
	.sleep_until = &hal_sleep_until,
	.get_timestamp = &hal_get_timestamp,
	.update_screen = &hal_update_screen,
	.set_lcd_matrix = &hal_set_lcd_matrix,
	.set_lcd_icon = &hal_set_lcd_icon,
	.set_frequency = &hal_set_frequency,
	.play_frequency = &hal_play_frequency,
	.handler = &hal_handler,
};

int update(void* userdata) 
{
	tamalib_mainloop();
	return 1;
}

void toggled_sound_enabled(void *isEnabled)
{
	preferences_sound_enabled = pd->system->getMenuItemValue(audioMenuItem);
	preferences_save_to_disk();
}

int eventHandler(PlaydateAPI *playdate, PDSystemEvent event, uint32_t arg) 
{
	if (event == kEventInit) 
	{
		pd = playdate;
		
		pd->display->setRefreshRate(0);
		
		beeper = pd->sound->synth->newSynth();
		frame = pd->graphics->newBitmap(LCD_WIDTH, LCD_HEIGHT, kColorWhite);
		
		background = pd->graphics->loadBitmap("assets/background", NULL);	
		
		icon0 = pd->graphics->loadBitmap("assets/icon0", NULL);		
		icon1 = pd->graphics->loadBitmap("assets/icon1", NULL);		
		icon2 = pd->graphics->loadBitmap("assets/icon2", NULL);		
		icon3 = pd->graphics->loadBitmap("assets/icon3", NULL);		
		icon4 = pd->graphics->loadBitmap("assets/icon4", NULL);		
		icon5 = pd->graphics->loadBitmap("assets/icon5", NULL);		
		icon6 = pd->graphics->loadBitmap("assets/icon6", NULL);		
		icon7 = pd->graphics->loadBitmap("assets/icon7", NULL);		
		
		icon0_off = pd->graphics->loadBitmap("assets/icon0_off", NULL);		
		icon1_off = pd->graphics->loadBitmap("assets/icon1_off", NULL);		
		icon2_off = pd->graphics->loadBitmap("assets/icon2_off", NULL);		
		icon3_off = pd->graphics->loadBitmap("assets/icon3_off", NULL);		
		icon4_off = pd->graphics->loadBitmap("assets/icon4_off", NULL);		
		icon5_off = pd->graphics->loadBitmap("assets/icon5_off", NULL);		
		icon6_off = pd->graphics->loadBitmap("assets/icon6_off", NULL);		
		icon7_off = pd->graphics->loadBitmap("assets/icon7_off", NULL);	
				
		pd->graphics->drawBitmap(background, 0, 0, 0);
		icon_changed = true;
		
		preferences_read_from_disk();
		audioMenuItem = pd->system->addCheckmarkMenuItem("Sound", preferences_sound_enabled, toggled_sound_enabled, NULL);
		
		tamalib_register_hal(&hal);
		tamalib_init((u12_t*)g_program, NULL, 1000);
		
		state_load();
		
		playdate->system->setUpdateCallback(update, playdate);
	} 
	else if (event == kEventTerminate || event == kEventPause || event == kEventLock) 
	{
		state_save();
		preferences_save_to_disk();
	}
	
	return 0;
}
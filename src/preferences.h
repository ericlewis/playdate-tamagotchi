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
#ifndef preferences_h
#define preferences_h

#include "stdbool.h"
#include "stdio.h"

extern bool preferences_sound_enabled;

void preferences_init(void);

void preferences_read_from_disk(void);
void preferences_save_to_disk(void);

#endif /* preferences_h */
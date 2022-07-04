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

#include "preferences.h"

#include "pd_api.h"

PlaydateAPI *pd;

static const int pref_version = 1;

static const char *pref_filename = "preferences.bin";
static SDFile *pref_file;

bool preferences_sound_enabled = false;

void cpu_endian_to_big_endian(unsigned char *src, unsigned char *buffer, size_t size, size_t len);

static uint8_t preferences_read_uint8(void);
static void preferences_write_uint8(uint8_t value);
static uint32_t preferences_read_uint32(void);
static void preferences_write_uint32(uint32_t value);

void preferences_init(void){
	
	if(pd->file->stat(pref_filename, NULL) != 0){
		preferences_save_to_disk();
	}
	else {
		preferences_read_from_disk();
	}
}

void preferences_read_from_disk(void){
	pref_file = pd->file->open(pref_filename, kFileReadData);
	if(pref_file != NULL){
		// read model version
		preferences_read_uint32();
		
		preferences_sound_enabled = preferences_read_uint8();
		
		pd->file->close(pref_file);
	} else {
		preferences_sound_enabled = true;
	}
}

void preferences_save_to_disk(void){
	
	pref_file = pd->file->open(pref_filename, kFileWrite);
	
	preferences_write_uint32(pref_version);
	
	preferences_write_uint8(preferences_sound_enabled ? 1 : 0);
	
	pd->file->close(pref_file);
}

uint8_t preferences_read_uint8(void){
	uint8_t buffer[1];
	pd->file->read(pref_file, buffer, sizeof(uint8_t));
	return buffer[0];
}

void preferences_write_uint8(uint8_t value){
	pd->file->write(pref_file, &value, sizeof(uint8_t));
}

uint32_t preferences_read_uint32(void){
	unsigned char buffer[sizeof(uint32_t)];
	pd->file->read(pref_file, buffer, sizeof(uint32_t));
	return buffer[0] << 24 | buffer[1] << 16 | buffer[2] << 8 | buffer[3];
}

void preferences_write_uint32(uint32_t value){
	
	unsigned char buffer[sizeof(uint32_t)];
	cpu_endian_to_big_endian((unsigned char*)&value, buffer, sizeof(uint32_t), 1);
	
	pd->file->write(pref_file, buffer, sizeof(uint32_t));
}

void cpu_endian_to_big_endian(unsigned char *src, unsigned char *buffer, size_t size, size_t len){
	
	int x = 1;
	
	if(*((char*)&x) == 1){
		// little endian machine, swap
		for(size_t i = 0; i < len; i++){
			for (size_t ix = 0; ix < size; ix++){
				buffer[size * i + ix] = src[size * i + (size - 1 - ix)];
			}
		}
	}
	else {
		*buffer = *src;
	}
}
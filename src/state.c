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
#include "tamalib.h"
#include "state.h"

#include "pd_api.h"

PlaydateAPI *pd;

#define STATE_SLOT_SIZE					821 // in bytes

#define STATE_FILE_MAGIC				"TLST"
#define STATE_FILE_VERSION				2

static uint8_t state_buf[STATE_SLOT_SIZE];

void state_save() {
	SDFile* f;
	state_t *state;
	uint8_t *ptr = state_buf;
	uint32_t i;

	state = tamalib_get_state();

	ptr[0] = (uint8_t) STATE_FILE_MAGIC[0];
	ptr[1] = (uint8_t) STATE_FILE_MAGIC[1];
	ptr[2] = (uint8_t) STATE_FILE_MAGIC[2];
	ptr[3] = (uint8_t) STATE_FILE_MAGIC[3];
	ptr += 4;
	
	ptr[0] = STATE_FILE_VERSION & 0xFF;
	ptr += 1;
	
	ptr[0] = *(state->pc) & 0xFF;
	ptr[1] = (*(state->pc) >> 8) & 0x1F;
	ptr += 2;
	
	ptr[0] = *(state->x) & 0xFF;
	ptr[1] = (*(state->x) >> 8) & 0xF;
	ptr += 2;
	
	ptr[0] = *(state->y) & 0xFF;
	ptr[1] = (*(state->y) >> 8) & 0xF;
	ptr += 2;
	
	ptr[0] = *(state->a) & 0xF;
	ptr += 1;
	
	ptr[0] = *(state->b) & 0xF;
	ptr += 1;
	
	ptr[0] = *(state->np) & 0x1F;
	ptr += 1;
	
	ptr[0] = *(state->sp) & 0xFF;
	ptr += 1;
	
	ptr[0] = *(state->flags) & 0xF;
	ptr += 1;
	
	ptr[0] = *(state->tick_counter) & 0xFF;
	ptr[1] = (*(state->tick_counter) >> 8) & 0xFF;
	ptr[2] = (*(state->tick_counter) >> 16) & 0xFF;
	ptr[3] = (*(state->tick_counter) >> 24) & 0xFF;
	ptr += 4;
	
	ptr[0] = *(state->clk_timer_timestamp) & 0xFF;
	ptr[1] = (*(state->clk_timer_timestamp) >> 8) & 0xFF;
	ptr[2] = (*(state->clk_timer_timestamp) >> 16) & 0xFF;
	ptr[3] = (*(state->clk_timer_timestamp) >> 24) & 0xFF;
	ptr += 4;
	
	ptr[0] = *(state->prog_timer_timestamp) & 0xFF;
	ptr[1] = (*(state->prog_timer_timestamp) >> 8) & 0xFF;
	ptr[2] = (*(state->prog_timer_timestamp) >> 16) & 0xFF;
	ptr[3] = (*(state->prog_timer_timestamp) >> 24) & 0xFF;
	ptr += 4;
	
	ptr[0] = *(state->prog_timer_enabled) & 0x1;
	ptr += 1;
	
	ptr[0] = *(state->prog_timer_data) & 0xFF;
	ptr += 1;
	
	ptr[0] = *(state->prog_timer_rld) & 0xFF;
	ptr += 1;
	
	ptr[0] = *(state->call_depth) & 0xFF;
	ptr[1] = (*(state->call_depth) >> 8) & 0xFF;
	ptr[2] = (*(state->call_depth) >> 16) & 0xFF;
	ptr[3] = (*(state->call_depth) >> 24) & 0xFF;
	ptr += 4;
	
	for (i = 0; i < INT_SLOT_NUM; i++) {
		ptr[0] = state->interrupts[i].factor_flag_reg & 0xF;
		ptr += 1;
	
		ptr[0] = state->interrupts[i].mask_reg & 0xF;
		ptr += 1;
	
		ptr[0] = state->interrupts[i].triggered & 0x1;
		ptr += 1;
	}
	
	/* First 640 half bytes correspond to the RAM */
	for (i = 0; i < MEM_RAM_SIZE; i++) {
		ptr[i] = GET_RAM_MEMORY(state->memory, i + MEM_RAM_ADDR) & 0xF;
	}
	ptr += MEM_RAM_SIZE;
	
	/* I/Os are from 0xF00 to 0xF7F */
	for (i = 0; i < MEM_IO_SIZE; i++) {
		ptr[i] = GET_RAM_MEMORY(state->memory, i + MEM_IO_ADDR) & 0xF;
	}
	ptr += MEM_IO_SIZE;

	f = pd->file->open("save.bin", kFileWrite);
	if (!f) {
		pd->system->error("Could not create save.bin!");
		return;
	}
	
	int bytesWritten = pd->file->write(f, state_buf, sizeof(state_buf));
	if (bytesWritten < sizeof(state_buf)) {
		pd->system->error("Could not write save.bin!");
		pd->file->close(f);
		return;
	}
	
	pd->file->close(f);
}

void state_load() {
	SDFile* f;
	state_t *state;
	uint8_t *ptr = state_buf;
	uint32_t i;

	state = tamalib_get_state();
		
	f = pd->file->open("save.bin", kFileReadData);
	if (!f) {
		pd->system->logToConsole("Could not load save.bin: %s", pd->file->geterr());
		return;
	}
	
	int bytesRead = pd->file->read(f, state_buf, sizeof(state_buf));
	if (bytesRead < sizeof(state_buf)) {
		pd->system->error("Could not read save.bin");
		pd->file->close(f);
		return;
	}
	
	pd->file->close(f);
	
	if (ptr[0] != (uint8_t) STATE_FILE_MAGIC[0] || ptr[1] != (uint8_t) STATE_FILE_MAGIC[1] ||
		ptr[2] != (uint8_t) STATE_FILE_MAGIC[2] || ptr[3] != (uint8_t) STATE_FILE_MAGIC[3]) {
		return;
	}
	ptr += 4;
	
	if (ptr[0] != STATE_FILE_VERSION) {
		/* TODO: Handle migration at a point */
		return;
	}
	ptr += 1;
	
	*(state->pc) = ptr[0] | ((ptr[1] & 0x1F) << 8);
	ptr += 2;
	
	*(state->x) = ptr[0] | ((ptr[1] & 0xF) << 8);
	ptr += 2;
	
	*(state->y) = ptr[0] | ((ptr[1] & 0xF) << 8);
	ptr += 2;
	
	*(state->a) = ptr[0] & 0xF;
	ptr += 1;
	
	*(state->b) = ptr[0] & 0xF;
	ptr += 1;
	
	*(state->np) = ptr[0] & 0x1F;
	ptr += 1;
	
	*(state->sp) = ptr[0];
	ptr += 1;
	
	*(state->flags) = ptr[0] & 0xF;
	ptr += 1;
	
	*(state->tick_counter) = ptr[0] | (ptr[1] << 8) | (ptr[2] << 16) | (ptr[3] << 24);
	ptr += 4;
	
	*(state->clk_timer_timestamp) = ptr[0] | (ptr[1] << 8) | (ptr[2] << 16) | (ptr[3] << 24);
	ptr += 4;
	
	*(state->prog_timer_timestamp) = ptr[0] | (ptr[1] << 8) | (ptr[2] << 16) | (ptr[3] << 24);
	ptr += 4;
	
	*(state->prog_timer_enabled) = ptr[0] & 0x1;
	ptr += 1;
	
	*(state->prog_timer_data) = ptr[0];
	ptr += 1;
	
	*(state->prog_timer_rld) = ptr[0];
	ptr += 1;
	
	*(state->call_depth) = ptr[0] | (ptr[1] << 8) | (ptr[2] << 16) | (ptr[3] << 24);
	ptr += 4;
	
	for (i = 0; i < INT_SLOT_NUM; i++) {
		state->interrupts[i].factor_flag_reg = ptr[0] & 0xF;
		ptr += 1;
	
		state->interrupts[i].mask_reg = ptr[0] & 0xF;
		ptr += 1;
	
		state->interrupts[i].triggered = ptr[0] & 0x1;
		ptr += 1;
	}
	
	/* First 640 half bytes correspond to the RAM */
	for (i = 0; i < MEM_RAM_SIZE; i++) {
		SET_RAM_MEMORY(state->memory, i + MEM_RAM_ADDR, ptr[i] & 0xF);
	}
	ptr += MEM_RAM_SIZE;
	
	/* I/Os are from 0xF00 to 0xF7F */
	for (i = 0; i < MEM_IO_SIZE; i++) {
		SET_RAM_MEMORY(state->memory, i + MEM_IO_ADDR, ptr[i] & 0xF);
	}
	ptr += MEM_IO_SIZE;
	
	tamalib_refresh_hw();
}
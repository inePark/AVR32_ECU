/*****************************************************************************
 *
 * \file
 *
 * \brief GUI
 *
 * Copyright (c) 2009 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 *****************************************************************************/

#include <stdarg.h>
#include <stdio.h>
#include "board.h"
#include "et024006dhu.h"
#include "conf_demo.h"
#include "gpio.h"
#include "gui.h"
#include "background_image.h"
#include "cycle_counter.h"
#include "can_task.h"
#include "dsp.h"

#include "compiler.h"
#include "defines.h"

#define YELLOW 0xFFF0

//Extern Variables
extern fw_state_t gFwState;

struct gui_box
{
  uint16_t x, y;
  uint16_t width, height;
  et024006_color_t fg_color, bg_color;
};

static void gui_draw_background(uint16_t x, uint16_t y, uint16_t width,
		uint16_t height, char bg_flag_);

static inline void gui_buffer_set_pixel(void *buffer, uint16_t width,
		uint16_t height, uint16_t x, uint16_t y);
static void gui_print_signal_box(int32_t box_id, dsp16_t *signal,
		uint16_t signal_size);

static const struct gui_box box[] = {
	GUI_BOXES
};

#define MAX_BUFFER_WIDTH    GUI_ZOOM_BOX_WIDTH
#define MAX_BUFFER_HEIGHT   GUI_ZOOM_BOX_HEIGHT

static uint8_t buffer[ ET024006_BITMAP_WIDTH(MAX_BUFFER_WIDTH)* \
	MAX_BUFFER_HEIGHT];
//static dsp16_t signal_buffer[BUFFER_LENGTH];

static struct
{
  t_cpu_time cpu_time;
  uint32_t time_ms;
} gui_fs;
static int gui_cpu_hz;

char gui_text_buffer[GUI_TEXT_BUFFER_SIZE];

extern dsp16_t signal1_buf[BUFFER_LENGTH];
extern dsp16_t signal2_buf[BUFFER_LENGTH];
extern dsp16_t signal3_buf[BUFFER_LENGTH];
extern dsp16_t signal4_buf[BUFFER_LENGTH];
extern dsp16_t *signal_in_buf;
extern unsigned char version_flag;
extern unsigned counter_;
extern dsp16_t signal_in_fft[BUFFER_LENGTH];
extern dsp16_t signal_out_buf[BUFFER_LENGTH];
extern dsp16_t signal_out_fft[BUFFER_LENGTH];

extern bool signals_are_updated;
void gui_change_update_fs(uint32_t time_ms)
{
	gui_fs.time_ms = time_ms;
	cpu_set_timeout(cpu_ms_2_cy(gui_fs.time_ms, gui_cpu_hz), &gui_fs.cpu_time);
}

uint32_t gui_get_update_fs(void)
{
  return gui_fs.time_ms;
}

void gui_init(uint32_t cpu_hz, uint32_t hsb_hz, uint32_t pba_hz, uint32_t pbb_hz)
{
	gui_cpu_hz = cpu_hz;
	// Init the display
	et024006_Init(cpu_hz, hsb_hz);
	gpio_set_gpio_pin(ET024006DHU_BL_PIN);
	// Draw the background
	gui_clear_view();
	// Set the frequency rate of the update of the display
	gui_change_update_fs(DEFAULT_SCREEN_UPDATE_FS_MS);
}

void gui_clear_view(void)
{
	if ( version_flag != 2)	{
		gui_draw_background(0, 0, ET024006_WIDTH, ET024006_HEIGHT, 0);
	}		
	
	
	else
	{
		gui_draw_background(0, 0, ET024006_WIDTH, ET024006_HEIGHT, 1);
	}
}

void gui_task(void)
{
  // Check the last time the printing has been done
	if (version_flag != 2)	{
		if (cpu_is_timeout(&gui_fs.cpu_time)) {

			gui_print_signal_box(GUI_OUTPUT3_ID, signal1_buf,
			sizeof(signal1_buf)/sizeof(signal1_buf[0]));
			gui_print_signal_box(GUI_OUTPUT2_ID, signal3_buf,
			sizeof(signal3_buf)/sizeof(signal3_buf[0]));

			cpu_set_timeout(cpu_ms_2_cy(gui_fs.time_ms, gui_cpu_hz),
			&gui_fs.cpu_time);
		}
	}
	else
	{		
		if (cpu_is_timeout(&gui_fs.cpu_time)) {
			gui_print_signal_box(GUI_SOURCE1_ID, signal1_buf,
			sizeof(signal1_buf)/sizeof(signal1_buf[0]));
			gui_print_signal_box(GUI_OUTPUT3_ID, signal1_buf,
			sizeof(signal1_buf)/sizeof(signal1_buf[0]));
			gui_print_signal_box(GUI_OUTPUT2_ID, signal3_buf,
			sizeof(signal3_buf)/sizeof(signal3_buf[0]));
			gui_print_signal_box(GUI_OUTPUT1_ID, signal4_buf,
			sizeof(signal4_buf)/sizeof(signal4_buf[0]));
			cpu_set_timeout(cpu_ms_2_cy(gui_fs.time_ms, gui_cpu_hz),
			&gui_fs.cpu_time);
		}
	}		
}

void gui_set_selection(int32_t box_id)
{
static int32_t prev_selection = GUI_NO_SEL;
struct gui_box *sb;
uint32_t i;

  // Clear previous selections
	if (prev_selection != GUI_NO_SEL && prev_selection != box_id) {
    
	i = prev_selection;
	if (version_flag == 2 && i >= 2)			
		i +=2;
				
    sb = (struct gui_box *) &box[i];
					
    // Don't do anything if the color is transparent
		if (sb->bg_color == GUI_NO_COLOR) {

				gui_draw_background(sb->x - 2,
					sb->y - 2,
					sb->width + 4,
					2,
					0);
				gui_draw_background(sb->x - 2,
					sb->y + sb->height,
					sb->width + 4,
					2,
					0);
				gui_draw_background(sb->x - 2,
					sb->y - 2,
					2,
					sb->height + 4,
					0);
				gui_draw_background(sb->x + sb->width,
					sb->y - 2,
					2,
					sb->height + 4,
					0);
						
		}
		else {

			et024006_DrawFilledRect(sb->x - 2,
				sb->y - 2,
				sb->width + 4,
				2,
				sb->bg_color);
			et024006_DrawFilledRect(sb->x - 2,
				sb->y + sb->height,
				sb->width + 4,
				2,
				sb->bg_color);
			et024006_DrawFilledRect(sb->x - 2,
				sb->y - 2,
				2,sb->height + 4,
				sb->bg_color);
			et024006_DrawFilledRect(sb->x + sb->width,
				sb->y - 2,
				2,
				sb->height + 4,
				sb->bg_color);

					
		}
  }

  prev_selection = box_id;
  	if (version_flag == 2 && box_id >= 2)
  	box_id+=2;
	if (box_id != GUI_NO_COLOR) {
  // Draw the selection
  sb = (struct gui_box *) &box[box_id];

		et024006_DrawFilledRect( sb->x - 2,
		sb->y - 2,
		sb->width + 4,
		2,
		GUI_SELECTION_COLOR);
		et024006_DrawFilledRect(sb->x - 2,
		sb->y + sb->height,
		sb->width + 4,
		2,
		GUI_SELECTION_COLOR);
		et024006_DrawFilledRect(sb->x - 2,
		sb->y - 2,
		2,
		sb->height + 4,
		GUI_SELECTION_COLOR);
		et024006_DrawFilledRect(sb->x + sb->width,
		sb->y - 2,
		2,
		sb->height + 4,
		GUI_SELECTION_COLOR);
			
	}
}


static void gui_print_signal_box(int32_t box_id, dsp16_t *signal, uint16_t signal_size)
{
	const struct gui_box *sb = &box[box_id];
	extern volatile bool zoom_view;
	extern volatile int32_t zoom_view_id;
	static bool can_tx_flag = false;
	char textPrint [256][4] = {{0, }, };
		
	if (version_flag == 2 && box_id >= 2)
		sb = &box[box_id + 2];

	if (zoom_view && zoom_view_id != box_id)
		return;

	if (zoom_view && zoom_view_id == box_id) {
		counter_ += 0x01;
		et024006_PutBitmap(buffer, GUI_ZOOM_BOX_WIDTH, GUI_ZOOM_BOX_X, GUI_ZOOM_BOX_Y, GUI_ZOOM_BOX_WIDTH, GUI_ZOOM_BOX_HEIGHT, sb->fg_color, sb->bg_color);
		
		if (version_flag != 2)	{
			switch (box_id)
			{
				case 1:	//LEFT
					sprintf (textPrint[box_id], "NONE");	
					break;
				
				case 0:	//RIGHT	
					sprintf (textPrint[box_id], "NONE");
					break;
					
				case 2:
					sprintf (textPrint[box_id], "[Front volume up!: %d]", gFwState.fVolSet );
					break;
				
				case 3:
					sprintf (textPrint[box_id], "[Front volume down!: %d]", gFwState.fVolSet );
					break;
			}
		}	
		else	{
							
				switch (box_id)
				{
					case 1:	//LEFT
						sprintf (textPrint[box_id], "[Rear volume up!: %d]",  gFwState.rVolSet );
						break;
								
					case 0:	//RIGHT
						sprintf (textPrint[box_id], "[Rear volume down!: %d]",  gFwState.rVolSet );
						break;
								
					case 2:
						sprintf (textPrint[box_id], "[Front volume up!: %d]",  gFwState.fVolSet);
						break;
								
					case 3:
						sprintf (textPrint[box_id], "[Front volume down!: %d]",  gFwState.fVolSet);
						break;
				}
			}		
			
			et024006_PrintString(textPrint[box_id], (const unsigned char*)FONT8x16, GUI_ZOOM_BOX_X, (uint16_t)100, BLACK, WHITE);
			
			if (!can_tx_flag)	{
				can_tx_flag = true;

				if (version_flag == 2)	{
					switch (box_id)
					{
						case 1:	//LEFT
							//can_example_prepare_data_to_send2 (REAR_UP);
							if (gFwState.rVolSet < AVN_VOLUME_MAX) {
								gFwState.rVolSet ++;
							}
							break;
						
						case 0:	//RIGHT
							//can_example_prepare_data_to_send2 (REAR_DOWN);
							if(gFwState.rVolSet > AVN_VOLUME_MIN) {
								gFwState.rVolSet --;	
							}
							break;
						
						case 2:
							//can_example_prepare_data_to_send2 (FRONT_UP);
							if (gFwState.fVolSet < AVN_VOLUME_MAX) {
								gFwState.fVolSet ++;
							}
							break;
						
						case 3:
							//can_example_prepare_data_to_send2 (FRONT_DOWN);
							if (gFwState.fVolSet > AVN_VOLUME_MIN) {
								gFwState.fVolSet --;
							}
							break;
						
						default:
							break;
					}
				}
				else
				{
					
					switch (box_id)
					{
						case 2:
							//can_example_prepare_data_to_send2 (FRONT_UP);
							if (gFwState.fVolSet < AVN_VOLUME_MAX) {
								gFwState.fVolSet ++;
							}
							break;
						
						case 3:
							//can_example_prepare_data_to_send2 (FRONT_DOWN);
							if (gFwState.fVolSet > AVN_VOLUME_MIN) {
								gFwState.fVolSet --;
							}
							break;
						
						default:
							break;
						
					} //switch
				}
			}
	
		
	}
	else {
		
		can_tx_flag = false;
			
		et024006_PutBitmap(buffer, sb->width, sb->x, sb->y, sb->width, sb->height, sb->fg_color, sb->bg_color);
			
		counter_ = 0;
			
		if (version_flag != 2)	{
			switch (box_id)
			{
				case 0:
				case 1:
					break;
				case 2:
					sprintf (textPrint[box_id], "Volume\nUP!\n( %d )", gFwState.fVolSet);
					et024006_PrintString(textPrint[box_id], (const unsigned char*)FONT8x8, GUI_OUTPUT2_X, GUI_OUTPUT2_Y, BLUE, YELLOW);
					break;
				case 3:
					sprintf (textPrint[box_id], "Volume\nDOWN!\n( %d )", gFwState.fVolSet);
					et024006_PrintString(textPrint[box_id], (const unsigned char*)FONT8x8, GUI_OUTPUT3_X, GUI_OUTPUT3_Y, BLUE, YELLOW);
					break;
			}
		}
		else
		{
			switch (box_id)
			{
				case 0:
					sprintf (textPrint[box_id], "R. Vol.\nDOWN\n(%d)", gFwState.rVolSet);
					et024006_PrintString(
					textPrint[box_id],
					(const unsigned char*)FONT8x8,
					GUI_SOURCE1_X,
					GUI_SOURCE1_Y,
					RED,
					YELLOW);
					break;
				case 1:
					sprintf (textPrint[box_id], "R. Vol.\nUP\n(%d)", gFwState.rVolSet);
					et024006_PrintString(textPrint[box_id], (const unsigned char*)FONT8x8, GUI_OUTPUT1_X, GUI_OUTPUT1_Y, RED, YELLOW);
					break;
				case 2:
					sprintf (textPrint[box_id], "F. Vol.\nUP\n(%d)", gFwState.fVolSet);
					et024006_PrintString(textPrint[box_id], (const unsigned char*)FONT8x8, GUI_OUTPUT2_X2, GUI_OUTPUT2_Y, BLUE, YELLOW);
					break;
				case 3:
					sprintf (textPrint[box_id], "F. Vol.\nDOWN\n(%d)", gFwState.fVolSet);
					et024006_PrintString(textPrint[box_id], (const unsigned char*)FONT8x8, GUI_OUTPUT3_X2, GUI_OUTPUT3_Y, BLUE, YELLOW);
					break;
			}
		}
	}
} 

static void gui_draw_background(uint16_t x, uint16_t y, uint16_t width, uint16_t height, char bg_flag)
{
	if (version_flag != 2)	{
		et024006_PutPixmap(background_image, ET024006_WIDTH, x, y, x, y, width, height);
	}
	else
	{
		et024006_PutPixmap(background_image2, ET024006_WIDTH, x, y, x, y, width, height);
	}
}


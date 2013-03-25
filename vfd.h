/*
 * Copyright (c) 2012
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

/**
 * \file
 *         Drivers for VFD display using a Motorola M66004SP driver IC, and a 74HC165
 *         parallell load serial out for reading a set of buttons.
 * \author
 *         Marcus Lunden <marcus.lunden@gmail.com>
 */

#ifndef __VFD_DRIVER_H__
#define __VFD_DRIVER_H__

#include <stdio.h>
#include "contiki.h"

/*---------------------------------------------------------------------------*/
#define VFD_PORT(type)        P1##type
#define VFD_SCK_PIN           (1<<1)  /* serial clock for both M66004 and '165 */
#define VFD_SDO_PIN           (1<<2)  /* serial data to M66004 */
#define VFD_QH_PIN            (1<<3)  /* serial out from '165 */
#define VFD_CS_PIN            (1<<4)  /* chip select M66004 */
#define VFD_SHLD_PIN          (1<<5)  /* shift/load '165. Keep high when reading '165 */
/*---------------------------------------------------------------------------*/
//uint8_t spinner[] = {'/', '-', '\', '|', '/', '-', '|'};
//uint8_t spinner[] = {0x3C, 0x2A, 0x3E, 0x2A};
#define VFD_USER_CHARS_LEN      40
/*---------------------------------------------------------------------------*/
/* Init the display board */
void vfd_init(void);

/* Read the buttons. */
uint8_t vfd_read_btns(void);

/* Print a single char on the display */
void vfd_print_char(uint8_t where, uint8_t chr);

/* Clear the display */
void vfd_clear(void);

/* Clear the display from decimal dots */
void vfd_clear_ddots(void);

/* Print an entire string on the display */
void vfd_set_string(char *str);

/* Define user chars, writes them to memory of the M66004. */
void vfd_set_dimmer(uint8_t val);

/* Set the decimal dot on a character. */
void vfd_set_ddot(uint8_t val, uint8_t doton);

#endif /* __VFD_DRIVER_H__ */


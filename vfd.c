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
 * OUT OF THE USE OF THIS SOFTWARE, EVEN ifADVISED OF THE POSSIBILITY OF
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

#include <stdio.h>
#include "contiki.h"
#include "vfd.h"
/*---------------------------------------------------------------------------*/
static void     tx_byte(uint8_t b);
static void     tx_bytevector(uint8_t *bv, uint8_t len);
/*---------------------------------------------------------------------------*/
#define VFD_ALL_OFF_LEN     17    // # bytes
static const uint8_t all_off[] = {0xE0, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                                  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                                  0x20};
static uint8_t  current_dim = 0;

#define VFD_SCK_PIN           (1<<1)  /* serial clock for both M66004 and '165 */
#define VFD_SDO_PIN           (1<<2)  /* serial data to M66004 */
#define VFD_QH_PIN            (1<<3)  /* serial out from '165 */
#define VFD_CS_PIN            (1<<4)  /* chip select M66004 */
#define VFD_SHLD_PIN          (1<<5)  /* shift/load '165. Keep high when reading '165 */

/* -------------------------------------------------------------------------- */
/* Init the display board */
void
vfd_init(void){
  /* Setup ports and pins */
  VFD_PORT(SEL) &=~ (VFD_SCK_PIN | VFD_SDO_PIN | VFD_QH_PIN);
  VFD_PORT(SEL2) &=~ (VFD_SCK_PIN | VFD_SDO_PIN | VFD_QH_PIN);
  VFD_PORT(DIR) |= (VFD_SCK_PIN | VFD_SDO_PIN);
  VFD_PORT(DIR) &=~ (VFD_QH_PIN);   // serial out from '165 is Input here
  VFD_PORT(OUT) |= (VFD_SCK_PIN);
  VFD_PORT(OUT) &=~ (VFD_SDO_PIN);

  VFD_PORT(SEL) &=~ (VFD_SHLD_PIN | VFD_CS_PIN);
  VFD_PORT(SEL2) &=~ (VFD_SHLD_PIN | VFD_CS_PIN);
  VFD_PORT(DIR) |= (VFD_SHLD_PIN | VFD_CS_PIN);
  VFD_PORT(OUT) |= (VFD_CS_PIN);
  VFD_PORT(OUT) &=~ (VFD_SHLD_PIN);
/*  VFD_PORT(REN) |= (VFD_QH_PIN);*/  // pull up resistor
/*  VFD_PORT(OUT) |= (VFD_QH_PIN);*/

  /* set 16 chars length of display */
  tx_byte(0x00 & 0x07);
  /* Enable auto increment (should never be turned off) */
  tx_byte(0xF4 & 0x01);
  /* Set dimmer value + freq (128/fOSC) */
  tx_byte(0x08 & 0x07);
  tx_byte(0xF6 & 0x00);
  /* Transmit user defined chars */
/*  tx_bytevector(&vfd_user_defined_chars, VFD_USER_CHARS_LEN);*/
}

/* -------------------------------------------------------------------------- */
/* Read the buttons. */
uint8_t
vfd_read_btns(void){
  uint8_t read = 0;
  uint8_t i;
  uint8_t bit = 0x01;    // XXX MSB or LSB first?
  VFD_PORT(OUT) |= (VFD_SHLD_PIN);
  for (i = 0; i < 8; i += 1) {
    VFD_PORT(OUT) &=~ (VFD_SCK_PIN);    // set SCK low XXX low to high or vice versa?
    if(VFD_PORT(IN) & VFD_QH_PIN) {
      read |= bit;
    }
    VFD_PORT(OUT) |= (VFD_SCK_PIN);   // set SCK high
    bit = bit << 1;
  }
  VFD_PORT(OUT) &=~ (VFD_SHLD_PIN);
  return read;
}

/* -------------------------------------------------------------------------- */
/* Print a single char on the display */
/*
  µ == can't find it on my keyboard :P
  This chart corresponds to the usual ASCII
  0x20            0x2F
     !"#$%&'()*+,-./
    0123456789:;<=>?
    @ABCDEFGHIJKLMNO
    PQRTSUVWXYZ[µ]^_
    µabcdefghijklmno
    pqrstuvwxyz{|}µµ
  0x70            0x7F
*/

void
vfd_print_char(uint8_t where, uint8_t chr){
  static uint8_t vect[2];
  if(where > 15) {
    // only positions between 0..15 accepted (M66004 adds 1).
    return;
  }
  vect[0] = 0xE0 | where;
  vect[1] = chr;    // XXX There is no input check on this char!
  tx_bytevector(&vect[0], 2);
}

/* -------------------------------------------------------------------------- */
/* Clear the display */
void
vfd_clear(void)
{
  tx_bytevector(&all_off[0], VFD_ALL_OFF_LEN);
}
/* -------------------------------------------------------------------------- */
/* Clear the display from decimal dots TODO */
void
vfd_clear_ddots(void)
{
/*  tx_bytevector(&all_off[0], VFD_ALL_OFF_LEN);*/
  return;
}
/* -------------------------------------------------------------------------- */
/* Show or hide the entire display (mem is not cleared) */
void
vfd_showhide(uint8_t show)
{
  if(show) {
    tx_byte(0xF0 & 0x01);
  } else {
    tx_byte(0xF0 & 0x00);
  }
}
/* -------------------------------------------------------------------------- */
/* Print an entire string on the display, passed vector must be >= 16 bytes long */
void
vfd_set_string(char *str)
{
  tx_byte(0xE0);    // first position
  tx_bytevector(str, 16);
}

/* -------------------------------------------------------------------------- */
/* Set dimmer value (1..8). */
void
vfd_set_dimmer(uint8_t val)
{
  val = val > 8 ? 8 : val;
  if(val == 0) {
    // shut off display
    return;
  }

  if(val == current_dim) {
    return;
  }
  
  current_dim = val;
  tx_byte(0x08 & val);
}
/* -------------------------------------------------------------------------- */
/* Set the decimal dot on a character. */
void
vfd_set_ddot(uint8_t val, uint8_t doton)
{
  if(val == 0 || val > 16) {
    return;
  }

  if(doton) {
    tx_byte(0x10 & val);    // turn dot on
  } else {
    tx_byte(0x80 & val);    // turn dot off
  }
}
/*---------------------------------------------------------------------------*/
static void
tx_byte(uint8_t b)
{
  uint8_t i;
  uint8_t bit = 0x80;
  // start condition
  VFD_PORT(OUT) &=~ (VFD_CS_PIN);
  // MSB first, shift on SCK lo->hi transition
  for (i = 0; i < 8; i += 1) {
    VFD_PORT(OUT) &=~ (VFD_SCK_PIN);    // set SCK low
    if(b & bit) {
      VFD_PORT(OUT) |= (VFD_SDO_PIN);
    } else {
      VFD_PORT(OUT) &=~ (VFD_SDO_PIN);
    }
    VFD_PORT(OUT) |= (VFD_SCK_PIN);   // set SCK high
    bit = bit >> 1;             // shift so we check the next one in order
  }
  // end condition
  VFD_PORT(OUT) |= (VFD_CS_PIN);
}
/* -------------------------------------------------------------------------- */
static void
tx_bytevector(uint8_t *bv, uint8_t len)
{
  uint8_t i, j, b;
  // start condition
  VFD_PORT(OUT) &=~ (VFD_CS_PIN);
  // transmit each byte
  for (j = 0; j < len; j += 1) {
    uint8_t bit = 0x80;
    /* load next byte to transmit */
    b = *(bv + j);

    /* MSB first, shift on SCK lo->hi transition */
    for (i = 0; i < 8; i += 1) {
      VFD_PORT(OUT) &=~ (VFD_SCK_PIN);  /* set SCK low */
      if(b & bit) {
        VFD_PORT(OUT) |= (VFD_SDO_PIN);
      } else {
        VFD_PORT(OUT) &=~ (VFD_SDO_PIN);
      }
      VFD_PORT(OUT) |= (VFD_SCK_PIN);   /* set SCK high */
      bit = bit >> 1;             /* shift so we check the next one in order */
    }
    /* small delay here, tBUSY = 6 cycles at 500 kHz =~ 14 µs == 14 cycles at 1 MHz. */
  }
  /* end condition */
  VFD_PORT(OUT) |= (VFD_CS_PIN);
}
/* -------------------------------------------------------------------------- */

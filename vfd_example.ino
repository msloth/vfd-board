/*
 * Copyright (c) 2013
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
 * 3. Neither the name of the Author nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
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
 *         VFD driver usage example
 * \author
 *         Marcus Lunden <marcus.lunden@gmail.com>
 */
/*---------------------------------------------------------------------------*/
#define VFD_SCK_PIN           (6)  /* serial clock for both M66004 and '165 */
#define VFD_SDO_PIN           (5)  /* serial data to M66004 */
#define VFD_CS_PIN            (4)  /* chip select M66004 */

#define VFD_QH_PIN            (8)  /* serial out from '165 */
#define VFD_SHLD_PIN          (10) /* shift/load '165. Keep high when reading '165 */
#define LED                   13
/*---------------------------------------------------------------------------*/
//uint8_t spinner[] = {'/', '-', '\', '|', '/', '-', '|'};
//uint8_t spinner[] = {0x3C, 0x2A, 0x3E, 0x2A};
#define VFD_USER_uint8_tS_LEN      40

/*---------------------------------------------------------------------------*/
static void     tx_byte(uint8_t b);
static void     tx_bytevector(uint8_t *bv, uint8_t len);
/*---------------------------------------------------------------------------*/
#define VFD_ALL_OFF_LEN     17    // # bytes
static  uint8_t all_off[] = {0xE0, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                                  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                                  0x20};
static uint8_t  current_dim = 0;
/* -------------------------------------------------------------------------- */
/* Init the display board */
void
vfd_init(void){
  /* Setup pins */
  pinMode(VFD_SCK_PIN, OUTPUT);
  pinMode(VFD_SDO_PIN, OUTPUT);
  pinMode(VFD_QH_PIN, INPUT);
  pinMode(VFD_CS_PIN, OUTPUT);
  pinMode(VFD_SHLD_PIN, OUTPUT);
  pinMode(LED, OUTPUT);

  digitalWrite(VFD_SCK_PIN, HIGH);
  digitalWrite(VFD_SDO_PIN, LOW);
  digitalWrite(VFD_CS_PIN, HIGH);
  digitalWrite(VFD_SHLD_PIN, LOW);
  digitalWrite(LED, LOW);

  /* set 16 uint8_ts length of display */
  tx_byte(0x00 | 0x07);
  /* Enable auto increment (should never be turned off) */
  tx_byte(0xF4 | 0x01);
  /* Set dimmer value + freq (128/fOSC) */
  tx_byte(0x08 | 0x07);
  tx_byte(0xF6 | 0x00);
  /* Transmit user defined uint8_ts */
/*  tx_bytevector(&vfd_user_defined_uint8_ts, VFD_USER_uint8_tS_LEN);*/
}

/* -------------------------------------------------------------------------- */
/* Read the buttons. */
uint8_t
vfd_read_btns(void){
  uint8_t read = 0;
  uint8_t i;
  uint8_t bit = 0x01;    // XXX MSB or LSB first?
  digitalWrite(VFD_SHLD_PIN, HIGH);
  for (i = 0; i < 8; i += 1) {
    digitalWrite(VFD_SCK_PIN, LOW);
    if(digitalRead(VFD_QH_PIN)) {
      read |= bit;
    }
    digitalWrite(VFD_SCK_PIN, HIGH);
    bit = bit << 1;
  }
  digitalWrite(VFD_SHLD_PIN, LOW);
  return read;
}

/* -------------------------------------------------------------------------- */
/* Print a single uint8_t on the display */
/*
  µ == can't find it on my keyboard :P
  This uint8_tt corresponds to the usual ASCII
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
vfd_print_uint8_t(uint8_t where, uint8_t chr){
  static uint8_t vect[2];
  if(where > 15) {
    // only positions between 0..15 accepted (M66004 adds 1).
    return;
  }
  vect[0] = 0xE0 | where;
  vect[1] = chr;    // XXX There is no input check on this uint8_t!
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
    tx_byte(0xF0 | 0x01);
  } else {
    tx_byte(0xF0 | 0x00);
  }
}
/* -------------------------------------------------------------------------- */
/* Print an entire string on the display, passed vector must be >= 16 bytes long */
void
vfd_set_string(uint8_t *str)
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
  tx_byte(0x08 | val);
}
/* -------------------------------------------------------------------------- */
/* Set the decimal dot on a uint8_tacter. */
void
vfd_set_ddot(uint8_t val, uint8_t doton)
{
  if(val == 0 || val > 16) {
    return;
  }

  if(doton) {
    tx_byte(0x10 | val);    // turn dot on
  } else {
    tx_byte(0x80 | val);    // turn dot off
  }
}
/*---------------------------------------------------------------------------*/
static void
tx_byte(uint8_t b)
{
  uint8_t i;
  uint8_t bit = 0x80;
  // start condition
  digitalWrite(VFD_CS_PIN, LOW);

  // MSB first, shift on SCK lo->hi transition
  for (i = 0; i < 8; i += 1) {
    digitalWrite(VFD_SCK_PIN, LOW);
    if(b & bit) {
      digitalWrite(VFD_SDO_PIN, HIGH);
    } else {
      digitalWrite(VFD_SDO_PIN, LOW);
    }
    delayMicroseconds(20);
    digitalWrite(VFD_SCK_PIN, HIGH);
    bit = bit >> 1;             // shift so we check the next one in order
  }

  delayMicroseconds(20);
  // end condition
  digitalWrite(VFD_CS_PIN, HIGH);
}
/* -------------------------------------------------------------------------- */
static void
tx_bytevector(uint8_t *bv, uint8_t len)
{
  uint8_t i, j, b;

  // start condition
  digitalWrite(VFD_CS_PIN, LOW);

  // transmit each byte
  for (j = 0; j < len; j += 1) {
    uint8_t bit = 0x80;
    /* load next byte to transmit */
    b = *(bv + j);

    /* MSB first, shift on SCK lo->hi transition */
    for (i = 0; i < 8; i += 1) {
      digitalWrite(VFD_SCK_PIN, LOW);

      if(b & bit) {
        digitalWrite(VFD_SDO_PIN, HIGH);
      } else {
        digitalWrite(VFD_SDO_PIN, LOW);
      }

      delayMicroseconds(20);
      digitalWrite(VFD_SCK_PIN, HIGH);
      bit = bit >> 1;             /* shift so we check the next one in order */
    }
    /* small delay here, tBUSY = 6 cycles at 500 kHz =~ 14 µs == 14 cycles at 1 MHz. */
    delayMicroseconds(20);
  }

  // end condition
  digitalWrite(VFD_CS_PIN, HIGH);
}
/* ------------------------------------------------------------------------




























/*---------------------------------------------------------------------------*/
void
setup(void)
{
  vfd_init();
  vfd_clear();
    // tx_byte(0x12);  // curson on on location 2
}
/*---------------------------------------------------------------------------*/
void
loop(void)
{
//  uint8_t btns;
//  btns = vfd_read_btns();
  vfd_print_uint8_t(2, 'A');
  vfd_print_uint8_t(3, 'd');
  vfd_print_uint8_t(4, 'q');
  vfd_print_uint8_t(5, 'U');
  delay(500);
  digitalWrite(LED, HIGH);
  delay(20);
  digitalWrite(LED, LOW);
}
/*---------------------------------------------------------------------------*/



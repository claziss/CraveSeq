/*
 Crave SEQ file library parser.
 Copyright (C) 2020  Claudiu Zissulescu

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef _CRAVE_FILE_H_
#define _CRAVE_FILE_H_

#include <stdbool.h>

typedef struct note
{
  unsigned char note;
  unsigned char octave;
  unsigned char ratchet;
  unsigned char velocity;
  unsigned char gate;
  bool glide;
  bool accent;
  bool rest;
} note_t;

typedef struct sequence
{
  unsigned char swing;
  unsigned char length;
  note_t *notes;
} sequence_t;

extern int readSequence (const char *name, sequence_t *sequence);

#endif

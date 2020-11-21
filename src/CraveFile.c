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

#include "CraveFile.h"
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

/*
 * Fixed header:
 * 00000000: 2398 5476 0000 000a 0043 0052 0041 0056  #.Tv.....C.R.A.V
 * 00000010: 0045 0000 000a 0031 002e 0031 002e 0031  .E.....1...1...1
 *
 * Sequence info:
 * 00000020: 0000 002e 0000 0000 0004 SSSS SSSS SSSS  ................
 *                  ^    ^    ^    ^  1st note info
 *                  |    |    +----+- Sequence length (0 for 1 note ...)
 *                  |    +----------- Swing info (50% + VALUE)
 *                  +---------------- Length in bytes (0x0e + SeqLength * 8)
 *
 * Note encoding:
 * 0300 0300 0400 0800
 *   ^    ^    ^    ^
 *   |    |    |    +- Efects (Glide:0x0100, Accent:0x0400, Rest:0x0800)
 *   |    |    +------ Velocity
 *   |    +----------- (MSB Gate Length [0-7])
 *   |    +----------- (LSB Ratchet [0-3])
 *   +---------------- Note (12 * (Octave - 1) + NoteNo)
 *
 * Fields encodings:
 * - Sequence Length = MSB * 8 + LSB + 1;
 * - Velocity = MSB * 16 + LSB;
 * - Swing = MSB * 16 + LSB;
 * - Note = MSB *16 + LSB;
 */

typedef struct infoNoteRaw
{
  unsigned char note[2];
  unsigned char gateLength;
  unsigned char ratchet;
  unsigned char velocity[2];
  unsigned char effects;
  unsigned char unk;
} infoNoteRaw_t;

typedef struct infoSeqRaw
{
  unsigned char unk0[2];
  unsigned char bytelength[2];
  unsigned char swing[2];
  unsigned char seqlength[4];
} infoSeqRaw_t;

typedef struct craveSeqRaw
{
  unsigned char header[32];
  infoSeqRaw_t info;
  infoNoteRaw_t notes[32];
} craveSeqRaw_t;

const unsigned char craveHeader[] =
   { 0x23, 0x98, 0x54, 0x76, 0x00, 0x00, 0x00, 0x0a,
     0x00, 0x43, 0x00, 0x52, 0x00, 0x41, 0x00, 0x56,
     0x00, 0x45, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x31,
     0x00, 0x2e, 0x00, 0x31, 0x00, 0x2e, 0x00, 0x31 };

int readSequence (const char *name, sequence_t *sequence)
{
  FILE *fd;
  unsigned fileSize;
  unsigned char *buffer;
  size_t sz;
  unsigned seqLength;

  fd = fopen (name, "r");
  if (fd == NULL)
    {
      printf ("Can't open the file: %s\n", strerror (errno));
      return 1;
    }

  fseek (fd, 0L, SEEK_END);
  fileSize = ftell (fd);
  rewind (fd);

  buffer = alloca (fileSize);
  sz = fread (buffer, fileSize, 1, fd);
  fclose (fd);

  if (sz != 1)
    {
      printf ("Not enough bytes\n");
      return 1;
    }

  craveSeqRaw_t *cseq = (craveSeqRaw_t *) buffer;

  for (int i = 0; i < 32; i ++)
    if (cseq->header[i] != craveHeader[i])
      {
	printf ("Unknown header\n");
	return 1;
      }

  seqLength = cseq->info.seqlength[1] * 8 + cseq->info.seqlength[3] + 1;

  sequence->swing = 50 + cseq->info.swing[0] * 0x10 + cseq->info.swing[1];
  sequence->length = seqLength;
  note_t *notes = malloc (sizeof (note_t) * seqLength);

  sequence->notes = notes;
  for (int i = 0; i < seqLength; i++)
    {
      unsigned noteval = cseq->notes[i].note[0] * 0x10 +
         cseq->notes[i].note[1];
      notes->octave = noteval / 12;
      notes->note =  (noteval - notes->octave * 12) % 12;
      notes->velocity = cseq->notes[i].velocity[0] * 0x10
	+ cseq->notes[i].velocity[1];
      notes->ratchet = cseq->notes[i].ratchet + 1;
      notes->glide = cseq->notes[i].effects & 0x01;
      notes->accent = cseq->notes[i].effects & 0x04;
      notes->rest = cseq->notes[i].effects & 0x08;
      notes->gate = cseq->notes[i].gateLength;
      notes++;
    }
  return 0;
}

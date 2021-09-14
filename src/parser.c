/*
 SEQ file library parser.
 Copyright (C) 2021  Claudiu Zissulescu

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

#include "sequence.h"
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

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

typedef struct td3SeqRaw
{
  unsigned char header[32];
  unsigned char fill[4];
  unsigned char notes[32];
  unsigned char accents[32];
  unsigned char slides[32];
  unsigned char fillzero1[2];
  unsigned char length[2];
  unsigned char fillzero2[2];
  unsigned char mask[4];
  unsigned char fillzero3[4];
} td3SeqRaw_t;

const unsigned char craveHeader[] =
   { 0x23, 0x98, 0x54, 0x76, 0x00, 0x00, 0x00, 0x0a,
     0x00, 0x43, 0x00, 0x52, 0x00, 0x41, 0x00, 0x56,
     0x00, 0x45, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x31,
     0x00, 0x2e, 0x00, 0x31, 0x00, 0x2e, 0x00, 0x31 };

static const unsigned char td3Header[] =
  { 0x23, 0x98, 0x54, 0x76, 0x00, 0x00, 0x00, 0x08,
    0x00, 0x54, 0x00, 0x44, 0x00, 0x2d, 0x00, 0x33,
    0x00, 0x00, 0x00, 0x0a, 0x00, 0x31, 0x00, 0x2e,
    0x00, 0x32, 0x00, 0x2e, 0x00, 0x36, 0x00, 0x00 };

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

int craveSequence (const char *name, sequence_t *sequence)
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

  /* Just ignore the Crave version number.  TODO: Parse it
     correctly.  */
  for (int i = 0; i < 18; i ++)
    if (cseq->header[i] != craveHeader[i])
      {
	printf ("Unknown header\n");
	return 1;
      }

  seqLength = cseq->info.seqlength[1] * 8 + cseq->info.seqlength[3] + 1;

  sequence->swing = 50 + cseq->info.swing[0] * 0x10 + cseq->info.swing[1];
  sequence->length = seqLength;
  note_t *notes = malloc (sizeof (note_t) * seqLength);
  memset (notes, 0, sizeof (note_t) * seqLength);

  sequence->notes = notes;
  for (int i = 0; i < seqLength; i++)
    {
      unsigned noteval = cseq->notes[i].note[0] * 0x10 +
         cseq->notes[i].note[1];
      notes->octave = noteval / 12 - 1;
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

/*
 * Fixed header:
 * 00000000: 2398 5476 0000 0008 0054 0044 002d 0033  #.Tv.....T.D.-.3
 *
 * Device Version:
 * 000000xx: 0000 0000 0000 (none)
 *            Null ^    ^----- Null terminator
 *                 +---------- Length version field (bytes)
 *
 * Length field:
 * 00000020: 0070 0000 <---- lenght field (including this one).
 *
 * Note fields:
 * 000000xx: NNN1 NNN2 NNN3 ..... NNNN16
 * - Note = MSB *16 + LSB
 *
 * Accent fields (16x16):
 * xxxxxxxx: 000A 000A 000A 000A 000A 000A 000A 000A 000A 000A 000A 000A 000A 000A 000A 000A
 *  - Accent (S): 1 - on, 0 - off
 *
 * Slide fields (16x16):
 * xxxxxxxx: 000S 000S 000S 000S 000S 000S 000S 000S 000S 000S 000S 000S 000S 000S 000S 000S
 *  - Slide (S): 1 - on, 0 - off
 *
 * 0000008x: 0000 0003 0000 0007 0000 0006 0000
 *                 ^          ^    ^   ^
 *                 |          |    |   Unk
 *                 |          +----+--- Mask enabled notes (1bit lsb, negated)
 *                 +--- Seq length
 */

int td3Sequence (const char *name, sequence_t *seq)
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

  td3SeqRaw_t *tdseq = (td3SeqRaw_t *) buffer;

  for (int i = 0; i < 16; i ++)
    if (tdseq->header[i] != td3Header[i])
      {
	printf ("Unknown header\n");
	return 1;
      }

  seq->swing = 50;
  seq->length = tdseq->length[0] * 0x10 + tdseq->length[1];
  note_t *notes = malloc (sizeof (note_t) * seq->length);
  memset (notes, 0, sizeof (note_t) * seq->length);
  seq->notes = notes;

  unsigned int mask = tdseq->mask[1] + (tdseq->mask[0] << 4)
    + (tdseq->mask[3] << 8) + (tdseq->mask[2] << 12);
  mask = ~mask;
  for (int i = 0; i < seq->length*2 ; i += 2)
    {
      unsigned noteval = tdseq->notes[i] * 0x10 +
	tdseq->notes[i+1];
      notes->octave = noteval / 12;
      notes->note =  (noteval - notes->octave * 12) % 12;
      notes->slide = tdseq->slides[i+1] & 0x01;
      notes->accent = tdseq->accents[i+1] & 0x01;
      notes->rest = mask & 0x01;
      mask >>= 1;
      notes++;
    }
  return 0;
}


int
dumpCraveSeq (const char *name, sequence_t *seq)
{
  FILE *fp;

  fp = fopen (name, "w");

  if (fp == NULL)
    {
      printf ("Can't open the file: %s\n", strerror (errno));
      return 1;
    }

  fwrite (craveHeader, sizeof (craveHeader), 1, fp);

  /* Write twice 0x00.  */
  fputc (0x00, fp);
  fputc (0x00, fp);

  /* Write length in bytes (2 bytes).  */
  unsigned byteslength = 0x0e + (seq->length - 1) * 8;
  fputc (byteslength >> 8, fp);
  fputc (byteslength & 0xff, fp);

  /* Write swing info (2 bytes).  */
  unsigned swing = seq->swing - 50;
  fputc (swing / 0x10, fp);
  fputc (swing % 0x10, fp);

  /* Write sequence length (4 bytes).  */
  unsigned seqlength = seq->length - 1;
  fputc (0x00, fp);
  fputc (seqlength / 8, fp);
  fputc (0x00, fp);
  fputc (seqlength % 8, fp);

  /* Write notes (&info) (8 bytes).  */
  note_t *notes = seq->notes;
  for (int i = 0; i < seq->length; i++)
    {
       //TD3 correction: TD3 Octave starts from 0/ Crave starts from -1
      unsigned noteval = notes->note + 12 * (notes->octave + 1);
      fputc (noteval / 0x10, fp);
      fputc (noteval % 0x10, fp);

      // Slide fully opens the gate.
      unsigned gate = notes->slide ? 0x07 : 0x03;
      fputc (gate, fp); //fputc (seq->notes->gate, fp);
      fputc (seq->notes->ratchet, fp);
      fputc (0x4, fp);  //fputc (seq->notes->velocity / 0x10, fp);
      fputc (0x0, fp);  //fputc (seq->notes->velocity % 0x10, fp);

      unsigned effects = notes->slide | (notes->accent << 2)
	| (notes->rest << 3);
      fputc (effects, fp);
      fputc (0x00, fp);
      notes ++;
    }


  fclose (fp);
  return 0;
}

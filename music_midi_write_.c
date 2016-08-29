/**************************************************************************************************************************

    These are functions for writing events to file.

    Each returns the number of bytes written:

	W_NOTE_ON    = FILE / DELTA / CHANNEL / NOTE / VELOCITY

	W_NOTE_OFF   = FILE / DELTA / CHANNEL / NOTE / VELOCITY

	W_TRACK_END  = FILE

	W_TRACK_HEAD = FILE (WRITES ZERO FOR TRACK LENGTH WHICH MUST BE FIXED LATER)

	W_FILE_HEAD  = FILE / SYNCH / TRACKS / PPQN

**************************************************************************************************************************/

/*
unsigned int w_note_on   (FILE, unsigned int, unsigned int, unsigned int, unsigned int);
unsigned int w_note_off  (FILE, unsigned int, unsigned int, unsigned int, unsigned int);
unsigned int w_track_end (FILE);
unsigned int w_track_head(FILE);
void         w_file_head (FILE, unsigned int, unsigned int, unsigned int);
*/

/*************************************************************************************************************************/

unsigned int w_note_on(FILE *fptr, unsigned int delta, unsigned int channel, unsigned int note, unsigned int velocity){

  unsigned int buffer,count;

  count=0;

  // delta time
  buffer=delta&0x7F;
  while((delta>>=7)){
    buffer<<=8;
    buffer|=((delta&0x7F)|0x80);
  }
  while(1){
    putc(buffer,fptr);
    count++;
    if(buffer&0x80)
      buffer>>=8;
    else
      break;
  }

  // event
  putc(9*16+channel,fptr);
  putc(note,fptr);
  putc(velocity,fptr);

  count+=3;

  return(count);
}

/*************************************************************************************************************************/

unsigned int w_note_off(FILE *fptr, unsigned int delta, unsigned int channel, unsigned int note, unsigned int velocity){

  unsigned int  buffer,count;

  count=0;

  // delta time
  buffer=delta&0x7F;
  while((delta>>=7)){
    buffer<<=8;
    buffer|=((delta&0x7F)|0x80);
  }
  while(1){
    putc(buffer,fptr);
    count++;
    if(buffer&0x80)
      buffer>>=8;
    else
      break;
  }

  // event
  putc(8*16+channel,fptr);
  putc(note,fptr);
  putc(velocity,fptr);

  count+=3;

  return(count);
}

/*************************************************************************************************************************/

unsigned int w_track_end(FILE *fptr){

  // the last event happens at the same time as the last event
  fputc(0x00,fptr);

  // this last event is the end of the track
  fputc(0xFF,fptr);
  fputc(0x2F,fptr);
  fputc(0x00,fptr);

  return(4);
}

/*************************************************************************************************************************/

unsigned int w_track_head(FILE *fptr){

  // this is a track header
  putc(0x4D,fptr);
  putc(0x54,fptr);
  putc(0x72,fptr);
  putc(0x6B,fptr);

  // length
  putc(0x00,fptr);
  putc(0x00,fptr);
  putc(0x00,fptr);
  putc(0x00,fptr);

  // the first event happens at delta t zero
  putc(0x00,fptr);

  // this first event is a tempo setting
  putc(0xFF,fptr);
  putc(0x51,fptr);
  putc(0x03,fptr);

  // here is the tempo info: the number of milliseconds in a quarter note
  putc(0x09,fptr);
  putc(0x27,fptr);
  putc(0xC0,fptr);

  // the second event happens at delta t zero
  putc(0x00,fptr);

  // this second event is a time signature
  putc(0xFF,fptr);
  putc(0x58,fptr);
  putc(0x04,fptr);

  // here is the time signature info
  putc(0x04,fptr);  // numerator
  putc(0x02,fptr);  // denominator (log base 2)
  putc(0x18,fptr);  // number of clocks in a metronome click (there are 24 clocks in a quarter note)
  putc(0x08,fptr);  // number of clocks in a 32nd note (8 makes sense, but you can fuck with space-time here)

  return(15);
}

/*************************************************************************************************************************/

void w_file_head(FILE *fptr, unsigned int synch, unsigned int tracks, unsigned int ppqn){

  // this is a file header
  putc(0x4D,fptr);
  putc(0x54,fptr);
  putc(0x68,fptr);
  putc(0x64,fptr);

  // this file header is 6 bytes long
  putc(0x00,fptr);
  putc(0x00,fptr);
  putc(0x00,fptr);
  putc(0x06,fptr);

  // this file is something
  putc(0x00,fptr);
  putc(synch,fptr);

  // this file has some tracks
  putc(0x00,fptr);
  putc(tracks,fptr);

  // this file has ppqn ticks in a quarter note ( 192 = 0xC0 )
  putc(0x00,fptr);
  putc(ppqn,fptr);

  return;
}

/*************************************************************************************************************************/




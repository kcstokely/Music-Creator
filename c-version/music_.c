/***

    This program generates a song and writes it to a midi file.
    

***/

#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include "music_write_.c"
#include "music_energy_.c"

#define  PPQN  48
#define  RAND  ((double)rand()/(double)RAND_MAX)

/**/

int main(int argc, char *argv[]){

  if(argc>0)
    printf("%s\n",argv[1]);
  
  // file

  FILE   *mfile;

  // song parameters

  unsigned int beatsPerMeasure = 4;
  unsigned int chordLength     = PPQN*beatsPerMeasure*3;
  unsigned int songLength      = PPQN*beatsPerMeasure*3*24;
  
  // bulk parameters

  unsigned int b_num       = 12;
  
  unsigned int b_nroot_min = 60-24;
  unsigned int b_nroot_max = 60+12;

  unsigned int b_vroot_min = 32;
  unsigned int b_vroot_max = 127;

  unsigned int b_gnum     = 2;
  double       b_gprb     = 0.01;

  unsigned int b_fnum     = 3;
  double       b_fprb     = 0.333;

  double       b_beta     = 1.25;
  double       b_wkey     = 1.0;
  double       b_wcrd     = 5.0;
  double       b_wetc     = 2.0;

  // sequence parameters
 
  unsigned int s_num       = 8;
  
  unsigned int s_gnum      = 1;
  double       s_gprb      = 0.02;

  unsigned int s_imax_min  = 2;
  unsigned int s_imax_max  = 6;

  unsigned int s_nrange    = 12;
  unsigned int s_nroot_min = 60-24;
  unsigned int s_nroot_max = 60+12;

  unsigned int s_vrange    = 0;
  unsigned int s_vroot_min = 64;
  unsigned int s_vroot_max = 127;

  unsigned int s_len_min   = 96*4*8;
  unsigned int s_len_max   = 96*4*8;

  double       s_tprb      = 0.18;

  double       s_beta      = 1.25;
  double       s_wkey      = 1.0;
  double       s_wcrd      = 1.0;

  // normalize energy parameters

  b_wkey/=(b_wkey+b_wcrd+b_wetc);
  b_wcrd/=(b_wkey+b_wcrd+b_wetc);
  b_wetc/=(b_wkey+b_wcrd+b_wetc);

  s_wkey/=(s_wkey+s_wcrd);
  s_wcrd/=(s_wkey+s_wcrd);

  // begin

  srand(time(0));

  // open file

  mfile = fopen("k.mid","w");
  
  unsigned int currentSeek = w_file_head(mfile,0,1,192);

  // chord structure
  
  unsigned int *chord = (unsigned int*) calloc(songLength/PPQN, sizeof(unsigned int));
  
  unsigned int c = 0;
  for(int t=0; t<songLength; t+=PPQN){
    chord[t]=c;
    if( RAND < 1.0/(double)chordLength )
      c=rand()%7;
  }
  
  // track one: bulk
  
    // variables
  
    unsigned int  t_now, t_prev;
  
    unsigned int  total       = 0;
    unsigned int  bulkChannel = 1;
    unsigned int *bulkValue   = calloc(b_num, sizeof(unsigned int));
    unsigned int *bulkOnOff   = calloc(b_num, sizeof(unsigned int));
  
    // track head
  
    total = 0;
    w_track_head(mfile);
  
    // set notes

    int n;
    for(int note=0; note<b_num; note++){
      while(1){
        n = b_nroot_min + rand()%(b_nroot_max-b_nroot_min+1);
        double x = en_key(n);
        double y = exp(-1.0*b_beta*x);
        if( RAND < y )
          break;
      }
      bulkValue[note] = n;
    }
  
    // set initial on/off
  
    unsigned int offset = 0;
    for(int note=0; note<b_num; note++){  
      bulkOnOff[note] = rand()%2;
      if(bulkOnOff[note]==1){
        int v = b_vroot_min + rand()%(b_vroot_max-b_vroot_min+1);
        total  += w_note_on(mfile,offset,bulkChannel,bulkValue[note],v);
        offset += PPQN/4;
      }
    }

    // loop time
  
    t_now = 0;
    while( t_now<songLength ){

      // flip on/off
      
      for(int note=0; note<b_gnum; note++){
        if( RAND < b_gprb ){ 
          c = rand()%b_num;
          bulkOnOff[c] = (bulkOnOff[c]+1)%2;
          if(bulkOnOff[c]==1){
            int v = b_vroot_min + rand()%(b_vroot_max-b_vroot_min+1);
            total += w_note_on(mfile,t_now-t_prev,bulkChannel,bulkValue[c],v);
            t_prev = t_now;
          }
          else{
            total += w_note_off(mfile,t_now-t_prev,bulkChannel,bulkValue[c],0);
            t_prev = t_now;
          }
        }
      }
      
      // flip values
      
      for(int note=0; note<b_fnum; note++){
        if( RAND < b_fprb ){
	      int c = rand()%b_num;
	      while(1){
            n = b_nroot_min + rand()%(b_nroot_max-b_nroot_min+1);
            double x = b_wkey * en_key(n) + b_wcrd * en_crd(n,chord[t_now/PPQN]) + b_wetc * en_etc(n,c,b_num,bulkValue,bulkOnOff);
            double y = exp(-1.0*b_beta*x);
            if( RAND < y )
              break;
          }
  	      if(bulkOnOff[c]==1){
	        total += w_note_off(mfile,t_now-t_prev,c,bulkValue[c],0);
	        t_prev = t_now;
	        int v  = b_vroot_min + rand()%(b_vroot_max-b_vroot_min+1);
	        total += w_note_on(mfile,0,bulkChannel,n,v);
          }
          bulkValue[c] = n;
        }
  	  }

      t_now += PPQN*(rand()%4);

    }
  
    // turn off
  
    for(int note=0; note<b_num; note++)
      if(bulkOnOff[note]==1)
        total += w_note_off(mfile,songLength-t_prev,bulkChannel,bulkValue[note],0);

    // free
  
    free(bulkValue);
    free(bulkOnOff);
           
    // track end

    total += w_track_end(mfile);
    w_track_fix(mfile,currentSeek+4,total);
               
    currentSeek += total;
   
  // track two: sequences
  
    // variables
           
    unsigned int *seqChannel = calloc(s_num, sizeof(unsigned int));
    unsigned int *seqOnOff   = calloc(s_num, sizeof(unsigned int));       
    unsigned int *seqValue   = calloc(s_num * s_imax_max, sizeof(unsigned int));
    unsigned int *seqSpeed   = calloc(s_num * s_imax_max, sizeof(unsigned int));
    unsigned int *seqTime    = calloc(s_num * s_imax_max, sizeof(unsigned int));
    unsigned int *seqIndex   = calloc(s_num, sizeof(unsigned int));       
    unsigned int *seqImax    = calloc(s_num, sizeof(unsigned int));
    unsigned int *seqNext    = calloc(s_num, sizeof(unsigned int));
    unsigned int *seqEnd     = calloc(s_num, sizeof(unsigned int)); 

    // track head
  
    total = 0;
    w_track_head(mfile);
  
    // time loop

    t_now = 0;
    while( t_now<songLength ){
      
      // print
      
      for(int c=0; c<s_num; c++){
        if(seqOnOff[c]==1){

          if( t_now==seqEnd[c] ){
            seqOnOff[c] = 0;
            total += w_note_off(mfile,t_now-t_prev,seqChannel[c],seqValue[c*s_imax_max+seqIndex[c]],0);	
	        t_prev=t_now;
          }
          else if( t_now==seqNext[c] ){
            total += w_note_off(mfile,t_now-t_prev,c,seqValue[c*s_imax_max+seqIndex[c]],0);
            t_prev=t_now;
	        seqIndex[c] = (seqIndex[c]+1)%seqImax[c];
	        total += w_note_on(mfile,0,c,seqValue[c*s_imax_max+seqIndex[c]],seqSpeed[c*s_imax_max+seqIndex[c]]);
	        seqNext[c] = t_now + seqTime[c*s_imax_max+seqIndex[c]];
          }
          
        }
      }
  
      // create  
      
      for(int note=0; note<s_gnum; note++){
        if( RAND<s_gprb ){
        
          c = rand()%s_num;
          if(seqOnOff[c]==0){
          
            seqOnOff[c] = 1;
            seqImax[c]  = s_imax_min + rand()%(s_imax_max-s_imax_min+1);
            seqEnd[c]   = s_len_min + rand()%(s_len_max-s_len_min+1);
            //seqEnd[c] = t_now + (PPQN*2)*(int)pow(2,rand()%4);
          
            unsigned int seqValRoot  = s_nroot_min + rand()%(s_nroot_max-s_nroot_min+1);
	        unsigned int seqSpdRoot  = s_vroot_min + rand()%(s_vroot_max-s_vroot_min+1);
          
	        unsigned int tmp=0;
            for(int s=0; s<seqImax[c]; s++){
              
              while(1){
                n = seqValRoot + rand()%(s_nrange+1);
                double x = s_wkey*en_key(n) + s_wcrd*en_crd(n,chord[t_now/PPQN]);
                double y = exp(-1.0*s_beta*x);
                if( RAND<y )
                  break;
              }
              seqValue[c*s_imax_max+s] = n;
	          seqSpeed[c*s_imax_max+s] = seqSpdRoot + rand()%(s_vrange+1);
              seqTime[c*s_imax_max+s]  = PPQN/4 * (int)pow(2,rand()%6);
	          if( seqTime[c*s_imax_max+s]>PPQN*4 && RAND<s_tprb ){
                seqTime[c*s_imax_max+s] *= 2;
                seqTime[c*s_imax_max+s] /= 3;
              }
	          tmp += seqTime[c*s_imax_max+s];
          
            }
	        seqTime[c*s_imax_max+seqImax[c]-1] += PPQN-tmp%PPQN;
            seqIndex[c] = 0;
            total += w_note_on(mfile,t_now-t_prev,c,seqValue[c*s_imax_max+seqIndex[c]],seqSpeed[c*s_imax_max+seqIndex[c]]);
            t_prev = t_now;
	        seqNext[c] = t_now + seqTime[c*s_imax_max+seqIndex[c]];
          
          }
        
        }
      }      

      t_now += PPQN*(rand()%6);
      //t_delta+=12*(int)pow(2,rand()%5);
      
    }

    // notes off
           
    for(int c=0; c<s_num ;c++){
      if(seqOnOff[c]==1){
        total += w_note_off(mfile,t_now-t_prev,c,seqValue[c*s_imax_max+seqIndex[c]],0);
        t_prev = t_now;
      }
    }
                      
    // free       
           
    free(seqChannel);
    free(seqOnOff);
    free(seqValue);
    free(seqSpeed);
    free(seqTime);
    free(seqIndex);
    free(seqImax);
    free(seqNext);
    free(seqEnd);
           
     // fix lengths

    total += w_track_end(mfile);
    w_track_fix(mfile,currentSeek+4,total);         

    currentSeek += total;
         
  // close
           
  free(chord);
  fclose(mfile);
  
  return 0;
}
   



                
                
                
                
                
                
                
                
                
                
                
                
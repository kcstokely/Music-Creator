/**************************************************************************************************************************

    This program generates a song and writes it to a midi file.

**************************************************************************************************************************/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "music_midi_write_.c"
#include "music_energy_.c"

#define	  PPQN 192

/*************************************************************************************************************************/


int main(int argc, char *argv[]){

  // files

  FILE   *pfile;
  FILE   *mfile[2];

  // general parameters

  unsigned int lcrd=96*4*4;     //in DELTAS!
  unsigned int length=96*4*64;

  // bulk parameters

  unsigned int b_channels=12;
  unsigned int b_range[16][4]={{60-24,77-24,56,127},
                               {60-12,77-12,56,127},
			       {60-12,77-12,56,127},
			       {60,77,56,127},
			       {60,77,56,127},
			       {60,77,56,127},
			       {60,77,56,127},
			       {60+12,77,56,127},
			       {60+12,77,56,127},
			       {60+24,77+24,56,127},
			       {60,77,56,127},
			       {60,77,56,127},
			       {60,77,56,127},
			       {60,77,56,127},
			       {60,77,56,127},
			       {60,77,56,127}};

  unsigned int b_gnum=2;
  double       b_gprb=0.01;

  unsigned int b_fnum=3;
  double       b_fprb=1.0/3.0;

  double       b_beta=1.25;
  double       b_wkey=1.0;
  double       b_wcrd=5.0;
  double       b_wetc=2.0;

  // sequence parameters

  unsigned int s_gnum=1;
  double       s_gprb=0.02;

  unsigned int s_channels=8;

  unsigned int s_num_min=2;
  unsigned int s_num_max=6;

  unsigned int s_nrange=12;
  unsigned int s_nroot_min=60-24;
  unsigned int s_nroot_max=60+12;

  unsigned int s_vrange=0;
  unsigned int s_vroot_min=64;
  unsigned int s_vroot_max=127;

  unsigned int s_len_min=96*4*8;
  unsigned int s_len_max=96*4*8;

  double       s_tprb=0.18;

  double       s_beta=1.25;
  double       s_wkey=1.0;
  double       s_wcrd=1.0;

  // variables

  unsigned int by[4],total[2];
  unsigned int crd,t_delta,t_now,t_prev;
  unsigned int ch,f,n,v,s,flag,tmp;
  unsigned int bulk[16],note[16];
  unsigned int seq[16],s_end[16],s_next[16],s_num[16],s_index[16],s_nroot,s_vroot,s_note[16][8],s_vel[16][8],s_time[16][8];
  double       x,y;

  // normalize parameters

  b_wkey/=(b_wkey+b_wcrd+b_wetc);
  b_wcrd/=(b_wkey+b_wcrd+b_wetc);
  b_wetc/=(b_wkey+b_wcrd+b_wetc);

  s_wkey/=(s_wkey+s_wcrd);
  s_wcrd/=(s_wkey+s_wcrd);

  // begin

  srand(time(0));

  // open file

  mfile[0]=fopen("k.b.mid","w");
  mfile[1]=fopen("k.s.mid","w");
  w_file_head(mfile[0],0,1,192);
  w_file_head(mfile[1],0,1,192);
  total[0]=w_track_head(mfile[0]);
  total[1]=w_track_head(mfile[1]);

  // set initial bulk notes

  for(ch=0;ch<b_channels;ch++){
    flag=0;
    while(flag==0){
      n=b_range[ch][0]+rand()%(b_range[ch][1]-b_range[ch][0]+1);
      y=exp(-1.0*b_beta*en_key(n));
      if(((double)rand()/(double)RAND_MAX)<y)
        flag=1;
    }
    note[ch]=n;
    bulk[ch]=rand()%2;
    if(bulk[ch]==1){
      v=b_range[ch][2]+rand()%(b_range[ch][3]-b_range[ch][2]+1);
      total[0]+=w_note_on(mfile[0],0,ch,note[ch],v);
    }
  }

  // time loop

  crd=0;
  t_prev=0;
  t_delta=0;

  for(t_now=0;t_now<length;t_now+=12){

    if(t_now-t_delta==0){

      // t_delta

      t_delta+=12*(int)pow(2,rand()%5);

      // flip chord

      if(((double)rand()/(double)RAND_MAX)<(1.0/(double)lcrd))
        crd=rand()%7;

      // flip on-off

      for(f=0;f<b_gnum;f++){
        ch=rand()%b_channels;
        x=(double)rand()/(double)RAND_MAX;
        if(x<b_gprb){
          bulk[ch]=(bulk[ch]+1)%2;
	  if(bulk[ch]==0){
	    total[0]+=w_note_off(mfile[0],t_now-t_prev,ch,note[ch],0);
	    t_prev=t_now;
 	  }
	  else{
	    v=b_range[ch][2]+rand()%(b_range[ch][3]-b_range[ch][2]+1); 
            total[0]+=w_note_on(mfile[0],t_now-t_prev,ch,note[ch],v);
	    t_prev=t_now;
	  }
        }
      }

      // flip notes

      for(f=0;f<b_fnum;f++){
        if(((double)rand()/(double)RAND_MAX)<b_fprb){
	  ch=rand()%b_channels;
	  flag=0;
	  while(flag==0){
            n=b_range[ch][0]+(rand()%(b_range[ch][1]-b_range[ch][0]+1));
            y=exp(-1.0*b_beta*(b_wkey*en_key(n)+b_wcrd*en_crd(n,crd)+b_wetc*en_etc(n,ch,b_channels,note,bulk)));
            if(((double)rand()/(double)RAND_MAX)<y)
              flag=1;
          }
	  if(bulk[ch]==1){
	    total[0]+=w_note_off(mfile[0],t_now-t_prev,ch,note[ch],0);
	    t_prev=t_now;
	    v=b_range[ch][2]+rand()%(b_range[ch][3]-b_range[ch][2]+1); 
	    total[0]+=w_note_on(mfile[0],0,ch,n,v);
  	  }
	  note[ch]=n;
        }
      }

    }

    // sequence: print

    for(ch=0;ch<s_channels;ch++){
      if(seq[ch]==1){
        if(t_now==s_end[ch]){
          seq[ch]=0;
          total[1]+=w_note_off(mfile[1],t_now-t_prev,ch,s_note[ch][s_index[ch]],0);	
	  t_prev=t_now;
        }
        else if(t_now==s_next[ch]){
          total[1]+=w_note_off(mfile[1],t_now-t_prev,ch,s_note[ch][s_index[ch]],0);
          t_prev=t_now;
	  s_index[ch]=(s_index[ch]+1)%s_num[ch];
	  total[1]+=w_note_on(mfile[1],0,ch,s_note[ch][s_index[ch]],s_vel[ch][s_index[ch]]);
	  s_next[ch]=t_now+s_time[ch][s_index[ch]];
        }
      }
    }

    // sequence: create

    for(f=0;f<s_gnum;f++){
      if(((double)rand()/(double)RAND_MAX)<s_gprb){
        ch=rand()%s_channels;
        if(seq[ch]==0){
          seq[ch]=1;
          s_end[ch]=s_len_min+rand()%(s_len_max-s_len_min+1);
          //s_end[ch]=t_now+(96*4)*(int)pow(2,rand()%4);
          s_num[ch]=s_num_min+rand()%(s_num_max-s_num_min+1);
          s_nroot=s_nroot_min+rand()%(s_nroot_max-s_nroot_min+1);
	  s_vroot=s_vroot_min+rand()%(s_vroot_max-s_vroot_min+1);
	  tmp=0;
          for(s=0;s<s_num[ch];s++){
            flag=0;
            while(flag==0){
              n=s_nroot+rand()%(s_nrange+1);
              x=(double)rand()/(double)RAND_MAX;
              y=exp(-1.0*s_beta*(s_wkey*en_key(n)+s_wcrd*en_crd(n,crd)));
              if(x<y)
                flag=1;
            }
            s_note[ch][s]=n;
	    s_vel[ch][s]=s_vroot+rand()%(s_vrange+1);
            s_time[ch][s]=24*(int)pow(2,rand()%5);
	    if(s_time[ch][s]>24 && ((double)rand()/(double)RAND_MAX)<s_tprb)
              s_time[ch][s]=(s_time[ch][s]*2)/3;
	    tmp+=s_time[ch][s];
          }
	  s_time[ch][s_num[ch]-1]=96*2*(int)ceil((double)tmp/(double)(96*2))-tmp;
          s_index[ch]=0;	    
          total[1]+=w_note_on(mfile[1],t_now-t_prev,ch,s_note[ch][s_index[ch]],s_vel[ch][s]);
          t_prev=t_now;
	  s_next[ch]=t_now+s_time[ch][s_index[ch]];
        }
      }
    }

  }

  // notes off

  for(ch=0;ch<s_channels;ch++)
    if(seq[ch]==1)
      total[1]+=w_note_off(mfile[1],0,ch,s_note[ch][s_index[ch]],0);

  total[0]+=w_note_off(mfile[0],96*4,0,note[0],0);
  for(ch=1;ch<b_channels;ch++)
    if(bulk[ch]==1)
      total[0]+=w_note_off(mfile[0],96,ch,note[ch],0);

  // fix lengths

  for(f=0;f<2;f++){
    total[f]+=w_track_end(mfile[f]);
    by[0]=total[f]/(int)pow(2,24);
    by[1]=(total[f]%(int)pow(2,24))/(int)pow(2,16);
    by[2]=(total[f]%(int)pow(2,16))/(int)pow(2,8);
    by[3]=total[f]%(int)pow(2,8);
    fseek(mfile[f],14+4,SEEK_SET);
    fputc(by[0],mfile[f]);
    fputc(by[1],mfile[f]);
    fputc(by[2],mfile[f]);
    fputc(by[3],mfile[f]);
  }

  // combine

  pfile=fopen("k.mid","w");
  w_file_head(pfile,1,2,192);
  for(f=0;f<2;f++){
    fseek(mfile[f],14,SEEK_SET);
    while(1){
      n=fgetc(mfile[f]);
      if(n!=EOF)
        fputc(n,pfile);
      else
        break;
    }
  }
  fclose(pfile);

  // close em

  fclose(mfile[0]);
  fclose(mfile[1]);

  //

  printf("\n  Writing %d bytes to k.mid\n\n",14+8+8+total[0]+total[1]);

  return;
}












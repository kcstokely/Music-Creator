#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define PPQN 192

#define SEQ_LENGTH 16*4*3 // 16*4*3 = 192
#define RUN_LENGTH 1      // IN SEQS

#include "music_midi_write_.c"

double en_key(unsigned int a);
double en_time(unsigned int a, unsigned int b);

main(){

  FILE *mfile;

  unsigned int n_min = 60+24;
  unsigned int n_max = 60-24;
  unsigned int n_num = 6;

  double n_beta = 0.8;
  double t_beta = 0.8;
  double p_trip = 0.2;

  unsigned int i,j,k,flg,tmp,count,by[4],t,t_last;
  unsigned int note[12][2],matrix[12][SEQ_LENGTH],clone[12][2];
 
  /***** GENERATION ************************************************************/

  /*
     for each channel, pick how long it's sequence will be
     each allowable value must be a factor of SEQ_LENGTH
     currently 1,2,4,8,16 quarters or 3,6,12 eigths are allowed
     which means SEQ_LENGTH = 192 works
  */

  for(i=0;i<n_num;i++){
    if((double)rand()/(double)RAND_MAX>p_trip)
      note[i][1]=4*pow(2,rand()%5);
    else
      note[i][1]=6*pow(2,rand()%3);
  }

  /*
     for each channel, pick it's note
  */

  for(i=0;i<n_num;i++){
    flg=0;
    while(flg==0){
      tmp=n_min+rand()%(n_max-n_min+1);
      if((double)rand()/(double)RAND_MAX<exp(-1.0*n_beta*en_key(tmp))){
        note[i][0]=tmp;
	flg=1;
      }
    }
  }

  /*
     order the notes from low to high
  */

  flg=0;
  while(flg==0){
    flg=1;
    for(i=0;i<n_num-1;i++){
      if(note[i+1][0]>note[i][0]){
        tmp=note[i][0];
	note[i][0]=note[i+1][0];
	note[i+1][0]=tmp;
	flg=0;
      }
    }
  }
 
  /*
     for each channel, at each sixteenth
     choose how long the note plays for
     make sure the note does not play longer than one sixteenth
  */

  for(i=0;i<n_num;i++){
    for(j=0;j<note[i][1];j++){
      if((double)rand()/(double)RAND_MAX<exp(-1.0*t_beta*en_time(j,16)))
        matrix[i][j]=rand()%(PPQN/4);
      else
        matrix[i][j]=0;
    }
  }

  /*
     for sequences longer then SEQ_LENGTH
     copy them however many times they need be copied
  */

  for(i=0;i<n_num;i++)
    for(j=0;j<SEQ_LENGTH/note[i][1]-1;j++)
      for(k=0;k<note[i][1];k++)
        matrix[i][k+(j+1)*note[i][1]]=matrix[i][k];


  /***** GENERATION ************************************************************/

  /***** FILE WRITING **********************************************************/

  //open
  mfile=fopen("m_rh.mid","w");
  w_file_head(mfile,0,1,PPQN);

  //begin track
  count=w_track_head(mfile);

  // time loop

  if(PPQN%4!=0){
    printf("Shit done be fucked up, yo.\n");
    exit;
  }
  t_last=0;
  for(t=0;t<(PPQN/4)*SEQ_LENGTH*RUN_LENGTH;t+=PPQN/4){

    // later do etc = t*4/ppqn, and get rid of the 48's

    //turn em on
    for(i=0;i<n_num;i++)
      if(matrix[i][t/48]!=0){
        count+=w_note_on(mfile,t-t_last,i,note[i][0],127);
        t_last=t;
      }

    //sort that shit
    for(i=0;i<n_num;i++){
      clone[i][0]=i;
      clone[i][1]=matrix[i][t/48];
    }
    flg=0;
    while(flg==0){
      flg=1;
      for(i=0;i<n_num-1;i++){
        if(clone[i+1][1]>clone[i][1]){
	  clone[i][1]=matrix[clone[i+1][0]][t/48];
	  clone[i+1][1]=matrix[clone[i][0]][t/48];
          tmp=clone[i][0];
	  clone[i][0]=clone[i+1][0];
	  clone[i+1][0]=tmp;
          flg=0;
	}
      }
    }

    //turn em off
    for(i=0;i<n_num;i++){
      count+=w_note_off(mfile,t+clone[i][1]-t_last,clone[i][0],note[clone[i][0]][0],127);
      t_last=t+clone[i][1];
    }

  }

  //end track
  count+=w_track_end(mfile);

  //fix track length
  by[0]=count/(int)pow(2,24);
  by[1]=(count%(int)pow(2,24))/(int)pow(2,16);
  by[2]=(count%(int)pow(2,16))/(int)pow(2,8);
  by[3]=count%(int)pow(2,8);
  fseek(mfile,14+4,SEEK_SET);
  putc(by[0],mfile);
  putc(by[1],mfile);
  putc(by[2],mfile);
  putc(by[3],mfile);

  //close
  fclose(mfile);
  printf("Writing %d bytes to m_rh.mid\n",14+8+count);

  /***** FILE WRITING **********************************************************/

}








/***** ENERGY FUNCTIONS ********************************************************/

double en_time(unsigned int a, unsigned int b){

  unsigned int c,d;

  c=b;
  d=1;
  while((a+b)%c!=0){
    c/=2;
    d++;
  }

  return(d);
}

double en_key(unsigned int a){

  double e[12];

  e[0]=1.0;
  e[1]=15.0;
  e[2]=8.0;
  e[3]=5.0;
  e[4]=4.0;
  e[5]=3.0;
  e[6]=45.0;
  e[7]=2.0;
  e[8]=5.0;
  e[9]=3.0;
  e[10]=5.0;
  e[11]=8.0;

  return(e[abs(a-60)%12]);
}














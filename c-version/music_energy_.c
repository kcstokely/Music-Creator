/***

    These are functions which calculate the energy of a note.

    Each returns a double:

	EN_KEY	= NOTE

	EN_CRD	= NOTE / CHORD

	EN_ETC	= NOTE / CHANNEL / NO. CHANNLELS / N / O

***/

double       en_key(unsigned int a);
double       en_crd(unsigned int a, unsigned int crd);
double       en_etc(unsigned int a, unsigned int chan, unsigned int chans, unsigned int n[], unsigned int o[]);

/**/

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

  a=abs(a-60)%12;

  return(e[a]);
}

/**/

double en_crd(unsigned int a, unsigned int crd){

  unsigned int c;
  double d,e[12];

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

  d=0;

  if(crd==0){
    d+=e[abs(a-60)%12];
    d+=e[abs(a-64)%12];
    d+=e[abs(a-67)%12];
  }
  else if(crd==1){
    d+=e[abs(a-62)%12];
    d+=e[abs(a-65)%12];
    d+=e[abs(a-69)%12];
  }
  else if(crd==2){
    d+=e[abs(a-64)%12];
    d+=e[abs(a-67)%12];
    d+=e[abs(a-71)%12];
  }
  else if(crd==3){
    d+=e[abs(a-65)%12];
    d+=e[abs(a-69)%12];
    d+=e[abs(a-60)%12];
  }
  else if(crd==4){
    d+=e[abs(a-67)%12];
    d+=e[abs(a-71)%12];
    d+=e[abs(a-62)%12];
  }
  else if(crd==5){
    d+=e[abs(a-69)%12];
    d+=e[abs(a-60)%12];
    d+=e[abs(a-64)%12];
  }
  else if(crd==6){
    d+=e[abs(a-71)%12];
    d+=e[abs(a-62)%12];
    d+=e[abs(a-65)%12];
  }

  d/=3;

  return(d);
}

/*************************************************************************************************************************/

double en_etc(unsigned int a, unsigned int c, unsigned int num, unsigned int n[], unsigned int o[]){
  
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

  double dn = 0.0;
  double dv = 0.0;
  
  for(int d=0; d<num; d++)
    if( d!=c && o[c]==1 ){
      dn += 1.0;
      dv += e[abs(n[c]-a)%12];
    }
  
  if(dn>0.0)
    dv/=dn;

  return(dv);
}

/*************************************************************************************************************************/



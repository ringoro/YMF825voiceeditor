/*
 *  VT100 Screen Voice Editor
 *        for YMF825 
 * 
 *    2017/9/16
 *         9/20
 *    Ringoro
 */

#include <avr/pgmspace.h>
#include <EEPROM.h>

struct ToneParams { 
  unsigned char boctv;
  unsigned char lfoalg;
  unsigned char op[4][7];
};

// defalut sample tone from github 
//   https://github.com/yamaha-webmusic/ymf825board

struct ToneParams toneparams ={
    0x01,0x85,
    0x00,0x7F,0xF4,0xBB,0x00,0x10,0x40,
    0x00,0xAF,0xA0,0x0E,0x03,0x10,0x40,
    0x00,0x2F,0xF3,0x9B,0x00,0x20,0x41,
    0x00,0xAF,0xA0,0x0E,0x01,0x10,0x40,
};   

#define BAUD 9600

char tname[16]="Defalut Tone";

// struct ToneParams toneparams;

int op=1;
int tnum=0;   /* Tone number 1..10 */

#define MAXTONE 16

// Parameter Matrix Position

int paramx=1;
int paramy=1;

// Global Cursor X,Y

int  cx;
int  cy;

//  Parameter Matrix  Home (2,9);

#define homex   9
#define homey   2

#define maxx   28
#define maxy   22

/*
 *        
       OP1   OP2   OP3   OP4
 SR:    0     0     0     0  
XOF:    0     0     0     0
KSR:    0     0     0     0
 RR:    7    10     2    10
 DR:   15    15    15    15
 AR:   15    10    15    10
 SL:    4     0     3     0
 TL:   46     3    38     3
KSL:    3     2     3     2
DAM:    0     0     0     0
EAM:    0     0     0     0
DVB:    0     1     0     0
EVB:    0     1     0     1
MLT:    1     1     2     1
 DT:    0     0     0     0
 WS:    8     8     8     8
 FB:    0     0     0     0
LFO:    2    ALG:   5  
OCT:    1    [Defalut Tone]

 */

// Patameter Matrix Y pos.

#define  SR 1  
#define XOF 2
#define KSR 3
#define  RR 4
#define  DR 5
#define  AR 6
#define  SL 7
#define  TL 8
#define KSL 9
#define DAM 10
#define EAM 11
#define DVB 12
#define EVB 13
#define MLT 14
#define  DT 15
#define  WS 16
#define  FB 17
#define  LFO 18
#define  ALG  18  
#define OCTV 19
#define TNAME 19


#define CR 0x0d
#define LF 0x0d
#define TAB 0x09
#define BS 0x08
#define ESC  0x1B
#define SPC  ' '

/*
 *   SetUp
 * 
 */
void setup() {
  Serial.begin(BAUD);
  ymf825setup();
  Serial.print(F("Hit CR"));
  
}
/*
 * Main Loop
 */
void loop() {
  char c;
  if (Serial.available() > 0) {
    c = Serial.read();
    if(c == ESC){
      escsqc();
      } else {
        keyinput(c);          
        }    
   }
}

void keyinput(char c)
{
  switch(c){

     case '+':
      valuechange(1);
      break;
    case '-':
      valuechange(-1);
      break;
    case CR:
      printparam();
      curspos(paramx,paramy);
      break;
    case 'p':
    case 'P':
      tonetest();
      curspos(paramx,paramy);
      break;
    case 'S':
    case 's':
      tonesave();
      curspos(paramx,paramy);
      break;
    case 'L':
    case 'l':
      toneload();
      curspos(paramx,paramy);
      break;
    case 'D':
    case 'd':
      printparam();
      dumpparams();
      curspos(paramx,paramy);
      break; 
    case 'i':
    case 'I':
      import();
      cls();
      printparam();
      curspos(paramx,paramy);
      break;
  }
}

extern unsigned char tone_data[35];

void tonetest(void)
{
  int i,j,k=3;

  tone_data[1]=toneparams.boctv;
  tone_data[2]=toneparams.lfoalg;
  for(i=0;i<4;i++){
    for(j=0;j<7;j++){
      tone_data[k++]=toneparams.op[i][j];
      }
  }


   Serial.print(F("  Now Playing..."));
   set_tone();
   testplay();
   Serial.read();
   cls();
   printparam();
   
}

void cls(void)
{
     Serial.print(F("\e[2J"));
}


/* 
 * Each Parameter Value INC/DEC
 * 
 */

void valuechange(int x)
{
  op = paramx;
  switch(paramy){
    case SR : setsr(x);
             break;
    case XOF : setxof(x);
             break;
    case KSR : setksr(x);
             break;
    case RR : setrr(x);
             break;
    case DR : setdr(x);
             break;
    case AR : setar(x);
             break;
    case SL : setsl(x);
             break;
    case TL : settl(x);
             break;
   case KSL : setksl(x);
          break;
 
  case DAM : setdam(x);
          break;
  case EAM : seteam(x);
          break;
  case DVB : setdvb(x);
          break;
  case EVB : setevb(x);
          break;
  case MLT : setmulti(x);
          break;
  case DT : setdt(x);
          break;
  case WS : setws(x);
          break;
  case FB : setfb(x);
          break;
  }
  if(paramy==LFO &&  op==1){
    setlfo(x);
  }
  if(paramy==LFO &&  op==3){
    setalg(x);
    printalgs();
  }
  if(paramy==OCTV &&  op==1){
    setoct(x);
  }
  if(paramy==OCTV &&  op==2){
    setname(x);
  }
  
  curspos(paramx,paramy);
}




// ESC Sequence Cursor To Go
 
void escsqc(void){
  char c;
//  Serial.print("esc");
  while(!(Serial.available())) {}
   c = Serial.read();
    if(c == '['){
       while(!(Serial.available())){}
      c = Serial.read();
      cursgo(c); 
    } else {  ; }
}

// Select Parameter Matrix

void cursgo(char c)
{

  char s[16];
   switch(c){
    case 'A': paramy--;
       break; 
    case 'B': paramy++;
      break;      
    case 'C': paramx++;
      break;
    case 'D': paramx--;
      break;
   }

   if(paramy > 19) paramy=1;
   if(paramy < 1) paramy=19;
   if(paramx < 1) paramx=4;
   if(paramx > 4) paramx=1;
   
    curspos(paramx,paramy);
   

//  Serial.print(s);

}

// Parameter Matrix to Cursor X,Y

void curspos(int px,int py)
{
  cy = py + 1; 
  cx = (px-1) * 6 +9;
  cursgoto(cx,cy);
}

void cursgoto(int x , int y)
{
  char s[16];
  sprintf(s,"\e[%d;%dH",y,x);
  Serial.print(s);
}


/*
 * Set Parameters ...
 */

// LFO

int setlfo(char d)
{
  char s[40];
  int v;
  unsigned char lfoalg;
  lfoalg = toneparams.lfoalg & 0b00000111;
     v  = (toneparams.lfoalg & 0b11000000)>>6;
  v = v + d;
  lfoalg = lfoalg | (v << 6);
  toneparams.lfoalg = lfoalg;

  sprintf(s,"LFO: %4d    ALG:%4d ",lfoalg>>6,lfoalg&0x7);

  cursgoto(1,LFO+1);
  Serial.println(s); 
  return 0;
}

// ALG 

int setalg(int d)
{
  char s[40];
  int v;
  unsigned char lfoalg;
  lfoalg = toneparams.lfoalg & 0xC0;
    v    = toneparams.lfoalg & 0x07;
  v = (v + d) & 0x07;
  lfoalg = lfoalg | v ;
  toneparams.lfoalg = lfoalg;
  sprintf(s,"LFO: %4d    ALG:%4d ",lfoalg>>6,lfoalg&0x7);
  cursgoto(1,LFO+1);
  Serial.println(s); 

  return 0;
}

// Set Tone Name

int setname(int d)
{
  char c;
  char s[40];
  int i=0,n;

  Serial.print("\e[7m");
  while(c != CR && c != ESC){
    if(Serial.available()){
      c = Serial.read();
      Serial.print(c);
      s[i++]=c; i=i & 31;
    }
  }  
  if(c==CR){
    for(n=0;n<i-1;n++)
       {  tname[n]=s[n]; }
    tname[n]=0;
  }
  
  Serial.print("\e[0m\e[0K");
  sprintf(s,"OCT: %4d    [%s]",toneparams.boctv,tname);
  cursgoto(1,OCTV+1);
  Serial.println(s);

  return 0;
}

// OCT Basic Octave

int setoct(int d )
{
   char s[40];
  unsigned char oct;
  oct = toneparams.boctv & 3;
  oct = (oct + d) & 3;
  toneparams.boctv = oct;
  sprintf(s,"OCT: %4d    [%s]",toneparams.boctv,tname);
  cursgoto(1,OCTV+1);
  Serial.println(s);

  return 0;
}

// SR 

int setsr(int d)
{
  int v; 
  char s[40];
  unsigned char sr,srv;
  sr = toneparams.op[op-1][0] & 0x0F; 
  srv = toneparams.op[op-1][0] >> 4;
  srv = ((srv + d) & 0x0F) << 4;  
  sr = srv | sr;
  toneparams.op[op-1][0] =sr ;
  sprintf(s," SR: %4d  %4d  %4d  %4d",
  toneparams.op[0][0] >> 4,toneparams.op[1][0] >> 4,toneparams.op[2][0] >> 4,toneparams.op[3][0] >> 4);
  cursgoto(1,SR+1);
  Serial.println(s);

  return 0;
}

// XOF

int setxof(int d)
{
  char s[40];
  unsigned char xf,xfv;
  xf  = toneparams.op[op-1][0] & 0b11110001;
  xfv = (toneparams.op[op-1][0] & 0b00001000) >> 3;
  xfv = (xfv + d) & 1 ;
  xf = xf | ( xfv  << 3 );
  toneparams.op[op-1][0] =xf ;
  sprintf(s,"XOF: %4d  %4d  %4d  %4d",
  toneparams.op[0][0] >> 3&1,toneparams.op[1][0] >> 3 &1,toneparams.op[2][0] >> 3 &1 ,toneparams.op[3][0] >> 3&1);
  cursgoto(1,XOF+1);
  Serial.println(s);
  
  return 0;
}

//  KSR

int setksr(int d )
{
  char s[40];
  unsigned char ksr,ksrv;
  ksr =  toneparams.op[op-1][0] & 0b11111000;
  ksrv = toneparams.op[op-1][0] & 0b00000001;
  ksrv = (ksrv + d) & 1;
   ksr = ksr |  ksrv ;
  toneparams.op[op-1][0] =ksr;
  sprintf(s,"KSR: %4d  %4d  %4d  %4d",
  toneparams.op[0][0] &1,toneparams.op[1][0] &1,toneparams.op[2][0] &1 ,toneparams.op[3][0] &1);
  cursgoto(1,KSR+1);
  Serial.println(s);

  return 0;
}

// RR 

int setrr(int d)
{
  char s[40];
  unsigned char rr,rrv;
  rr  =  toneparams.op[op-1][1] & 0b00001111;
  rrv =  toneparams.op[op-1][1]  >> 4;
  rrv = rrv + d;
  rrv = (rrv & 0x0F) << 4;
   rr = rr |  rrv  ;
  toneparams.op[op-1][1] =rr;
  sprintf(s," RR: %4d  %4d  %4d  %4d",
  toneparams.op[0][1] >> 4,toneparams.op[1][1] >>4,toneparams.op[2][1] >>4 ,toneparams.op[3][1] >> 4);
  cursgoto(1,RR+1);
  Serial.println(s);
  return 0;
}

//   DR

int setdr(int d)
{
  char s[40];
  unsigned char drv,dr;

  dr = toneparams.op[op-1][1] & 0b11110000;
  drv = toneparams.op[op-1][1] & 0b00001111;
  
   drv = (drv + d) & 0x0F;
   dr = dr |  drv  ;
  toneparams.op[op-1][1] =dr;
  sprintf(s," DR: %4d  %4d  %4d  %4d",
  toneparams.op[0][1] & 0x0f,toneparams.op[1][1] &0x0F,toneparams.op[2][1] &0x0F ,toneparams.op[3][1]&0x0F);
  cursgoto(1,DR+1);
  Serial.println(s);

  return 0;
}

// AR

int setar(int d)
{
  char s[40];
  unsigned char ar,arv;
  ar  =  toneparams.op[op-1][2] & 0b00001111;
  arv = (toneparams.op[op-1][2] & 0b11110000) >> 4;
  arv =( (arv + d) & 0x0F) << 4;
  ar = ar |  arv  ;
  toneparams.op[op-1][2] =ar;
  sprintf(s," AR: %4d  %4d  %4d  %4d",
  toneparams.op[0][2] >> 4,toneparams.op[1][2] >>4,toneparams.op[2][2] >>4 ,toneparams.op[3][2] >> 4);
  cursgoto(1,AR+1);
  Serial.println(s);
  return 0;
}

// SL

int setsl(int d)
{
  char s[40];
  unsigned char sl,slv;
   sl  = toneparams.op[op-1][2] & 0b11110000;
   slv = toneparams.op[op-1][2] & 0b00001111;
   slv = (slv + d) & 0x0F;
   sl = sl |  slv;
  toneparams.op[op-1][2] =sl;
  sprintf(s," SL: %4d  %4d  %4d  %4d",
  toneparams.op[0][2] & 0x0f,toneparams.op[1][2] &0x0F,toneparams.op[2][2] &0x0F ,toneparams.op[3][2]&0x0F);
  cursgoto(1,SL+1);
  Serial.println(s);
  return 0;
}

// TTL  Total Level

int settl(int d)
{
  char s[40];
  unsigned char tl,tlv;
  tl  =  toneparams.op[op-1][3] & 0b00000011;
  tlv = (toneparams.op[op-1][3] & 0b11111100)>>2;
  tlv = (tlv + d ) & 0b00111111;
  tlv = tlv << 2;
  tl = tl |  tlv  ;
  toneparams.op[op-1][3] =tl;
  sprintf(s," TL: %4d  %4d  %4d  %4d",
  toneparams.op[0][3] >> 2,toneparams.op[1][3] >>2,toneparams.op[2][3] >>2 ,toneparams.op[3][3] >> 2);
  cursgoto(1,TL+1);
  Serial.println(s);

  return 0;
}

//  KSL

int setksl(int d)
{
  char s[40];
  unsigned char ksl,kslv;
  ksl  = toneparams.op[op-1][3] & 0b11111100;
  kslv = toneparams.op[op-1][3] & 0b00000011;
  
   kslv = (kslv + d)  & 0x3;
   ksl = ksl | kslv ;
  toneparams.op[op-1][3] =ksl;
   sprintf(s,"KSL: %4d  %4d  %4d  %4d",
  toneparams.op[0][3] & 0x03,toneparams.op[1][3] &0x03,toneparams.op[2][3] &0x03 ,toneparams.op[3][3]&0x03);
  cursgoto(1,KSL+1);
  Serial.println(s);
  return 0;
}

//    DAM

int setdam(int d)
{
  char s[40];
  unsigned char rr,v;
   rr = toneparams.op[op-1][4] & 0b00011111;
   v = (toneparams.op[op-1][4] & 0b01100000) >> 5;
   v =( v + d) & 0x03;
   rr = rr |  (v << 5) ;
  toneparams.op[op-1][4] =rr;
  sprintf(s,"DAM: %4d  %4d  %4d  %4d",
  toneparams.op[0][4] >>5,toneparams.op[1][4] >>5,toneparams.op[2][4] >>5 ,toneparams.op[3][4]>>5);
  cursgoto(1,DAM+1);
  Serial.println(s);

  return 0;
}

// EAM 

int seteam(int d)
{
  char s[40];
  unsigned char v,rr;
  rr = toneparams.op[op-1][4] &  0b01100111;
   v = (toneparams.op[op-1][4] & 0b00010000)>>4;
   v = (v + d) & 1;
   rr = rr | ( v << 4) ;
  toneparams.op[op-1][4] =rr; 
  sprintf(s,"EAM: %4d  %4d  %4d  %4d",
  (toneparams.op[0][4] & 0x10) >> 4,(toneparams.op[1][4]&0x10) >>4 ,(toneparams.op[2][4]&0x10)>>4 ,(toneparams.op[3][4]&0x10) >>4);
  cursgoto(1,EAM+1);
  Serial.println(s);
  return 0;
}


//  DVB

int setdvb(int d)
{
  char s[40];
  unsigned char v,rr;
  rr =  toneparams.op[op-1][4] & 0b11110001;
  v  = (toneparams.op[op-1][4] & 0b00000110)>>1;
   v = (v +d) & 0x3;
   rr = rr | ( v << 1 );
  toneparams.op[op-1][4] =rr;
  sprintf(s,"DVB: %4d  %4d  %4d  %4d",
  (toneparams.op[0][4] & 0x06) >> 1,(toneparams.op[1][4]&0x06) >>1 ,(toneparams.op[2][4]&0x06)>>1 ,(toneparams.op[3][4]&0x06) >>1);
  cursgoto(1,DVB+1);
  Serial.println(s);

  return 0;
}

//  EVB

int setevb(int d)
{
  char s[40];
  int v; 
  unsigned char rr;
  rr = toneparams.op[op-1][4] & 0b11111110;
  v  = toneparams.op[op-1][4] & 0b00000001;
  v = (v + d) & 1;
  rr = rr |  v;
  toneparams.op[op-1][4] =rr;
  sprintf(s,"EVB: %4d  %4d  %4d  %4d",
  toneparams.op[0][4] & 0x01 ,toneparams.op[1][4]&0x01 ,toneparams.op[2][4]&0x01 ,toneparams.op[3][4]&0x01);
  cursgoto(1,EVB+1);
  Serial.println(s);
  return 0;
}

// MULTI

int setmulti(int d)
{
  char s[40];
  int v;
  unsigned char rr;
  rr =  toneparams.op[op-1][5] & 0b00000111;
  v  = (toneparams.op[op-1][5] & 0xF0) >> 4;
  v =  ((v + d) & 0x0F) << 4;
  rr = rr |  v ;
  toneparams.op[op-1][5] =rr;
  sprintf(s,"MLT: %4d  %4d  %4d  %4d",
  toneparams.op[0][5] >>4 ,toneparams.op[1][5]>>4 ,toneparams.op[2][5]>>4 ,toneparams.op[3][5]>>4);
  cursgoto(1,MLT+1);
  Serial.println(s);
  return 0;
}

// DT 

int setdt(int d)
{
  char s[40];
  int v;
  unsigned char rr;
  rr = toneparams.op[op-1][5] & 0b11110000;
   v = toneparams.op[op-1][5] & 0b00000111;
   v = (v + d) & 0x07;
   rr = rr |  v ;
  toneparams.op[op-1][5] =rr;
  sprintf(s," DT: %4d  %4d  %4d  %4d",
  toneparams.op[0][5] & 7 ,toneparams.op[1][5]&7 ,toneparams.op[2][5]&7 ,toneparams.op[3][5]&7);
  cursgoto(1,DT+1);
  Serial.println(s);

  return 0;
}

// WS

int setws(int d)
{
  char s[40];
  int v;
  unsigned char rr;
  rr =  toneparams.op[op-1][6] & 0b00000111;
  v  = (toneparams.op[op-1][6] & 0b11111000)>>3;
  v = (v + d) & 0x1F;
  rr = rr |  ( v << 3) ;
  toneparams.op[op-1][6] =rr;
  sprintf(s," WS: %4d  %4d  %4d  %4d",
  toneparams.op[0][6] >>3 ,toneparams.op[1][6]>>3 ,toneparams.op[2][6]>>3 ,toneparams.op[3][6]>>3);
  cursgoto(1,WS+1);
  Serial.println(s);

  return 0;
}

// FB

int setfb(int d)
{
  char s[40];
  int v;
  unsigned char rr;
  rr = toneparams.op[op-1][6] & 0b11111000;
  v  = toneparams.op[op-1][6] & 0b00000111;
  v =  (v + d ) & 0x7;
  rr = rr |  v ;
  toneparams.op[op-1][6] =rr;
  sprintf(s," FB: %4d  %4d  %4d  %4d",
  toneparams.op[0][6] &7 ,toneparams.op[1][6]&7 ,toneparams.op[2][6]&7 ,toneparams.op[3][6]&7);
  cursgoto(1,FB+1);
  Serial.println(s);
  return 0;
}



/*  
 *   Dump Parameter Hex
 */
void dumpparams(void)
{
  char s[32];
  int i;
  Serial.print(F("\n/* "));
  Serial.print(tname);
  Serial.println(F(" */\n"));

  sprintf(s,"%d,%d,",toneparams.boctv,toneparams.lfoalg);
  Serial.print(s);
  for(i=0;i<4;i++){
      sprintf(s,"%d,%d,%d,%d,%d,%d,%d,",
      toneparams.op[i][0],toneparams.op[i][1],toneparams.op[i][2],toneparams.op[i][3],toneparams.op[i][4],toneparams.op[i][5],toneparams.op[i][6]);
      Serial.print(s);
  }
  Serial.println("");

  return;
  
  sprintf(s,"0x%02X,0x%02X,",toneparams.boctv,toneparams.lfoalg);
  Serial.println(s);
  for(i=0;i<4;i++){
      sprintf(s,"0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,",
      toneparams.op[i][0],toneparams.op[i][1],toneparams.op[i][2],toneparams.op[i][3],toneparams.op[i][4],toneparams.op[i][5],toneparams.op[i][6]);
      Serial.println(s);
    }
 
  
}

/*
 *   Screen Disp
 */

void printalgs(void)
{
  int alg;
  char s[12];

  cursgoto(0,0);
  sprintf(s,"TN.%2d",tnum);
  Serial.print(s);
  
  alg=toneparams.lfoalg & 0x07;
  
  if(alg==0)
     Serial.println(F("  OP1 ->OP2*  ---   --- "));
  if(alg==1)
     Serial.println(F("  OP1*  OP2*  ---   --- "));
  if(alg==2)
     Serial.println(F("  OP1*  OP2*  OP3*  OP4*"));
  if(alg==3)
     Serial.println(F("  OP1>* OP2 ->OP3 ->OP4*"));
  if( alg==4)
     Serial.println(F("  OP1 ->OP2 ->OP3 ->OP4*"));
  if(alg==5)
     Serial.println(F("  OP1 ->OP2*  OP3 ->OP4*"));
  if(alg==6)
     Serial.println(F("  OP1*  OP2 ->OP3 ->OP4*"));
  if(alg==7)
     Serial.println(F("  OP1*  OP2 ->OP3*  OP4*"));


}

void printparam(void)
{
  char s[64];

  printalgs();
  
  sprintf(s," SR: %4d  %4d  %4d  %4d",
  toneparams.op[0][0] >> 4,toneparams.op[1][0] >> 4,toneparams.op[2][0] >> 4,toneparams.op[3][0] >> 4);
  Serial.println(s);

  sprintf(s,"XOF: %4d  %4d  %4d  %4d",
  toneparams.op[0][0] >> 3&1,toneparams.op[1][0] >> 3 &1,toneparams.op[2][0] >> 3 &1 ,toneparams.op[3][0] >> 3&1);
  Serial.println(s);

  sprintf(s,"KSR: %4d  %4d  %4d  %4d",
  toneparams.op[0][0] &1,toneparams.op[1][0] &1,toneparams.op[2][0] &1 ,toneparams.op[3][0] &1);
  Serial.println(s);

  sprintf(s," RR: %4d  %4d  %4d  %4d",
  toneparams.op[0][1] >> 4,toneparams.op[1][1] >>4,toneparams.op[2][1] >>4 ,toneparams.op[3][1] >> 4);
  Serial.println(s);
  sprintf(s," DR: %4d  %4d  %4d  %4d",
  toneparams.op[0][1] & 0x0f,toneparams.op[1][1] &0x0F,toneparams.op[2][1] &0x0F ,toneparams.op[3][1]&0x0F);
  Serial.println(s);
  
  sprintf(s," AR: %4d  %4d  %4d  %4d",
  toneparams.op[0][2] >> 4,toneparams.op[1][2] >>4,toneparams.op[2][2] >>4 ,toneparams.op[3][2] >> 4);
  Serial.println(s);
  sprintf(s," SL: %4d  %4d  %4d  %4d",
  toneparams.op[0][2] & 0x0f,toneparams.op[1][2] &0x0F,toneparams.op[2][2] &0x0F ,toneparams.op[3][2]&0x0F);
  Serial.println(s);

  sprintf(s," TL: %4d  %4d  %4d  %4d",
  toneparams.op[0][3] >> 2,toneparams.op[1][3] >>2,toneparams.op[2][3] >>2 ,toneparams.op[3][3] >> 2);
  Serial.println(s);
  sprintf(s,"KSL: %4d  %4d  %4d  %4d",
  toneparams.op[0][3] & 0x03,toneparams.op[1][3] &0x03,toneparams.op[2][3] &0x03 ,toneparams.op[3][3]&0x03);
  Serial.println(s);

 sprintf(s,"DAM: %4d  %4d  %4d  %4d",
  toneparams.op[0][4] >>5,toneparams.op[1][4] >>5,toneparams.op[2][4] >>5 ,toneparams.op[3][4]>>5);
  Serial.println(s);

 sprintf(s,"EAM: %4d  %4d  %4d  %4d",
  (toneparams.op[0][4] & 0x10) >> 4,(toneparams.op[1][4]&0x10) >>4 ,(toneparams.op[2][4]&0x10)>>4 ,(toneparams.op[3][4]&0x10) >>4);
  Serial.println(s);

 sprintf(s,"DVB: %4d  %4d  %4d  %4d",
  (toneparams.op[0][4] & 0x06) >> 1,(toneparams.op[1][4]&0x06) >>1 ,(toneparams.op[2][4]&0x06)>>1 ,(toneparams.op[3][4]&0x06) >>1);
  Serial.println(s);

 sprintf(s,"EVB: %4d  %4d  %4d  %4d",
  toneparams.op[0][4] & 0x01 ,toneparams.op[1][4]&0x01 ,toneparams.op[2][4]&0x01 ,toneparams.op[3][4]&0x01);
  Serial.println(s);

 sprintf(s,"MLT: %4d  %4d  %4d  %4d",
  toneparams.op[0][5] >>4 ,toneparams.op[1][5]>>4 ,toneparams.op[2][5]>>4 ,toneparams.op[3][5]>>4);
  Serial.println(s);

 sprintf(s," DT: %4d  %4d  %4d  %4d",
  toneparams.op[0][5] &7 ,toneparams.op[1][5]&7 ,toneparams.op[2][5]&7 ,toneparams.op[3][5]&7);
  Serial.println(s);

  sprintf(s," WS: %4d  %4d  %4d  %4d",
  toneparams.op[0][6] >>3 ,toneparams.op[1][6]>>3 ,toneparams.op[2][6]>>3 ,toneparams.op[3][6]>>3);
  Serial.println(s);

 sprintf(s," FB: %4d  %4d  %4d  %4d",
  toneparams.op[0][6] &7 ,toneparams.op[1][6]&7 ,toneparams.op[2][6]&7 ,toneparams.op[3][6]&7);
  Serial.println(s);


  unsigned char lfoalg;
  lfoalg=toneparams.lfoalg;
  sprintf(s,"LFO: %4d    ALG:%4d  \r\nOCT: %4d    [%s]",lfoalg>>6,lfoalg&0x7,toneparams.boctv,tname);
   Serial.println(s);

}

/*
 * Input Line
 * 
 * maxc : max charactor
 */

char *serialinline(int maxc)
{
    static char s[130];
    char c=0;
    int i=0;

  while((c != CR) && (c != ESC) && (i<maxc)){
    if(Serial.available()){
      c=Serial.read();
      Serial.print(c);
      s[i++]=c;
      s[i]=0;
      if(c == BS){
         Serial.print("\e[K");
         i--;
         s[--i]=0;
        }
      }
  }

  
  if(c==CR){
    return s;
   } else {
    return NULL;
   }
}

void tonesave()
{
  char *s;
  int n;

  cls();
  shownames();
   
  Serial.print(F("Save Tone No.? "));
  if((s=serialinline(10))==NULL)
      {return;}

  n=atoi(s);
  if(n <= 0 || n>MAXTONE)
     { return;  }
  Serial.print(n,DEC);
  saveparams(n-1);
  tnum=n;
  cls();
  printparam();
  
}

void toneload()
{
  char *s;
  int n;

  cls();
  shownames();

  Serial.print(F("Load Tone No.? "));
  if((s=serialinline(10))==NULL)
      {return;}

  n=atoi(s);
  if(n <= 0 || n>MAXTONE)
    {  return; }   
  Serial.print(n,DEC);
  loadparams(n-1);
  tnum=n;
  cls();
  printparam();
  
}

/*
 *  Save,Load Tone Parameter
 *  
*/

void saveparams(int n)
{
  EEPROM.put(n*48   ,toneparams);
  EEPROM.put(n*48+32,tname);  
}
void loadparams(int n)
{
  EEPROM.get(n*48   ,toneparams);
  EEPROM.get(n*48+32,tname);
}

void shownames()
{
  char tn[16];
  int n;
  cursgoto(1,1);
  Serial.println("");
  for(n=0;n<MAXTONE;n++){
      EEPROM.get(n*48+32,tn);
      Serial.print(n+1,DEC);
      Serial.print(F("[ "));
      Serial.print( tn );
      Serial.println(F(" ]"));
  }
}
/*
 * Import Tone parameter csv
 * 
 */

void import()
{
  char *s, *tok;
  unsigned char d[32],*tg;
  int i,j,k=0;

  cursgoto(1,TNAME+1);
  Serial.println(F("Import : "));
 if((s=serialinline(120))==NULL)
      {return;}
  // Serial.print(s);
   Serial.println("");
  tok=strtok(s,",");
  while(tok != NULL && k < 31){
    d[k++] = (unsigned char) atoi(tok);
//    Serial.println(d[k-1],DEC);
    tok=strtok(NULL,",");
  }
  if(k<29){
    Serial.println(F("Format Error"));
    return;
  }
  
  tg = (unsigned char *) &toneparams;
  for(i=0;i<30;i++){
    *tg++ = d[i];
  }

  
}


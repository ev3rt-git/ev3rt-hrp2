/**
 * This sample program balances a two-wheeled Segway type robot such as Gyroboy in EV3 core set.
 *
 * References:
 * http://www.hitechnic.com/blog/gyro-sensor/htway/
 * http://www.cs.bgu.ac.il/~ami/teaching/Lejos-2013/classes/src/lejos/robotics/navigation/Segoway.java
 */

#include "ev3api.h"
#include "app.h"

#if defined(BUILD_MODULE)
#include "module_cfg.h"
#else
#include "kernel_cfg.h"
#endif

#define DEBUG

#ifdef DEBUG
#define _debug(x) (x)
#else
#define _debug(x)
#endif

void main_task(intptr_t unused) {
    // Draw information
    lcdfont_t font = EV3_FONT_SMALL;
    ev3_lcd_set_font(font);

    main();
}

/**
 * Functions to support hw brickbench
 */

SYSTIM TimerMS(int unused) {
    SYSTIM tim;
    get_tim(&tim);
    return tim;
}

ER LcdText(char Color, short X, short Y, char* Text) {
    return ev3_lcd_draw_string(Text, X, Y);
}

void LcdClearDisplay() {
    ev3_lcd_fill_rect(0, 0, EV3_LCD_WIDTH, EV3_LCD_HEIGHT, EV3_LCD_WHITE);
}

void CircleOut(int x, int y, int radius) {
    syslog(LOG_ERROR, "%s(): not supported yet", __FUNCTION__);
}

void EllipseOut(int x, int y, int rx, int ry) {
    syslog(LOG_ERROR, "%s(): not supported yet", __FUNCTION__);
}

void LineOut(int x1, int y1, int x2, int y2) {
    ev3_lcd_draw_line(x1, y1, x2, y2);
}

void RectOut(int x, int y, int width, int height) {
    ev3_lcd_draw_line(x, y, x, y + height);
    ev3_lcd_draw_line(x, y, x + width, y);
    ev3_lcd_draw_line(x + width, y, x + width, y + height);
    ev3_lcd_draw_line(x, y + height, x + width, y + height);
}

#define ClearTimer(...)
#define ClearTimerMS(...)
#define LcdInit(...)
#define LcdClean(...) LcdClearDisplay()
#define LcdExit(...)
#define ButtonLedInit(...)
#define ButtonLedExit(...)

// hw brickbench
// benchmark test for NXT/EV3 and similar Micro Controllers
// PL: gpp CSLite C/C++, C-API and BCC by John Hansen
// Autor: (C) Helmut Wunder 2013,2014
// freie Verwendung für private Zwecke
// für kommerzielle Zwecke nur nach Genehmigung durch den Autor.
// protected under the friendly Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License
// http://creativecommons.org/licenses/by-nc-sa/3.0/
// version 1.08.1

#if 1
#include <math.h>
#else
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include "lms2012.h"
#include "ev3_button.h"
#include "ev3_lcd.h"
#include "ev3_constants.h"
#include "ev3_command.h"
#include "ev3_timer.h"
#include "ev3_lcd.h"
#include "ev3_sound.h"
#include "ev3_output.h"
#endif

unsigned long runtime[8];

int a[500], b[500], c[500], t[500];


//--------------------------------------------
// Mersenne Twister
//--------------------------------------------

unsigned long randM(void) {
   const int M = 7;
   const unsigned long A[2] = { 0, 0x8ebfd028 };

   static unsigned long y[25];
   static int index = 25+1;

   if (index >= 25) {
     int k;
     if (index > 25) {
        unsigned long r = 9, s = 3402;
        for (k=0 ; k<25 ; ++k) {
          r = 509845221 * r + 3;
          s *= s + 1;
          y[k] = s + (r >> 10);
        }
     }
     for (k=0 ; k<25-M ; ++k)
        y[k] = y[k+M] ^ (y[k] >> 1) ^ A[y[k] & 1];
     for (; k<25 ; ++k)
        y[k] = y[k+(M-25)] ^ (y[k] >> 1) ^ A[y[k] & 1];
     index = 0;
   }

   unsigned long e = y[index++];
   e ^= (e << 7) & 0x2b5b2500;
   e ^= (e << 15) & 0xdb8b0000;
   e ^= (e >> 16);
   return e;
}

//--------------------------------------------
// Matrix Algebra
//--------------------------------------------

// matrix * matrix multiplication (matrix product)

void MatrixMatrixMult(int N, int M, int K, double A[][M], double B[][K], double C[][K]){
  int i, j, s;                                       // matrix A: N x M // B: M x K // C: N x K
  for (i=0; i<N; ++i) {
    for (j=0; j<K; ++j) {
       C[i][j]=0;
       for (s=0; s<M; ++s) {
         C[i][j]=C[i][j] + A[i][s]*B[s][j];
      }
    }
  }
}


// matrix determinant

double MatrixDet(int N, double A[N][N])
{
    int i,j,i_count,j_count, count=0;
    double Asub[N-1][N-1], det=0;

    if(N==1) return A[0][0];
    if(N==2) return (A[0][0]*A[1][1] - A[0][1]*A[1][0]);

    for(count=0; count<N; count++)
    {
        i_count=0;
        for(i=1; i<N; i++)
        {
            j_count=0;
            for(j=0; j<N; j++)
            {
                if(j == count) continue;
                Asub[i_count][j_count] = A[i][j];
                j_count++;
            }
            i_count++;
        }
        det += pow(-1, count) * A[0][count] * MatrixDet(N-1,Asub);
    }
    return det;
}


//--------------------------------------------
// shell sort
//--------------------------------------------

void shellsort(int size, int* A)
{
  int i, j, increment;
  int temp;
  increment = size / 2;

  while (increment > 0) {
    for (i = increment; i < size; i++) {
      j = i;
      temp = A[i];
      while ((j >= increment) && (A[j-increment] > temp)) {
        A[j] = A[j - increment];
        j = j - increment;
      }
      A[j] = temp;
    }

    if (increment == 2)
       increment = 1;
    else
       increment = (unsigned int) (increment / 2.2);
  }
}

//--------------------------------------------
// gnu quick sort
// (0ptional)
//--------------------------------------------

int compare_int (const int *a, const int *b)
{
  int  temp = *a - *b;

  if (temp > 0)          return  1;
  else if (temp < 0)     return -1;
  else                   return  0;
}

// gnu qsort:
// void qsort (void *a , size_a count, size_a size, compare_function)
// gnu qsort call for a[500] array of int:
// qsort (a , 500, sizeof(a), compare_int)



//--------------------------------------------
// benchmark test procedures
//--------------------------------------------


int test_Int_Add() {
   int i=1, j=11, k=112, l=1111, m=11111, n=-1, o=-11, p=-111, q=-1112, r=-11111;
   int x;
   volatile long s=0;
   for(x=0;x<10000;++x) {
     s+=i; s+=j; s+=k; s+=l; s+=m; s+=n; s+=o; s+=p; s+=q; s+=r;
   }
   return s;
}



long test_Int_Mult() {
  int x,y;
  volatile long s;

  for(y=0;y<2000;++y) {
    s=1;
    for(x=1;x<=13;++x) { s*=x;}
    for(x=13;x>0;--x) { s/=x;}

  }
  return s;
}


#define PI  M_PI


float test_float_math() {

  volatile float s=PI;
  int y;

  for(y=0;y<5000;++y) {
     s*=sqrt(s);
     s=sin(s);
     s*=cos(10.5*s);
     s=sqrt(s);
     s=exp(s);
  }
  return s;
}


long test_rand_MT(){
  volatile unsigned long s;
  int y;

  for(y=0;y<5000;++y) {
     s=randM()%10001;
  }
  return s;
}


float test_matrix_math() {
  int x;

  double A[2][2], B[2][2], C[2][2];
  double O[3][3], T[3][3];
  unsigned long s;

  for(x=0;x<250;++x) {

    A[0][0]=1;   A[0][1]=3;
    A[1][0]=2;   A[1][1]=4;

    B[0][0]=10;  B[0][1]=30;
    B[1][0]=20;  B[1][1]=40;

    MatrixMatrixMult(2,2,2, A,B,C);

    A[0][0]=1;   A[0][1]=3;
    A[1][0]=2;   A[1][1]=4;

    MatrixDet(2, A);

    O[0][0]=1;   O[0][1]=4;  O[0][2]=7;
    O[1][0]=2;   O[1][1]=5;  O[1][2]=8;
    O[2][0]=3;   O[2][1]=6;  O[2][2]=9;

    MatrixDet(3, O);

  }

  s=(O[0][0]*O[1][1]*O[2][2]);
  return s;
}



// for array copy using void *memcpy(void *dest, const void *src, size_t n);

long test_Sort(){
  unsigned long s;
  int y, i;
  int t[500];

  for(y=0;y<30;++y) {
    memcpy(t, a, sizeof(a));
    shellsort(500, t);
    
    memcpy(t, a, sizeof(b));
    shellsort(500, t);
    
    memcpy(t, a, sizeof(c));
    shellsort(500, t);
  }

  return y;
}



long test_TextOut(){

  int  y;
  char buf[120];

  for(y=0;y<20;++y) {
    LcdClearDisplay();
    sprintf (buf, "%3d %4d  int_Add",    0, 1000); LcdText(1, 0,10, buf);
    sprintf (buf, "%3d %4d  int_Mult",   0, 1010); LcdText(1, 0,20, buf);
    sprintf (buf, "%3d %4d  float_op",   0, 1020); LcdText(1, 0,30, buf);
    sprintf (buf, "%3d %4d  randomize",  0, 1030); LcdText(1, 0,40, buf);
    sprintf (buf, "%3d %4d  matrx_algb", 0, 1040); LcdText(1, 0,50, buf);
    sprintf (buf, "%3d %4d  arr_sort",   0, 1050); LcdText(1, 0,60, buf);
    sprintf (buf, "%3d %4d  displ_txt",  0, 1060); LcdText(1, 0,70, buf);
    sprintf (buf, "%3d %4d  testing...", 0, 1070); LcdText(1, 0,80, buf);

  }
  return 99;
}


long test_graphics(){
  int x, y;
  for(y=0;y<100;++y) {

    LcdClearDisplay();

    // TODO: support this -- ertl-liyixiao
    CircleOut(50, 40, 10);
    //CircleOutEx(30, 24, 10, DRAW_OPT_FILL_SHAPE);
    LineOut(10, 10, 60, 60);
    LineOut(50, 20, 90, 70);
    RectOut(20, 20, 40, 40);
    //RectOutEx(65, 25, 20, 30, DRAW_OPT_FILL_SHAPE);
    EllipseOut(70, 30, 15, 20);

  }
  return x;
}


inline void displayValues() {

  char buf[120];

    sprintf (buf, "%3d %4d  int_Add",    0, runtime[0]); LcdText(1, 0,10, buf);
    sprintf (buf, "%3d %4d  int_Mult",   1, runtime[1]); LcdText(1, 0,20, buf);
    sprintf (buf, "%3d %4d  float_op",   2, runtime[2]); LcdText(1, 0,30, buf);
    sprintf (buf, "%3d %4d  randomize",  3, runtime[3]); LcdText(1, 0,40, buf);
    sprintf (buf, "%3d %4d  matrx_algb", 4, runtime[4]); LcdText(1, 0,50, buf);
    sprintf (buf, "%3d %4d  arr_sort",   5, runtime[5]); LcdText(1, 0,60, buf);
    sprintf (buf, "%3d %4d  displ_txt",  6, runtime[6]); LcdText(1, 0,70, buf);
    sprintf (buf, "%3d %4d  graphics",   7, runtime[7]); LcdText(1, 0,80, buf);
}


void Handler(int sig)               /// ???
{
  //printf("handler %d\n", sig);    /// ???
}

int main(){

  unsigned long time0, x, y;
  float s;
  char  buf[120];
  int   i;

  //SetTimerCallback(ti1sec, &Handler); /// ???

  ClearTimer(0);
  ClearTimerMS(0);

  ButtonLedInit();
  LcdInit();
  LcdClean();


  LcdText(1, 0,10, "hw brickbench");
  LcdText(1, 0,20, "(C)H.Wunder 2013");
  LcdText(1, 0,50, "initializing...");

  for(y=0;y<500;++y) {
    a[y]=randM()%30000; b[y]=randM()%30000; c[y]=randM()%30000;
  }


  LcdClearDisplay();

  time0= TimerMS(0);;
  s=test_Int_Add();
  runtime[0]=TimerMS(0)-time0;
  sprintf (buf, "%3d %4d  int_Add",    0, runtime[0]); LcdText(1, 0,10, buf);

  time0=TimerMS(0);
  s=test_Int_Mult();
  runtime[1]=TimerMS(0)-time0;
  sprintf (buf, "%3d %4d  int_Mult",   0, runtime[1]); LcdText(1, 0,20, buf);

  time0=TimerMS(0);
  s=test_float_math();
  runtime[2]=TimerMS(0)-time0;
  sprintf (buf, "%3d %4d  float_op",   0, runtime[2]); LcdText(1, 0,30, buf);

  time0=TimerMS(0);
  s=test_rand_MT();
  runtime[3]=TimerMS(0)-time0;
  sprintf (buf, "%3d %4d  randomize",  0, runtime[3]); LcdText(1, 0,40, buf);

  time0=TimerMS(0);
  s=test_matrix_math();
  runtime[4]=TimerMS(0)-time0;
  sprintf (buf, "%3d %4d  matrx_algb", 0, runtime[4]); LcdText(1, 0,50, buf);


  time0=TimerMS(0);
  s=test_Sort();
  runtime[5]=TimerMS(0)-time0;
  sprintf (buf, "%3d %4d  arr_sort",   0, runtime[5]); LcdText(1, 0,60, buf);

  time0=TimerMS(0);
  s=test_TextOut();
  runtime[6]=TimerMS(0)-time0;
  LcdClearDisplay();
  displayValues();

  time0=TimerMS(0);
  s=test_graphics();
  runtime[7]=TimerMS(0)-time0;
  LcdClearDisplay();
  displayValues();


  LcdText(1, 0,100, "cont: press btn < LEFT...");

#if 1
  while(!ev3_button_is_pressed(LEFT_BUTTON));
  while(ev3_button_is_pressed(LEFT_BUTTON));
#else
  while(ButtonWaitForAnyPress(100) != BUTTON_ID_LEFT);
#endif

  LcdClearDisplay();
  y=0;
  for(x=0;x<8;++x) {y+= runtime[x];}

  sprintf (buf, "gesamt ms: %d ", y);           LcdText(1, 0,10, buf);
  sprintf (buf, "benchmark: %d ", 50000000/y ); LcdText(1, 0,20, buf);

  LcdText(1, 0,40, "quit: press btn < LEFT...");   // <<<<<<<<< no reaction, just for left + ESC !

#if 1
  while(!ev3_button_is_pressed(LEFT_BUTTON));
  while(ev3_button_is_pressed(LEFT_BUTTON));
#else
  while(ButtonWaitForAnyPress(100) != BUTTON_ID_LEFT);
#endif

  LcdExit();
  ButtonLedExit();
  return 1;

}

#include <string.h>
#include <mgrx.h>
#include <mgrxkeys.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <windows.h>
#include <mmsystem.h>
//#include "serialport.h"
#include "serialib.h"
#include <pthread.h>     /* pthread functions and data structures */

GrTextOption grt;
//extern GrColor *egacolors;
#define BLACK        egacolors[0]
#define BLUE         egacolors[1]
#define GREEN        egacolors[2]
#define CYAN         egacolors[3]
#define RED          egacolors[4]
#define MAGENTA      egacolors[5]
#define BROWN        egacolors[6]
#define LIGHTGRAY    egacolors[7]
#define DARKGRAY     egacolors[8]
#define LIGHTBLUE    egacolors[9]
#define LIGHTGREEN   egacolors[10]
#define LIGHTCYAN    egacolors[11]
#define LIGHTRED     egacolors[12]
#define LIGHTMAGENTA egacolors[13]
#define YELLOW       egacolors[14]
#define WHITE        egacolors[15]

GrColor *egacolors;
// global dimensions
int w = 1000;
int h = 500;
int bpp = 8;

int ECGX1=50;
int ECGY1=40;
int ECGX2=1000-160;
int ECGY2=500-155;
int ECGW = ECGX2-ECGX1;
int ECGH = ECGY2-ECGY1;

int ECG_CON_X1=900;
int ECG_CON_Y1=50;
int ECG_CON_X2=950;
int ECG_CON_Y2=100;

int TEM_CON_X1=900;
int TEM_CON_Y1=150;
int TEM_CON_X2=950;
int TEM_CON_Y2=200;



GrColor r, gray;
GrContext *grc;
GrFBoxColors icolors;
GrTextOption ECG1_grt;

int FOUND_ECG1=0, FOUND_TEM1=0;
pthread_mutex_t FOUND_ECG1_mutex, FOUND_TEM1_mutex;

pthread_mutex_t TEM1RETCODE_mutex, TEM1CHAR_mutex, TEM1READY_mutex;

serialib serial_ECG1, serial_TEM1;

int  ECG1_poskok=0;
int ECG1_x=ECGX1, ECG1_y, ECG1_x0=ECGX1, ECG1_y0;
uint8_t c[10], ECG1_cnt;

uint8_t d[10], TEM1_cnt;


int brzda =0;
GrEvent ev;

int RUNNING=1;

int TEM1READY=0, TEM1RETCODE=0;
char TEM1CHAR;

void inicializuj_grafiku()
{
    GrSetDriverExt(NULL, "rszwin");
    GrSetMode(GR_width_height_bpp_graphics, w, h, bpp);
    //GrSetMode(GR_biggest_graphics);
    GrSetWindowTitle("ECG RECTO MON");
    GrEventInit();

    r = GrAllocColor(255,0,0);
    gray = GrAllocColor(200,200,200);


    if ((grc = GrCreateContext(GrScreenX(), GrScreenY(), NULL, NULL)) == NULL) {
         printf("context error\n"); exit(-1);
    }

    //GrFont *ECG1_font = GrLoadFont("");

    ECG1_grt.txo_font = &GrFont_PX14x28;
    ECG1_grt.txo_fgcolor = GrBlack();
    ECG1_grt.txo_bgcolor = GrAllocColor(220,220,220);
    ECG1_grt.txo_chrtype = GR_ISO_8859_1_TEXT; //GR_UTF8_TEXT;
    ECG1_grt.txo_direct = GR_TEXT_RIGHT;
    ECG1_grt.txo_xalign = GR_ALIGN_LEFT;
    ECG1_grt.txo_yalign = GR_ALIGN_TOP;
    ECG1_grt.txo_chrtype = GR_BYTE_TEXT;


    egacolors = GrAllocEgaColors();

    icolors.fbx_intcolor = GrAllocColor(255,255,255);
	icolors.fbx_bottomcolor = GrAllocColor(0,180,150);
	icolors.fbx_rightcolor = GrAllocColor(0,180,150);
	icolors.fbx_leftcolor = GrAllocColor(0,90,60);
	icolors.fbx_topcolor = GrAllocColor(0,90,60);
    //GrMouseEraseCursor();
    GrMouseDisplayCursor();


}

void zobraz_data (double *data, long int zacatek, long int konec, double miny, double maxy, GrColor barva)
{

    double x,y, y0, x0, k, l;
    long int i, step;
    k = (double)GrMaxY()/(double)(maxy-miny) ;
    l = (double)GrMaxX()/(double)(konec-zacatek) ;
    x0= (0) *l  ;
    y0= data[zacatek]*k - miny*k;
    //step= (konec-zacatek)/GrMaxX();
    step=1;
    //GrCircle( x0,y0, 5, GrAllocColor(0,255,0) );
    for(i=zacatek+1; i<=konec; i+=step)
    {

        x= (i) * l - l*zacatek;
        y= data[i] * k - miny*k;
        GrLine( x0,GrScreenY()-y0, x, GrScreenY()-y, barva );
        x0=x;
        y0=y;

    }

}

void zobraz_popis_os(long int zacatek, long int konec)
{
    char szacatek[20], skonec[20];
    sprintf(szacatek,"%15li",zacatek);
    sprintf(skonec,"%15li",konec);
    GrDrawString( szacatek, strlen( szacatek ), 10, GrMaxY()-20, &grt );
    GrDrawString( skonec, strlen( skonec ), GrMaxX()-130, GrMaxY()-20, &grt );
}



// pracuje s globalni promennou serial_ECG1 FOUND_ECG1_mutex
void *search_device_ECG1(void* data)
{ char device_name[50];
  pthread_t tid = (pthread_t)data;    /* data received by thread */
  int attempts=0;
  int najito=0;

  //printf("Hello from new thread %u - got %u\n", pthread_self(), data);
  //GrSleep(1);

  while (najito==0) {
    // Test each port between COM1 and COM99 on Windows and between /dev/ttyS0 and /dev/ttyS99 on Linux
    for (int i=1;i<99 && najito==0;i++)
    {
    // Prepare the port name (Windows)
    #if defined (_WIN32) || defined( _WIN64)
        sprintf (device_name,"\\\\.\\COM%d",i);
    #endif

    // Prepare the port name (Linux)
    #ifdef __linux__
        sprintf (device_name,"/dev/ttyACM%d",i-1);
    #endif

    // try to connect to the device
    if (serial_ECG1.openDevice(device_name,115200)==1)
    {
         //printf ("ECG1 port detected on %s  \n", device_name);
          // reset
         serial_ECG1.writeChar('r');
         char readbuffer[100];
         int ret = serial_ECG1.readBytes (readbuffer,99,100);
         readbuffer[ret]=0;
         //printf("%d Bytes were read:%s\n",ret,readbuffer);
         if (readbuffer[0]=='E' && readbuffer[1]=='C' && readbuffer[2]=='G' && readbuffer[3]=='1')
         {
             printf ("ECG1 device found !\n");
             najito = 1;
         }
         else{
             printf (".");
             // Close the device before testing the next port
             serial_ECG1.closeDevice();
         }

    }
    }
    GrSleep(100);
    //attempts++;    if(attempts>20) najito=1;
  } // while FOUND_ECG1 ==0

  printf ("exiting ECG1 search\n");
  serial_ECG1.writeChar('s'); // start data stream

  pthread_mutex_lock(&FOUND_ECG1_mutex);
  FOUND_ECG1 = 1;
  pthread_mutex_unlock(&FOUND_ECG1_mutex);

  pthread_exit(NULL);	                /* terminate the thread    */


}


// pracuje s globalni promennou serial_TEM 1 a FOUND_TEM1_mutex
void *search_device_TEM1(void* data)
{ char device_name[50];
  pthread_t tid = (pthread_t)data;    /* data received by thread */
  int attempts=0;
  int najito=0;

  //printf("Hello from new thread %u - got %u\n", pthread_self(), data);
  //GrSleep(1);

  while (najito==0) {
    // Test each port between COM1 and COM99 on Windows and between /dev/ttyS0 and /dev/ttyS99 on Linux
    for (int i=1;i<99 && najito==0;i++)
    {
    // Prepare the port name (Windows)
    #if defined (_WIN32) || defined( _WIN64)
        sprintf (device_name,"\\\\.\\COM%d",i);
    #endif

    // Prepare the port name (Linux)
    #ifdef __linux__
        sprintf (device_name,"/dev/ttyACM%d",i-1);
    #endif

    // try to connect to the device
    if (serial_TEM1.openDevice(device_name,115200)==1)
    {
         //printf ("TEM1 port detected on %s \n", device_name);
          // reset
         serial_TEM1.writeChar('r');
         char readbuffer[100];
         int ret = serial_TEM1.readBytes (readbuffer,99,100);
         readbuffer[ret]=0;
         //printf("%d Bytes were read:%s\n",ret,readbuffer);
         if (readbuffer[0]=='T' && readbuffer[1]=='E' && readbuffer[2]=='M' && readbuffer[3]=='1')
         {
             printf ("TEM1 device found !\n");
             najito = 1;
         }
         else{
             printf ("*");
             // Close the device before testing the next port
             serial_TEM1.closeDevice();
         }

    }
    }
    GrSleep(100);
    //attempts++;    if(attempts>50) najito=1;
  } // while FOUND_ECG1 ==0

  serial_TEM1.writeChar('s'); // start data stream

  pthread_mutex_lock(&FOUND_TEM1_mutex);
  FOUND_TEM1 = 1;
  pthread_mutex_unlock(&FOUND_TEM1_mutex);

  printf ("exiting TEM1 search\n");
  pthread_exit(NULL);	                /* terminate the thread    */

}

// globalni prom RUNNING by se mela zamutexovat.
void *GREVENT_loop(void* data)
{
   pthread_t tid = (pthread_t)data;    /* data received by thread */

   while(RUNNING) {



     brzda++; if (brzda>5) {
            brzda=0;
            GrEventRead(&ev);
            if ((ev.type == GREV_KEY) && (ev.p1 == GrKey_q)) { // terminate
                               RUNNING=0;
            }

         }

   }


  pthread_exit(NULL);	                /* terminate the thread    */

}

// globalni prom RUNNING by se mela zamutexovat.
// globalni promenna TEM1READY, FOUND_TEM1, TEM1RETCODE, TEM1CHAR
void *ECG1_loop(void* data)
{  pthread_t tid = (pthread_t)data;    /* data received by thread */
   char car; int retc;

   while(RUNNING) {

     if (TEM1READY==0 && FOUND_TEM1==1) {


              retc = serial_TEM1.readChar (&car, 1);

              pthread_mutex_lock(&TEM1RETCODE_mutex); pthread_mutex_lock(&TEM1CHAR_mutex);pthread_mutex_lock(&TEM1READY_mutex);
              TEM1CHAR=car; TEM1READY=1; TEM1RETCODE=retc;
              pthread_mutex_unlock(&TEM1RETCODE_mutex); pthread_mutex_unlock(&TEM1CHAR_mutex);pthread_mutex_unlock(&TEM1READY_mutex);
     }

}
}


pthread_t start_ECG1_loop()
{
    int        rc;         	       /* return value                  */
    pthread_t  thread_id;     	       /* thread's ID (just an integer) */
    int        tid;

    tid = pthread_self();
    rc = pthread_create(&thread_id, NULL, ECG1_loop, (void*)tid);
    if(rc)
    {
        printf("\n ERROR: return code from ECG1 loop pthread_create is %d \n", rc);
        exit(1);
    }
    GrSleep(1);
    printf("\n Created new ECG1 loop thread (%u) ... \n", thread_id);
    //pthread_mutex_init(&FOUND_ECG1_mutex, NULL);
     //pthread_join(tid, NULL);            /* wait for thread tid     */
    //phread_exit(NULL);
    return thread_id;
}


pthread_t start_ECG1_searching()
{
    int        rc;         	       /* return value                  */
    pthread_t  thread_id;     	       /* thread's ID (just an integer) */
    int        tid;

    tid = pthread_self();
    rc = pthread_create(&thread_id, NULL, search_device_ECG1, (void*)tid);
    if(rc)
    {
        printf("\n ERROR: return code from ECG1 searching pthread_create is %d \n", rc);
        exit(1);
    }
    GrSleep(1);
    printf("\n Created new ECG1 searching thread (%u) ... \n", thread_id);
    pthread_mutex_init(&FOUND_ECG1_mutex, NULL);
     //pthread_join(tid, NULL);            /* wait for thread tid     */
    //phread_exit(NULL);
    return thread_id;
}

pthread_t start_TEM1_searching()
{
    int        rc;         	       /* return value                  */
    pthread_t  thread_id;     	       /* thread's ID (just an integer) */
    int        tid;

    tid = pthread_self();
    rc = pthread_create(&thread_id, NULL, search_device_TEM1, (void*)tid);
    if(rc)
    {
        printf("\n ERROR: return code from TEM1 searching pthread_create is %d \n", rc);
        exit(1);
    }
    GrSleep(1);
    printf("\n Created new TEM1 searching thread (%u) ... \n", thread_id);
    pthread_mutex_init(&FOUND_TEM1_mutex, NULL);
     //pthread_join(tid, NULL);            /* wait for thread tid     */
    //phread_exit(NULL);
    return thread_id;
}

pthread_t start_GREVENT_loop()
{
    int        rc;         	       /* return value                  */
    pthread_t  thread_id;     	       /* thread's ID (just an integer) */
    int        tid;

    tid = pthread_self();
    rc = pthread_create(&thread_id, NULL, GREVENT_loop, (void*)tid);
    if(rc)
    {
        printf("\n ERROR: return code from GREVENT loop pthread_create is %d \n", rc);
        exit(1);
    }
    GrSleep(1);
    printf("\n Created new GREVENT loop thread (%u) ... \n", thread_id);


    //thread_mutex_init(&FOUND_TEM1_mutex, NULL);
    pthread_mutex_init(&TEM1RETCODE_mutex, NULL); pthread_mutex_init(&TEM1CHAR_mutex, NULL);pthread_mutex_init(&TEM1READY_mutex, NULL);

     //pthread_join(tid, NULL);            /* wait for thread tid     */
    //phread_exit(NULL);
    return thread_id;
}

void inform_ecg_connected(int b)
{
  if (b==1)  GrFilledBox(ECG_CON_X1,ECG_CON_Y1,ECG_CON_X2,ECG_CON_Y2, GrAllocColor(0,255, 0));
  else       GrFilledBox(ECG_CON_X1,ECG_CON_Y1,ECG_CON_X2,ECG_CON_Y2, GrAllocColor(255,0, 0));

}

void inform_tem_connected(int b)
{
  if (b==1)  GrFilledBox(TEM_CON_X1,TEM_CON_Y1,TEM_CON_X2,TEM_CON_Y2, GrAllocColor(0,255, 0));
  else       GrFilledBox(TEM_CON_X1,TEM_CON_Y1,TEM_CON_X2,TEM_CON_Y2, GrAllocColor(255,0, 0));

}

void process_next_ECG1_byte(uint8_t ECG1_read_c)
{
    if (ECG1_read_c!=0) {
                         c[ECG1_cnt] = ECG1_read_c;
                         ECG1_cnt++;
                         if (ECG1_cnt > 10) { printf("long ECG packet, size %i\n", ECG1_cnt); ECG1_cnt=0; }

                   } else {

                       if (ECG1_cnt!=2) { printf("bad ECG packet size, instead of 2 bytes has %i\n", ECG1_cnt); ECG1_cnt=0; }
                       //y = (unsigned char) readbuffer[0];//* (GrMaxY()/4096.0) ;
                       else {

                           ECG1_y= ((c[0] >> 1) + ( (c[0+1] << 6) & 128 ))  +  (c[0+1] >> 2)*256;


                           ECG1_y /= (4096.0/(ECGH) ) ;
                           ECG1_y += (ECGY1+2);
                           ECG1_cnt=0;

                           //GrFilledBox(ECG1_x+1, 0+10, ECG1_x+10, 250+10, GrWhite() );
                           //GrLine(ECG1_x+15,0+10,ECG1_x+25,20+10, GrBlack() );

                           if(ECG1_x0==ECGX1){ ECG1_y0=ECG1_y;
                             //GrLine(GrScreenX()-8,0+10,GrScreenX()-8,250+10, GrWhite() );
                             GrLine(ECGX2,ECGY1,ECGX2,ECGY2, GrWhite() );
                             GrLine(ECGX1,ECGY1,ECGX1,ECGY2, GrWhite() );
                             //GrLine(GrScreenX()-10,0+10,GrScreenX()-10,250+10, GrWhite() );
                             //GrLine(GrScreenX()-11,0+10,GrScreenX()-11,250+10, GrWhite() );
                             //GrLine(GrScreenX()-12,0+10,GrScreenX()-12,250+10, GrWhite() );
                             }


                           GrFilledBox(ECG1_x+2, ECGY1, ECG1_x+2, ECGY2, GrBlack() );
                           GrLine(ECG1_x+1,ECGY1,ECG1_x+1, ECGY2, GrWhite() );

                           inform_ecg_connected(1);

                           GrLine(ECG1_x0, ECG1_y0, ECG1_x, ECG1_y, r );
                           // BLIT
                           GrBitBlt(GrScreenContext(), 0, 0, grc, 0, 0, grc->gc_xmax, grc->gc_ymax, GrWRITE);


                           ECG1_y0=ECG1_y;
                           ECG1_x0=ECG1_x;

                           ECG1_x++; if (ECG1_x>=ECGX2-1) { ECG1_x=ECGX1; ECG1_x0=ECGX1; }

                           ECG1_poskok++; if( ECG1_poskok==300) {
                                PlaySound("pip.wav", NULL, SND_ASYNC);
                                //Beep( 750, 300 );
                                ECG1_poskok = 0;
                           }

                       }
                   }
}



int process_next_TEM1_byte(char TEM1_read_c)
{
    if (TEM1_read_c!=0) {
                         c[TEM1_cnt] = TEM1_read_c;
                         TEM1_cnt++;
                         if (TEM1_cnt > 10) { printf("long TEM packet, size %i\n", TEM1_cnt); TEM1_cnt=0; }
    } else {
                          if (TEM1_cnt!=2) { printf("bad TEM packet size, instead of 2 bytes has %i\n", TEM1_cnt); TEM1_cnt=0; }
                          else {
                                TEM1_cnt=0;
                                int TEM1_y= ((c[0] >> 1) + ( (c[0+1] << 6) & 128 ))  +  (c[0+1] >> 2)*256;

                                inform_tem_connected(1);

                                //BLIT
                                GrBitBlt(GrScreenContext(), 0, 0, grc, 0, 0, grc->gc_xmax, grc->gc_ymax, GrWRITE);


                                // v TEM1 je ted TEPLOTA

                                printf("%i\n", TEM1_y);
                          }


    }
    return 0;

}

void MyGradientBox(int x1, int y1, int x2, int y2, int width)
{

    for (int d=0; d<width; d++) {
     //GrBox(x1-d,y1-d,x2+d,y2+d, GrAllocColor(255-d*255/width,255-d*255/width,255-d*255/width));
     GrBox(x1-d,y1-d,x2+d,y2+d, GrAllocColor(d*255/width,d*255/width,d*255/width));
    }



}



void draw_background(void)
{

    //GrLine(1,1,GrMaxX(), GrMaxY(), GrAllocColor(12,200,123));



    //GrBox(ECGX1-1,ECGY1-1,ECGX2+1,ECGY2+1,GrAllocColor(12,200,123));
    MyGradientBox(ECGX1-1,ECGY1-1,ECGX2+1,ECGY2+1, 10);

    char s[]="ECG";
    GrDrawString(s, strlen(s), 1,1, &ECG1_grt);

	//GrFramedBox(10,10,GrScreenX()-10,GrScreenY()-10, 10, &icolors);

	 if (FOUND_ECG1==1) inform_ecg_connected(1); else inform_ecg_connected(0);
	 if (FOUND_TEM1==1) inform_tem_connected(1); else inform_tem_connected(0);

     // BLIT
    GrBitBlt(GrScreenContext(), 0, 0, grc, 0, 0, grc->gc_xmax, grc->gc_ymax, GrWRITE);

}


int main()
{


    char read_c;

    //PlaySound(TEXT("pip.wav"), NULL, SND_FILENAME | SND_ASYNC);


    inicializuj_grafiku();



    // BLIT
    GrSetContext( grc );

    GrClearContext( GrAllocColor(220,220,220) );
    draw_background();


    pthread_t thread_id_TEM1_searching = start_TEM1_searching();
    pthread_t thread_id_ECG1_searching = start_ECG1_searching();
    pthread_t thread_id_GREVENT_loop = start_GREVENT_loop();
    pthread_t thread_id_ECG1_loop = start_ECG1_loop();



    ECG1_cnt=0;
    int retcode;

    while( RUNNING ) {

         //GrEventWaitKeyOrClick(&ev);



         if (FOUND_ECG1==1) {

              /*
              if (serial_ECG1.isDeviceOpen())
                   {         retcode = serial_ECG1.readChar (&read_c,1); }
              else {
                             // toto by nemelo nastat, ale projistotu....
                             FOUND_ECG1=0; printf("ECG1 device is closed.\n");
                             thread_id_ECG1_searching = start_ECG1_searching();
                             GrFilledBox(130,110,140,120, GrAllocColor(255,0,0));
                             draw_background();
                             continue;
                   }
               */
              retcode = serial_ECG1.readChar (&read_c, 1);

              if (retcode==1) {

                   process_next_ECG1_byte(read_c) ; // tady i probehne potrebny BLIT


               } else { // if retcode

                   if (retcode<0) {
                             FOUND_ECG1=0; printf("ECG1 device disconnected.\n");
                             thread_id_ECG1_searching = start_ECG1_searching();
                             inform_ecg_connected(0);
                             draw_background();
                   }

               }

         }


         if (FOUND_TEM1==1) {

              //int retcode = serial_TEM1.readChar (&read_c, 1);
              if (TEM1READY==1) { TEM1READY=0; retcode=TEM1RETCODE; read_c=TEM1CHAR; } else retcode=0;

              if (retcode==1) {

                   process_next_TEM1_byte(read_c) ; // tady i probehne potrebny BLIT


               } else { // if retcode

                   if (retcode<0) {
                             FOUND_TEM1=0; printf("TEM1 device disconnected.\n");
                             thread_id_TEM1_searching = start_TEM1_searching();
                             inform_tem_connected(0);
                             draw_background();

                   }

               }

         } // if FOUND_ECG1







    } // while RUNNING

    serial_ECG1.writeChar('t'); // stop data stream
    if (serial_ECG1.isDeviceOpen()) serial_ECG1.closeDevice();

    return 0;
}





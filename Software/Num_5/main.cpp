#include <string.h>
#include <math.h>
#include <mgrx.h>
#include <mgrxkeys.h>
extern "C" {
 #include <grgui.h>
}
#include <mgrxcolr.h>
#include "grchart.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <sys/time.h>

#if defined (_WIN32) || defined( _WIN64)
#include <windows.h>
#include <mmsystem.h>
#else
#include <X11/Xlib.h>
#endif

//#include "serialport.h"
#include "serialib.h"
#include <pthread.h>     /* pthread functions and data structures */
#include "qrsdet.h"
#include "csound.h"
#include <AL/al.h>
#include <AL/alc.h>
#include "AL/alext.h"
#include <vector>



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


///////////////////
int FSAMP=40000;
#define RECVLEN 128 //128
char recvbuf[RECVLEN];
int OBSKOK=100;    //value will be changed by user later
int outputmode=0;  //value will be changed by user later

///////////////////
ALCdevice *device;                                                          //Create an OpenAL Device
ALCcontext *context;
csound mysound1, mysound2;

#define MORMYBUFLEN 50
uint16_t mormybuf[MORMYBUFLEN];
int mormyindex=0;
int dobeh = 0;
double mormyheight=335; //400
double mormyxfakt=4.1; //5.2
double my(double y);

char *buffer;
char mode;
char str[100];
char outputpath[150] = "/home/fedor/Desktop/EOD_Zaznamy/";
char inputpath[150] = "/home/fedor/Desktop/input_setings.txt";
FILE *fcsv;
int countforindex = 0;
int countforeod = 0;
int counteod = 0;
int indexlow, indexmidle, indexhigh;
int maxspi = 0;
int minspi = 9999999;
int spi;
long long firstindex, lastindex, curindex, previndex;
//time_t starttime;
//time_t endtime;
//time_t prevv;
bool first = true;

bool deteod = false;
timeval tp;

// zda se bud vykreslovat pozicni cara
#define POSLINE
// double buffer pro EKG
#define BLIT


GrColor *egacolors;
// global dimensions
int w = 248; //10.12.2022 1060
int h = 355; //10.12.2022 750 440 pro kit fbmi
int bpp = 8;

int ECGX1=10;
int ECGY1=40;
int ECGX2=w-150;
int ECGY2=200;
int ECGW = ECGX2-ECGX1;
int ECGH = ECGY2-ECGY1;

int ECG_CON_X1=w-30;
int ECG_CON_Y1=50;
int ECG_CON_X2=w-10;
int ECG_CON_Y2=100;

int TEM_CON_X1=w-100;
int TEM_CON_Y1=150;
int TEM_CON_X2=w-50;
int TEM_CON_Y2=200;

int BEEP_HZ=2000;
int BEEP_MS=200;



QRSDET *qrsdetektor;


GrColor r, g, gray, black, white;
GrContext *grc;
GrFBoxColors icolors;
GrTextOption ECG1_grt;

int FOUND_ECG1=0, FOUND_TEM1=0;
pthread_mutex_t FOUND_ECG1_mutex, FOUND_TEM1_mutex;

pthread_mutex_t TEM1RETCODE_mutex, TEM1CHAR_mutex, TEM1READY_mutex;

serialib serial_ECG1, serial_TEM1;

int  ECG1_poskok=0;
int ECG1_x=ECGX1, ECG1_y, ECG1_x0=ECGX1, ECG1_y0, ECG1_y_raw;
int ECG1_LOP; // LO+
int ECG1_LOM; // LO-


uint8_t c[10], ECG1_cnt;

uint8_t d[10], TEM1_cnt;


int brzda =0;
GrEvent ev;

int RUNNING=1;

int TEM1READY=0, TEM1RETCODE=0;
char TEM1CHAR;

void inicializuj_grafiku()
{   static int firstpass=1;

    GrEventGenWMEnd(GR_GEN_WMEND_YES);

    if(firstpass==1) {  GrSetDriverExt(NULL, "rszwin"); /* GrEventGenWMEnd(GR_GEN_WMEND_YES); */}

    GrSetMode(GR_width_height_bpp_graphics, w, h, bpp);
    //GrSetMode(GR_biggest_graphics);
    GrSetWindowTitle("Mormy Lab 5");

    GrEventInit();
    GrMouseDisplayCursor();

    r = GrAllocColor(255,0,0) | GrWRITE;
    g = GrAllocColor(0,255,0) | GrWRITE;
    gray = GrAllocColor(200,200,200) | GrWRITE;
    black=GrAllocColor(0,0,0) | GrWRITE;
    white=GrAllocColor(255,255,255) | GrWRITE;


    if ((grc = GrCreateContext(w, h, NULL, NULL)) == NULL) {
         printf("context error\n"); exit(-1);
    }

    //GrFont *ECG1_font = GrLoadFont("mgrx/fonts/px16x36b.fnt");
    GrFont *ECG1_font = GrLoadFont("mgrx/fonts/tmgrx32b.fnt");
    GrFont *normal_font = GrLoadFont("mgrx/fonts/tmgrx16n.fnt");


    //ECG1_grt.txo_font = &GrFont_PX14x28;
    ECG1_grt.txo_font = ECG1_font;
    ECG1_grt.txo_fgcolor = GrBlack();
    ECG1_grt.txo_bgcolor = GrAllocColor(220,220,220);
    ECG1_grt.txo_chrtype = GR_ISO_8859_1_TEXT; //GR_UTF8_TEXT;
    ECG1_grt.txo_direct = GR_TEXT_RIGHT;
    ECG1_grt.txo_xalign = GR_ALIGN_LEFT;
    ECG1_grt.txo_yalign = GR_ALIGN_TOP;
    ECG1_grt.txo_chrtype = GR_BYTE_TEXT;

    GrSetDefaultFont(normal_font);

    egacolors = GrAllocEgaColors();

    icolors.fbx_intcolor = GrAllocColor(255,255,255);
	icolors.fbx_bottomcolor = GrAllocColor(0,180,150);
	icolors.fbx_rightcolor = GrAllocColor(0,180,150);
	icolors.fbx_leftcolor = GrAllocColor(0,90,60);
	icolors.fbx_topcolor = GrAllocColor(0,90,60);
    //GrMouseEraseCursor();



/*
    if (firstpass==1) {
    GrClearScreen(gray);
    GrGenWebColorTable();
    GrSetFontPath("./mgrx/fonts"); // strednikem se daji oddelovat cesty
    GUIInit(0, 0);
    char *abouttext[4] = {
        "ECG monitor SUJMON",
        "(C) 2022 Pavel Smrcka",
        "FBME CTU",
        "licensed to: SUJCHBO v.v.i"};

    char s[]="INFO";
    GUIObjectsSetColors(WEBC_KHAKI, WEBC_PERU, WEBC_SIENNA);
    GUIObjectsSetFontByName("tmgrx16b.fnt");
    GUIDialogsSetColors(WEBC_BLACK, WEBC_ORANGE, WEBC_MAROON, WEBC_ANTIQUEWHITE);
    GUICDialogsSetColors(WEBC_TAN, WEBC_BLACK);
    GUIDialogsSetTitleFontByName("ncen40bi.fnt");
    GUICDialogsSetFontByName("tmgrx18b.fnt");
    GUICDialogInfo(static_cast<void*>(s),static_cast<void**>(reinterpret_cast<void**>(abouttext)), 4, static_cast<void*>(s));
    firstpass=0;
    }
*/
 firstpass=0;

/*
    double v[24] = { 70, 74, 76, 89, 106, 81,
    27, 39, 41, 79, 78, 72 , 90, 120, 160, 150, 130, 100,
    90, 95, 89, 80, 76, 77 };
    char *xlabels[12] = { "one", "two", "three", "four", "five", "six", "one", "two", "three", "four", "five", "six"  };
    char *slabels[2] = { "ser1", "ser2" };
    GrChartData d;
    GrChartStyle s;
    d.title = "Chart Title";
    d.xtitle = "X-Axis Title";
    d.ytitle = "Y-Axis Title";
    d.ymin = 30 ;
    d.ymax = 230;
    d.ystep = 10.0;
    d.ylabelstep = 10.0;
    d.nsamples = 12;
    d.nseries = 2;
    d.yvalues = v;
    d.xlabels = xlabels;
    d.slabels = slabels;

    GrChartSetDefaultStyle(&s);
    s.cbg = GrAllocColor(200, 200, 200);
    s.cvalues[0] = GrAllocColor(0, 127, 0);
    s.cvalues[1] = GrAllocColor(127, 0, 0);
    s.type = GR_CHART_TYPE_LINES;


     GrContext *ctx= GrCreateSubContext(10, 300, 990, 600, NULL, NULL);

    GrChartDraw(ctx, &d, &s);
    GrEventWaitKeyOrClick(&ev);

*/

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




void inform_ecg_connected(int b)
{
  if (b==1)  GrFilledBox(ECG_CON_X1,ECG_CON_Y1,ECG_CON_X2,ECG_CON_Y2, g );
  else       GrFilledBox(ECG_CON_X1,ECG_CON_Y1,ECG_CON_X2,ECG_CON_Y2, r );

}

void inform_tem_connected(int b)
{
  if (b==1)  GrFilledBox(TEM_CON_X1,TEM_CON_Y1,TEM_CON_X2,TEM_CON_Y2, g );
  else       GrFilledBox(TEM_CON_X1,TEM_CON_Y1,TEM_CON_X2,TEM_CON_Y2, r );

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

    #ifdef BLIT
    GrSetContext(grc);
    GrClearContext(white);
    GrBitBlt(GrScreenContext(), ECGX1, ECGY1, grc, ECGX1, ECGY1, ECGX2, ECGY2, GrWRITE);
    GrSetContext(NULL);
    #endif // BLIT

    GrClearScreen(gray);

    GrTextXY(10, (GrScreenY()-20),"(C) 2022 KIT FBMI",
             GrWhite(), gray);


    //10.12.2022GrBox(ECGX1-1,ECGY1-1,ECGX2+1,ECGY2+1,black);
    //MyGradientBox(ECGX1-1,ECGY1-1,ECGX2+1,ECGY2+1, 10);

    char s[]="---";
    //10.12.2022GrDrawString(s, strlen(s), 1,1, &ECG1_grt);

	//GrFramedBox(10,10,GrScreenX()-10,GrScreenY()-10, 10, &icolors);

	 if (FOUND_ECG1==1) inform_ecg_connected(1); else inform_ecg_connected(0);
	 //10.12.2022if (FOUND_TEM1==1) inform_tem_connected(1); else inform_tem_connected(0);

	  GrFilledBox(ECGX1, 10, ECGX1+MORMYBUFLEN*mormyxfakt, 10+my(4096), white);

}


// pracuje s globalni promennou serial_ECG1 FOUND_ECG1_mutex
void *search_device_ECG_TEM(void* data)
{ char device_name[50];
  pthread_t tid = (pthread_t)data;    /* data received by thread */
  int najitoECG=0, najitoTEM=0;

  //printf("Hello from new thread %u - got %u\n", pthread_self(), data);
  //GrSleep(1);
  printf ("\nECG1 and TEM1 searching ");


  while (RUNNING) {

         // thread ceka az se nekde shodi FOUND_ECG1 do 0

        if(pthread_mutex_trylock(&FOUND_ECG1_mutex)==0) {
             najitoECG = FOUND_ECG1 ;
             pthread_mutex_unlock(&FOUND_ECG1_mutex);
        }
        if(pthread_mutex_trylock(&FOUND_TEM1_mutex)==0) {
             najitoTEM = FOUND_TEM1 ;
             pthread_mutex_unlock(&FOUND_TEM1_mutex);
        }

        // Test each port between COM1 and COM99 on Windows and between /dev/ttyS0 and /dev/ttyS99 on Linux
        for (int i=1;i<99 && (najitoTEM==0 || najitoECG==0) ;i++)
        {
            // Prepare the port name (Windows)
            #if defined (_WIN32) || defined( _WIN64)
            sprintf (device_name,"\\\\.\\COM%d",i);
            #endif

            // Prepare the port name (Linux)
            #ifdef __linux__
            sprintf (device_name,"/dev/ttyACM%d",i-1);
            #endif

            //////////// try to connect to the ECG device
            if (najitoECG==0){
            if (serial_ECG1.openDevice(device_name,115200)==1)
            {
                //printf ("free port for ECG1 detected on %s \n", device_name);
                // reset
                serial_ECG1.writeChar('r');
                char readbuffer[100];
                int ret = serial_ECG1.readBytes (readbuffer,99,100);
                readbuffer[ret]=0;
                //printf("%d Bytes were read:%s\n",ret,readbuffer);
                if (ret>=4 &&readbuffer[0]=='E' && readbuffer[1]=='C' && readbuffer[2]=='G' && readbuffer[3]=='5')
                {

                    najitoECG = 1;
                    printf ("ECG1 device found !\n");


                    inform_ecg_connected(1);
                    //fInSet = fopen("input_setings.txt", "r");
                    //if (fInSet == NULL){
                     //   printf("!!! Vstupni soubor se nepodarilo otevrit !!!");
                    //    exit(1);
                    //}
                    //fclose(fInSet);

                    /*
                    printf ("Choose output mode for divice.\n   1 - All data\n   2 - Only detected EOD\n   3 - Only detected EOD + create CSV file and save data\n");
                    scanf("%c", &mode);
                    while (mode != '1' && mode != '2' && mode != '3'){
                        printf ("\n!!! Only '1' , '2' or '3' !!!\n\n");
                        printf ("Choose output mode for divice.\n   1 - All data\n   2 - Only detected EOD\n   3 - Only detected EOD + create CSV file and save data\n");
                        scanf("%c", &mode);
                    }
                    */
                    if (mode == '1'){
                        serial_ECG1.writeChar('a');
                        OBSKOK=100;
                        outputmode=0;
                    }
                    else{
                        serial_ECG1.writeChar('e');
                        OBSKOK=1;
                        outputmode=1;
                        if (mode == '3'){
                            //printf ("Enter the filename without spaces amd format: "); //dopsat!!!!
                            //scanf("%s", str);

                            //scanf("%[^\n]s", str);
                            //fgets(str, 100, stdin);
                            outputmode=2;
                            strcat(outputpath, str);
                            fcsv=fopen(outputpath, "w+");
                            if(fcsv==NULL){
                                outputmode=1;
                                printf("!!! Vystupni soubor se nepodarilo otevrit !!!");
                            }else{
                                fprintf(fcsv, "Date Time,SPI in ms,Index High,Index Middle,Index Low,,Samples");
                            }
                        //if (str[0]!=NULL)
                        }
                    }

                    serial_ECG1.writeChar('s'); // start data stream
                    pthread_mutex_lock(&FOUND_ECG1_mutex);
                    FOUND_ECG1 = 1;
                    pthread_mutex_unlock(&FOUND_ECG1_mutex);

                }
                else{
                    //printf (".");
                    // Close the device before testing the next port
                    serial_ECG1.closeDevice();
                    GrSleep(8);
                }

            }


            }
            /////////// try to connect to the TEM device
            /*if (najitoTEM==0) {
            if (serial_TEM1.openDevice(device_name,115200)==1)
            {
                //printf ("free port for TEM1 detected on %s \n", device_name);
                // reset
                serial_TEM1.writeChar('r');
                char readbuffer[100];
                int ret = serial_TEM1.readBytes (readbuffer,99,100);
                readbuffer[ret]=0;
                //printf("%d Bytes were read:%s\n",ret,readbuffer);
                if (ret>=4 &&readbuffer[0]=='T' && readbuffer[1]=='E' && readbuffer[2]=='M' && readbuffer[3]=='1')
                {

                    najitoTEM = 1;
                    printf ("TEM1 device found !\n");
                    serial_TEM1.writeChar('s'); // start data stream
                    pthread_mutex_lock(&FOUND_TEM1_mutex);
                    FOUND_TEM1 = 1;
                    pthread_mutex_unlock(&FOUND_TEM1_mutex);

                }
                else{
                    //printf (".");
                    // Close the device before testing the next port
                    serial_TEM1.closeDevice();
                    GrSleep(8);
                }

            }


        }*/
        }


        GrSleep(50);

    }

  printf ("exiting ECG and TEM search thread\n");
  pthread_exit(NULL);


}



// globalni prom RUNNING by se mela zamutexovat.
void *GREVENT_loop(void* data)
{
   pthread_t tid = (pthread_t)data;    /* data received by thread */

   while(RUNNING) {

     printf("redraw ");
     inicializuj_grafiku();
     draw_background();

     while(1)

     {
            //GrSleep(1);

            GrEventRead(&ev);
             if (ev.type == GREV_WMEND ) {
                RUNNING=0; printf("CROSS pushed\n");


             }

            if ((ev.type == GREV_KEY) && (ev.p1 == GrKey_q)) { // terminate
                               RUNNING=0; break;
            }

            if (ev.type == GREV_WSZCHG) { // manage a window resize
                w = ev.p3;
                h = ev.p4;
                //...save the program state
                ECGX2=w-30-50-30;
                ECGY2=200;
                ECGW = ECGX2-ECGX1;
                ECGH = ECGY2-ECGY1;

                ECG_CON_X1=w-30; //10.12.2022 w-80
                ECG_CON_Y1=50;
                ECG_CON_X2=w-10; //10.12.2022 w-30
                ECG_CON_Y2=100;

                TEM_CON_X1=w-80;
                TEM_CON_Y1=150;
                TEM_CON_X2=w-30;
                TEM_CON_Y2=200;

                break;
            }


      }

      GrMouseEraseCursor();
      GrEventUnInit();

   }

   //GrSetMode(GR_default_text);

  pthread_exit(NULL);	                /* terminate the thread    */

}

// globalni prom RUNNING by se mela zamutexovat.
// globalni promenna TEM1READY, FOUND_TEM1, TEM1RETCODE, TEM1CHAR
void *TEM1_loop(void* data)
{  pthread_t tid = (pthread_t)data;    /* data received by thread */
   char car; int retc;

   while(RUNNING) {

     GrSleep(1);


     if (TEM1READY==0 && FOUND_TEM1==1) {

              // linux: retc = serial_TEM1.readChar (&car, 1000);
              // windows: retc = serial_TEM1.readChar (&car, 1);
              retc = serial_TEM1.readChar (&car, 1000);

              pthread_mutex_lock(&TEM1RETCODE_mutex); pthread_mutex_lock(&TEM1CHAR_mutex);pthread_mutex_lock(&TEM1READY_mutex);
              TEM1CHAR=car; TEM1READY=1; TEM1RETCODE=retc;
              pthread_mutex_unlock(&TEM1RETCODE_mutex); pthread_mutex_unlock(&TEM1CHAR_mutex);pthread_mutex_unlock(&TEM1READY_mutex);

     }
   }

   printf("Exiting TEM1_loop thread\n");
   pthread_exit(NULL);	                /* terminate the thread    */

}


pthread_t start_new_thread(char *label, void *(*start_routine)(void *) )
{
    int        rc;         	       /* return value                  */
    pthread_t  thread_id;     	       /* thread's ID (just an integer) */
    int        tid;

    tid = pthread_self();
    rc = pthread_create(&thread_id, NULL, start_routine, (void*)tid);
    if(rc)
    {
        printf("\nERROR: return code from %s create is %d \n", label, rc);
        exit(1);
    }
    GrSleep(1);
    printf("\nCreated new %s thread (%u) ... \n", label, thread_id);

    return thread_id;
}


int mormydet(int sample)
{  static int d1=0,d2=0,d3=0,d4=0,d5=0,d6=0,d7=0,d8=0,d9=0, d10=0 ;
   int ret;

   #define TRES 20
   //if ( abs(sample-d2)>TRES && abs(d2-d4)>TRES  && abs(d4-d6)>TRES*0.5 && abs(d6-d8)>TRES*0.7 && abs(d8-d10)>TRES*0.5) ret=10;  // 10
   if ( abs(sample-d10)>TRES ) ret=10;  // 10
   //if (sample>198) ret=10;
   //if ( (d1-sample)>TRES && (d2-d1)>TRES ) ret=8;  // 10
   else ret=0;

   d10=d9; d9=d8; d8=d7; d7=d6; d6=d5; d5=d4; d4=d3; d3=d2; d2=d1; d1=sample;

   return ret;


}



double my(double y) {
   return y/(4096.0/mormyheight);

}


void vykresli_eod() {



  //ECG1_y /= (4096.0/(ECGH) ) ;
  GrFilledBox(ECGX1-2, 10-2, ECGX1+MORMYBUFLEN*mormyxfakt, 10+my(4096)+2, white);

  uint16_t eody0=mormybuf[mormyindex], eodx0=0;

  for(int xi=mormyindex; xi<MORMYBUFLEN; xi++) {
    GrCircle(ECGX1+(xi-mormyindex)*mormyxfakt, 10+my(4096)-my(mormybuf[xi]),2, RED);
    GrLine(ECGX1+eodx0*mormyxfakt, 10+my(4096)-my(eody0), ECGX1+(xi-mormyindex)*mormyxfakt, 10+my(4096)-my(mormybuf[xi]), BLACK );
    eodx0=xi-mormyindex; eody0=mormybuf[xi];
  }

  for(int xi=0; xi<mormyindex; xi++) {
    GrCircle(ECGX1+(MORMYBUFLEN-mormyindex+xi)*mormyxfakt, 10+my(4096)-my(mormybuf[xi]), 2, RED);
    GrLine(ECGX1+eodx0*mormyxfakt, 10+my(4096)-my(eody0), ECGX1+(MORMYBUFLEN-mormyindex+xi)*mormyxfakt, 10+my(4096)-my(mormybuf[xi]), BLACK );
    eodx0=  (MORMYBUFLEN-mormyindex)+xi; eody0=mormybuf[xi];
  }


}

void process_next_ECG1_byte(uint8_t ECG1_read_c, FILE *ff)
{   static char s[]="---"; // do globalek
    static int qrscnt=0, qrstimeout=0, xrr0=0, xrr1=0;
    static float rr=0;
    static int obskok=0;
    static int dojezd=10, odjezd=0;

    if (ECG1_read_c!=0) {
                         c[ECG1_cnt] = ECG1_read_c;
                         ECG1_cnt++;
                         if (ECG1_cnt > 10) { printf("long ECG packet, size %i\n", ECG1_cnt); ECG1_cnt=0; }

                    } else {

                       if (ECG1_cnt!=2) { printf("bad ECG packet size, instead of 2 bytes has %i\n", ECG1_cnt); ECG1_cnt=0; }
                       //y = (unsigned char) readbuffer[0];//* (GrMaxY()/4096.0) ;
                       else {
                           // c[0] a c[1] jsou bajty prijate po seriaku mezi dvema nulami
                           // LO+
                           //if (c[1] & 0b10000000) ECG1_LOP=1; else ECG1_LOP=0;
                           // LO-
                           //if (c[1] & 0b01000000) ECG1_LOM=1; else ECG1_LOM=0;
                           // vymaskovani odpadlych elektrodq
                           //c[1] = c[1] & 0b00111111 ;
                           // dekoder - v ECG1_y je prijakty vzorek EKG

                           //ECG1_y_raw= ((c[0] >> 1) + ( (c[0+1] << 6) & 128 ))  +  (c[0+1] >> 2)*256;
                           //ECG1_y_raw= (c[0]) +  (c[0+1])*256;
                           //printf("------%i", ECG1_y_raw);
                           //return;
                           c[1] = c[1] & 0b00111111 ;
                           ECG1_y_raw= ((c[0] >> 1) + ( (c[0+1] << 6) & 128 ))  +  (c[0+1] >> 2)*256;



                           ECG1_y = ECG1_y_raw/(4096.0/(ECGH) ) ;
                           //ECG1_y += (ECGY1+2);
                           ECG1_y = (ECGH-ECG1_y)+ECGY1-2;


                           ECG1_cnt=0;

                           if(countforeod>0) countforeod--;

                           if(countforindex>0){
                                switch (countforindex){
                                    case 3:
                                        indexlow=ECG1_y_raw;
                                    case 2:
                                        indexmidle=ECG1_y_raw;
                                    case 1:
                                        indexhigh=ECG1_y_raw;
                                }
                                countforindex--;

                                if(countforindex==0 && outputmode==2){
                                    curindex = (indexhigh*4096.0*4096.0) + (indexmidle*4096.0) + indexlow;
                                    //fprintf(ff, "%d,%d,%d,", indexhigh, indexmidle, indexlow);
                                    if (first) {
                                        firstindex=curindex;
                                        previndex=curindex;
                                        first = false;
                                        fprintf(ff, ",");
                                    }else{
                                        spi = (curindex-previndex)/40;
                                        if(spi<minspi) minspi=spi;
                                        if(spi>maxspi) maxspi=spi;
                                        fprintf(ff, "%d,", spi);
                                        previndex=curindex;
                                    }
                                    fprintf(ff, "%d,%d,%d,,", indexhigh, indexmidle, indexlow);
                                    lastindex=curindex;
                                }
                                return;
                           }

                           if((outputmode >= 1) && (ECG1_y_raw==4093) && countforeod==0){
                            countforindex=3;
                            countforeod=28;
                            deteod = true;
                            if(outputmode == 2){
                                gettimeofday(&tp, 0);
                                time_t curtime = tp.tv_sec;
                                tm *t = localtime(&curtime);
                                fprintf(ff, "\n");
                                fprintf(ff, "%d-%d-%d   %02d:%02d:%02d,", t->tm_mday, t->tm_mon+1, t->tm_year+1900, t->tm_hour, t->tm_min, t->tm_sec);
                                counteod++;
                            }
                            return;
                           }

                           if(!first && outputmode == 2) fprintf(ff, "%d,", ECG1_y_raw);
/*
                           if((outputmode == 1) && (ECG1_y_raw==4093) && countforeod==0){
                            countforindex=3;
                            countforeod=28;
                            deteod = true;
                            return;
                           }

                           if(outputmode == 2){
                            if(ECG1_y_raw==4093 && countforeod==0){
                                gettimeofday(&tp, 0);
                                time_t curtime = tp.tv_sec;
                                tm *t = localtime(&curtime);
                                fprintf(ff, "\n");
                                fprintf(ff, "%02d:%02d:%02d,", t->tm_hour, t->tm_min, t->tm_sec);
                                counteod++;
                                countforindex=3;
                                countforeod=28;
                                deteod = true;
                                return;
                            }

                            if(!first) fprintf(ff, "%d,", ECG1_y_raw);

                            //fprint("%d\n", ECG1_y_raw);
                           }
                           */




                           obskok++;if(obskok>=OBSKOK)  {
                                obskok=0;

                                /* 10.12.2022
                                #ifdef BLIT
                                GrSetContext( grc );
                                #endif

                                if(ECG1_x0==ECGX1){
                                    ECG1_y0=ECG1_y;

                                    #ifdef POSLINE
                                    GrLine(ECGX2,ECGY1,ECGX2,ECGY2, white );
                                    GrLine(ECGX1,ECGY1,ECGX1,ECGY2, white );
                                    #else
                                    GrFilledBox(ECGX1, ECGY1, ECGX1+2, ECGY2, white );
                                    #endif
                                }

                                #ifdef POSLINE
                                GrLine(ECG1_x+2, ECGY1, ECG1_x+2, ECGY2, gray );
                                GrLine(ECG1_x+1,ECGY1,ECG1_x+1, ECGY2, white );
                                // linux: GrSleep(1);
                                #else
                                GrLine(ECG1_x+5, ECGY2-5, ECG1_x+5, ECGY2, black );
                                GrBox(ECG1_x+2, ECGY1, ECG1_x+4, ECGY2, white );
                                #endif

                                GrLine(ECG1_x0, ECG1_y0, ECG1_x, ECG1_y, r );

                                #ifdef BLIT
                                if (ECG1_x>=ECGX2-10) dojezd--;
                                GrBitBlt(GrScreenContext(), ECG1_x0-odjezd+1, ECGY1, grc, ECG1_x0-odjezd, ECGY1, ECG1_x0+dojezd+1, ECGY2, GrWRITE);
                                if (ECG1_x<=ECGX1+10) odjezd++;
                                GrSetContext(NULL);
                                #endif // BLIT
                                */

                                inform_ecg_connected(1);
                                /*
                                if (ECG1_LOP==1 && ECG1_LOM==1) sprintf(s,"- -");
                                if (ECG1_LOP==0 && ECG1_LOM==1) sprintf(s,"L -");
                                if (ECG1_LOP==1 && ECG1_LOM==0) sprintf(s,"- R");
                                if (ECG1_LOP==0 && ECG1_LOM==0) sprintf(s,"L R");
                                GrDrawString(s, strlen(s), 100,1, &ECG1_grt);
                                */


                                ECG1_y0=ECG1_y;
                                ECG1_x0=ECG1_x;

                                ECG1_x++; if (ECG1_x>=ECGX2-1) { ECG1_x=ECGX1; ECG1_x0=ECGX1; dojezd=10; odjezd=0; }

                            } // OBSKOK


                            qrscnt++;
                            qrstimeout++;
                            if (qrstimeout>2000) {
                               //10.12.2022sprintf(s,"---");
                               //10.12.2022GrDrawString(s, strlen(s), 1,1, &ECG1_grt);
                            }
                            ////////////////////////
                            //int det = qrsdetektor->QRS_detect( ECG1_y*50  ); // *4 je ok


                            /*int det = mormydet (ECG1_y);

                            if (det>0 && dobeh==0) {

                                dobeh=MORMYBUFLEN*0.5;
                                mysound2.play();
                               //det /= (double)OBSKOK ;
                                det=50;
                                if (ECG1_x>=(ECGX1+det) )
                                    GrLine(ECG1_x-det, ECGY1, ECG1_x-det, ECGY2, black );
                                //           else
                                //             GrLine(ECGX2 - (det-ECG1_x+ECGX1), ECGY1, ECGX2 - (det-ECG1_x+ECGX1), ECGY1+20, black );
                            }

                            if(dobeh>0) {

                                dobeh--;

                            }

                            if (dobeh==1) {
                                dobeh==0;

                                vykresli_eod();


                            }*/

                            if (outputmode == 0){
                                int det = mormydet (ECG1_y);

                                if (det>0 && dobeh==0) {

                                    dobeh=MORMYBUFLEN*0.5;
                                    mysound2.play();
                               //det /= (double)OBSKOK ;
                                    det=50;
                                    //10.12.2022if (ECG1_x>=(ECGX1+det) )
                                             //10.12.2022GrLine(ECG1_x-det, ECGY1, ECG1_x-det, ECGY2, black );
                                //           else
                                //             GrLine(ECGX2 - (det-ECG1_x+ECGX1), ECGY1, ECGX2 - (det-ECG1_x+ECGX1), ECGY1+20, black );



                                }

                                if(dobeh>0) {

                                    dobeh--;

                                }

                                if (dobeh==1) {
                                    dobeh==0;

                                    vykresli_eod();


                                }
                            }

                            if(outputmode >= 1){
                                //int det = mormydet (ECG1_y);

                                if (deteod&& dobeh==0) { // && dobeh==0

                                    //dobeh=MORMYBUFLEN*0.5;
                                    dobeh=5;
                                    mysound2.play();
                                    deteod = false;
                                    //vykresli_eod();
                               //det /= (double)OBSKOK ;
                                    //det=50;
                                    //if (ECG1_x>=(ECGX1+50) )
                                             //GrLine(ECG1_x-20, ECGY1, ECG1_x-20, ECGY2, black );
                                //           else
                                //             GrLine(ECGX2 - (det-ECG1_x+ECGX1), ECGY1, ECGX2 - (det-ECG1_x+ECGX1), ECGY1+20, black );



                                }

                                if(dobeh>0) {

                                    dobeh--;

                                }

                                if (dobeh==1) {
                                    dobeh==0;

                                    vykresli_eod();


                                }
                            }


/*
                            if (det>0 && ECG1_LOP==0 && ECG1_LOM==0)  {




                                        if ( (qrscnt-det-xrr0) > 0) {
                                           qrstimeout=0;
                                           xrr1=qrscnt-det;

                                           rr=  60.0 / ( (xrr1-xrr0)/(float)250 ) ;
                                           xrr0=xrr1;

                                           if (rr>999) rr=999;

                                           sprintf(s,"%3.0f", rr);

                                           mysound2.play();

                                           GrDrawString(s, strlen(s), 1,1, &ECG1_grt);
                                           det+=3;

                                           if (ECG1_x>(ECGX1+det) )
                                             GrLine(ECG1_x-det, ECGY1, ECG1_x-det, ECGY1+20, black );
                                           else
                                             GrLine(ECGX2 - (det-ECG1_x+ECGX1), ECGY1, ECGX2 - (det-ECG1_x+ECGX1), ECGY1+20, black );

                                       }

                            }
*/

                            mormybuf[mormyindex]=ECG1_y_raw;
                            mormyindex++; if(mormyindex>=MORMYBUFLEN) mormyindex=0;

                       ////////////



                       }
                   }
}



int process_next_TEM1_byte(char TEM1_read_c)
{   static char s[20];

    if (TEM1_read_c!=0) {
                         d[TEM1_cnt] = TEM1_read_c;
                         TEM1_cnt++;
                         if (TEM1_cnt > 10) { printf("long TEM packet, size %i\n", TEM1_cnt); TEM1_cnt=0; }
    } else {
                          if (TEM1_cnt!=2) { printf("bad TEM packet size, instead of 2 bytes has %i\n", TEM1_cnt); TEM1_cnt=0; }
                          else {
                                TEM1_cnt=0;
                                int TEM1_y= ((d[0] >> 1) + ( (d[0+1] << 6) & 128 ))  +  (d[0+1] >> 2)*256;
                                inform_tem_connected(1);

                                // v TEM1_y je ted TEPLOTA
                                //printf("%i\n", TEM1_y);

                                sprintf(s,"%4i", TEM1_y);
                                GrDrawString(s, strlen(s), 300, 1, &ECG1_grt);
                          }


    }
    return 0;

}



void ReadFile(char *name)
{
	FILE *file;

	unsigned long fileLen;

	//Open file
	file = fopen(name, "rb");
	if (!file)
	{
		fprintf(stderr, "Unable to open file %s", name);
		exit(-2);
	}

	//Get file length
	fseek(file, 0, SEEK_END);
	fileLen=ftell(file);
	fseek(file, 0, SEEK_SET);

	//Allocate memory
	buffer=(char *)malloc(fileLen+1);
	if (!buffer)
	{
		fprintf(stderr, "Memory error!");
                                fclose(file);
		exit(-2);
	}

	//Read file contents into buffer
	fread(buffer, fileLen, 1, file);
	fclose(file);

	//Do what ever with buffer
	//free(buffer);
}

int main()
{
#if defined (__linux__) || defined(__APPLE__)
  XInitThreads();
  system("rm /tmp/_dev*");
  system("sync");

#endif

    char read_c;
    //char fc;

    qrsdetektor = new QRSDET(FSAMP);

    FILE *fInSet=fopen(inputpath, "r");
    if (fInSet==NULL) {printf("\n\n!!! Vstupni soubor se nepodarilo otevrit !!!\n\n"); exit(-1); }
    fscanf(fInSet, "%[^\n]", &mode);
    //printf("File data: %c", fc);
    if (mode != '1' && mode != '2' && mode != '3'){
        printf("\n\n!!! Vstupni soubor je spatne sformatovan !!! Misto 1,2,3 je %c\n\n", mode);
        exit(-1);
    }
    //printf("File data: %c", mode);
    if (mode == '3'){
        fscanf(fInSet, "%s", str);
        //scanf("%s", str);
        strcat(str, "5.csv");
        //printf("Filenamee: %s", str);
    }
    fclose(fInSet);

    FILE *f=fopen("pip.wav", "r"); if (f==NULL) {printf("wav not found\n"); exit(-1); }

    //filename=strcat(filename,".csv");


    ReadFile("pip.wav");
    //PlaySound(TEXT("pip.wav"), NULL, SND_FILENAME | SND_ASYNC);

    //PlaySoundA(buffer, NULL, SND_MEMORY | SND_ASYNC);


    ////////////////

  // OpenAL init
  printf("................Initializing sound subsystem................\n");
  const char * devicename = alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);

                                                  //And an OpenAL Contex
  device = alcOpenDevice(devicename);
  //device = alcOpenDevice(NULL);
  if(!device) { printf("AOL no sound device"); return 1; }
  context = alcCreateContext(device, NULL);                                   //Give the device a context

  alcMakeContextCurrent(context);                                             //Make the context the current
  if(!context) { printf("AOL no sound context"); return 1; }                      //Error during context handeling



    mysound2.init_sound("beep-07.wav", device, context);
    mysound2.play();
    ///GrSleep(200);

    mysound2.play();


//////////////////////




//////////////////
    inicializuj_grafiku();

    draw_background();

    pthread_t thread_id_ECG1_searching = start_new_thread("ECG & TEM_searching loop", search_device_ECG_TEM );
    pthread_mutex_init(&FOUND_ECG1_mutex, NULL);

    pthread_t thread_id_GREVENT_loop = start_new_thread("GREVENT_loop", GREVENT_loop);

    pthread_t thread_id_TEM1_loop = start_new_thread("TEM1_loop", TEM1_loop);
    pthread_mutex_init(&TEM1RETCODE_mutex, NULL); pthread_mutex_init(&TEM1CHAR_mutex, NULL);pthread_mutex_init(&TEM1READY_mutex, NULL);


/*
    if(outputmode==2){

        //strcat(str, ".csv");
        //fcsv=fopen(str, "w+");
        printf("-----");
        fcsv=fopen("data2.csv", "w+");
        printf("-----%i", fcsv);
    }
*/

    ECG1_cnt=0;
    int retcode;
    bool conn = true;
    //if (FOUND_ECG1==1) serial_ECG1.writeChar('s');

    while( RUNNING ) {

         //GrEventWaitKeyOrClick(&ev);



         if (FOUND_ECG1==1) {


              // linux: retcode = serial_ECG1.readChar (&read_c, 1000);
              // windows: retcode = serial_ECG1.readChar (&read_c, 1);
              //retcode = serial_ECG1.readChar (&read_c, 1000);

              retcode = serial_ECG1.readBytes (recvbuf,RECVLEN,100, 1);

              ///////////////////////
              if (retcode>0) {

                    for( int counter=0; counter<retcode; counter++)
                    {
                        read_c = recvbuf[counter];
                        process_next_ECG1_byte(read_c, fcsv) ; // tady i probehne potrebny BLIT
                    }


               }
               else { // if retcode

                   // linux: if (retcode<=0) {
                   // windows:     if (retcode< 0) {
                    if (retcode<=0 && !conn) {

                            char readbuffer[4];
                            serial_ECG1.writeChar('r');
                            int ret = serial_ECG1.readBytes (readbuffer,4,100);
                            //readbuffer[ret]=0;
                            //printf("%d Bytes were read:%s\n",ret,readbuffer);
                            //if (!(ret>=4 &&readbuffer[0]=='E' && readbuffer[1]=='C' && readbuffer[2]=='G' && readbuffer[3]=='1')){
                            if (ret<4){
                            //if(serial_ECG1.available()==0){
                                serial_ECG1.unlockDevice(); // zaroven smaze lock
                                pthread_mutex_lock(&FOUND_ECG1_mutex);
                                FOUND_ECG1=0;
                                pthread_mutex_unlock(&FOUND_ECG1_mutex);
                                printf("ECG1 device disconnected.\n");
                                //thread_id_ECG1_searching = start_ECG1_searching();
                                inform_ecg_connected(0);
                                draw_background();

                                if(outputmode==2){
                                    float dur = (lastindex-firstindex)/FSAMP;
                                    float eps = counteod/dur;
                                    fprintf(fcsv, "\n\nDuration in sec,%d", (lastindex-firstindex)/FSAMP);
                                    fprintf(fcsv, "\nEOD per sec,%.2f", eps);
                                    fprintf(fcsv, "\nAverage time\nbetween EOD in ms,%d", ((lastindex-firstindex)/40)/counteod);
                                    fprintf(fcsv, "\nMax SPI interval,%d", maxspi);
                                    fprintf(fcsv, "\nMin SPI interval,%d", minspi);
                                    fclose(fcsv);
                                    countforindex = 0;
                                    countforeod = 0;
                                    maxspi = 0;
                                    minspi = 9999999;
                                    deteod = false;
                                    counteod=0;
                                    first = true;
                                }
                            }else{
                                conn = true;
                            }


                   }else{
                        conn = false;
                    }



               }
               ///////////////////////


         }


         if (FOUND_TEM1==1) {

              //int retcode = serial_TEM1.readChar (&read_c, 1);
              if (TEM1READY==1) { TEM1READY=0; retcode=TEM1RETCODE; read_c=TEM1CHAR; }  else retcode=2;

              if (retcode==1) {

                   process_next_TEM1_byte(read_c) ; // tady i probehne potrebny BLIT


               } else { // if retcode

                   // linux: if (retcode<=0) {
                   // windows: if (retcode<0) {
                    if (retcode<=0) {
                             serial_TEM1.unlockDevice(); // zaroven smaze lock
                             pthread_mutex_lock(&FOUND_TEM1_mutex);
                             FOUND_TEM1=0;
                             pthread_mutex_unlock(&FOUND_TEM1_mutex);
                             printf("TEM1 device disconnected.\n");
                             //thread_id_TEM1_searching = start_TEM1_searching();
                             inform_tem_connected(0);
                             draw_background();

                   }

               }

         } // if FOUND_TEM1







    } // while RUNNING

    if(outputmode==2){
        float dur = (lastindex-firstindex)/FSAMP;
        float eps = counteod/dur;
        //fprintf(fcsv, "\n\nDuration in sec,EOD per sec,Average time between EOD in ms,Max SPI interval,Min SPI interval");
        //fprintf(fcsv, "\n%d,%.0f,%d,%d,%d", (lastindex-firstindex)/FSAMP, eps, ((lastindex-firstindex)/40)/counteod, maxspi, minspi);
        fprintf(fcsv, "\n\nDuration in sec,%d", (lastindex-firstindex)/FSAMP);
        fprintf(fcsv, "\nEOD per sec,%.2f", eps);
        fprintf(fcsv, "\nAverage time\nbetween EOD in ms,%d", ((lastindex-firstindex)/40)/counteod);
        fprintf(fcsv, "\nMax SPI interval,%d", maxspi);
        fprintf(fcsv, "\nMin SPI interval,%d", minspi);
        fclose(fcsv);
        //counteod=0;
        //first = true;
    }

    serial_TEM1.unlockDevice();
    serial_ECG1.unlockDevice();

    if (serial_ECG1.isDeviceOpen()) serial_ECG1.writeChar('t'); // stop data stream
    if (serial_TEM1.isDeviceOpen()) serial_TEM1.writeChar('t'); // stop data stream
    if (serial_ECG1.isDeviceOpen()) serial_ECG1.closeDevice();
    if (serial_TEM1.isDeviceOpen()) serial_TEM1.closeDevice();
    //free(recvbuf);
    //delete recvbuf;


    printf("End point.\n");
    return 0;
}





#ifndef QRSDETECT
#define QRSDETECT

/* hlavickovy soubor s exportovanymi symboly */


/* exportovane symboly */


class QRSDET {



public:

  
  
  
  
  
QRSDET(int sampl_freq);  // konstruktor vola QRS_create a QRS_init  
~QRSDET(); // destruktor vola QRS_delete
  
  
/* popis:
 *          alokuje pamet pro QRS detektor, prednastavi meze bufferu
 *
 * vstupni parametry: 
 *     sampl_freq ..... vzorkovaci frekvence v Hz, musi byt vetsi nez 200 Hz a mensi nez 1000 Hz
 * 
 * navratova hodnota:
 *    0 .... OK, inicializace se zdarila
 *   -1 .... ERROR, inicializace se nezdarila, zrejme malo pameti
 *
 */

int QRS_create (int sampl_freq);





/* popis:
 *          inicializuje (resetuje) vnitrni promenne QRS detektoru
 *
 * vstupni parametry: 
 *						zadne
 * 
 * navratova hodnota:
 *						vzdy 0, netreba odchytavat
 *   
 *
 */

int QRS_init (void);






/* popis:
 *          rusi QRS detektor, uvolnuje pridelenou pamet
 *
 * vstupni parametry: 
 *					     zadne
 * 
 * navratova hodnota:
 *    0 .... OK, uklid se zdaril
 *   -1 .... ERROR, uklid se nezdaril, toto by nemelo nikdy nastat, pokud proces nehavaruje
 *
 */

int QRS_delete (void);




/* popis:
 *          vlastni QRS detektor
 *
 * vstupni parametry: 
 *     datum ..... aktualni vzorek EKG
 *    
 * navratova hodnota:
 *    0 .... QRS komplex zatim neni detkovan
 *   >0 .... prave jsem nalezl dalsi QRS komplex, vracene cislo udava jeho zpetnou polohu
 *           (zpozdeni oproti aktualnimu vzorku EKG)
 *
 */

int QRS_detect( int ecg_sample );




///////////*////////////*/////////////*////////////*////////////*///////////*//////////*/////////*//////////


private:
  
  // soukrome metody
int QRS_detector ( int datum, int init );
int Peak( int datum, int init ) ;
int median(int *array, int datnum) ;
int thresh(int qmedian, int nmedian) ;
int BLSCheck(int *dBuf,int dbPtr,int *maxder) ;
int QRSFilter(int datum, int init) ;
int lpfilt( int datum ,int init) ;
int hpfilt( int datum, int init ) ;
int deriv1( int x0, int init ) ;
int deriv2( int x0, int init ) ;
int mvwint(int datum, int init) ;
int earlyThresh(int qmedian, int nmedian) ;


// soukrome globalni promenne
int SAMPLE_RATE	;	
double MS_PER_SAMPLE;
int MS10;
int MS25;	
int MS30;	
int MS80;	
int MS95;	
int MS100;	
int MS125;	
int MS150;	
int MS160;	
int MS175;	
int MS195;	
int MS200;	
int MS220;	
int MS250;	
int MS300;	
int MS360;	
int MS450;	
int MS1000;	
int MS1500;	
int DERIV_LENGTH;  
int LPBUFFER_LGTH; 
int HPBUFFER_LGTH; 
int WINDOW_WIDTH;
int PRE_BLANK;   
int FILTER_DELAY;
int DER_DELAY;
int MIN_PEAK_AMP;
int *lpfilt_data;
int *hpfilt_data;
int *deriv1_derBuff;
int *deriv2_derBuff;
int *mwint_data;

double TH;
int *DDBuffer;
int DDPtr ;
int Dly ;
int MEMMOVELEN;
int datum, init;

    // puvodne staticke promenne z funkce QRS_detector ( int datum, int init )
    int det_thresh, qpkcnt ;
	int qrsbuf[8], noise[8], rrbuf[8] ;
	int rsetBuff[8], rsetCount ;
	int nmedian, qmedian, rrmedian ;
	int count, sbpeak, sbloc, sbcount ; // puvodne sbcout = MS1500
	int maxder, lastmax ;
	int initBlank, initMax ;
	int preBlankCnt, tempPeak ;
	
	// puvodne static v deriv1(int x, int init)
	int derI ;
	
	// puvodne static v deriv2(int x, int init)
	int derI2 ;
	
	// puvodne static v Peak( int datum, int init )
	int max , timeSinceMax , lastDatum ;
	
	// puvodne static v lpfilt( int datum ,int init)
	long y1, y2 ;
	int ptr ;
	
	// puvodne static v hpfilt( int datum, int init )
	long y ;
	int ptr1 ;
	
	// puvodne static v mvwint(int datum, int init)
	long sum  ;
	int ptr2  ;

  
  
} ;// class QRS_detector



#endif

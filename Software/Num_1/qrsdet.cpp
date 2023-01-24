
/* For memmove. */
#ifdef __STDC__
#include <string.h>
#else
#include <mem.h>
#endif

#include <math.h>
#include <stdlib.h>




#include "qrsdet.h" /* hlavickovy soubor s exportovanymi symboly */


/////////////////*////////*///////////*///////////*///////////*////////////*///////////*////////







///////////////////////////////////////////////////////////


// telo exportovane funkce
int QRSDET::QRS_create (int sampl_freq)
{
 
 TH = 0.475;
 Dly = 0;
 MEMMOVELEN = 7*sizeof(int);
 

 SAMPLE_RATE	= sampl_freq;	// Sample rate in Hz
 MS_PER_SAMPLE	= ( (double) 1000/ (double) SAMPLE_RATE);
 MS10	= ((int) (10/ MS_PER_SAMPLE + 0.5));
 MS25	= ((int) (25/MS_PER_SAMPLE + 0.5));
 MS30	= ((int) (30/MS_PER_SAMPLE + 0.5));
 MS80	= ((int) (80/MS_PER_SAMPLE + 0.5));
 MS95	= ((int) (95/MS_PER_SAMPLE + 0.5));
 MS100	= ((int) (100/MS_PER_SAMPLE + 0.5));
 MS125	= ((int) (125/MS_PER_SAMPLE + 0.5));
 MS150	= ((int) (150/MS_PER_SAMPLE + 0.5));
 MS160	= ((int) (160/MS_PER_SAMPLE + 0.5));
 MS175	= ((int) (175/MS_PER_SAMPLE + 0.5));
 MS195	= ((int) (195/MS_PER_SAMPLE + 0.5));
 MS200	= ((int) (200/MS_PER_SAMPLE + 0.5));
 MS220	= ((int) (220/MS_PER_SAMPLE + 0.5));
 MS250	= ((int) (250/MS_PER_SAMPLE + 0.5));
 MS300	= ((int) (300/MS_PER_SAMPLE + 0.5));
 MS360	= ((int) (360/MS_PER_SAMPLE + 0.5));
 MS450	= ((int) (450/MS_PER_SAMPLE + 0.5));
 MS1000	= SAMPLE_RATE;
 MS1500	= ((int) (1500/MS_PER_SAMPLE));
 DERIV_LENGTH   = MS10; //10
 LPBUFFER_LGTH  = ((int) (2*MS25));
 HPBUFFER_LGTH  = MS125; //125
 WINDOW_WIDTH   = MS80;			// Moving window integration width. MS80
 PRE_BLANK      = MS195;  //200
 FILTER_DELAY   = (int) (((double) DERIV_LENGTH/2) + ((double) LPBUFFER_LGTH/2 - 1) + (((double) HPBUFFER_LGTH-1)/2) + PRE_BLANK);  // filter delays plus 200 ms blanking delay
 DER_DELAY      = WINDOW_WIDTH + FILTER_DELAY + MS100; //100
 MIN_PEAK_AMP   = 7; // Prevents detections of peaks smaller than 150 uV.


 lpfilt_data = (int*) malloc( sizeof(int) * LPBUFFER_LGTH );
 hpfilt_data = (int*) malloc( sizeof(int) * HPBUFFER_LGTH );
 deriv1_derBuff = (int*) malloc( sizeof(int) * DERIV_LENGTH );
 deriv2_derBuff = (int*) malloc( sizeof(int)* DERIV_LENGTH );
 mwint_data = (int*) malloc( sizeof(int) *  WINDOW_WIDTH );

 DDBuffer = (int*) malloc( sizeof(int) * DER_DELAY );
 
 if ((lpfilt_data==NULL)||(hpfilt_data==NULL)||(deriv1_derBuff==NULL)||
	 (deriv2_derBuff==NULL)||(mwint_data==NULL)||(DDBuffer==NULL) )
	return -1;
 else 
	return 0;

}	


// telo exportovane funkce
int QRSDET::QRS_delete (void)
{

   free(lpfilt_data);
   free(hpfilt_data);
   free(deriv1_derBuff);
   free(deriv2_derBuff);
   free(mwint_data);
   return 0;

}


// telo exportovane funkce
int QRSDET::QRS_init (void)
{
	return QRS_detector(0,1);
}

// telo exportovane funkce
int QRSDET::QRS_detect ( int ecg_sample )
{ 
	return QRS_detector ( ecg_sample, 0 ); 
}



int QRSDET::QRS_detector ( int datum, int init )
	{
	
	
	int fdatum, QrsDelay = 0 ;
	int i, newPeak, aPeak ;

/*	Initialize all buffers to 0 on the first call.	*/            

	if( init )
		{		

		for(i = 0; i < 8; ++i)
			{
			noise[i] = 0 ;	/* Initialize noise buffer */
			rrbuf[i] = MS1000 ;/* and R-to-R interval buffer. */
			}

		qpkcnt = maxder = lastmax = count = sbpeak = 0 ;
		initBlank = initMax = preBlankCnt = DDPtr = 0 ;
		sbcount = MS1500 ;
		QRSFilter(0,1) ;	/* initialize filters. */
		Peak(0,1) ;
		}

	fdatum = QRSFilter(datum,0) ;	/* Filter data. */


	/* Wait until normal detector is ready before calling early detections. */

	aPeak = Peak(fdatum,0) ;
        if(aPeak < MIN_PEAK_AMP) aPeak = 0 ; 

	// Hold any peak that is detected for 200 ms
	// in case a bigger one comes along.  There
	// can only be one QRS complex in any 200 ms window.

	newPeak = 0 ;
	if(aPeak && !preBlankCnt)			// If there has been no peak for 200 ms
		{										// save this one and start counting.
		tempPeak = aPeak ;
		preBlankCnt = PRE_BLANK ;			// MS200
		}

	else if(!aPeak && preBlankCnt)	// If we have held onto a peak for
		{										// 200 ms pass it on for evaluation.
		if(--preBlankCnt == 0)
			newPeak = tempPeak ;
		}

	else if(aPeak)							// If we were holding a peak, but
		{										// this ones bigger, save it and
		if(aPeak > tempPeak)				// start counting to 200 ms again.
			{
			tempPeak = aPeak ;
			preBlankCnt = PRE_BLANK ; // MS200
			}
		else if(--preBlankCnt == 0)
			newPeak = tempPeak ;
		}

/*	newPeak = 0 ;
	if((aPeak != 0) && (preBlankCnt == 0))
		newPeak = aPeak ;
	else if(preBlankCnt != 0) --preBlankCnt ; */



	/* Save derivative of raw signal for T-wave and baseline
	   shift discrimination. */
	
	DDBuffer[DDPtr] = deriv1( datum, 0 ) ;
	if(++DDPtr == DER_DELAY)
		DDPtr = 0 ;

	/* Initialize the qrs peak buffer with the first eight 	*/
	/* local maximum peaks detected.						*/

	if( qpkcnt < 8 )
		{
		++count ;
		if(newPeak > 0) count = WINDOW_WIDTH ;
		if(++initBlank == MS1000)
			{
			initBlank = 0 ;
			qrsbuf[qpkcnt] = initMax ;
			initMax = 0 ;
			++qpkcnt ;
			if(qpkcnt == 8)
				{
				qmedian = median( qrsbuf, 8 ) ;
				nmedian = 0 ;
				rrmedian = MS1000 ;
				sbcount = MS1500+MS150 ;
				det_thresh = thresh(qmedian,nmedian) ;
				}
			}
		if( newPeak > initMax )
			initMax = newPeak ;
		}

	else	/* Else test for a qrs. */
		{
		++count ;
		if(newPeak > 0)
			{
			
			
			/* Check for maximum derivative and matching minima and maxima
			   for T-wave and baseline shift rejection.  Only consider this
			   peak if it doesn't seem to be a base line shift. */
			   
			if(!BLSCheck(DDBuffer, DDPtr, &maxder))
				{


				// Classify the beat as a QRS complex
				// if the peak is larger than the detection threshold.

				if(newPeak > det_thresh)
					{
					memmove(&qrsbuf[1], qrsbuf, MEMMOVELEN) ;
					qrsbuf[0] = newPeak ;
					qmedian = median(qrsbuf,8) ;
					det_thresh = thresh(qmedian,nmedian) ;
					memmove(&rrbuf[1], rrbuf, MEMMOVELEN) ;
					rrbuf[0] = count - WINDOW_WIDTH ;
					rrmedian = median(rrbuf,8) ;
					sbcount = rrmedian + (rrmedian >> 1) + WINDOW_WIDTH ;
					count = WINDOW_WIDTH ;

					sbpeak = 0 ;

					lastmax = maxder ;
					maxder = 0 ;
					QrsDelay =  WINDOW_WIDTH + FILTER_DELAY ;
					initBlank = initMax = rsetCount = 0 ;

			//		preBlankCnt = PRE_BLANK ;
					}

				// If a peak isn't a QRS update noise buffer and estimate.
				// Store the peak for possible search back.


				else
					{
					memmove(&noise[1],noise,MEMMOVELEN) ;
					noise[0] = newPeak ;
					nmedian = median(noise,8) ;
					det_thresh = thresh(qmedian,nmedian) ;

					// Don't include early peaks (which might be T-waves)
					// in the search back process.  A T-wave can mask
					// a small following QRS.

					if((newPeak > sbpeak) && ((count-WINDOW_WIDTH) >= MS360))
						{
						sbpeak = newPeak ;
						sbloc = count  - WINDOW_WIDTH ;
						}
					}
				}
			}
		
		/* Test for search back condition.  If a QRS is found in  */
		/* search back update the QRS buffer and det_thresh.      */

		if((count > sbcount) && (sbpeak > (det_thresh >> 1)))
			{
			memmove(&qrsbuf[1],qrsbuf,MEMMOVELEN) ;
			qrsbuf[0] = sbpeak ;
			qmedian = median(qrsbuf,8) ;
			det_thresh = thresh(qmedian,nmedian) ;
			memmove(&rrbuf[1],rrbuf,MEMMOVELEN) ;
			rrbuf[0] = sbloc ;
			rrmedian = median(rrbuf,8) ;
			sbcount = rrmedian + (rrmedian >> 1) + WINDOW_WIDTH ;
			QrsDelay = count = count - sbloc ;
			QrsDelay += FILTER_DELAY ;
			sbpeak = 0 ;
			lastmax = maxder ;
			maxder = 0 ;
			initBlank = initMax = rsetCount = 0 ;
			}
		}

	// In the background estimate threshold to replace adaptive threshold
	// if eight seconds elapses without a QRS detection.

	if( qpkcnt == 8 )
		{
		if(++initBlank == MS1000)
			{
			initBlank = 0 ;
			rsetBuff[rsetCount] = initMax ;
			initMax = 0 ;
			++rsetCount ;

			// Reset threshold if it has been 8 seconds without
			// a detection.

			if(rsetCount == 8)
				{
				for(i = 0; i < 8; ++i)
					{
					qrsbuf[i] = rsetBuff[i] ;
					noise[i] = 0 ;
					}
				qmedian = median( rsetBuff, 8 ) ;
				nmedian = 0 ;
				rrmedian = MS1000 ;
				sbcount = MS1500+MS150 ;
				det_thresh = thresh(qmedian,nmedian) ;
				initBlank = initMax = rsetCount = 0 ;
            sbpeak = 0 ;
				}
			}
		if( newPeak > initMax )
			initMax = newPeak ;
		}

	return(QrsDelay) ;	
	}




/* 
 * tela PRIVATNICH FUNKCI 
/*


/**************************************************************
* peak() takes a datum as input and returns a peak height
* when the signal returns to half its peak height, or 
**************************************************************/

int QRSDET::Peak( int datum, int init )
	{
	
	int pk = 0 ;

	if(init)
		max = timeSinceMax = 0 ;
		
	if(timeSinceMax > 0)
		++timeSinceMax ;

	if((datum > lastDatum) && (datum > max))
		{
		max = datum ;
		if(max > 2)
			timeSinceMax = 1 ;
		}

	else if(datum < (max >> 1))
		{
		pk = max ;
		max = 0 ;
		timeSinceMax = 0 ;
		Dly = 0 ;
		}

	else if(timeSinceMax > MS95)
		{
		pk = max ;
		max = 0 ;
		timeSinceMax = 0 ;
		Dly = 3 ;
		}
	lastDatum = datum ;
	return(pk) ;
	}

/********************************************************************
median returns the median of an array of integers.  It uses a slow
sort algorithm, but these arrays are small, so it hardly matters.
********************************************************************/

int QRSDET::median(int *array, int datnum)
	{
	int i, j, k, temp, sort[20] ;
	for(i = 0; i < datnum; ++i)
		sort[i] = array[i] ;
	for(i = 0; i < datnum; ++i)
		{
		temp = sort[i] ;
		for(j = 0; (temp < sort[j]) && (j < i) ; ++j) ;
		for(k = i - 1 ; k >= j ; --k)
			sort[k+1] = sort[k] ;
		sort[j] = temp ;
		}
	return(sort[datnum>>1]) ;
	}
/*
int median(int *array, int datnum)
	{
	long sum ;
	int i ;

	for(i = 0, sum = 0; i < datnum; ++i)
		sum += array[i] ;
	sum /= datnum ;
	return(sum) ;
	} */

/****************************************************************************
 thresh() calculates the detection threshold from the qrs median and noise
 median estimates.
****************************************************************************/

int QRSDET::thresh(int qmedian, int nmedian)
	{
	int thrsh, dmed ;
	double temp ;
	dmed = qmedian - nmedian ;
/*	thrsh = nmedian + (dmed>>2) + (dmed>>3) + (dmed>>4); */
	temp = dmed ;
	temp *= TH ;
	dmed = temp ;
	thrsh = nmedian + dmed ; /* dmed * THRESHOLD */
	return(thrsh) ;
	}

/***********************************************************************
	BLSCheck() reviews data to see if a baseline shift has occurred.
	This is done by looking for both positive and negative slopes of
	roughly the same magnitude in a 220 ms window.
***********************************************************************/

int QRSDET::BLSCheck(int *dBuf,int dbPtr,int *maxder)
	{
	int max, min, maxt, mint, t, x ;
	max = min = 0 ;

	return(0) ;
	
	for(t = 0; t < MS220; ++t)
		{
		x = dBuf[dbPtr] ;
		if(x > max)
			{
			maxt = t ;
			max = x ;
			}
		else if(x < min)
			{
			mint = t ;
			min = x;
			}
		if(++dbPtr == DER_DELAY)
			dbPtr = 0 ;
		}

	*maxder = max ;
	min = -min ;
	
	/* Possible beat if a maximum and minimum pair are found
		where the interval between them is less than 150 ms. */
	   
	if((max > (min>>3)) && (min > (max>>3)) &&
		(abs(maxt - mint) < MS150))
		return(0) ;
		
	else
		return(1) ;
	}





/******************************************************



/******************************************************************************
* Syntax:
*	int QRSFilter(int datum, int init) ;
* Description:
*	QRSFilter() takes samples of an ECG signal as input and returns a sample of
*	a signal that is an estimate of the local energy in the QRS bandwidth.  In
*	other words, the signal has a lump in it whenever a QRS complex, or QRS
*	complex like artifact occurs.  The filters were originally designed for data
*  sampled at 200 samples per second, but they work nearly as well at sample
*	frequencies from 150 to 250 samples per second.
*
*	The filter buffers and static variables are reset if a value other than
*	0 is passed to QRSFilter through init.
*******************************************************************************/
int QRSDET::QRSFilter(int datum,int init)
	{
	int fdatum ;
	if(init)
		{
		hpfilt( 0, 1 ) ;		// Initialize filters.
		lpfilt( 0, 1 ) ;
		mvwint( 0, 1 ) ;
		deriv1( 0, 1 ) ;
		deriv2( 0, 1 ) ;
		}
	fdatum = lpfilt( datum, 0 ) ;		// Low pass filter data.
	fdatum = hpfilt( fdatum, 0 ) ;	// High pass filter data.
	fdatum = deriv2( fdatum, 0 ) ;	// Take the derivative.
	fdatum = abs(fdatum) ;				// Take the absolute value.
	fdatum = mvwint( fdatum, 0 ) ;	// Average over an 80 ms window .
	return(fdatum) ;
	}

/*************************************************************************
*  lpfilt() implements the digital filter represented by the difference
*  equation:
*
* 	y[n] = 2*y[n-1] - y[n-2] + x[n] - 2*x[t-24 ms] + x[t-48 ms]
*
*	Note that the filter delay is (LPBUFFER_LGTH/2)-1
*
**************************************************************************/
int QRSDET::lpfilt( int datum ,int init)
	{
	
	long y0 ;
	int output, halfPtr ;
	if(init)
		{
		for(ptr = 0; ptr < LPBUFFER_LGTH; ++ptr)
			lpfilt_data[ptr] = 0 ;
		y1 = y2 = 0 ;
		ptr = 0 ;
		}
	halfPtr = ptr-(LPBUFFER_LGTH/2) ;	// Use halfPtr to index
	if(halfPtr < 0)							// to x[n-6].
		halfPtr += LPBUFFER_LGTH ;
	y0 = (y1 << 1) - y2 + datum - (lpfilt_data[halfPtr] << 1) + lpfilt_data[ptr] ;
	y2 = y1;
	y1 = y0;
	output = y0 / ((LPBUFFER_LGTH*LPBUFFER_LGTH)/4);
	lpfilt_data[ptr] = datum ;			// Stick most recent sample into
	if(++ptr == LPBUFFER_LGTH)	// the circular buffer and update
		ptr = 0 ;					// the buffer pointer.
	return(output) ;
	}

/******************************************************************************
*  hpfilt() implements the high pass filter represented by the following
*  difference equation:
*
*	y[n] = y[n-1] + x[n] - x[n-128 ms]
*	z[n] = x[n-64 ms] - y[n] ;
*
*  Filter delay is (HPBUFFER_LGTH-1)/2
******************************************************************************/
int QRSDET::hpfilt( int datum, int init )
	{
	
	int z, halfPtr ;
	if(init)
		{
		for(ptr1 = 0; ptr1 < HPBUFFER_LGTH; ++ptr1)
			hpfilt_data[ptr1] = 0 ;
		ptr1 = 0 ;
		y = 0 ;
		}
	y += datum - hpfilt_data[ptr1];
	halfPtr = ptr1-(HPBUFFER_LGTH/2) ;
	if(halfPtr < 0)
		halfPtr += HPBUFFER_LGTH ;
	z = hpfilt_data[halfPtr] - (y / HPBUFFER_LGTH);
	hpfilt_data[ptr1] = datum ;
	if(++ptr1 == HPBUFFER_LGTH)
		ptr1 = 0 ;
	return( z );
	}
/*****************************************************************************
*  deriv1 and deriv2 implement derivative approximations represented by
*  the difference equation:
*
*	y[n] = x[n] - x[n - 10ms]
*
*  Filter delay is DERIV_LENGTH/2
*****************************************************************************/
int QRSDET::deriv1(int x, int init)
	{
	//static int derBuff[MAX_DERIV_LENGTH];
	int derI = 0 ;
	int y ;
	if(init != 0)
		{
		for(derI = 0; derI < DERIV_LENGTH; ++derI)
			deriv1_derBuff[derI] = 0 ;
		derI = 0 ;
		return(0) ;
		}
	y = x - deriv1_derBuff[derI] ;
	deriv1_derBuff[derI] = x ;
	if(++derI == DERIV_LENGTH)
		derI = 0 ;
	return(y) ;
	}

int QRSDET::deriv2(int x, int init)
	{
	//static int derBuff[MAX_DERIV_LENGTH];
	
	int y ;
	if(init != 0)
		{
		for(derI2 = 0; derI2 < DERIV_LENGTH; ++derI2)
			deriv2_derBuff[derI2] = 0 ;
		derI2 = 0 ;
		return(0) ;
		}
	y = x - deriv2_derBuff[derI2] ;
	deriv2_derBuff[derI2] = x ;
	if(++derI2 == DERIV_LENGTH)
		derI2 = 0 ;
	return(y) ;
	}


/*****************************************************************************
* mvwint() implements a moving window integrator.  Actually, mvwint() averages
* the signal values over the last WINDOW_WIDTH samples.
*****************************************************************************/
int QRSDET::mvwint(int datum, int init)
	{
	
	int output;
	if(init)
		{
		for(ptr2 = 0; ptr2 < WINDOW_WIDTH ; ++ptr2)
			mwint_data[ptr2] = 0 ;
		sum = 0 ;
		ptr2 = 0 ;
		}
	sum += datum ;
	sum -= mwint_data[ptr2] ;
	mwint_data[ptr2] = datum ;
	if(++ptr2 == WINDOW_WIDTH)
		ptr2 = 0 ;
	if((sum / WINDOW_WIDTH) > 32000)
		output = 32000 ;
	else
		output = sum / WINDOW_WIDTH ;
	return(output) ;
	}


//////////////////////////////////////////////////////
QRSDET::QRSDET(int sampl_freq)  // konstruktor vola QRS_create a QRS_init  
{
   QRS_create(sampl_freq);
   QRS_init();
   // neni osetreny navratovy chybovy kod inicializacnich fci !!!
   
   det_thresh=0; qpkcnt = 0 ;
   rsetCount = 0;
   sbpeak = 0; sbcount = 0 ;
   derI = 0 ;
   derI2 = 0 ;
   max = 0; timeSinceMax = 0;
   y1 = 0; y2 = 0 ;
   ptr = 0 ;
   y=0 ;
   ptr1 = 0 ;
   sum = 0 ;
   ptr2 = 0 ;
}

QRSDET::~QRSDET() // destruktor vola QRS_delete
{
   QRS_delete(); 
}




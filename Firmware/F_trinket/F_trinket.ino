/*
 *  MORMY-LAB pro Trinket M0 
 * 
 *  vzorkovaci stroj pro 1 kanal vnitniho 12 bit AD
 
 *  
 *  Implementovan double-buffering, datovy tok se zvysil.
 * 
 *  Odzkouseno: 40 kHz vzorkovacka.
 *  
 * V dalsi verzi by se mel vysledek AD prevodu vycitat v preruseni od dokonceni prevodu.
 * Tato verze v rutine preteceni casovace vola analogRead(), coz je blokujici.
 * Sice je snizeny delici faktor hodin AD prevodniku (rutina AdcBooster),
 * ale stejne to zere zbytecne hodne casu. Diky teto uprave by mela jit vzorkovacka jeste bezpecne zvysit.
 * 
 * 
 * 
 */
 
 
 
#define TEST_PIN 0
#define LOP_PIN 3 
#define LOM_PIN 4
 
#define FSAMP 40000  /// 40000
#define BUFLEN 128   /// 64
 
#define OVERSAMPLE 1 /// 1 
 

#define CPU_HZ 48000000
//#define TIMER_PRESCALER_DIV 1024
#define TIMER_PRESCALER_DIV 16
 
// timer functions
void startTimer(int frequencyHz);
void setTimerFrequency(int frequencyHz);
void TC3_Handler();
 
// adc functions
void AdcBooster();
uint16_t adcvalue;
int adccomplete=0;
 

int gval, gcnt=499, sample;
long int gsuma=0, gcntsuma=0;
 
uint8_t tmp1, tmp2, tmp3, tmp4; uint16_t tmp5=0;
uint8_t bufferA[3*BUFLEN];
uint8_t bufferB[3*BUFLEN];
uint16_t indexA=0;
uint16_t indexB=0;
bool fillingA=true;
bool fillingB=false;
bool switched2A=true;
bool switched2B=false;
bool outputmode=true; //true - send all data; false - send only detected EOD
long randNumber;
 
uint16_t ad_res;
uint16_t indexhigh=0;//, indexlow, indexmidle;
uint16_t indexlow=0;
uint16_t indexmidle=0;
int samples[13]={0,0,0,0,0,0,0,0,0,0,0,0,0};
int count=0;
int i;
int transmit_stream = 0;
char povel, obskok=0;
int glop, glom;
 
extern "C" char *sbrk(int i);
 
int FreeRam () {
  char stack_dummy = 0;
  return &stack_dummy - sbrk(0);
}
 
 
void setup() {
 
  Serial.begin(9600);


  while (!Serial);
 

  __disable_irq();
  AdcBooster();
  __enable_irq();

  ADC->SWTRIG.bit.START = true; // start next ADC conversion    

  delay(1000);

  pinMode(TEST_PIN, OUTPUT);


  glop=glom=0;
  pinMode(LOP_PIN, INPUT);   // odpala elektroda, pin LO+ 
  pinMode(LOM_PIN, INPUT);   //                       LO-

  //for (i=0; i<10; i++){
    //samples[i]=0;
  //}

  //analogReadResolution(12);
 
  startTimer(FSAMP); 
 
  
}
 
 
void loop() {
 


   if (switched2A) {    
     //digitalWrite(TEST_PIN, !digitalRead(TEST_PIN) );
     //digitalWrite(TEST_PIN, HIGH );

      switched2A=false;
      //Serial.print("switcehd2A "); Serial.println(indexB);
       if (transmit_stream==1) Serial.write( (uint8_t *)bufferB,3*BUFLEN);  
      //Serial.flush();   
      //digitalWrite(TEST_PIN, LOW ); 


   } 
 
   else if (switched2B) {
      //digitalWrite(TEST_PIN, !digitalRead(TEST_PIN) );
      // digitalWrite(TEST_PIN, HIGH );

      switched2B=false;
      //Serial.print("switcehd2B "); Serial.println(indexA);
       if (transmit_stream==1) Serial.write( (uint8_t *)bufferA,3*BUFLEN); 
      //Serial.flush();
      //digitalWrite(TEST_PIN, LOW );


   } 

  //while (Serial.available() > 0)
  //obskok++; 
 // if (obskok>10) {
//      obskok=0;  
      povel = Serial.read();
      switch(povel) {
         case 's': transmit_stream=1;  break; // znak 's' z PC nastartuje data-stream indexhigh=0; indexlow=0; indexmidle=0;
         case 't': transmit_stream=0; indexhigh=0; indexlow=0; indexmidle=0; outputmode=true; count=10; break; // znak 't' z PC zastavi data-stream
         case 'a': outputmode=true; break; // znak 'a' z PC rika zarizeni posilat hruba data
         case 'e': outputmode=false; break; // znak 'e' z PC rika zarizeni posilat detekovane EOD
         case 'r': //transmit_stream=0; delay(5); 
                   uint8_t tb[]={'E','C','G','1'};
                   Serial.write(tb, 4);
                   //Serial.println("ECG1"); break; // znak 'r' z PC zpusobi vyslani sekvence 'E' 'C' 'G' '1'


      }    
 //}
 
  
}
 
void setTimerFrequency(int frequencyHz) {
  int compareValue = (CPU_HZ / (TIMER_PRESCALER_DIV * frequencyHz)) - 1;
  TcCount16* TC = (TcCount16*) TC3;  
  // Make sure the count is in a proportional position to where it was
  // to prevent any jitter or disconnect when changing the compare value.
  TC->COUNT.reg = map(TC->COUNT.reg, 0, TC->CC[0].reg, 0, compareValue);
  TC->CC[0].reg = compareValue;
  Serial.println(TC->COUNT.reg);
  Serial.println(TC->CC[0].reg);
  while (TC->STATUS.bit.SYNCBUSY == 1);
}
 
void startTimer(int frequencyHz) {
  REG_GCLK_CLKCTRL = (uint16_t) (GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID_TCC2_TC3) ;
  while ( GCLK->STATUS.bit.SYNCBUSY == 1 ); // wait for sync
 
  TcCount16* TC = (TcCount16*) TC3;
 
  TC->CTRLA.reg &= ~TC_CTRLA_ENABLE;
  while (TC->STATUS.bit.SYNCBUSY == 1); // wait for sync
 
  // Use the 16-bit timer
  TC->CTRLA.reg |= TC_CTRLA_MODE_COUNT16;
  while (TC->STATUS.bit.SYNCBUSY == 1); // wait for sync
 
  // Use match mode so that the timer counter resets when the count matches the compare register
  TC->CTRLA.reg |= TC_CTRLA_WAVEGEN_MFRQ;
  while (TC->STATUS.bit.SYNCBUSY == 1); // wait for sync
 
  // Set prescaler to 1024
  TC->CTRLA.reg |= TC_CTRLA_PRESCALER_DIV16;
  while (TC->STATUS.bit.SYNCBUSY == 1); // wait for sync
 
  setTimerFrequency(frequencyHz);
 
  // Enable the compare interrupt
  TC->INTENSET.reg = 0;
  TC->INTENSET.bit.MC0 = 1;
 
  NVIC_EnableIRQ(TC3_IRQn);
 
  TC->CTRLA.reg |= TC_CTRLA_ENABLE;
  while (TC->STATUS.bit.SYNCBUSY == 1); // wait for sync
}
 
void TC3_Handler() {
  TcCount16* TC = (TcCount16*) TC3;
  // If this interrupt is due to the compare register matching the timer count
  // we toggle the LED.
  if (TC->INTFLAG.bit.MC0 == 1) {  
     //digitalWrite(TEST_PIN, HIGH );  
     digitalWrite(TEST_PIN, !digitalRead(TEST_PIN) );
    TC->INTFLAG.bit.MC0 = 1;

    //digitalWrite(TEST_PIN, !digitalRead(TEST_PIN)); // kvuli zmereni vzorkovacky

   // digitalWrite(TEST_PIN, !digitalRead(TEST_PIN) );
    if (outputmode){
      gval = analogRead(A0);
      //gval=3070;
      //Serial.println(gval);
      ad_res = (uint16_t) gval;
      //Serial.println(ad_res);
    }
    else{
      sample = analogRead(A0);   // toto je blokujici cekani na dokonceni ad prevodu, zde se zbytecne ztraci cas
      //randNumber = random(10000);
      //Serial.println(randNumber);
      //if (randNumber<2){sample = 3500;}
      
      if (samples[0]!= 0 and count==0) {            //and count==0

        if ( abs(sample-samples[3])>150 ){
           count=40;
           samples[0]=4093;
           samples[1]=indexlow;
           samples[2]=indexmidle;
           samples[3]=indexhigh;
           //samples[12]=4093;
           //samples[1]=55000;
        }
          //Serial.println("_________");
      }
      
      if(count>0){
        ad_res = (uint16_t) samples[0];
        //transmit_stream=1;
        //samples[50-count]=0;
        count--;
      }

      for (i=0; i<=11; i++){
        samples[i]=samples[i+1];
      }
      samples[12]=sample;

      //if(count==0){
        //ad_res = (uint16_t) gval;
        //transmit_stream=0;
      //}

      if(indexlow==4095){
        indexlow=0;
        if(indexmidle==4095){
          indexmidle=0;
          indexhigh++;
        }
        else{
          indexmidle++;
        }
      }
      else{
        indexlow++;
      }
      
    }

    /*
    sample = analogRead(A0);   // toto je blokujici cekani na dokonceni ad prevodu, zde se zbytecne ztraci cas
    //gval= 2200;
    //glop += digitalRead(LOP_PIN);
    //glom += digitalRead(LOM_PIN);
 
    /*
      // casem by to chtelo zprovoznit vycitani pres preruseni od dokonceniAD prevodu, blokujici analogRead zpusobuje jitter 
     if (adccomplete==1) { 
      gval = adcvalue; adccomplete=0; 

    } else { Serial.println("FATAL ERROR, ADC nestiha"); }
     ADC->SWTRIG.bit.START = true; // start next ADC conversion    
    */
 
  /*
    gval = (uint16_t)tmp5; 
    tmp5++;if(tmp5>4095)tmp5=0;
    */
/*
    //gsuma += gval;
    //gcntsuma++ ;

   // if (gcntsuma>=OVERSAMPLE) {
   //   gsuma /= (float)OVERSAMPLE;

      //Serial.println(gsuma);
      
      //count++;
      //Serial.println(samples[0]);
      if (samples[0]!= 0 ) {            //and count==0
        //Serial.println("_________");
        //count=50;
        if ( abs(sample-samples[0])>100 ){
           count=15;
           
        }
          //Serial.println("_________");
      }
      

      for (i=0; i<=8; i++){
        samples[i]=samples[i+1];
      }
      samples[9]=sample;
     /* samples[0]=samples[1];
      samples[1]=samples[2];
      samples[2]=samples[3];
      samples[3]=samples[4];
      samples[4]=samples[5];
      samples[5]=samples[6];
      samples[6]=samples[7];
      samples[7]=samples[8];
      samples[8]=samples[9];
      samples[9]=sample;*/

 /*     
      if(count>0){
        ad_res = (uint16_t) samples[0];
        transmit_stream=1;
        //samples[50-count]=0;
        count--;
      }
      //if(count>0 and count<=40){
       // ad_res = (uint16_t) sample;
       // count--;
      //}
      if(count==0){
        //ad_res = (uint16_t) gval;
        transmit_stream=0;
      }
      */
 
 
      //ad_res = (uint16_t) gval; // 2 byte vysledku AD prevodu 
      //Serial.println(gval);
      //gcntsuma=0;
      //gsuma=0; 
      if (((count != 0) and !outputmode) or outputmode){
        tmp1 = lowByte(ad_res);  //  A1 A2 A3 A4 A5 A6 A7 A8
        //Serial.println(tmp1);
        tmp2 = highByte(ad_res); //  B1 B2 B3 B4 B5 B6 B7 B8  
        //Serial.println(tmp2);
      // pozn.: protoze je AD 12 bitove, jsou v tmp2 v tuto chvili bity  B1, B2, B3, B4 nevyuzite (a nulove)
 
      // zaradim do toho odpadle elektrody ....
        if (glop!=0) tmp2 = tmp2 | 0b00100000 ; // do B3 se da 1, pokud LO+ rovno 0 (BIT +)
        if (glom!=0) tmp2 = tmp2 | 0b00010000 ; // do B4 se da 1, pokud LO- rovno 0 (BIT -)
 
      //if (glop!=0) Serial.print("P"); else Serial.print("-");
      //if (glom!=0) Serial.println("L"); else Serial.println("-");

      // KOD s vyloucenim nuly
        tmp3 = (tmp1 << 1) | 1;                        // 1. byte vysledku = A2 A3 A4 A5 A5 A7 A8 1
        tmp4 = ( (tmp2 << 2) + (tmp1 >> 6) ) | 1;      // 2. byte vysledku = +  -  B5 B6 B7 B8 A1 1
        //tmp3 = tmp1;
        //tmp4 = tmp2;
 

        if(fillingA) {    

          bufferA[indexA]=tmp3; bufferA[indexA+1]=tmp4; bufferA[indexA+2]=0; 
          indexA+=3; if (indexA>=(BUFLEN*3)) {  indexB=0;  fillingA=false; fillingB=true;  switched2B=true;      }    

        } else if(fillingB) {


          bufferB[indexB]=tmp3; bufferB[indexB+1]=tmp4; bufferB[indexB+2]=0;
          indexB+=3; if (indexB>=(BUFLEN*3)) {  indexA=0; fillingA=true; fillingB=false;  switched2A=true;        }      

        }
      }
      
 
                               
      glop=glom=0;   
     // digitalWrite(TEST_PIN, LOW);            
    }


}
 

//////// adc job
 
void AdcBooster()
{
  ADC->CTRLA.bit.ENABLE = 0;                     // Disable ADC
  while( ADC->STATUS.bit.SYNCBUSY == 1 );        // Wait for synchronization
  ADC->CTRLB.reg = ADC_CTRLB_PRESCALER_DIV16 |   // Divide Clock by 16 (orig. 64).
                   ADC_CTRLB_RESSEL_12BIT;       // Result on 12 bits
  ADC->AVGCTRL.reg = ADC_AVGCTRL_SAMPLENUM_1 |   // 1 sample
                     ADC_AVGCTRL_ADJRES(0x00ul); // Adjusting result by 0
  ADC->SAMPCTRL.reg = 0x00;                      // Sampling Time Length = 0
  ADC->INPUTCTRL.bit.GAIN=0xF;                   // reference is set at VADDA/2 ie 1.7V, so to have full scale measurement to 3.3 division by /2
  ADC->CTRLA.bit.ENABLE = 1;                     // Enable ADC
  while( ADC->STATUS.bit.SYNCBUSY == 1 );        // Wait for synchronization
} // AdcBooster

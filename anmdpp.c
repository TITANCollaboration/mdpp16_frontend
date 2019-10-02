#include <stdio.h>
#include <iostream>
#include <time.h>
#include <unistd.h> /* usleep */
#include <math.h>
#include "stdint.h" // uint64_t 

#include "midas.h"
#include "rmidas.h"

#include "experim.h"

#include <TH1F.h>
#include <TH2F.h>
#include <TTree.h>
#include <TDirectory.h>

static int debug; // only accessible through gdb

/*-- Module declaration --------------------------------------------*/
int mdpp16_event(EVENT_HEADER *, void *);
int mdpp16_init(void);
int mdpp16_bor(INT run_number);
int mdpp16_eor(INT run_number);
static void odb_callback(INT hDB, INT hseq, void *info){ }

ANA_MODULE mdpp16_module = {
  "mdpp16",               /* module name           */
  "CP",                   /* author                */
  mdpp16_event,          /* event routine         */
  mdpp16_bor,            /* BOR routine           */
  mdpp16_eor,            /* EOR routine           */
  mdpp16_init,           /* init routine          */
  NULL,                   /* exit routine          */
  NULL, //&mdpp16_settings,         /* parameter structure   */
  0,    //sizeof(mdpp16_settings),  /* structure size        */
  NULL, //mdpp16_settings_str,      /* initial parameters    */
};
/*-------------------------------------------------------------------*/

#define NUM_CHAN    16
#define HIT_CHAN    64
#define ADC_CHAN    65536

// Declare histograms here. TH1F = 4 bytes/cell (float), TH1D = 8 bytes/cell (double)
static TH1F *hHitPat, *hEnergy[NUM_CHAN], *hEnergy_flagsrm[NUM_CHAN], *hTDC[NUM_CHAN], *hRate[NUM_CHAN];
static TH1F *hCalEnergy1[NUM_CHAN];
static TH2F *hEnergy_vs_ts[NUM_CHAN];

int hist_init_roody();

//---------------------------------------------------------------------

int mdpp16_bor(INT run_number){ return SUCCESS; }
int mdpp16_eor(INT run_number){ return SUCCESS; }

extern HNDLE hDB;                     /* Odb Handle */
MDPP16_ANALYSER_PARAMETERS ana_param;                    /* Odb settings */
MDPP16_ANALYSER_PARAMETERS_STR(ana_param_str);     /* Book Setting space */

int mdpp16_init(void)
{
  char odb_str[128], errmsg[128];
  char set_str[80];
  int size, status;
  HNDLE hSet;

  size = sizeof(MDPP16_ANALYSER_PARAMETERS);

  sprintf(set_str, "/mdpp16_analyser/Parameters");
  status = db_create_record(hDB, 0, set_str, strcomb(ana_param_str));
  if( (status = db_find_key (hDB, 0, set_str, &hSet)) != DB_SUCCESS ){
    cm_msg(MINFO,"FE","Key %s not found", set_str);
  }
  if ((status =db_open_record(hDB, hSet, &ana_param, size, /* enable link */
			      MODE_READ, NULL, NULL)) != DB_SUCCESS ){
    cm_msg(MINFO,"FE","Failed to enable ana param hotlink", set_str);
  }

  hist_init_roody();
  return SUCCESS;
}



int hist_init_roody()
{
   char name[256];
   int i;

   sprintf(name,  "Hitpattern");  
   hHitPat = H1_BOOK(name, name, HIT_CHAN, 0, HIT_CHAN );

   open_subfolder("Calibrated Energy");
   for(i=0; i<NUM_CHAN; i++){
     // Scaled histogram based on ODB analyzer parameters
     sprintf(name, "First Calibrated Energy %d", i);
     hCalEnergy1[i] = H1_BOOK(name, name, ADC_CHAN, 0, (ADC_CHAN-ana_param.peak1_channel)*(ana_param.peak2_energy - ana_param.peak1_energy)/(ana_param.peak2_channel - ana_param.peak1_channel )+ana_param.peak1_energy );
   }
   close_subfolder();
   
   open_subfolder("TDC time difference");
   for(i=0; i<NUM_CHAN; i++){
     sprintf(name, "TDC time difference %d", i);
     hTDC[i] = h1_book<TH1F>(name, name, ADC_CHAN, 0, ADC_CHAN);
   }
   close_subfolder();

   open_subfolder("Count Rates");
   for(i=0; i<NUM_CHAN; i++){
     sprintf(name, "Count Rate %d", i);
     hRate[i] = h1_book<TH1F>(name, name, ADC_CHAN, 0, ADC_CHAN);
   }
   close_subfolder();

   open_subfolder("Energy vs timestamp colormaps");
   for(i=0; i<NUM_CHAN; i++){
     sprintf(name, "Energy vs timestamp %d", i);
     hEnergy_vs_ts[i] = h2_book<TH2F>(name, name, ADC_CHAN, 0, ADC_CHAN, 70, 0, 70);
   }
   close_subfolder();
   
   open_subfolder("raw");
   for(i=0; i<NUM_CHAN; i++){
     // H1_BOOK(name, name, num, min, max )
      sprintf(name,  "ADC channel%d", i);  
      hEnergy[i] = H1_BOOK(name, name, ADC_CHAN, 0, ADC_CHAN );

      sprintf(name, "ADC chan, flags removed %d", i);
      hEnergy_flagsrm[i] = H1_BOOK(name, name, ADC_CHAN, 0, ADC_CHAN );
   }
   close_subfolder();
   return(0);
}

int mdpp16_event(EVENT_HEADER *pheader, void *pevent)
{
  /* BeginTime needs to be global? startTime should be set to 0 at the beginning of each event? and then
something something...  */
   int i, bank_len, err = 0;
   DWORD *data;

   int hsig, subhead, mod_id, tdc_res, adc_res, nword;
   int dsig, fix, flags, t, chan, evdata;
   uint32_t ts; // needed for 30-bit ts
   int esig, counter;
   int evadcdata, evtdcdata, evrstdata, extts, trigchan;
   static int evcount;

   /* Added these to give a time interval to accrue counts. Current bugs:
      - drop in count rate at regular intervals. For INTERVAL=5, counts drop every 5 bins... odd
          DOES depend on interval and can start in the middle of the 5 bin set
      - does not reset to 0 between runs, requires analyzer be reset.
    */
   static time_t startTime, beginTime;
   time_t currentTime = time(NULL); 
   static int rates[NUM_CHAN];
   float cal_energy;
   
   // Added this chunk for the count rate vs time histogram. startTime is run, beginTime is interval
   if( startTime == 0 ) {
     startTime=beginTime=currentTime; } // equivalent to beginTime=currentTime; startTime=beginTime;
   if( currentTime - startTime > ana_param.update_interval ){
       for (i=0; i<NUM_CHAN; i++){
	 // If enough time elapsed, populate hRate at beginning of event with previous values
	 hRate[i] -> SetBinContent((currentTime-beginTime)/ana_param.update_interval, rates[i]);
	 rates[i]=0;

          }
       startTime=currentTime; }

   // bank_len defined here. bk_locate(event,name,pdata) finds "MDPP" in event and returns bank length
   if( (bank_len = bk_locate(pevent, "MDPP", &data) ) == 0 ){ return(0); }
   ++evcount;
   if( debug ){
      printf("Event Dump ...\n");
      for(i=0; i<bank_len; i+=4){
         printf("   word[%d] = 0x%08x    0x%08x    ", i, data[i], data[i+1]);
         printf("0x%08x    0x%08x    \n", data[i+2], data[i+3] );
      }
   }
   
   // hsig 2 + subhead 2 + xxxx + mod_id 8 + tdc_res 3 + adc_res 3 + num of following 4byte words incl EOE 10
   hsig    = (data[0]>>30) & 0x3;
   subhead = (data[0]>>24) & 0x3F;
   mod_id  = (data[0]>>16) & 0xFF;
   tdc_res = (data[0]>>13) & 0x7;
   adc_res = (data[0]>>10) & 0x7;
   nword   = (data[0]>> 0) & 0x3FF;

   if( debug ){
     printf("Header ...\n");
     printf("   hsig  =%d    subheader=%d    mod_id=%d\n",hsig,subhead,mod_id);
     printf("   tdc_res=%d    adc_res=%d    nword=%d\n",tdc_res,adc_res,nword);
   }
   
   for(i=1; i<bank_len; i++){ // covers both ADC and TDC event words
     /* Highest 4 bits:
           0100 for header
           0001 for data event
           0010 for ext ts
           11   for EOE mark (30-bit ts)
           0000 for fill dummy */
      dsig    = (data[i] >> 30) & 0x3;
      fix     = (data[i] >> 28) & 0x3;
      
      // DATA word for TDC, ADC or reset event. 0b0001
      if ( ( (data[i]>>28) & 0xF) == 1 ){

	flags    = (data[i] >> 22) & 0x3; // pileup or overflow/underflow
	trigchan = (data[i] >> 16) & 0x3F; // All 6 bits for determining ADC, TDC or trig0/trig1 (reset)

	if ( trigchan == 33 ){ // Reset event is 33 on 6 bits: 100001
	   evrstdata = (data[i]>>0) & 0xF; // channel index, 4 bits
	   hHitPat->Fill(evrstdata); // Fill on reset events
	}
	 
	 // hHitPat->Fill( chan );

	else if (     trigchan < 16 ){ // ADC value caught
	   chan      = (data[i] >> 16) & 0x1F;
	   evadcdata = (data[i] >> 0 ) & 0xFFFF;
	 }
	else if (trigchan < 32){ // TDC time difference caught if above 16 and less than 32
	   evtdcdata = (data[i] >> 0 ) & 0xFFFF;
	 }	 
      }

      // Extended timestamp word. If dsig+fix==0010, ext ts caught
      if ( ((data[i]>>28) & 0xF) == 2 ){
	extts = (data[i] >> 0) & 0xFFFF;
      }

      // EOE marker, event counter / timestamp. If dsig == 0b11, ts caught
      if ( dsig ==3 ){
	ts =  ((data[i] >> 0) & 0x3FFFFFFF); // concatenate 14 bits and 16 bits...
      }
      
      // unused                 0xF  // xxxx   0000 for reset
      /* flags   = (data[i]>>22) & 0x3; //  pileup/overrange */
      /* t       = (data[i]>>21) & 0x1; // if 1, chan>=16,  means data word has TDC */
      /* chan    = (data[i]>>16) & 0x1F;//             reset if chan=33 */
      /* evdata  = (data[i]>> 0) & 0xFFFF; // 16bits of TDC or ADC */
      
      
      /* if( debug ){ */
      /*    printf("Data[%d] ...\n", i); */
      /*    printf("   dsig  =%d    fix=%d    flags=%d\n", dsig, fix, flags); */
      /*    printf("   T=%d    chan=%d    data=%d\n", t, chan, evdata); */
      /* } */
      /* chan += 32*t; */


      /* hHitPat->Fill( chan ); //Mesytec uses chan >16 to store TDC time difference */


      // ADC value fills energy histograms
      /* if(      chan < 16 ){ */
      /* 	hEnergy[chan   ]->Fill(evdata); // raw */
      /* 	if(   flags==0){ */
      /* 	  hEnergy_flagsrm[chan]->Fill(evdata); // fill non flagged events */
      /* 	} */

      /*   //cal_energy = evdata*ana_param.peak1_channel+ana_param.peak1_energy; */
      /* 	cal_energy = (evdata-ana_param.peak1_channel)*(ana_param.peak2_energy-ana_param.peak1_energy)/(ana_param.peak2_channel - ana_param.peak1_channel )+ ana_param.peak1_energy; */

      /*   hCalEnergy1[chan]->Fill(cal_energy); // first calibrated */
      /* } */

      // TDC timing difference fills hTime histograms
      /* else if( chan < 32 ){  */
      /* 	hTime  [chan-16]->Fill(evdata);  */
      /* } */
      
      
      /* if( dsig != 0 || fix != 1 ){ err=1; } */
      /* if( flags != 0 ){ */
      /* 	 // printf("Flags=%d: event %d\n", flags, evcount); */
      /* } */

      // Section where we populate rates . 
      /* if ( chan >= 0 && chan < NUM_CHAN ){ */
      /* 	if ( evdata < ana_param.HIGHCHANNEL && evdata > ana_param.LOWCHANNEL ){ */
      /* 	    	rates[chan]++; */
      /* 	  } */
      /* 	} */

      /* if(  dsig==3  ){ // If EOE marker, grab 30 low bits of timestamp */
      /* 	ts_lo=(data[i]) & 0x1E; */
      /* 	if (chan != 33) */
      /* } */
   }

   // After full event is read, fill histograms
   //
   //

   //hEnergy          [chan]-> Fill(evadcdata); // raw ADC
   if (flags==0){
     hEnergy_flagsrm[chan]-> Fill(evadcdata); // raw ADC, flags removed
   }
   hEnergy_vs_ts    [chan]-> Fill(evadcdata, ts/16000000);
   hTDC             [chan]-> Fill(evtdcdata); // TDC value (event time after window opened)
   //hCalEnergy1      [chan]->Fill((evadcdata-ana_param.peak1_channel)*(ana_param.peak2_energy-ana_param.peak1_energy)/(ana_param.peak2_channel - ana_param.peak1_channel )+ ana_param.peak1_energy);
   

   //hHitPat[chan   ]-> Fill(chan);
   /* if (flags==0){ // Fill ADC data only when no flags are on */
   /*   hEnergy_flagsrm[chan]-> Fill(evadcdata); */
   /* } */
   
   
   esig    = (data[i]>>30) & 0x3;
   counter = (data[i]>> 0) & 0x3FFFFFFF; // low 30bits
   if( debug ){
      printf("Trailer ...\n");
      printf("   esig  =%d    counter=%d\n", esig, counter);
      printf("\n");
   }

   if( hsig != 1 || esig != 3 || t != 0 || subhead != 0 || mod_id != 1 ){
      err = 1;
   }
   //if( err == 1 ){ printf("Error: event %d\n", evcount); }
   return(0);
}

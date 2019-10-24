#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include "midas.h"
#include "mvmestd.h"
#include "experim.h"
#include "vmicvme.h"

#include "mdpp16.h"

/*-- Golbal declarations -------------------------------------------*/
char *frontend_name      = "feMdpp16";     /* Client name seen by MIDAS clients */
char *frontend_file_name = __FILE__;                    /* Don't change this */
BOOL frontend_call_loop  = FALSE; /* call frontend_loop periodically if TRUE */
INT display_period       = 0;   /* Frontend status page update interval [ms] */
INT max_event_size       = 524288;      /* max event size from this frontend */
INT max_event_size_frag  = 5 * 1024*1024; /* max event fragmented event size */
INT event_buffer_size = 4 * 800000;            /* buffer size to hold events */

#define VME_DMA_BUFSIZE 0x80000   // 0.5MByte
#define EVT_BUFSIZE     0x800000  // 8MByte - we don't check for overwriting
                                  //  this buffer - so make large enough
                                  // for several DMA buffers of data
#define MDPP16_NUMCHAN 16
#define MDPP16_BASE     0x00100000   /* set by jumpers on board */
const int mdpp16_addr = MDPP16_BASE; /* VME base address */
int   hVme;                           /* Vme handle */
extern HNDLE hDB;                     /* Odb Handle */
unsigned *dma_buffer, dma_bufpos, dma_bufend;
unsigned *evt_buffer, evt_bufpos;
int ierror;

/*-- Function declarations -----------------------------------------*/
INT frontend_init (             ), frontend_exit (             );
INT  begin_of_run ( INT, char * ), end_of_run    ( INT, char * );
INT     pause_run ( INT, char * ), resume_run    ( INT, char * );
INT frontend_loop (             );
INT read_trigger_event ( char *, INT );
INT  read_scaler_event ( char *, INT );
int config_mdpp16 (int module_addr);

BANK_LIST trigger_bank_list[] = {
    {"MDP0", TID_DWORD, 128, NULL},
    {""},
};

/*-- Equipment list ------------------------------------------------*/
#undef USE_INT
EQUIPMENT equipment[] = {

    {"MDPP16",               /* equipment name */
     {1, 0,                   /* event ID, trigger mask */
      "SYSTEM",               /* event buffer */
#ifdef USE_INT
      EQ_INTERRUPT,           /* equipment type */
#else
      EQ_POLLED, //PERIODIC, // POLLED,              /* equipment type */
#endif
      LAM_SOURCE(0, 0x0),     /* event source crate 0, all stations */
      "MIDAS",                /* format */
      TRUE,                   /* enabled */
      RO_RUNNING,          /* read only when running */
      500,                    /* poll for 500ms */
      0,                      /* stop run after this event limit */
      0,                      /* number of sub events */
      0,                      /* don't log history */
      "", "", "",},
     read_trigger_event,      /* readout routine */
     NULL, NULL,
     trigger_bank_list,
    },

#ifdef use_scaler
    {"Scaler",                /* equipment name */
     {2, 0,                   /* event ID, trigger mask */
      "SYSTEM",               /* event buffer */
      EQ_PERIODIC ,           /* equipment type */
      0,                      /* event source */
      "MIDAS",                /* format */
      TRUE,                   /* enabled */
      RO_RUNNING | RO_TRANSITIONS |   /* read when running and on transitions */
      RO_ODB,                 /* and update ODB */
      5000,                  /* read every 5 sec */
      0,                      /* stop run after this event limit */
      0,                      /* number of sub events */
      0,                      /* log history */
      "", "", "",},
     read_scaler_event,       /* readout routine */
    },
#endif

    {""}

};

//////////////////////////////////////////////////////////////////////
INT       frontend_exit (                               ){ return SUCCESS; }
//INT          end_of_run (INT run_number, char *error    ){ return SUCCESS; }
INT           pause_run (INT run_number, char *error    ){ return SUCCESS; }
INT          resume_run (INT run_number, char *error    ){ return SUCCESS; }
INT       frontend_loop (                               ){ return SUCCESS; }
INT interrupt_configure ( INT cmd, INT source, PTYPE adr){ return SUCCESS; }

void seq_callback(INT hDB, INT hseq, void *in)
{
    printf("odb ... sis3302 settings touched\n");
}

/////////////////////////////////////////////////////////////////////
// each equipment/XXX/settings odb tree can be saved to experim.h
// experim.h then contains a structure  XXX_SETTINGS
//           also a string definition   XXX_SETTINGS_STR
// the structure can be populated from the odb with db_open_record, 
//    the string is created by the command:  XXX_SETTINGS_STR(NAME);
//    the string is only used to pass to db_create_record
//       which will remake the odb tree if altered/deleted
/////////////////////////////////////////////////////////////////////
MDPP16_SETTINGS mdpp16;                       /* Odb settings */
MDPP16_SETTINGS_STR(mdpp16_settings_str);     /* Book Setting space */

INT frontend_init()
{
    MVME_INTERFACE *vme_handle;
    int size, status;
    char set_str[80];
    HNDLE hSet;

    size = sizeof(MDPP16_SETTINGS);
    
    sprintf(set_str, "/Equipment/MDPP16/Settings");
    status = db_create_record(hDB, 0, set_str, strcomb(mdpp16_settings_str));
    if( (status = db_find_key (hDB, 0, set_str, &hSet)) != DB_SUCCESS ){
     cm_msg(MINFO,"FE","Key %s not found", set_str);
    }
    if ((status =db_open_record(hDB, hSet, &mdpp16, size, /* enable hot-link */
    			       MODE_READ, seq_callback, NULL)) != DB_SUCCESS ){
      cm_msg(MINFO,"FE","Failed to enable ts hotlink", set_str);
    }
    mvme_open(&vme_handle, 0);
    if( vme_handle == NULL ){
	printf("VME init failed\n");
	return -1;
    }
    hVme = (int)vme_handle;
    if( (dma_buffer=(unsigned*)malloc(VME_DMA_BUFSIZE*sizeof(int))) == NULL ){
	printf("VME_DMA_BUF malloc failed\n");
	return -1;
    }
    if( (evt_buffer=(unsigned*)malloc(EVT_BUFSIZE*sizeof(int))) == NULL ){
	printf("EVT_BUF malloc failed\n");
	return -1;
    }

    mdpp16_reset(hVme, mdpp16_addr);
    printf("config_mdpp16: Reset ...\n");
    ss_sleep(500); // 200ms minimum wait

//    if( mdpp16_config() ){ return(-1); }
    return SUCCESS;
}

// there are several ways of reading the mdpp16
// for now, poll datafifo wordcount
INT poll_event(INT source, INT count, BOOL test)
{
    int i, words;
    for(i=0; i<count; i++){
	if( dma_bufpos ){ words=1; } // still data in buffer
	else {
	    words = mdpp16_read_fifo_words(hVme, mdpp16_addr);
	}
	if( words && !test ){ return words; }
    }  
    return 0;
}

void dump_event(int *data, int size)
{
    int i;
    for(i=0; i<size; i++){
	printf(" 0x%08x", data[i] );
	if( ((i+1) % 8 ) == 0 ){ printf("\n    "); }
    }
    if( (i % 8) != 0 ){ printf("\n"); }
}


static int current_chan=0, bank1_flag = 1, bank_done=1;
static int readout_counter=0;
INT read_trigger_event(char *pevent, INT off)
{
    DWORD *pdata;
    int i, words;

    words = mdpp16_read_fifo_words(hVme, mdpp16_addr);
    if( words <= 0 ){ return(0); }
    if( words > max_event_size ){
	printf("read_trigger_event: event-size[%d] > limit[%d]\n",
	       words, max_event_size);
	for(i=0; i<words; i++){
	    // read and discard data (do not inc pdata)
	    vme_A32D32_read(hVme, mdpp16_addr, pdata);
	}
	vme_A32D16_write(hVme, mdpp16_addr+MDPP16_READOUT_RESET,  0);
	return(0);
    }

    bk_init(pevent);                               /* init bank structure */
    bk_create(pevent, "MDPP", TID_DWORD, (void **)&pdata); /* create SCLR bank */
    for(i=0; i<words; i++){
	vme_A32D32_read(hVme, mdpp16_addr, pdata++);
    }
    bk_close(pevent, pdata);                      /* close scaler bank */
    vme_A32D16_write(hVme, mdpp16_addr+MDPP16_READOUT_RESET,  0);
    readout_counter+=1;
    printf("Event: %8d\r", readout_counter);
    return bk_size(pevent);
}

INT read_scaler_event(char *pevent, INT off)
{
    DWORD *pdata;

    bk_init(pevent);                                    /* init bank structure */
    bk_create(pevent, "SCLR", TID_DWORD, (void **)&pdata); /* create SCLR bank */

    pdata[0] = 1;  pdata   += 1; // fake scalar data - no equipment atm
    bk_close(pevent, pdata);                      /* close scaler bank */

    return bk_size(pevent);
}

INT end_of_run(INT run_number, char *error)
{
    printf("Begin of EOR %i\n",run_number);

    mdpp16_acq_stop(hVme, mdpp16_addr);

    printf("End of EOR %i\n",run_number);
    return SUCCESS;
}

INT begin_of_run(INT run_number, char *error) {
    int i,j;

    printf("Begin of BOR %i\n",run_number);

    mdpp16_config(hVme, mdpp16_addr);
    mdpp16_acq_start(hVme, mdpp16_addr);

    printf("End of BOR %i\n",run_number);
    return SUCCESS;
}

void print_return(const char* desc, int val)
{
    printf("Setting >>%s<< returned: %d\n", desc, val);
}


void set_wr(const char* desc, unsigned int offset, unsigned int data)
{
    printf("   setting: %-24s to value: %5d ...", desc, data);
    int retval = vme_A32D16_write(hVme, mdpp16_addr+offset, data);

    unsigned int val = 0;
    vme_A32D16_read(hVme, mdpp16_addr+offset,  &val);
    data != val 
	? printf("NOT succesful; reread value: %d, expected: %d\n", val, data) 
	: printf("successful\n");
}

void set_w(const char* desc, unsigned int offset, unsigned int data)
{
    printf("   setting: %-24s to value: %5d ...", desc, data);
    vme_A32D16_write(hVme, mdpp16_addr+offset, data);
    printf("done\n");
}

int mdpp16_config(int hVme, unsigned mdpp16_addr)
{
    int version = mdpp16_version(hVme, mdpp16_addr);
    if( version != -1 ){
	printf("mdpp16: firmware version=0x%04x\n", version);
    } else {
	printf("mdpp16: not found\n");
	return(-1);
    }
    printf("setting MDPP16 values\n");

    // in multi-event-modes, [48k]buffer read will terminate(berr)
    //    at first end of event after max_datalen
    //    in this case - write READOUT_RESET to allow new read
    // OR can read buffer_datalen(need to read +1?), and read that many words
    //    seem also to need to write RESET for next read
    set_wr("MDPP16_MULTI_EVENT",    MDPP16_MULTI_EVENT,   mdpp16.multi_event);
    set_wr("MDPP16_MAX_DATALEN",    MDPP16_MAX_DATALEN,   mdpp16.max_datalen);
    set_wr("MDPP16_WINDOW_START",   MDPP16_WINDOW_START,  mdpp16.window_start);
    set_wr("MDPP16_WINDOW_WIDTH",   MDPP16_WINDOW_WIDTH,  mdpp16.window_width);
    set_wr("MDPP16_TDC_RESOLUTION", MDPP16_TDC_RESOLUTION,mdpp16.tdc_resln);
    set_wr("MDPP16_ADC_RESOLUTION", MDPP16_ADC_RESOLUTION,mdpp16.adc_resln);
    set_wr("MDPP16_TRIG_SOURCE",    MDPP16_TRIG_SOURCE,   mdpp16.trig_source);
    set_wr("MDPP16_NIM2_INPUT",     MDPP16_NIM2_INPUT,    mdpp16.nim2_input);
    
    printf("setting MDPP16 channel 0 and 1 values ...\n");
    set_w( "MDPP16_CHANPAIR_SELECT",MDPP16_CHANPAIR_SELECT, 0x4);

    set_wr("MDPP16_TF_INT_DIFF",   MDPP16_TF_INT_DIFF,   mdpp16.tf_int_diff);
    set_wr("MDPP16_GAIN",          MDPP16_GAIN,          mdpp16.gain );
    set_wr("MDPP16_THRESHOLD_CH0", MDPP16_THRESHOLD_CH0, mdpp16.threshold0 );
    set_wr("MDPP16_THRESHOLD_CH1", MDPP16_THRESHOLD_CH1, mdpp16.threshold1 );
    set_wr("MDPP16_SHAPING_TIME",  MDPP16_SHAPING_TIME,  mdpp16.shaping_time );
    set_wr("MDPP16_BLR",           MDPP16_BLR,           mdpp16.blr );
    set_wr("MDPP16_RESET_TIME",    MDPP16_RESET_TIME,    mdpp16.reset_time );
    set_wr("MDPP16_RISE_TIME",     MDPP16_RISE_TIME,     mdpp16.rise_time );


    set_wr("MDPP16_PULSER_CONTROL",MDPP16_PULSER_CONTROL,  mdpp16.pulser_ctrl);
    set_wr("MDPP16_PULSER_AMP",    MDPP16_PULSER_AMPLITUDE,mdpp16.pulser_amp);
    set_wr("MDPP16_MARK_TYPE",     MDPP16_MARK_TYPE,       mdpp16.mark_type );
    set_wr("MDPP16_TRIG_OUTPUT",   MDPP16_TRIG_OUTPUT,     mdpp16.trig_output);
    set_wr("MDPP16_TS_SOURCE",     MDPP16_TS_SOURCE,       mdpp16.ts_source );
    set_wr("MDPP16_TRIG_OUTPUT",   MDPP16_TRIG_OUTPUT,     mdpp16.trig_output);

    printf("config_mdpp16: done\n");
    return(0);
}


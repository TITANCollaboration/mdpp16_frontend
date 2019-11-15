/********************************************************************\

  Name:         experim.h
  Created by:   ODBedit program

  Contents:     This file contains C structures for the "Experiment"
                tree in the ODB and the "/Analyzer/Parameters" tree.

                Additionally, it contains the "Settings" subtree for
                all items listed under "/Equipment" as well as their
                event definition.

                It can be used by the frontend and analyzer to work
                with these information.

                All C structures are accompanied with a string represen-
                tation which can be used in the db_create_record function
                to setup an ODB structure which matches the C structure.

  Created on:   Fri Oct 26 15:04:21 2018

\********************************************************************/

#define MDPP16_COMMON_DEFINED

typedef struct {
  WORD      event_id;
  WORD      trigger_mask;
  char      buffer[32];
  INT       type;
  INT       source;
  char      format[8];
  BOOL      enabled;
  INT       read_on;
  INT       period;
  double    event_limit;
  DWORD     num_subevents;
  INT       log_history;
  char      frontend_host[32];
  char      frontend_name[32];
  char      frontend_file_name[256];
  char      status[256];
  char      status_color[32];
  BOOL      hidden;
  INT       write_cache_size;
} MDPP16_COMMON;

#define MDPP16_COMMON_STR(_name) const char *_name[] = {\
"[.]",\
"Event ID = WORD : 400",\
"Trigger mask = WORD : 0",\
"Buffer = STRING : [32] SYSTEM",\
"Type = INT : 2",\
"Source = INT : 0",\
"Format = STRING : [8] MIDAS",\
"Enabled = BOOL : y",\
"Read on = INT : 1",\
"Period = INT : 500",\
"Event limit = DOUBLE : 0",\
"Num subevents = DWORD : 0",\
"Log history = INT : 0",\
"Frontend host = STRING : [32] lxebit2.triumf.ca",\
"Frontend name = STRING : [32] feMdpp16",\
"Frontend file name = STRING : [256] femdpp.c",\
"Status = STRING : [256] feMdpp16@lxebit2.triumf.ca",\
"Status color = STRING : [32] greenLight",\
"Hidden = BOOL : n",\
"Write cache size = INT : 100000",\
"",\
NULL }

#define MDPP16_SETTINGS_DEFINED

typedef struct {
  DWORD     multi_event;
  DWORD     max_datalen;
  DWORD     window_width;
  DWORD     window_start;
  DWORD     tdc_resln;
  DWORD     adc_resln;
  DWORD     trig_source;
  DWORD     nim2_input;
  DWORD     pulser_ctrl;
  DWORD     pulser_amp;
  DWORD     mark_type;
  DWORD     trig_output;
  DWORD     ts_source;
  char      temp[17];
  DWORD     tf_int_diff;
  DWORD     gain;
  DWORD     threshold0;
  DWORD     threshold1;
  DWORD     shaping_time;
  DWORD     blr;
  DWORD     reset_time;
  DWORD     rise_time;
} MDPP16_SETTINGS;

#define MDPP16_SETTINGS_STR(_name) const char *_name[] = {\
"[.]",\
"multi_event = DWORD : 0",\
"max_datalen = DWORD : 0",\
"window_width = DWORD : 512",\
"window_start = DWORD : 16128",\
"tdc_resln = DWORD : 2",\
"adc_resln = DWORD : 0",\
"trig_source = DWORD : 256",\
"nim2_input = DWORD : 2",\
"pulser_ctrl = DWORD : 0",\
"pulser_amp = DWORD : 400",\
"mark_type = DWORD : 1",\
"trig_output = DWORD : 256",\
"ts_source = DWORD : 0",\
"temp = STRING : [17] channel settings",\
"tf_int_diff = DWORD : 20",\
"gain = DWORD : 218",\
"threshold0 = DWORD : 112",\
"threshold1 = DWORD : 112",\
"shaping_time = DWORD : 240",\
"blr = DWORD : 1",\
"reset_time = DWORD : 4000",\
"rise_time = DWORD : 80",\
"",\
NULL }

//#endif

//#ifndef EXCL_MDPP16_ANALYSER

#define MDPP16_ANALYSER_COMMON_DEFINED

typedef struct {
  WORD      event_id;
  WORD      trigger_mask;
  char      buffer[32];
  INT       type;
  INT       source;
  char      format[8];
  BOOL      enabled;
  INT       read_on;
  INT       period;
  double    event_limit;
  DWORD     num_subevents;
  INT       log_history;
  char      frontend_host[32];
  char      frontend_name[32];
  char      frontend_file_name[256];
  char      status[256];
  char      status_color[32];
  BOOL      hidden;
  INT       write_cache_size;
} MDPP16_ANALYSER_COMMON;

#define MDPP16_ANALYSER_COMMON_STR(_name) const char *_name[] = {\
"[.]",\
"Event ID = WORD : 0",\
"Trigger mask = WORD : 0",\
"Buffer = STRING : [32] SYSTEM",\
"Type = INT : 0",\
"Source = INT : 0",\
"Format = STRING : [8] MIDAS",\
"Enabled = BOOL : n",\
"Read on = INT : 0",\
"Period = INT : 0",\
"Event limit = DOUBLE : 0",\
"Num subevents = DWORD : 0",\
"Log history = INT : 0",\
"Frontend host = STRING : [32] ",\
"Frontend name = STRING : [32] ",\
"Frontend file name = STRING : [256] ",\
"Status = STRING : [256] ",\
"Status color = STRING : [32] ",\
"Hidden = BOOL : n",\
"Write cache size = INT : 100000",\
"",\
NULL }
#define MDPP16_ANALYSER_PARAMETERS_DEFINED

typedef struct {
  float      peak1_channel;
  float      peak1_energy;
  float      peak2_channel;
  float      peak2_energy;
  INT        LOWCHANNEL;
  INT        HIGHCHANNEL;
  float      update_interval;
} MDPP16_ANALYSER_PARAMETERS;

#define MDPP16_ANALYSER_PARAMETERS_STR(_name) const char *_name[] = {\
"[.]",\
"Peak1_Channel = FLOAT : 7000.0",\
"Peak1_Energy = FLOAT : 81.0",\
"Peak2_Channel = FLOAT : 30000.0",\
"Peak2_Energy = FLOAT : 356.0",\
"LOWCHANNEL = INT : 100",\
"HIGHCHANNEL = INT : 10000",\
"update_interval = FLOAT : 1.0",\
"",\
NULL }

//#endif


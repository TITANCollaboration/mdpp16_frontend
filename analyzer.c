#include <stdio.h>
#include <string.h>
#include <time.h>
#include <iostream>

#include "midas.h"
#include "experim.h"

RUNINFO runinfo;
time_t tBOR=0;

// EXP_PARAM exp_param;

/* The analyzer name (client name) as seen by other MIDAS clients   */
const char *analyzer_name = "mdpp16_analyser";

/* analyzer_loop is called with this interval in ms (0 to disable)  */
INT analyzer_loop_period = 0;

/* default ODB size */
INT odb_size = DEFAULT_ODB_SIZE;

/*-- Module declarations -------------------------------------------*/
extern ANA_MODULE mdpp16_module;


ANA_MODULE *mdpp16_module2[] = {
   &mdpp16_module,
   NULL
};


/*-- Bank definitions ----------------------------------------------*/
BANK_LIST ana_mdpp16_bank_list[] = {
  {"MDPP", TID_DWORD, 256, NULL},
  {""} ,
};
/*-- Event request list --------------------------------------------*/


ANALYZE_REQUEST analyze_request[] = {
   {"mdpp16_analyser",          /* equipment name */
    {1,                         /* event ID */
     TRIGGER_ALL,               /* trigger mask */
     GET_NONBLOCKING,                  /* get some events */
     "SYSTEM",                  /* event buffer */
     TRUE,                      /* enabled */
     "", "",}
    ,
    NULL,                       /* analyzer routine */
    mdpp16_module2,             /* module list */
    ana_mdpp16_bank_list,      /* bank list */
    1000,                       /* RWNT buffer size */
    TRUE,                       /* Use tests for this event */
   },
   {""},
};


/*--=================-------------------------------------------------*/

INT analyzer_exit(){ return CM_SUCCESS; }
//INT ana_begin_of_run(INT run_number, char *error){ return CM_SUCCESS; }
INT ana_pause_run(INT run_number, char *error){ return CM_SUCCESS; }
INT ana_resume_run(INT run_number, char *error){ return CM_SUCCESS; }
INT analyzer_loop(){ return CM_SUCCESS; }
// INT ana_end_of_run(INT run_number, char *error){ return CM_SUCCESS; }
INT analyzer_init(){ return SUCCESS; }



INT ana_begin_of_run(INT run_number, char *error) {
  HNDLE hDB, hKey;
  //char str[80];
  RUNINFO_STR(runinfo_str);
  time(&tBOR);

  cm_get_experiment_database(&hDB, NULL);
  db_create_record(hDB, 0, "/Runinfo", strcomb(runinfo_str));
  db_find_key(hDB, 0, "/Runinfo", &hKey);
  if (db_open_record(hDB, hKey, &runinfo, sizeof(runinfo), MODE_READ, NULL, NULL) !=
      DB_SUCCESS) {
    cm_msg(MERROR, "analyzer_init", "Cannot open \"/Runinfo\" tree in ODB");
    return 0;
  }

  return SUCCESS;
}



INT ana_end_of_run(INT run_number, char *error)
{
  FILE *f;
  time_t now;
  char str[256];
  int size;
  //double n;
  HNDLE hDB;
  BOOL flag;
  double tdiff;
  std::cout << "ana_end_of_run()" << std::endl;
  cm_get_experiment_database(&hDB, NULL);

  /* update run log if run was written and running online */

  size = sizeof(flag);
  db_get_value(hDB, 0, "/Logger/Write data", &flag, &size, TID_BOOL, TRUE);
  if (flag && runinfo.online_mode == 1) {
    /* update run log */
    size = sizeof(str);
    str[0] = 0;
    db_get_value(hDB, 0, "/Logger/Data Dir", str, &size, TID_STRING, TRUE);
    if (str[0] != 0)
      if (str[strlen(str) - 1] != DIR_SEPARATOR)
	strcat(str, DIR_SEPARATOR_STR);
    strcat(str, "runlog.txt");

    f = fopen(str, "a");

    time(&now);
    strcpy(str, ctime(&now));
    str[10] = 0;

    fprintf(f, "%s\t%5d\t", str, runinfo.run_number);

    strcpy(str, runinfo.start_time);
    str[19] = 0;
    fprintf(f, "%s\t", str + 11);

    strcpy(str, ctime(&now));
    str[19] = 0;
    fprintf(f, "%s\t", str + 11);
    tdiff=difftime(now,tBOR);
    std::cout << "delta " << tdiff << std::endl;

    str[10] = 0;
    fprintf(f, "\t%f\t", tdiff);

    fprintf(f,"\n");
    /*
    size = sizeof(n);
    db_get_value(hDB, 0, "/Equipment/Trigger/Statistics/Events sent", &n, &size,
		 TID_DOUBLE, TRUE);

    fprintf(f, "%5.1lfk\t", n / 1000);
    fprintf(f, "%s\n", exp_param.comment);
    */
    fclose(f);
    std::cout << "wrote runlog "<<str<<" for run number" << runinfo.run_number << std::endl;
    cm_msg(MINFO, "analyzer_eor", "wrote run info");
  }
  else {
    cm_msg(MINFO, "analyzer_eor", "not writing runlog for some reason...");
  }
  return CM_SUCCESS;
}

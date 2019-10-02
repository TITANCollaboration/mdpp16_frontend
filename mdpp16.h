/* MDPP16 Registers */
#define MDPP16_DATA_FIFO         0x0000 // D32/64 R/W
// The following registers are all D16  //#Bits  Dir  Default
#define MDPP16_ADDR_SRC          0x6000 //  1    R/W   0[board-switches]
#define MDPP16_ADDR_REG          0x6002 // 16    R/W   0
#define MDPP16_MODULE_ID         0x6004 //  8    R/W   0xFF=>baseaddr[31:24]
#define MDPP16_SOFT_RESET        0x6008 //  1      W   *takes 200ms*
#define MDPP16_FIRMWARE_VSN      0x600E // 16    R     0x1012
//                                      //#Bits  Dir  Default
#define MDPP16_IRQ_LEVEL         0x6010 //  3    R/W   0[off]
#define MDPP16_IRQ_VECTOR        0x6012 //  8    R/W   0
#define MDPP16_IRQ_TEST          0x6014 //         W
#define MDPP16_IRQ_RESET         0x6016 //         W
#define MDPP16_IRQ_DATA_THR      0x6018 // 15    R/W   1
#define MDPP16_MAX_DATALEN       0x601A // 15    R/W   1 [0=>unlimited]
#define MDPP16_IRQ_SOURCE        0x601C //  1    R/W   1[data] 0=>event
#define MDPP16_IRQ_EVENT_THR     0x601E // 15    R/W   1 [0=>unlimited]
//                                      //#Bits  Dir  Default
#define MDPP16_CBLT_MCST_CTRL    0x6020 //  8    R/W   0 multicast/chained xfr
#define MDPP16_CBLT_ADDR         0x6022 //  8    R/W   0xAA
#define MDPP16_MCST_ADDR         0x6024 //  8    R     0xBB
//                                      //#Bits  Dir  Default
#define MDPP16_BUFFER_DATALEN    0x6030 // 16    R      data currently in buf
#define MDPP16_DATALEN_FORMAT    0x6032 //  2    R/W   2[32bit]
#define MDPP16_READOUT_RESET     0x6034 //         W
#define MDPP16_MULTI_EVENT       0x6036 //  4    R/W   0[single events]
#define MDPP16_MARK_TYPE         0x6038 //  2    R/W   0[#evt] 1=ts 3=ext-ts
#define MDPP16_START_ACQ         0x603A //  1    R/W   1
#define MDPP16_FIFO_RESET        0x603C //         W   
#define MDPP16_DATA_READY        0x603E //  1    R     1=>Data-Avail
//                                      //#Bits  Dir  Default
#define MDPP16_TDC_RESOLUTION    0x6042 //  3    R/W   5 [0-5:25ns/2^10-x)
#define MDPP16_OUTPUT_FORMAT     0x6044 //  2    R/W   0:Q+T 1:Q 2:T
#define MDPP16_ADC_RESOLUTION    0x6046 //  3    R/W   4  0:16 ... 4:12
#define MDPP16_WINDOW_START      0x6050 // 15    R/W   0x4000=0  unit=25ns/16
#define MDPP16_WINDOW_WIDTH      0x6054 // 14    R/W   32        same units
#define MDPP16_TRIG_SOURCE       0x6058 // 10    R/W   0x100 bit fields
//#define MDPP16_TRIG_SOURCE2       0x605A   Not in RCP firmware
#define MDPP16_FIRST_HIT         0x605C //  1    R/W   1=>only 1st hit
#define MDPP16_TRIG_OUTPUT       0x605E // 10    R/W   0x100 bit fields
//                                      //#Bits  Dir  Default
#define MDPP16_ECL3_INPUT        0x6060 //  8    R/W   0x00 term/un off/on
#define MDPP16_ECL2_INPUT        0x6062 //  8    R/W   0x00
#define MDPP16_ECL1_INPUT        0x6064 //  8    R/W   0x00
#define MDPP16_ECL0_OUTPUT       0x6066 //  4    R/W   0
#define MDPP16_NIM4_INPUT        0x6068 //  2    R/W   1[trig0_in]
#define MDPP16_NIM3_INPUT        0x606A //  2    R/W   1[off] [2=sync_in]
#define MDPP16_NIM2_INPUT        0x606C //  2    R/W   1[trig_in] [2=reset]
#define MDPP16_NIM1_OUTPUT       0x606E //             always trig_out       
//                                      //#Bits  Dir  Default
#define MDPP16_PULSER_CONTROL    0x6070 //  1    R/W   0[off]
#define MDPP16_PULSER_AMPLITUDE  0x6072 // 12    R/W   400
#define MDPP16_NIM0_OUTPUT       0x6074 //  4    R/W   1 [Cbus/busy/thr_out]
#define MDPP16_MONITOR_CONTROL   0x607A //  1    R/W   0 monitor_on
#define MDPP16_MONITOR_CHAN      0x607C //  4    R/W   0 monitor #Chan
#define MDPP16_MONITOR_WAVE      0x607E //  2    R/W   0 monitor #Wavefm
// registers 0x6080-608A are for mesytec remote-control bus which we dont use
// 32/48bit counters below are latched when reading low word - read it first!
//                                      //#Bits  Dir  Default
#define MDPP16_CTR_AB_RESET      0x6090 //  2    R/W   0
#define MDPP16_EVENT_COUNT_LOW   0x6092 // 16    R/W   0 #events-loword
#define MDPP16_EVENT_COUNT_HIGH  0x6094 // 16    R/W   0 #events-hiword
#define MDPP16_TS_SOURCE         0x6096 //  5    R/W   0[vme] [1:ext]
#define MDPP16_TS_DIVISOR        0x6098 // 16    R/W   1 [0=>65536]
#define MDPP16_TIMESTAMP_LOW     0x609C // 16    R       Timestamp-loword 
#define MDPP16_TIMESTAMP_HIGH    0x609E // 16    R       Timestamp-hiword 
#define MDPP16_TIME48_LOW        0x60A8 // 16    R/W     48bit, 1us units
#define MDPP16_TIME48_MED        0x60AA // 16    R/W     48bit, 1us units
#define MDPP16_TIME48_HIGH       0x60AC // 16    R/W     48bit, 1us units
#define MDPP16_STOP_COUNT        0x60AE //  2    R/W   0 0=go,1=stop, 2bits:b,a
#define MDPP16_CHAN_MULT_HI      0x60B0 //  8    R/W   255 to accept events
#define MDPP16_CHAN_MULT_LO      0x60B2 //  8    R/W   0   lolim<=#chan<=#hilim
// all but threshold and PZ are common for two channels .....
//      channel addressing ...          //#Bits  Dir  Default
#define MDPP16_CHANPAIR_SELECT   0x6100 //  4    R/W   8[all] [0=0,1:7=15,16]
#define MDPP16_TF_INT_DIFF       0x6110 // 14    R/W  20
#define MDPP16_GAIN              0x611A // 16    R/W  2000[=20] 1-25000
#define MDPP16_THRESHOLD_CH0     0x611C // 16    R/W  0xFF      0-64k
#define MDPP16_THRESHOLD_CH1     0x611E // 16    R/W  0xFF      0-64k
#define MDPP16_SHAPING_TIME      0x6124 // 11    R/W  160       4-2000
#define MDPP16_BLR               0x6126 //  2    R/W  2[8*shape] 0=off 1=4*
#define MDPP16_RESET_TIME        0x6128 //       R/W  1000
#define MDPP16_RISE_TIME         0x612A //  7    R/W  20
// PZ0,1 only for SCP firmare

int vme_A32D32_read (int p, unsigned int module_addr, unsigned int *data);
int vme_A32D32_write(int p, unsigned int module_addr, unsigned int  data);
int vme_A32D16_read (int p, unsigned int module_addr, unsigned int *data);
int vme_A32D16_write(int p, unsigned int module_addr, unsigned int  data);
int vme_A32MBLT64_read(int p, unsigned int addr, unsigned int *dest, unsigned int  dma_req_length, unsigned int *dma_length);

///////////////////////      General Registers      ///////////////////////
int mdpp16_reset          (int h, unsigned module_addr);
int mdpp16_version        (int h, unsigned module_addr);
int mdpp16_acq_start      (int h, unsigned module_addr);
int mdpp16_acq_stop       (int h, unsigned module_addr);
int mdpp16_read_fifo_words(int h, unsigned module_addr);

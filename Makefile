CC      = gcc
CXX	= g++
OSFLAGS = -DOS_LINUX -Dextname
CFLAGS  = -g -Wno-deprecated $(OSFLAGS)
FLAGS32 = -m32
LIBS    = -lm -lz -lutil -lnsl -lpthread -lrt

DRV_DIR   = $(MIDASSYS)/drivers/vme
INC_DIR   = $(MIDASSYS)/include
LIB_DIR   = $(MIDASSYS)/linux-m32/lib
LIB64_DIR = $(MIDASSYS)/linux/lib
SRC_DIR   = $(MIDASSYS)/src
VME_DIR   = /home/olchansk/daq/vmisft-7433-3.4-KO1/vme_universe
VME_DIR   = /triumfcs/trshare/olchansk/daq/vmisft-7433-3.4-KO1/vme_universe
#/home1/midascd /homne

CFLAGS += -g -I$(DRV_DIR) -I. -I$(INC_DIR) -I$(DRV_DIR)/vmic -I$(VME_DIR)/include -L$(VME_DIR)/lib

all: femdpp 

femdpp: femdpp.o vmicvme_mdpp.o mdpp16.o $(LIB_DIR)/mfe.o $(LIB_DIR)/libmidas.a 
	$(CC) $(CFLAGS) $(FLAGS32) -o $@ $^ -lvme $(LIBS)

vmicvme_mdpp.o: vmicvme_mdpp.c vmicvme_mdpp.h
	$(CC) $(CFLAGS) $(FLAGS32) -o $@ -c $< 

mdpp16.o: mdpp16.c mdpp16.h
	$(CC) $(CFLAGS) $(FLAGS32) -o $@ -c $< 

fesis.c:    mdpp16.h

ROOTCFLAGS := $(shell  $(ROOTSYS)/bin/root-config --cflags)
ROOTCFLAGS += -DHAVE_ROOT -DUSE_ROOT
ROOTLIBS   := $(shell  $(ROOTSYS)/bin/root-config --libs)
ROOTLIBS   += -lThread

#analyzer: $(LIB64_DIR)/rmana.o $(LIB64_DIR)/libmidas.a analyzer.o anmdpp.o
#	$(CXX) $(CFLAGS) -o $@ $^ $(ROOTLIBS) $(LIBS)

#analyzer.o: analyzer.c
#	$(CXX) $(CFLAGS) $(ROOTCFLAGS) -o $@ -c $<

#anmdpp.o: anmdpp.c
#	$(CXX) $(ROOTCFLAGS) $(CFLAGS) -o $@ -c $<

clean::
	rm -f *.o *~ \#*

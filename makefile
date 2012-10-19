#----------------------------------------------------------------------------
# Author: Brendan Loyot
#----------------------------------------------------------------------------

#variables
REC_NAME=temp_recording.oni
RECORD_STATE=record_false
DEBUG=true

# directory locations
SDIR=./src/
IDIR=./include/
LDIR=./lib/
RDIR=./resources/
EDIR=./exe/

# include directories
OPEN_NI_DIR=/home/brendan/Programming/kinect/OpenNI/
NITE_DIR=/home/brendan/Programming/kinect/NITE-Bin-Dev-Linux-x64-v1.5.2.21/

#includes and libraries
INCLUDE=-I$(OPEN_NI_DIR)Include -I/usr/include/nite
LIB=-L /usr/lib64 -lOpenNI  -lXnVNite_1_4_2 -lglut

#files
SRC=$(SDIR)hand_manipulation.cpp 
DEPS=$(IDIR)hand_manipulation.hpp 
OBJS=hand_manipulation.o 

# compiler
CC=g++
FLAGS= $(INCLUDE) $(LIB)
ifeq ($(DEBUG), true)
FLAGS= $(INCLUDE) $(LIB) -g
endif


build: $(OBJS)
	clear 
	$(CC) $(FLAGS) -o hand_manipulation $(SRC)
	$(CC) $(FLAGS) -o depth_map_viewer depth_map_viewer.cpp
	$(CC) $(FLAGS) -o image_map_viewer image_map_viewer.cpp	
	mv hand_manipulation $(EDIR)

%.o: %.cpp $(DEPS)
	$(CC) $(FLAGS) -c $(SDIR)$(.SOURCE)

$(OBJS): $(IDIR)def.hpp

.phony: run
run:
	$(EDIR)hand_manipulation $(RECORD_STATE)

.phony: debug
debug:
	valgrind --leak-check=yes $(EDIR)hand_manipulation 

.phony: open
open:
	gedit ./src/*.cpp ./makefile ./include/*.hpp

.phony: clean
clean:
	rm ./*~ ./*.o $(SDIR)*~ $(IDIR)*~

.phony: backup
backup:
	cp -r ../hand_manipulation ~/Backups/CSC410ProjectBackups/$(shell date +"%d-%m-%y@%H:%M:%S")

.phony: migrate_recording
migrate_recording:
	mv ./$(REC_NAME) $(RDIR)/recordings/recording$(shell date +"%d-%m-%y@%H:%M:%S").oni
	

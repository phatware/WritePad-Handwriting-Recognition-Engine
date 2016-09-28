################################################################################
#
# WritePad SDK for Linux 
# Handwriting Recognition Engine makefile
# Created by Stan Miasnikov 23/04/2009
#
# Copyright (c) 1997-2014 PhatWare(r) Corp. All rights reserved. 
#
################################################################################
#
#	Unauthorized distribution of this code is prohibited.
#	Contractor/manufacturer is PhatWare Corp.
#	1314 S. Grand Blvd. Ste. 2-175 Spokane, WA 99202
#
################################################################################

# Folders
SRC_FOLDER := src

vpath %.cpp $(SRC_FOLDER)

OUT_FOLDER := obj
LIB_FOLDER := ../Lib

# Commands
RM := rm -rf

CPP := g++

# Compiler flags
CFLAGS := -std=gnu++11 -O3 -Wall -c 

LFLAGS := -g -L $(LIB_FOLDER)

# WARNIGNS := -Wno-char-subscripts \
#	    -Wno-missing-braces \
#	    -Wno-unused-but-set-variable \
#	    -Wno-multichar

INCLUDES := -I"../include"

DEFINES := -D_OS_LINUX

LIBS :=	-lWritePadLib -lpthread 

SRCS := \
	writepadreco.cpp 
	
OBJS = $(patsubst %.cpp,$(OUT_FOLDER)/%.o,$(SRCS)) 

TARGET = writepadreco

.PHONY: depend clean

# All Target
all: $(OUT_FOLDER) $(TARGET)

# Aux folders creation
$(OUT_FOLDER):
	@mkdir -p $@


# Tool invocations
$(TARGET): $(OBJS)
	@echo 'Building target: $@'
	@$(CPP) $(LFLAGS) -o $(TARGET) $(OBJS) $(LIBS)
	@echo 'All done! Created binary:  $@'
	@echo ' '

$(OUT_FOLDER)/%.o : %.cpp
	@echo 'Compiling:  $<'
	@$(CPP) $(CFLAGS) $(WARNIGNS) $(INCLUDES) $(DEFINES) -c $< -o $(OUT_FOLDER)/$(@F)

clean:
	@echo 'Deleting object files'
	@$(RM) $(TARGET) $(OUT_FOLDER) 
	@echo 'Done.'
	@echo ' '

depend: $(SRCS)
	makedepend $(INCLUDES) $^

# DO NOT DELETE THIS LINE -- make depend needs it



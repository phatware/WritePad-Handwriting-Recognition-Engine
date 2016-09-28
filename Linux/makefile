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
SRC_FOLDER := ../UniversalRecognizer/V300

vpath %.cpp $(SRC_FOLDER)/RecognizerWrapper 
vpath %.cpp $(SRC_FOLDER)/InkData
vpath %.cpp $(SRC_FOLDER)/reco/src
vpath %.cpp $(SRC_FOLDER)/reco/data

OUT_FOLDER := Release
LIB_FOLDER := Lib

# Commands
RM := rm -rf

CPP := g++

# Compiler flags
CFLAGS := -std=gnu++11 -O3 -Wall -c 

WARNIGNS := -Wno-char-subscripts \
	    -Wno-missing-braces \
	    -Wno-unused-but-set-variable \
	    -Wno-multichar

INCLUDES := -I"$(SRC_FOLDER)/include" \
	    -I"$(SRC_FOLDER)/InkData" \
	    -I"$(SRC_FOLDER)/reco/include" \
	    -I"$(SRC_FOLDER)/reco/data" \
	    -I"$(SRC_FOLDER)/RecognizerWrapper"

DEFINES := -D_OS_LINUX -DHWR_SYSTEM=HWR_LINUX -D_EMBEDDED_DEVICE -DRECODICT \
	   -DLANG_ENGLISH -DLANG_FRENCH -DLANG_DUTCH -DLANG_SPANISH -DLANG_ITALIAN \
	   -DLANG_GERMAN -DLANG_PORTUGUESE -DLANG_PORTUGUESEB -DLANG_FINNISH \
	   -DLANG_SWED -DLANG_DAN -DLANG_NORW -DLANG_INDONESIAN -DLANG_ENGLISHUK

SRCS := \
	PhatCalc.cpp \
	RecognizerWrapper.cpp \
	WordFile.cpp \
	WordLrnFile.cpp \
	gestures.cpp \
	ImageObject.cpp \
	InkData.cpp \
	InkWrapper.cpp \
	PHStream.cpp \
	PHStroke.cpp \
	ShapesRec.cpp \
	TextObject.cpp \
	UndoAction.cpp \
	angle.cpp \
	apostroph.cpp \
	arcs.cpp \
	bitmaps.cpp \
	breaks.cpp \
	calccell.cpp \
	check.cpp \
	circle.cpp \
	connections.cpp \
	convert.cpp \
	cross.cpp \
	cross_geometry.cpp \
	dct_functions.cpp \
	dtiutil.cpp \
	filter.cpp \
	hwrctype.cpp \
	hwrmath.cpp \
	hwrmem.cpp \
	hwrstr.cpp \
	hwrswap.cpp \
	langutil.cpp \
	ldbutil.cpp \
	letimagestodte.cpp \
	ligstate.cpp \
	link_begin.cpp \
	link_next.cpp \
	links.cpp \
	low3.cpp \
	low_data.cpp \
	low_dbg.cpp \
	low_level.cpp \
	low_util.cpp \
	luspecl.cpp \
	mlp.cpp \
	newdict.cpp \
	param.cpp \
	pict.cpp \
	poly.cpp \
	post_cap_tbl.cpp \
	post_capital.cpp \
	precutil.cpp \
	prep.cpp \
	reco.cpp \
	recodict.cpp \
	recokern.cpp \
	recolibs.cpp \
	recoutil.cpp \
	sketch.cpp \
	spell.cpp \
	stroke.cpp \
	stroke1.cpp \
	tracedata.cpp \
	triadsutil.cpp \
	utils.cpp \
	wordbreaks.cpp \
	wordcorrector.cpp \
	worddict.cpp \
	wordsegment.cpp \
	xrattrib.cpp \
	xrdict.cpp \
	xrmatrix.cpp \
	dti_img.cpp \
	ldb_img.cpp \
	snn_img.cpp \
	trd_img.cpp \
	vprf_img.cpp \
	vsuf_img.cpp 


# OBJS = $(SRCS:.cpp=.o)
# OBJS :=  $(subst ../,,$(SRCS:.cpp=.o))
# OBJS = $(patsubst %.cpp, %.o, $(SRCS))
OBJS = $(patsubst %.cpp,$(OUT_FOLDER)/%.o,$(SRCS)) 

TARGET = $(LIB_FOLDER)/libWritePadLib.a

.PHONY: depend clean

# All Target
all: $(OUT_FOLDER) $(LIB_FOLDER) $(TARGET)

# Aux folders creation
$(OUT_FOLDER):
	@mkdir -p $@

$(LIB_FOLDER):
	@mkdir -p $@

# Tool invocations
$(TARGET): $(OBJS)
	@echo 'Building target: $@'
	@ar -r  "$(TARGET)" $(OBJS) $(LIBS)
	@echo 'All done! Created library:  $@'
	@echo ' '

$(OUT_FOLDER)/%.o : %.cpp
	@echo 'Compiling:  $<'
	@$(CPP) $(CFLAGS) $(WARNIGNS) $(INCLUDES) $(DEFINES) -c $< -o $(OUT_FOLDER)/$(@F)

clean:
	@echo 'Deleting object files'
	@$(RM) $(OUT_FOLDER) $(LIB_FOLDER)/libWritePadLib.a
	@echo 'Done.'
	@echo ' '

depend: $(SRCS)
	makedepend $(INCLUDES) $^

# DO NOT DELETE THIS LINE -- make depend needs it



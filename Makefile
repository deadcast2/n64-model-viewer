include $(ROOT)\usr\include\make\PRdefs

N64KITDIR = c:\nintendo\n64kit
NUSYSINCDIR = $(N64KITDIR)\nusys\include
NUSYSLIBDIR = $(N64KITDIR)\nusys\lib
LIB = $(ROOT)\usr\lib
LPR = $(LIB)\PR
INC = $(ROOT)\usr\include
PROJ64 = Project64
64DRIVEUSB = 64drive_usb

CC = gcc
LD = ld
MAKEROM = mild
LCDEFS = -DNU_DEBUG -DF3DEX_GBI_2
LCINCS = -I. -I$(NUSYSINCDIR) -I$(ROOT)\usr\include\PR
LCOPTS = -G 0
LDFLAGS = $(MKDEPOPT) -L$(LIB) -L$(NUSYSLIBDIR) -lnusys_d -lgultra_d -L$(GCCDIR)\mipse\lib -lkmc
OPTIMIZER =	-g
APP = main.out
TARGETS = main.n64
CODEFILES = main.c
CODEOBJECTS = $(CODEFILES:.c=.o)  $(NUSYSLIBDIR)\nusys.o
DATAOBJECTS = $(DATAFILES:.c=.o)
CODESEGMENT = codesegment.o
OBJECTS = $(CODESEGMENT) $(DATAOBJECTS)

default: $(TARGETS)

include $(COMMONRULES)

.PHONY: run load

run:
	$(PROJ64) $(TARGETS)

load:
	$(64DRIVEUSB) -l $(TARGETS)

$(CODESEGMENT):	$(CODEOBJECTS) Makefile
	$(LD) -o $(CODESEGMENT) -r $(CODEOBJECTS) $(LDFLAGS)

$(TARGETS):	$(OBJECTS)
	$(MAKEROM) spec -s 8 -I$(NUSYSINCDIR) -r $(TARGETS) -e $(APP)
	makemask $(TARGETS) // Allows the rom to be run on real hardware

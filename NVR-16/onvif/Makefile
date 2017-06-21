CROSS	= arm-hisiv100nptl-linux-
#CROSS	= 
CC	= $(CROSS)gcc
CPP	= $(CROSS)g++
LD	= $(CROSS)ld
AR	= $(CROSS)ar
RM	= rm -f
STRIP 	= $(CROSS)strip

LLIB	   = .
IINCLUDE   = .
#CFLAGS += -Wall
#CFLAGS += -s -O2 -g
#CFLAGS += -O2 -g
CFLAGS += -O2
CFLAGS += -march=armv7-a -mcpu=cortex-a9
#CFLAGS += -D_GNU_SOURCE -DLINUX
#CFLAGS += -D__DEBUG__
CFLAGS += -I./
CFLAGS += -I./include/uuid
CFLAGS += -I./include/xml2
CFLAGS += -I./include

MYLIB +=  -L./lib -luuid -lxml2
SYSLIB +=  -lpthread 

.PHONY = clean

TARGET = libyzonvif.so
#COMPILE: = $(CPP) $(CFLAGS)

LDFLAGS += $(MYLIB)
LDFLAGS += $(SYSLIB)






OUT_PATH = .
OBJ_PATH = $(OUT_PATH)

#Project's objs
SRC = $(wildcard *.cpp)
#OBJ = $(patsubst %.cpp,%.o,$(SRC))

OBJS = $(SRC:%.cpp=%.o)

all: $(TARGET)

$(TARGET): $(OBJS)
#	$(CPP)  -o $@ $^ $(LDFLAGS)
	$(CPP) -shared -o $@ $^ $(LDFLAGS)	
	@echo ""
	@echo "All ok! Output:$(TARGET)"
	@echo ""


.c.o:
	$(CC) -fPIC $(CFLAGS) -c $^ -o $@

.cpp.o:
	$(CPP) -fPIC $(CFLAGS) -c $^ -o $@


RM:
	rm -f $(TARGET)
clean:
	rm -f $(TARGET)
	rm -f $(OBJS)



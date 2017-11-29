TARGET = lab6
SRCS = $(wildcard *.cpp)
OBJS = $(subst .cpp,.o,$(SRCS))
HDRS = $(wildcard *.h)

RES = $(wildcard *.rc)
RESC = $(subst .rc,.res,$(RES))

INCLUDES = -I../../../SP/KP/include -I../../Lab4/src
DEFINES = -D_UNICODE -DUNICODE
CFLAGS = -std=c++11 -g

all: ${TARGET}

${TARGET}: ${OBJS} ${RESC}
	g++ -mwindows -o $@ $^

%.o: %.cpp ${HDRS}
	g++ ${DEFINES} ${CFLAGS} ${INCLUDES} -c -o $@ $<

%.res: %.rc resource.h
	windres $< $@ -O coff

run: ${TARGET}
	${TARGET}.exe

clean:
	del *.o
	del *.res

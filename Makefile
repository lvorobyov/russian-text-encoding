TARGET = lab6
SRCS = $(wildcard *.cpp)
OBJS = $(subst .cpp,.o,$(SRCS))
HDRS = $(wildcard *.h)

RES = $(wildcard *.rc)
RESC = $(subst .rc,.res,$(RES))

all: ${TARGET}

${TARGET}: ${OBJS} ${RESC}
	g++ -mwindows -o $@ $^

%.o: %.cpp ${HDRS}
	g++ -D_UNICODE -DUNICODE -std=c++11 -g -c -o $@ $<

%.res: %.rc resource.h
	windres $< $@ -O coff

run: ${TARGET}
	${TARGET}.exe

clean:
	del *.o
	del *.res

CXX := clang++
LDD := g++

TARGET	:= srender
SRC	:= main.cpp Renderer.cpp Line.cpp RenderTarget.cpp Viewport.cpp
OBJS	:= ${SRC:.cpp=.o} 

CCFLAGS := -I/usr/local/include -Wall -O2
LDDFLAGS := -L/usr/local/lib
LIBS :=  -lGLEW -lGL -lGLU -lglut

.PHONY: all clean

all: ${TARGET}

${TARGET}: ${OBJS}
	${LDD} ${LDDFLAGS} -o $@ $^ ${LIBS} 

${OBJS}: %.o: %.cpp 
	${CXX} ${CCFLAGS} -o $@ -c $< 

clean:
	rm -f *~ *.o *.out ${TARGET} 

distclean: clean


CXX := clang++
LDD := g++

TARGET	:= srender
SRC	:= main.cpp Line.cpp Renderer.cpp Framebuffer.cpp Depthbuffer.cpp Viewport.cpp Shader.cpp Plane.cpp Frustum.cpp
OBJS	:= ${SRC:.cpp=.o} 

CCFLAGS := -I/usr/local/include -Wall -O0 -g
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


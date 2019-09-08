CXX := clang++
LDD := clang++

TARGET	:= srender
SRC	:= main.cpp Rasteriser.cpp Framebuffer.cpp Depthbuffer.cpp Viewport.cpp Shader.cpp ShadingGeometry.cpp RenderPrimitive.cpp Geometry.cpp Geometry2.cpp Camera.cpp CubeGeometry.cpp RandomTriangleGeometry.cpp
OBJS	:= ${SRC:.cpp=.o} 

CCFLAGS := -I/usr/local/include -Wall -O0 -g -DGLM_ENABLE_EXPERIMENTAL
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


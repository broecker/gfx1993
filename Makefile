CXX := clang++
LDD := g++

TARGET	:= strikemission
SRC		:= main.cpp Message.cpp Engine.cpp Entity.cpp
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


DIR_INC = ./include
DIR_SRC = ./src
DIR_OBJ = ./obj
DIR_BIN = ./bin

SRC = $(wildcard ${DIR_SRC}/*.cpp)  
OBJ = $(patsubst %.cpp,${DIR_OBJ}/%.o,$(notdir ${SRC})) 

TARGET = an

BIN_TARGET = ${DIR_BIN}/${TARGET}

CC = g++
CFLAGS = -g -Wall -I${DIR_INC}
LDFLAGS = -lyaml

${BIN_TARGET}:${OBJ}
	    $(CC) $(OBJ)  -o $@ $(LDFLAGS)
		    
${DIR_OBJ}/%.o:${DIR_SRC}/%.cpp prereq
	    $(CC) $(CFLAGS) -c  $< -o $@

prereq: 
	mkdir -p obj
	mkdir -p bin

.PHONY:clean
clean:
	find ${DIR_OBJ} -name *.o -exec rm -rf {}

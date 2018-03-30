CC = arm-hisiv400-linux-g++
# CC = g++

DIR_INC = ./include
DIR_SRC = ./src
DIR_OBJ = ./obj
DIR_BIN = ./bin

SRC = $(wildcard ${DIR_SRC}/*.cpp)

OBJ = $(patsubst %.cpp,${DIR_OBJ}/%.o,$(notdir ${SRC} ))

TAEGET = testnetwork

BIN_TAEGET = ${DIR_BIN}/${TAEGET} 

CFLAGS = -g -O2 -Wall -Wno-unused -I${DIR_INC} -lLog-32 -lpthread

${BIN_TAEGET} : ${OBJ}
	@echo 链接生成文件目标文件
	${CC}  ${OBJ}  -o $@ ${CFLAGS}

${DIR_OBJ}/%.o : ${SRC} 
	@echo 编译文件开始..
	$(CC) ${CFLAGS} -c $? 
	-mv *.o $(DIR_OBJ)/
	@echo 编译文件完成


.PHONY : clean run
clean:
	@echo 删除编译结果文件
	-rm -f ${DIR_OBJ}/*.o ${BIN_TAEGET} 

run:
	@echo 执行文件
	${BIN_TAEGET}

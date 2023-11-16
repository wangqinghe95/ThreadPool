
All:testWorkThread

SOURCE_CPP = WorkThread/WorkerThread.cpp test/testWorkThread.cpp ../include/WorkerThread.hpp
CXXFLAG = -std=c++11 -lpthread
INCLUDE = -I ../include

testWorkThread:$(SOURCE_CPP)
	g++ $(SOURCE_CPP) ${CXXFLAG} ${INCLUDE} -o testWorkThread

PRE_CHECK:
	@if [ ! -d $(DIR_OBJ) ];then \
		echo "Folder $(DIR_OBJ) does not exist, creating it..."; \
		mkdir $(DIR_OBJ); \
	else	\
		echo "Folder $(DIR_OBJ) already exists, nothing to do";	\
	fi

.PHONY:clean
clean:
	rm -rf testWorkThread



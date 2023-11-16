threadpool:*.cpp
	g++ *.cpp -std=c++11 -lpthread -fsanitize=address -o $@ 

.PHONY:clean
clean:
	rm -rf threadpool
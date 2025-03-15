build: ./include/* ./src/*
	mkdir -p bin
	gcc -O3 -o ./bin/oh-come-on -I./include ./src/*
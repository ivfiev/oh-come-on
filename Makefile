build: ./include/* ./src/*
	mkdir -p bin
	cc -O2 -o ./bin/oh-come-on -I./include ./src/*

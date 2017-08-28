OUTPUT = TestCWCmdArgsParser
CWUTILS = ../../../../CWUtils/C++/Linux
PREPROCESSOR = -D_CONSOLE


all:help

help:
	@echo make build
	@echo make run

build:
	clear
	clear
	clear
	g++ -o $(OUTPUT) $(PREPROCESSOR) -I$(CWUTILS) $(CWUTILS)/*.cpp *.cpp

run:
	./$(OUTPUT)

CC = gcc  
C++ = g++  
LINK = g++  

LIBS = -L/usr/local/lib -pthread

#must add -fPIC option  
CCFLAGS = $(COMPILER_FLAGS) -c -g  -fPIC -Wall
C++FLAGS = $(COMPILER_FLAGS) -c -g -fPIC -Wall -std=c++11

TARGET=cppco_demo
  
INCLUDES = -I. -I../ -I../libtask


C++FILES = ../libtask/cppco.cpp ../libtask/task_coroutine.cpp ./cppco_demo.cpp

OBJFILE = $(CFILES:.c=.o) $(C++FILES:.cpp=.o)  

all:$(TARGET)  
  
$(TARGET): $(OBJFILE)  
	$(LINK) $^ -Wall $(LIBS) -o $@
 
%.o:%.c  
	$(CC) -o $@ $(CCFLAGS) $< $(INCLUDES)  
  
%.o:%.cpp  
	$(C++) -o $@ $(C++FLAGS) $< $(INCLUDES)  

  
clean:  
	rm -rf $(TARGET)  
	rm -rf $(OBJFILE)  


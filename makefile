cflags	= -Iinclude -std=c++11
target	= pimpl_test $(wildcard test/*.cpp)

include $(PRJMAKE)/makefile


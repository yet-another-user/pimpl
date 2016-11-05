cflags	= -Iinclude -std=c++11
target	= pimpl-test $(wildcard test/*.cpp)

include $(PRJMAKE)/makefile


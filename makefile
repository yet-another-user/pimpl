cflags	= -Iinclude -std=c++11 -I$(BOOST_ROOT)
target	= pimpl-test $(wildcard test/*.cpp)

include $(PRJROOT)/make/makefile


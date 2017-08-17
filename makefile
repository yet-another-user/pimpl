cflags	= -Iinclude -I$(BOOST_ROOT)
target	= test/impl-ptr $(wildcard test/*.cpp)

include $(PRJROOT)/make/makefile


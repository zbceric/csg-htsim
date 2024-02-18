CC = g++
CFLAGS = -Wall -std=c++11 -g -Wsign-compare
CRT=`pwd`
SUBDIRS=tests
INCLUDE= -I/$(CRT)/.. -I$(CRT)
OBJS=schedule.o progress.o progress_utils.o delay.o

LIB=-L..
DEPS=../libhtsim.a

# CFLAGS += -O2

all: libextension.a $(SUBDIRS)

$(SUBDIRS):	libextension.a
	$(MAKE) -C $@

.PHONY: all clean

libextension.a:	$(OBJS) $(HDRS)
	ar -rvu libextension.a $(OBJS)

schedule.o: schedule.cpp
progress.o: progress.cpp
progress_utils.o: progress_utils.cpp
delay.o: delay.cpp

.cpp.o:		$(DEPS) # suffix rule
	source='$<' object='$@' libtool=no depfile='$(DEPDIR)/$*.Po' tmpdepfile='$(DEPDIR)/$*.TPo' $(CXXDEPMODE) $(depcomp) $(CC) $(CFLAGS) $(INCLUDE) -c -o $@ `test -f $< || echo '$(srcdir)/'`$<

clean:	
	rm -f *.o
	rm -f *.a
	@for dir in $(SUBDIRS); do \
        $(MAKE) -C $$dir clean; \
    done
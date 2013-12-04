
   OBJS    = \
	     htab.o \
             cbinky.o \
             util.o \
             strpool.o \
             dmeta.o \
             trie.o \
             pagepool.o \
	     hash_32.o 

  INCDIRS += -I../include

 CPPFLAGS += $(INCDIRS) -DDEBUG=3

        LD = $(CC)

      LIBS = -L. -lbinky

    CFLAGS = -g -fPIC 

   LIBRARY = libbinky.a

SHARED_LIBRARY = libbinky.so

   LDFLAGS = $(CFLAGS)




all: $(LIBRARY)


$(LIBRARY): $(OBJS)
	ar rcv $(LIBRARY) $(OBJS)


$(SHARED_LIBRARY): $(OBJS)
	gcc -pthread -shared $(OBJS) -o libbinky.so



htab.o: ../include/htab.h ../include/fnv.h 

mi.o: ../include/mi.h

docm.o: ../include/mi.h ../include/docm.h

cbinky.o: ../include/mi.h ../include/docm.h ../include/htab.h ../include/cbinky.h ../include/dmeta.h

dmeta.o: ../include/dmeta.h


clean:
	rm -f $(OBJS) $(LIBRARY) $(SHARED_LIBRARY)

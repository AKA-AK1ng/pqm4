SOURCES = kem.c newhope.c poly.c error_correction.c ntt.c reduce.c precomp.c crypto_stream_chacha20.c
HEADERS = api.h params.h newhope.h poly.h error_correction.h ntt.h reduce.h crypto_stream_chacha20.h

INCPATH ?= ../../../common

CFLAGS += -I$(INCPATH)

all: libpqm4.a

libpqm4.a: $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) -c $(SOURCES)
	$(AR) rcs $@ *.o

clean:
	rm -f *.o libpqm4.a

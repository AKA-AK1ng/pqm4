SOURCES = kem.c nike.c poly.c toom_static.c error_correction.c reduce.c \
          crypto_stream_chacha20.c ../../../common/randombytes.c
HEADERS = api.h params.h nike.h poly.h toom_static.h error_correction.h \
          reduce.h crypto_stream_chacha20.h

INCPATH ?= ../../../common

PARAM_DEFS = -DNIKE_LEVEL=128 -DPARAM_N=512 -DPARAM_Q=8192 -DLOG2Q=13 \
             -DPARAM_T_PK=9 -DPARAM_T_U=9 -DPARAM_T_V=6

CFLAGS += -I$(INCPATH) $(PARAM_DEFS)

all: libpqm4.a

params:
	@echo "SCHEME=MAMBA-NIKE-128"
	@echo "PARAM_DEFS=$(PARAM_DEFS)"

libpqm4.a: $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) -c $(SOURCES)
	$(AR) rcs $@ *.o

clean:
	rm -f *.o libpqm4.a

.PHONY: all params clean

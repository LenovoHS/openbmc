all: pwr-seq
CFLAGS += -Wall -Werror

fpc-util: pwr-seq.c 
	$(CC) $(CFLAGS) -std=c99 -o $@ $^ $(LDFLAGS)

.PHONY: clean

clean:
	rm -rf *.o pwr-seq

all: fpga
CFLAGS += -Wall -Werror

fpc-util: fpga.c 
	$(CC) $(CFLAGS) -std=c99 -o $@ $^ $(LDFLAGS)

.PHONY: clean

clean:
	rm -rf *.o fpga

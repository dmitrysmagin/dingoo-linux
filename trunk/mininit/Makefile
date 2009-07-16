CROSS = mipsel-linux-

all : init

clean:
	rm -rf init *.o

init : init.o fstype.o loop.o
	$(CROSS)gcc -static -o $@ $^
	$(CROSS)strip --strip-all $@

%.o : %.c
	$(CROSS)gcc -Wall -O2 -fno-stack-protector -c -o $@ $<


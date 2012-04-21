.SUFFIXES:

.PHONY: all clean

FOLDERS := ntxm7 ntxm9 tobkit ntracker

all:
	@for i in $(FOLDERS); do make -C $$i; done

clean:
	@for i in $(FOLDERS); do make -C $$i clean; done

install:
	@for i in $(FOLDERS); do make -C $$i install; done

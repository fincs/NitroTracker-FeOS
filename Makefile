.SUFFIXES:
.PHONY: all clean

FEOSMK = $(FEOSSDK)/mk

MANIFEST    := manifest.txt
PACKAGENAME := ntracker

FOLDERS := ntxm7 ntxm9 tobkit ntracker

include $(FEOSMK)/packagetop.mk

all:
	@for i in $(FOLDERS); do make -C $$i; done

clean:
	@for i in $(FOLDERS); do make -C $$i clean; done

install:
	@for i in $(FOLDERS); do make -C $$i install; done

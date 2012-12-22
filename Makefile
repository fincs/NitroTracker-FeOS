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
	@mkdir -p $(FEOSDEST)/data/FeOS/gui/ftypes
	@grit apptile.png -ftr -fh! -gb -gB16 -gT! -gzl -p! -o $(FEOSDEST)/data/FeOS/gui/$(PACKAGENAME).grf
	@grit fileicon.png -ftr -fh! -gb -gB16 -gT7FFE -gzl -p! -o $(FEOSDEST)/data/FeOS/gui/ftypes/$(PACKAGENAME).grf
	@cp filetypes.cfg $(FEOSDEST)/data/FeOS/gui/ftypes/$(PACKAGENAME).cfg
	@fmantool application.manifest $(FEOSDEST)/data/FeOS/gui/$(PACKAGENAME).app

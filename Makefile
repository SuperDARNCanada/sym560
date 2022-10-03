APPDIR=./userapp/app/
DRVDIR=./driver/
BINDIR=/usr/bin/
KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD       := $(shell pwd)
MODLOADED ?= $(shell cat /proc/modules | grep sym560)
obj-m	:= sym560_driver.o

# running make or make all will compile the userapp and the driver
all: $(APPDIR)sym560_cmdline sym560driver

$(APPDIR)sym560_cmdline: $(APPDIR)sym560_functions.o $(APPDIR)sym560_cmdline.o $(APPDIR)event_cap
	cd $(APPDIR); gcc -g sym560_cmdline.o sym560_functions.o -o sym560_cmdline -lm -lncurses -lreadline

$(APPDIR)event_cap: $(APPDIR)sym560_functions.o $(APPDIR)event_cap.o
	cd $(APPDIR); gcc -g event_cap.o sym560_functions.o -o event_cap -lm -lncurses -lreadline

$(APPDIR)sym560_functions.o: $(APPDIR)sym560_functions.c $(APPDIR)sym560_functions.h
	cd $(APPDIR); gcc -g -c sym560_functions.c

$(APPDIR)event_cap.o: $(APPDIR)event_cap.c
	cd $(APPDIR); gcc -g -c event_cap.c

$(APPDIR)sym560_cmdline.o: $(APPDIR)sym560_cmdline.c $(APPDIR)sym560_functions.h
	cd $(APPDIR); gcc -g -c sym560_cmdline.c

sym560driver:
	cd $(DRVDIR); $(MAKE) -C $(KERNELDIR) M="$(PWD)/driver" modules
	sed -e 's%PATHTOMODULE%$(PWD)/driver/sym560_driver.ko%g' $(PWD)/driver/sym560.org > $(PWD)/driver/sym560
	chmod 0755 $(PWD)/driver/sym560
	

# running make install will put everything where it belongs
# put compiled scripts into path,
# but sym560 into /etc/init.d
install:
	@echo "Installing (will likely fail unless run as root)"
	cp -p $(PWD)/userapp/app/stopstamp.bash $(BINDIR)
	cp -p $(PWD)/userapp/app/restartstamp.bash $(BINDIR)
	cp -p $(PWD)/userapp/app/sym560_cmdline $(BINDIR)
	cp -p $(PWD)/userapp/app/event_cap $(BINDIR)
	cp -p $(PWD)/userapp/pulse_seq_script/findpulse.pl $(BINDIR)
	cp -p $(PWD)/driver/sym560 /usr/lib/systemd/scripts/
	cp -p $(PWD)/driver/sym560.service /usr/lib/systemd/system/
uninstall:
	@echo "Uninstalling (will likely fail unless run as root)"
	@if [ "$(MODLOADED)" != "" ]; then \
		 /usr/lib/systemd/scripts/sym560 stop; fi
	rm -f $(BINDIR)stopstamp.bash $(BINDIR)restartstamp.bash $(BINDIR)sym560_cmdline $(BINDIR)event_cap $(BINDIR)findpulse.pl
	

#running make clean will uninstall everything
clean:
	@echo "Cleaning"
	cd $(APPDIR); rm -f *.o *~ sym560_cmdline event_cap
	cd $(DRVDIR); rm -rf *.o *~ core .depend .*.cmd *.ko *.mod *.mod.c .tmp_versions sym560

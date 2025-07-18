#############################
#
# Makefile vi53xx
#
############################

include ../../defines.mk

BASESUBDIRS:= \
	 driver \
	 lib 

SUBDIRS:= $(BASESUBDIRS) 

DEBINSTALL=debinstall

all clean install: $(SUBDIRS)


.PHONY: $(SUBDIRS)
$(SUBDIRS): 
	$(MAKE) -C $@ $(MAKECMDGOALS)

$(filter-out driver lib, $(SUBDIRS)): lib driver

lib: driver

test:

debinstall: 
	set -e; for s in $(BASESUBDIRS) ; do $(MAKE) -C $$s all ; $(MAKE) -C $$s install; done 


include $(RTPC_BUILD_ROOT)/debian.mk



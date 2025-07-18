


include ../../../defines.mk


OBJ= vi53xx_xdma.o es5341_di_cfg.o es5341_do_cfg.o


INST_HEADER= vi53xx_api.h
#RTIO_HEADER= es53xx_rtio_adopt.h $(LABCARLIB)/target/rtio/global/es53xx_rtio.h
HEADER= $(INST_HEADER)

# lib
LIB= libvi53xx.so


INSTLIBDIR=$(ETAS_BASE_DIR)/lib
INSTHEADERDIR=$(ETAS_BASE_DIR)/include
LABCARLIB=../../../labcar-lib

LDLIBS= -ldl

#CCOPT= -c -O1 -Wall  -ggdb3  -march=i686 -fPIC -pthread -save-temps
CCOPT= -O0 -Wall  -Wpointer-arith -ggdb3  $(CC_ARCH) -fPIC -pthread 

RTIO_CCOPT:= -g \
            -DRTIO \
            -DSIM_PPC \
            -DSIM_PPC_VPPC \
            -DSIM_RTPC \
            -DRTIO_PCIE \
            -DSVN_REVISION="$(SVNREVISION)" \
            -I../../../rtos \
            -I$(LABCARLIB)/target/es1130 \
            -I$(LABCARLIB)/target/es1130/common \
            -I$(LABCARLIB)/target/common \
            -I$(LABCARLIB)/target/rtio/global \
            -I$(LABCARLIB)/target


# ================================================
#  T a r g e t s
# ================================================

all: $(LIB)

clean:
	$(RM) $(LIB) $(OBJ) $(OBJ:.o=.d)


$(LIB): $(OBJ)
	$(LD) -g -rdynamic -shared -fPIC -pthread -o $(LIB)  $(OBJ) $(LDLIBS)


install: all
	$(INSTALL) -d $(INSTLIBDIR)
	$(INSTALL) -d $(INSTHEADERDIR)
	$(INSTALL) -m644 $(LIB) $(INSTLIBDIR)
	$(INSTALL) -m644 $(INST_HEADER) $(INSTHEADERDIR)
	$(LDCONFIG)

%.o:%.c *.h $(RTIO_HEADER)
	$(CC) -c $(CCOPT) $(RTIO_CCOPT) $<
	@$(CC) -MM -MD $(CCOPT) $(RTIO_CCOPT) $<

-include $(OBJ:.o=.d)

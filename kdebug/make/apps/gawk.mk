SOURCE=$(APPS_DIR)/gawk/gawk-$(VERSION)
VERSION=4.1.0

all: build install 

build: config 
	@cd $(SOURCE) && make 

config:
	@if [ ! -f $(SOURCE)/Makefile ]; then \
	    cd $(SOURCE) && ./configure --host=$(ARCH) CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)"; \
	fi


install:
	@cd $(SOURCE) && ln -sf `which install` install 
	@cd $(SOURCE) && make install

clean:
	@cd $(SOURCE) && make clean

distclean:
	@cd $(SOURCE) && make distclean


.PHONY: build config install all clean

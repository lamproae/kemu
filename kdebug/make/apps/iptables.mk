SOURCE=$(APPS_DIR)/iptables/iptables-$(VERSION)
VERSION=1.4.20

all: build install 

build: config 
	@cd $(SOURCE) && make 

config:
	@if [ ! -f $(SOURCE)/Makefile ]; then \
	    cd $(SOURCE) && ./configure --enable-static --prefix=$(ROOT_DIR) --host=$(ARCH) CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS) -L$(SOURCE)/libiptc/.libs" ; \
	fi


install:
	find $(SOURCE)/ -perm 775 -a ! -name ".deps" -a ! -type d | xargs -i $(INSTALL) {} $(ROOT_DIR)/bin/

clean:
	@cd $(SOURCE) && make clean

distclean:
	@cd $(SOURCE) && make distclean

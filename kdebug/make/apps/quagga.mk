SOURCE=$(APPS_DIR)/quagga/quagga
VERSION=""
LIBREADLINE=$(ROOT_DIR) 

all: build install 

build: config  
	@cd $(SOURCE) && make 

config: 
	@if [ ! -f $(SOURCE)/Makefile ]; then \
	    cd $(SOURCE) && ./bootstrap.sh && ./configure --enable-user=root --enable-group=root --localstatedir=$(ROOT_DIR)/var/run --disable-vtysh --prefix=$(ROOT_DIR) --host=$(ARCH)-unknown-linux-gnu CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)"; \
	fi

install: 
	@cd $(SOURCE) && ln -sf `which install` install 
	@cd $(SOURCE) && make install

clean:
	@cd $(SOURCE) && make clean && make distclean

distclean:
	@cd $(SOURCE) && make distclean

.PHONY: config build clean install

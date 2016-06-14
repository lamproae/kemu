#/bin/sh

top=`pwd`
# Here we should use the source from network. .. Github
sudo apt-get -y install vim make qemu bc

mkdir -p $top/tools
mkdir -p $top/tar

cd $top/tools 
if [ ! -d crosstool-ng ]; then
    #
    # This is for build crosstool-ng
    # RUN apt-get -y install gperf bison flex texinfo help2man gawk automake
    # RUN git clone https://github.com/lamproae/lib-tool.git \
        # && cd lib-tool \
        # && ./bootstrap \
        # && ./configure \
        # && make \
        # && make install

    # This is for get croostool-ng
    # WORKDIR $HOME
    git clone https://github.com/lamproae/crosstool-ng.git \
        && cd $top/crosstool-ng \
        && ./bootstrap \
        && ./configure \
        && make \
        && sudo make install

    # Build cross-compile toolchains
    ct-ng x86_64-unknown-linux-gnu \
        && ct-ng build
fi

# cd $top/ 
# if [ ! -d kdebug ]; then
#    git clone https://github.com/lamproae/kdebug.git
#    cd $top/ && mkdir -p $top/kdebug/tools/toolchain/x86_64
#    cp -a $HOME/x-tools/x86_64-unknown-linux-gnu $top/kdebug/tools/toolchain/x86_64
# fi

# Get kernel source code
cd $top/tar 
if [ ! -d linux-4.6.2 ]; then
    curl -o linux-4.6.2.tar.xz https://www.kernel.org/pub/linux/kernel/v4.x/linux-4.6.2.tar.xz \
        && xz -d linux-4.6.2.tar.xz \
        && tar -xvf linux-4.6.2.tar 
    cd ..
    mkdir -p $top/kdebug/kernel
    cp -a tar/linux-4.6.2 $top/kdebug/kernel/
fi

# Get busybox source code
cd $top/tar 
if [ ! -d busybox-1.24.2 ]; then
    curl -o busybox-1.24.2.tar.bz2 https://www.busybox.net/downloads/busybox-1.24.2.tar.bz2 \
        && tar -jxvf busybox-1.24.2.tar.bz2 
    cd ..
    mkdir -p $top/kdebug/apps/busybox
    cp -a tar/busybox-1.24.2 $top/kdebug/apps/busybox/
fi

# Get coreutils source code
cd $top/tar 
if [ ! -d coreutils-8.23 ]; then
    curl -o coreutils-8.23.tar.xz http://ftp.gnu.org/gnu/coreutils/coreutils-8.23.tar.xz\
        && xz -d coreutils-8.23.tar.xz \
        && tar -xvf coreutils-8.23.tar 
    cd ..
    mkdir -p $top/kdebug/apps/coreutils
    cp -a tar/coreutils-8.23 $top/kdebug/apps/coreutils/
fi

# Get inetutils source code
cd $top/tar 
if [ ! -d inetutils-1.9.4 ]; then
    curl -o inetutils-1.9.4.tar.xz http://ftp.gnu.org/gnu/inetutils/inetutils-1.9.4.tar.xz \
        && xz -d inetutils-1.9.4.tar.xz \
        && tar -xvf inetutils-1.9.4.tar 
    cd ..
    mkdir -p $top/kdebug/apps/inetutils
    cp -a tar/inetutils-1.9.4 $top/kdebug/apps/inetutils/
fi

cd $top/tar 
if [ ! -d bash-4.3 ]; then
    curl -o bash-4.3.tar.gz http://ftp.gnu.org/gnu/bash/bash-4.3.tar.gz \
        && tar -zxvf bash-4.3.tar.gz 
    cd ..
    mkdir -p $top/kdebug/apps/bash
    cp -a tar/bash-4.3 $top/kdebug/apps/bash/
fi

cd $top
# RUN apt-get update
# RUN apt-get -y install vim make

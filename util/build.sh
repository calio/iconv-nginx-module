#!/bin/bash

# this file is mostly meant to be used by the author himself.

root=`pwd`
cd ~/work
version=$1
home=~
opts=$2

if [ ! -s "nginx-$version.tar.gz" ]; then
    wget "http://sysoev.ru/nginx/nginx-$version.tar.gz" -O nginx-$version.tar.gz
    if [ "$?" != 0 ]; then
        echo Abort.
        exit 1;
    fi
    tar -xzvf nginx-$version.tar.gz
    if [ "$?" != 0 ]; then
        echo Abort.
        exit 1;
    fi
    if [ "$version" = "0.8.41" ]; then
        cp $root/../no-pool-nginx/nginx-0.8.41-no_pool.patch ./
        patch -p0 < nginx-0.8.41-no_pool.patch
        if [ "$?" != 0 ]; then
            echo Abort.
            exit 1
        fi
    fi
fi

#tar -xzvf nginx-$version.tar.gz || exit 1
#cp $root/../no-pool-nginx/nginx-0.8.41-no_pool.patch ./
#patch -p0 < nginx-0.8.41-no_pool.patch

cd nginx-$version/

if [[ "$BUILD_CLEAN" -eq 1 || ! -f Makefile || "$root/config" -nt Makefile || "$root/util/build.sh" -nt Makefile ]]; then
    ./configure --prefix=/opt/nginx \
          --add-module=$root/../echo-nginx-module \
          --add-module=$root/../ndk-nginx-module \
          --add-module=$root/../set-misc-nginx-module \
          --add-module=$root/../form-input-nginx-module \
          --add-module=$root/../lua-nginx-module \
          --add-module=$root $opts \
          --with-debug
          #--add-module=$home/work/ngx_http_auth_request-0.1 #\
          #--with-rtsig_module
          #--with-cc-opt="-g3 -O0"
          #--add-module=$root/../echo-nginx-module \
  #--without-http_ssi_module  # we cannot disable ssi because echo_location_async depends on it (i dunno why?!)

fi
if [ -f /opt/nginx/sbin/nginx ]; then
    rm -f /opt/nginx/sbin/nginx
fi
if [ -f /opt/nginx/logs/nginx.pid ]; then
    kill `cat /opt/nginx/logs/nginx.pid`
fi
make -j3
make install


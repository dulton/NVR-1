#!/bin/sh

echo "start..."
cd src/ ; make clean ; make ; cd - ;
cp obj/libpanel.so ../../common/lib/debug -rf ;
cp obj/libpanel.so /tftpboot/ -rf ;

echo "OK"

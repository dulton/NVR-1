#arm-hisiv100nptl-linux-g++ test.cpp -o testonvif -I./include/xml2 -L./ -L./lib -lyzonvif -lxml2 -luuid
#cp testonvif /mnt/nfs/ -f

make clean;make

cp -f libyzonvif.so ../Common/lib/onvif/
cp -f libyzonvif.so /mnt/nfs/

rm -f /home/Hi3520D/10xx/tl/NR1008/rootfs_uclibc/home/root/onvif/libyzonvif.so
rm -f /home/Hi3520D/10xx/tl/NR1004/rootfs_uclibc/home/root/onvif/libyzonvif.so
rm -f /home/Hi3520D/10xx/nvp/NR1008/rootfs_uclibc/home/root/onvif/libyzonvif.so
rm -f /home/Hi3520D/10xx/nvp/NR1004/rootfs_uclibc/home/root/onvif/libyzonvif.so
rm -f /home/Hi3531/31xx/NR3116/rootfs_uclibc/home/root/onvif/libyzonvif.so
rm -f /home/Hi3531/31xx/NR3124/rootfs_uclibc/home/root/onvif/libyzonvif.so
rm -f /home/Hi3531/31xx/NR3132/rootfs_uclibc/home/root/onvif/libyzonvif.so
rm -f /home/Hi3521/21xx/NR2116/rootfs_uclibc/home/root/onvif/libyzonvif.so
rm -f /home/P2P/NR3116/rootfs_uclibc/home/root/onvif/libyzonvif.so
rm -f /home/P2P/NR1004/rootfs_uclibc/home/root/onvif/libyzonvif.so
rm -f /home/P2P/NR1008/rootfs_uclibc/home/root/onvif/libyzonvif.so
rm -f /home/P2P/NH-916/rootfs_uclibc/home/root/onvif/libyzonvif.so

exit

cp -f libyzonvif.so /home/Hi3520D/10xx/tl/NR1008/rootfs_uclibc/home/root/onvif/
cp -f libyzonvif.so /home/Hi3520D/10xx/tl/NR1004/rootfs_uclibc/home/root/onvif/
cp -f libyzonvif.so /home/Hi3520D/10xx/nvp/NR1008/rootfs_uclibc/home/root/onvif/
cp -f libyzonvif.so /home/Hi3520D/10xx/nvp/NR1004/rootfs_uclibc/home/root/onvif/
cp -f libyzonvif.so /home/Hi3531/31xx/NR3116/rootfs_uclibc/home/root/onvif/
cp -f libyzonvif.so /home/Hi3531/31xx/NR3124/rootfs_uclibc/home/root/onvif/
cp -f libyzonvif.so /home/Hi3531/31xx/NR3132/rootfs_uclibc/home/root/onvif/
cp -f libyzonvif.so /home/Hi3521/21xx/NR2116/rootfs_uclibc/home/root/onvif/
cp -f libyzonvif.so /home/P2P/NR3116/rootfs_uclibc/home/root/onvif/
cp -f libyzonvif.so /home/P2P/NR1004/rootfs_uclibc/home/root/onvif/
cp -f libyzonvif.so /home/P2P/NR1008/rootfs_uclibc/home/root/onvif/
cp -f libyzonvif.so /home/P2P/NH-916/rootfs_uclibc/home/root/onvif/


all:
	cd src/; make; cd -; 
	cp obj/*.so /mnt/nfs -rf;
	cp obj/*.so /tftpboot -rf;
	cp obj/*.so ../common/lib/debug -rf;

clean:
	cd src/; make clean; cd -; 


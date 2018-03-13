CLION_VERSION=2016.2
CLANG_VERSION=3.8

apt-get update && apt-get -y upgrade && apt-get -y autoremove

echo -----------------------------------
echo INSTALL COMPILER
echo -----------------------------------
for PROG in clang lldb; do
	sudo apt-get -y install ${PROG}-${CLANG_VERSION}
	for C in /usr/bin/${PROG}*${CLANG_VERSION}; do
		L=${C%-$CLANG_VERSION}
		B=$(basename $L)
		sudo update-alternatives --install $L $B $C 1000
	done
done

sudo update-alternatives --install /usr/bin/cc cc /usr/bin/clang 100
sudo update-alternatives --install /usr/bin/c++ c++ /usr/bin/clang++ 100

echo -----------------------------------
echo INSTALL CMAKE
echo -----------------------------------
sudo apt-get -y install build-essentials cmake


echo -----------------------------------
echo INSTALL DARKNET
echo -----------------------------------

git clone https://github.com/pjreddie/darknet
cd darknet
make -j2
wget https://pjreddie.com/media/files/yolo.weights --no-check-certificate
cd ..

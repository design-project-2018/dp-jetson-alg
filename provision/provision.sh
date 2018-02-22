CLION_VERSION=2016.2
CLANG_VERSION=3.8

apt-get update && apt-get -y upgrade && apt-get -y autoremove

echo -----------------------------------
echo INSTALL GIT
echo -----------------------------------
sudo apt-get -y install git

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
sudo apt-get -y install cmake

echo -----------------------------------
echo DOWNLOAD CLION
echo -----------------------------------
sudo wget -q http://download.jetbrains.com/cpp/CLion-${CLION_VERSION}.tar.gz
sudo tar xfz CLion-${CLION_VERSION}.tar.gz -C /opt

echo -----------------------------------
echo INSTALL CONAN
echo -----------------------------------
sudo apt-get -y install python-pip
sudo pip install --upgrade pip
sudo pip install conan
su -l -c conan vagrant
sed -i "s/compiler=gcc/compiler=clang/; s/version=[0-9.]*/version=$CLANG_VERSION/; s/libstdc++/libstdc++11/" ~vagrant/.conan/conan.conf

echo -----------------------------------
echo INSTALL OPENCV
echo -----------------------------------

# 1. INSTALL THE DEPENDENCIES

# Build tools:
sudo apt-get install -y build-essential cmake

# GUI (if you want to use GTK instead of Qt, replace 'qt5-default' with 'libgtkglext1-dev' and remove '-DWITH_QT=ON' option in CMake):
sudo apt-get install -y qt5-default libvtk6-dev

# Media I/O:
sudo apt-get install -y zlib1g-dev libjpeg-dev libwebp-dev libpng-dev libtiff5-dev libjasper-dev libopenexr-dev libgdal-dev

# Video I/O:
sudo apt-get install -y libdc1394-22-dev libavcodec-dev libavformat-dev libswscale-dev libtheora-dev libvorbis-dev libxvidcore-dev libx264-dev yasm libopencore-amrnb-dev libopencore-amrwb-dev libv4l-dev libxine2-dev

# Parallelism and linear algebra libraries:
sudo apt-get install -y libtbb-dev libeigen3-dev

# Python:
sudo apt-get install -y python-dev python-tk python-numpy python3-dev python3-tk python3-numpy

# Java:
sudo apt-get install -y ant default-jdk

# Documentation:
sudo apt-get install -y doxygen


# 2. INSTALL THE LIBRARY (YOU CAN CHANGE '3.2.0' FOR THE LAST STABLE VERSION)

sudo apt-get install -y unzip wget
wget https://github.com/opencv/opencv/archive/3.2.0.zip
unzip 3.2.0.zip
rm 3.2.0.zip
mv opencv-3.2.0 OpenCV
cd OpenCV
mkdir build
cd build
cmake -DWITH_QT=ON -DWITH_OPENGL=ON -DFORCE_VTK=ON -DWITH_TBB=ON -DWITH_GDAL=ON -DWITH_XINE=ON -DBUILD_EXAMPLES=ON -DENABLE_PRECOMPILED_HEADERS=OFF ..
make -j2
sudo make install
sudo ldconfig

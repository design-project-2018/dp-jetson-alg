echo -----------------------------------
echo INSTALL DARKNET
echo -----------------------------------

cd ..
git clone https://github.com/pjreddie/darknet
cd darknet
make -j2
wget https://pjreddie.com/media/files/yolov2.weights
cd ..
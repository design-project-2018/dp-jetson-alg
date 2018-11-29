echo -----------------------------------
echo INSTALL WEBSOCKETPP
echo -----------------------------------

cd ../websocketpp
mkdir build && cd build
cmake .. && make install
cd ../../
echo -----------------------------------
echo INSTALL WEBSOCKETPP
echo -----------------------------------

cd ..
git clone https://github.com/zaphoyd/websocketpp
cd websocketpp
mkdir build && cd build
cmake .. && make install
cd ../../
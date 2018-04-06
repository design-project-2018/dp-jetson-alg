CLION_VERSION=2016.2

apt-get update && apt-get -y upgrade && apt-get -y autoremove

echo -----------------------------------
echo DOWNLOAD CLION
echo -----------------------------------
sudo wget -q http://download.jetbrains.com/cpp/CLion-${CLION_VERSION}.tar.gz
sudo tar xfz CLion-${CLION_VERSION}.tar.gz -C /opt
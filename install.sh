echo 'Creating Directories...'

mkdir tmpSlideLapse tmpSnapshots tmpTimeLapse tmpVideos

echo 'Compiling...'

g++ HypRaspCam.cpp -o HypRaspCam

echo 'Linking...'

sudo cp HypCamAutorun /etc/init.d/HypRaspCam

sudo chmod 755 /etc/init.d/HypRaspCam

sudo update-rc.d HypRaspCam defaults

echo 'Setting DHCP on WiFi...'

sudo ./shareWiFi.sh

echo 'Instalation Finished Successfully...'

echo 'Please, Press Any Key to Reboot...'

read varAnswer

sudo reboot

# RaspHypCam
raspistill -q 100 -gc -ifx colourbalance -ifx denoise  -o test.RGB888
raspistill -q 100 -gc -ifx colourbalance -ifx denoise  -o test.RGB888 -t 8000 -ss 2000000




//------------------------------
// INSTALL
//------------------------------

git clone https://github.com/JairoSalazarV/RaspHypCam

cd RaspHypCam/

mkdir tmpSlideLapse, tmpSnapshots, tmpTimeLapse, tmpVideos

g++ HypRaspCam.cpp -o HypRaspCam

sudo cp HypCamAutorun /etc/init.d/HypCamAutorun

sudo chmod 755 /etc/init.d/HypCamAutorun

sudo /etc/init.d/HypCamAutorun start

//Test connection

sudo /etc/init.d/HypCamAutorun stop

sudo update-rc.d HypCamAutorun defaults

//If required
//sudo update-rc.d -f HypCamAutorun remove
 







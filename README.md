# RaspHypCam
raspistill -q 100 -gc -ifx colourbalance -ifx denoise  -o test.RGB888
raspistill -q 100 -gc -ifx colourbalance -ifx denoise  -o test.RGB888 -t 8000 -ss 2000000




//------------------------------
// INSTALL
//------------------------------

git clone --depth=1 https://github.com/JairoSalazarV/RaspHypCam

cd RaspHypCam/

mkdir tmpSlideLapse, tmpSnapshots, tmpTimeLapse, tmpVideos

g++ HypRaspCam.cpp -o HypRaspCam

sudo cp HypCamAutorun /etc/init.d/RaspHypCam

sudo chmod 755 /etc/init.d/RaspHypCam

sudo /etc/init.d/RaspHypCam start

//Test connection

sudo /etc/init.d/RaspHypCam stop

sudo update-rc.d RaspHypCam defaults

//If required
//sudo update-rc.d -f RaspHypCam remove
 







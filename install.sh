mkdir tmpSlideLapse tmpSnapshots tmpTimeLapse tmpVideos

g++ HypRaspCam.cpp -o HypRaspCam

sudo cp HypCamAutorun /etc/init.d/RaspHypCam

sudo chmod 755 /etc/init.d/RaspHypCam

sudo update-rc.d RaspHypCam defaults

sudo /etc/init.d/RaspHypCam start

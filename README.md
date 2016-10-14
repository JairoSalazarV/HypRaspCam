# RaspHypCam
raspistill -q 100 -gc -ifx colourbalance -ifx denoise  -o test.RGB888
raspistill -q 100 -gc -ifx colourbalance -ifx denoise  -o test.RGB888 -t 8000 -ss 2000000

g++ HypRaspCam.cpp -o HypRaspCam -L/opt/vc/lib -I/usr/local/include -lraspicam -lmmal
-lmmal_core -lmmal_util

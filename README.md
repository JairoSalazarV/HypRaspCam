# RaspHypCam
raspistill -q 100 -gc -ifx colourbalance -ifx denoise  -o test.RGB888
raspistill -q 100 -gc -ifx colourbalance -ifx denoise  -o test.RGB888 -t 8000 -ss 2000000




//------------------------------
// INSTALL
//------------------------------

git clone --depth=1 https://github.com/JairoSalazarV/RaspHypCam

cd HypRaspCam/

chmod a+x install.sh

./install.sh

sudo reboot

// Change Camera ID

nano camName.RaspHypCam

sudo reboot // and Test

//If required
//sudo update-rc.d -f RaspHypCam remove

//------------------------------
// DHCP
//------------------------------

Connect Ethernet

https://frillip.com/using-your-raspberry-pi-3-as-a-wifi-access-point-with-hostapd/
 







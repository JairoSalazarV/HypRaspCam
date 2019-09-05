echo 'Enter WiFi ID: '
read SSIDName
echo 'Enter WiFi Password: '
read SSIDPsw

sudo apt-get install dnsmasq hostapd

echo 'denyinterfaces wlan0' >> /etc/dhcpcd.conf

echo 'allow-hotplug wlan0' >> /etc/network/interfaces
echo 'iface wlan0 inet static' >> /etc/network/interfaces
echo '    address 172.24.1.1' >> /etc/network/interfaces
echo '    netmask 255.255.255.0' >> /etc/network/interfaces
echo '    network 172.24.1.0' >> /etc/network/interfaces
echo '    broadcast 172.24.1.255' >> /etc/network/interfaces
echo '#    wpa-conf /etc/wpa_supplicant/wpa_supplicant.conf' >> /etc/network/interfaces

sudo systemctl unmask dhcpcd.service
sudo systemctl unmask hostapd.service

sudo service dhcpcd restart

sudo ifup wlan0; sudo ifdown wlan0

sudo cp hostapd.conf /etc/hostapd/hostapd.conf
sudo echo "ssid=${SSIDName}" >> /etc/hostapd/hostapd.conf
sudo echo "wpa_passphrase=${SSIDPsw}" >> /etc/hostapd/hostapd.conf

sudo echo 'DAEMON_CONF="/etc/hostapd/hostapd.conf"' >> /etc/default/hostapd

sudo mv /etc/dnsmasq.conf /etc/dnsmasq.conf.orig

sudo cp dnsmasq.conf /etc/dnsmasq.conf

sudo echo 'net.ipv4.ip_forward=1' >> /etc/sysctl.conf

sudo sh -c "echo 1 > /proc/sys/net/ipv4/ip_forward"

sudo iptables -t nat -A POSTROUTING -o eth0 -j MASQUERADE
sudo iptables -A FORWARD -i eth0 -o wlan0 -m state --state RELATED,ESTABLISHED -j ACCEPT
sudo iptables -A FORWARD -i wlan0 -o eth0 -j ACCEPT

sudo sh -c "iptables-save > /etc/iptables.ipv4.nat"

sudo mv /etc/rc.local /etc/rc.local.bkp
sudo cp rc.local /etc/rc.local

sudo service hostapd start
sudo service dnsmasq start


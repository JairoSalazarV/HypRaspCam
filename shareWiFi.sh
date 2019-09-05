echo 'Enter WiFi ID: '
read SSIDName
echo 'Enter WiFi Password: '
read SSIDPsw

sudo apt-get install dnsmasq hostapd

sudo systemctl stop dnsmasq
sudo systemctl stop hostapd

echo 'interface wlan0' >> /etc/dhcpcd.conf
echo '    static ip_address=172.24.1.1/24' >> /etc/dhcpcd.conf
echo '    nohook wpa_supplicant' >> /etc/dhcpcd.conf

sudo service dhcpcd restart

sudo mv /etc/dnsmasq.conf /etc/dnsmasq.conf.orig
echo 'interface=wlan0' >> /etc/dnsmasq.conf
echo 'dhcp-range=172.24.1.2,172.24.1.20,255.255.255.0,24h' >> /etc/dnsmasq.conf

sudo systemctl reload dnsmasq

sudo cp hostapd.conf /etc/hostapd/hostapd.conf
sudo echo "ssid=${SSIDName}" >> /etc/hostapd/hostapd.conf
sudo echo "wpa_passphrase=${SSIDPsw}" >> /etc/hostapd/hostapd.conf

sudo echo 'DAEMON_CONF="/etc/hostapd/hostapd.conf"' >> /etc/default/hostapd

sudo systemctl unmask hostapd
sudo systemctl enable hostapd
sudo systemctl start hostapd

sudo echo 'net.ipv4.ip_forward=1' >> /etc/sysctl.conf

sudo iptables -t nat -A  POSTROUTING -o eth0 -j MASQUERADE

sudo sh -c "iptables-save > /etc/iptables.ipv4.nat"

sudo mv /etc/rc.local /etc/rc.local.bkp
sudo cp rc.local /etc/rc.local


# Quspin

![Preview](Quspin.png)

## How to build on Linux:
```shell
cd ~/Desktop
git clone git@github.com:kroharu/Quspin.git
sudo apt update
sudo apt-install qt5-default
sudo apt-get install build-essential
sudo apt-get install libgt5serialport5
sudo apt-get install libgt5serialport5-dev
sudo apt-get install libqt5charts5
sudo apt-get install libqt5charts5-dev
sudo apt dist-upgrade
```
-- Turn ON your VPN here --
```shell
sudo apt install ./Quspin/NILinux2020DeviceDrivers/ni-software-2020-bionic_20.1.0.49152-0+f0_all.deb
sudo apt update
reboot
```
-- After rebooting your PC --
```shell
cd ~/Desktop/Quspin/Quspin
qmake
make && ./Quspin
```
## How to build on Windows:
https://visualstudio.microsoft.com/ru/thank-you-downloading-visual-studio/?sku=BuildTools&rel=15
https://www.ni.com/ru-ru/support/downloads/drivers/download/unpackaged.ni-daqmx.288260.html
https://www.qt.io/download-qt-installer?hsCtaTracking=99d9dd4f-5681-48d2-b096-470725510d34%7C074ddad0-fdef-4e53-8aa8-5e8a876d6ab4
https://download.qt.io/archive/qtcreator/4.11/4.11.2/
https://drive.google.com/file/d/1btAtmC5bpDIZeOk5qBX7EvpbCyRZcYcm/view?usp=sharing
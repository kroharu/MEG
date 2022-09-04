# Quspin

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

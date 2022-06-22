# Packages required to be installed in this order for Tails.

cd /live/persistence/TailsData_unlocked/apt/cache

dpkg -i libcairo-gobject2_1.10.2-7~bpo60+1_i386.deb
dpkg -i libcairo-script-interpreter2_1.10.2-7~bpo60+1_i386.deb
dpkg -i libpixman-1-dev_0.24.0-1~bpo60+1_i386.deb

dpkg -i libxcb1-dev_1.6-1+squeeze1_i386.deb

dpkg -i libxcb-shm0-dev_1.6-1+squeeze1_i386.deb
dpkg -i libcairo2-dev_1.10.2-7~bpo60+1_i386.deb
dpkg -i libpango1.0-dev_1.28.3-1+squeeze2_i386.deb
dpkg -i libgtk2.0-dev_2.20.1-2_i386.deb
dpkg -i libmysqlclient-dev_5.1.73-1_i386.deb

apt-get -f install

#!/usr/bin/env bash

if [[ $EUID -ne 0 ]]; then
	echo "This script must be run as root" 1>&2
	exit 1
fi

#::: Determine releases
if [[ -f /etc/debian_version ]]; then
	export OS=Debian
elif [[ -f /etc/fedora-release ]]; then
	export OS=fedora_core
elif [[ -f /etc/redhat-release ]]; then
	export OS=red_hat
else
	echo "This script must be run on a Debian or RedHat derivative"
	exit 1
fi

echo "#########################################################                         "
echo "#::: EverQuest Emulator Modular Installer                                         "
echo "#::: Installer Author: Akkadius                                                   "
echo "#::: Installer Co-Author(s): N0ctrnl                                              "
echo "#:::                                                                              "
echo "#::: EQEmulator Server Software is developed and maintained                       "
echo "#:::	by the EQEmulator Developement team                                         "
echo "#:::                                                                              "
echo "#::: Everquest is a registered trademark                                          "
echo "#::: Daybreak Game Company LLC.                                                   "
echo "#:::                                                                              "
echo "#::: EQEmulator is not associated or                                              "
echo "#::: affiliated in any way with Daybreak Game Company LLC.                        "
echo "#########################################################                         "
echo "#:                                                                                "
echo "#########################################################                         "
echo "#::: To be installed:                                                             "
echo "#########################################################                         "
echo "- Server running folder - Will be installed to the folder you ran this script     "
echo "- MariaDB (MySQL) - Database engine                                               "
echo "- Perl 5.X :: Scripting language for quest engines                             "
echo "- LUA Configured :: Scripting language for quest engines                          "
echo "- Latest PEQ Database                                                             "
echo "- Latest PEQ Quests                                                               "
echo "- Latest Plugins repository                                                       "
echo "- Maps (Latest V2) formats are loaded                                             "
echo "- New Path files are loaded                                                       "
echo "- Optimized server binaries                                                       "
echo "#########################################################                         "

# Installation variables (Don't need to change, only for advanced users)

export eqemu_server_directory=/home/eqemu
export apt_options="-y -qq" # Set autoconfirm and silent install

################################################################

if [ ! -f ./install_variables.txt ]; then

	read -n1 -r -p "Press any key to continue..." key
	
	#::: Setting up user environment (eqemu)
	echo "First, we need to set your passwords..."
	echo "Make sure that you remember these and keep them somewhere"
	echo ""
	echo ""
	groupadd eqemu
	useradd -g eqemu -m -d $eqemu_server_directory eqemu
	passwd eqemu

	#::: Go to server directory
	cd $eqemu_server_directory

	#::: Setup MySQL root user PW
	read -p "Enter MySQL root (Database) password: " eqemu_db_root_password

	#::: Write install variables (later use)
	echo "mysql_root:$eqemu_db_root_password" > install_variables.txt

	#::: Setup MySQL server 
	read -p "Enter Database Name (single word, no special characters, lower case):" eqemu_db_name
	read -p "Enter (Database) MySQL EQEmu Server username: " eqemu_db_username
	read -p "Enter (Database) MySQL EQEmu Server password: " eqemu_db_password

	#::: Write install variables (later use)
	echo "mysql_eqemu_db_name:$eqemu_db_name" >> install_variables.txt
	echo "mysql_eqemu_user:$eqemu_db_username" >> install_variables.txt
	echo "mysql_eqemu_password:$eqemu_db_password" >> install_variables.txt
fi

if [[ "$OS" == "Debian" ]]; then
	# Install pre-req packages
	apt-get -y update
	apt-get $apt_options install bash
	apt-get $apt_options install build-essential
	apt-get $apt_options install cmake
	apt-get $apt_options install cpp
	apt-get $apt_options install curl
	apt-get $apt_options install debconf-utils
	apt-get $apt_options install g++
	apt-get $apt_options install gcc
	apt-get $apt_options install git
	apt-get $apt_options install git-core
	apt-get $apt_options install libio-stringy-perl
	apt-get $apt_options install liblua5.1
	apt-get $apt_options install liblua5.1-dev
	apt-get $apt_options install libluabind-dev
	apt-get $apt_options install libmysql++
	apt-get $apt_options install libperl-dev
	apt-get $apt_options install libperl5i-perl
	apt-get $apt_options install libwtdbomysql-dev
	apt-get $apt_options install libmysqlclient-dev
	apt-get $apt_options install minizip
	apt-get $apt_options install lua5.1
	apt-get $apt_options install make
	apt-get $apt_options install mariadb-client
	apt-get $apt_options install open-vm-tools
	apt-get $apt_options install unzip
	apt-get $apt_options install uuid-dev
	apt-get $apt_options install wget
	apt-get $apt_options install zlib-bin
	apt-get $apt_options install zlibc
	apt-get $apt_options install libsodium-dev
	apt-get $apt_options install libsodium18
	apt-get $apt_options install libjson-perl
	apt-get $apt_options install libssl-dev
	
	# Install libsodium
	wget http://ftp.us.debian.org/debian/pool/main/libs/libsodium/libsodium-dev_1.0.11-2_amd64.deb -O /home/eqemu/libsodium-dev.deb
        wget http://ftp.us.debian.org/debian/pool/main/libs/libsodium/libsodium18_1.0.11-2_amd64.deb -O /home/eqemu/libsodium18.deb

	dpkg -i /home/eqemu/libsodium*.deb
	# Cleanup after ourselves
	rm -f /home/eqemu/libsodium-dev.deb
	rm -f /home/eqemu/libsodium18.deb
	
	#::: Install FTP for remote FTP access
	echo "proftpd-basic shared/proftpd/inetd_or_standalone select standalone" | debconf-set-selections
	apt-get -y -q install proftpd

	#::: Install MariaDB Server
	export DEBIAN_FRONTEND=noninteractive
	debconf-set-selections <<< 'mariadb-server-10.0 mysql-server/root_password password PASS'
	debconf-set-selections <<< 'mariadb-server-10.0 mysql-server/root_password_again password PASS'
	apt-get install -y mariadb-server
	mysql -uroot -pPASS -e "SET PASSWORD = PASSWORD('$eqemu_db_root_password');"

elif [[ "$OS" == "red_hat" ]]; then
	# Do RedHat / CentOS stuff
	# Install prereqs
        yum -y install epel-release deltarpm
        yum -y update
	yum -y install \
                open-vm-tools \
                vim \
                cmake3 \
                boost-* \
                zlib-devel \
		mariadb \
                mariadb-server \
                mariadb-devel \
                mariadb-libs \
                perl-DBD-MySQL \
                perl-JSON \
                perl-IO-stringy \
                perl-devel \
                perl-Time-HiRes \
                lua-devel \
                dos2unix \
                php-mysql \
                proftpd \
                libuuid-devel \
                libsodium \
                libsodium-devel \
                openssl-devel 

	yum -y groupinstall \
		"Development Tools" \
		"Basic Web Server" \
		"Compatibility Libraries" 
	# Deal with the cmake 3 prerequisite on RHEL/CentOS 6/7 Note: Might break with RHEL/CentOS 8 
	alternatives --install /usr/local/bin/cmake cmake /usr/bin/cmake3 20 \
	--slave /usr/local/bin/ctest ctest /usr/bin/ctest3 \
	--slave /usr/local/bin/cpack cpack /usr/bin/cpack3 \
	--slave /usr/local/bin/ccmake ccmake /usr/bin/ccmake3 \
	--family cmake

elif [[ "$OS" == "fedora_core" ]]; then
	# Do Fedora stuff
	dnf -y install \
                open-vm-tools \
                vim \
                cmake \
                boost-devel \
                zlib-devel \
                mariadb-server \
                mariadb-devel \
                perl \
                perl-DBD-MySQL \
                perl-IO-stringy \
                perl-devel \
                lua-devel \
                lua-sql-mysql \
                dos2unix \
                php-mysqlnd \
                proftpd \
                wget \
                compat-lua-libs \
                compat-lua-devel \
                compat-lua \
                perl-Time-HiRes \
                perl-JSON \
                libuuid-devel \
                libsodium \
                libsodium-devel \
                openssl-devel 

        dnf -y group install "Development Tools" "Basic Web Server" "C Development Tools and Libraries"
fi

if [[ "$OS" == "fedora_core" ]] || [[ "$OS" == "red_hat" ]]; then
	# Start MariaDB server and set root password
	echo "Starting MariaDB server..."
	systemctl enable mariadb.service --now
	sleep 5
	/usr/bin/mysqladmin -u root password $eqemu_db_root_password
fi

#::: Configure game server database user
mysql -uroot -p$eqemu_db_root_password -e "CREATE USER '$eqemu_db_username'@'localhost' IDENTIFIED BY '$eqemu_db_password';"
mysql -uroot -p$eqemu_db_root_password -e "GRANT GRANT OPTION ON *.* TO '$eqemu_db_username'@'localhost';"
mysql -uroot -p$eqemu_db_root_password -e "GRANT ALL ON *.* TO '$eqemu_db_username'@'localhost';"

#::: Create source and server directories
mkdir $eqemu_server_directory/source
mkdir $eqemu_server_directory/server
mkdir $eqemu_server_directory/server/export
mkdir $eqemu_server_directory/server/logs
mkdir $eqemu_server_directory/server/shared
mkdir $eqemu_server_directory/server/maps

#::: Grab loginserver dependencies
# cd $eqemu_server_directory/source/Server/dependencies
# if [[ "$OS" == "Debian" ]]; then
# 	wget http://eqemu.github.io/downloads/ubuntu_LoginServerCrypto_x64.zip
# 	unzip ubuntu_LoginServerCrypto_x64.zip
# 	rm ubuntu_LoginServerCrypto_x64.zip
# elif [[ "$OS" == "fedora_core" ]] || [[ "$OS" == "red_hat" ]]; then
# 	wget http://eqemu.github.io/downloads/fedora12_LoginServerCrypto_x64.zip
# 	unzip fedora12_LoginServerCrypto_x64.zip
# 	rm fedora12_LoginServerCrypto_x64.zip
# fi
# cd $eqemu_server_directory/source/Server/build

#::: Back to server directory
cd $eqemu_server_directory/server
wget https://raw.githubusercontent.com/EQEmu/Server/master/utils/scripts/eqemu_server.pl

#::: Map lowercase to uppercase to avoid issues
ln -s maps Maps

#::: Notes

perl $eqemu_server_directory/server/eqemu_server.pl new_server

#::: Chown files
chown eqemu:eqemu $eqemu_server_directory/ -R 
chmod 755 $eqemu_server_directory/server/*.pl
chmod 755 $eqemu_server_directory/server/*.sh

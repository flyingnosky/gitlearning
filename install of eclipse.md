##Installation of eclipse
###1. Install OpenJDK Java 7
####(1)Remove old version OpenJDK
If you have installed OpenJDK before, please move it with follow command:
	
	sudo apt-get purge openjdk*

####(2)Add PPA source

	sudo add-apt-repository ppa:webupd8team/java

####(3)update source database

	sudo apt-get update

####(4)Install Oracle Java 7

	sudo apt-get install oracle-java7-installer

Then the installation of OpenJDK is finished,and we can install eclipse next.
###2.Install eclipse
####(1)Download the latest version of eclipse
You can access follow website to download the latest version of eclipse:

	https://www.eclipse.org/downloads/download.php?file=/technology/epp/downloads/release/luna/SR1/eclipse-java-luna-SR1-linux-gtk-x86_64.tar.gz

####(2)decompress eclipse
You can decompress eclipse to current directory or specify directory.

	tar -zxvf eclipse-java-luna-SR1-linux-gtk-x86_64.tar.gz



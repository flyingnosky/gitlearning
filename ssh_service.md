How to access a linux host by windows computer?
We can access a linux host by windows computer through service ssh. We can 
install service sshd in linux A as a server, and then we can access the linux A 
through service ssh by windows B.
The steps are as follows:
    1. Check there is a ssh service in linux A:
        ssh localhost
        if  not install, it will display "connection refuse"
    2. Install ssh service when not install
         sudo apt-get  install openssh-server
    3. restart service
         sudo /etc/init.d/ssh  start
    4. modify config /etc/ssh/ssh_config 
      PermitRoot....=y
    5. Test ssh.
     ssh localhost

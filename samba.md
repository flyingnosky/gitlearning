##vm ubuntu14.04 samba

###1.Install service samba

	sudo apt-get install samba

###2.Create share directory /home/shawn/samba_share

	mkdir -p /home/shawn/samba_share
	chmod 777 /home/shawn/samba_share

###3.modify samba configure file /etc/samba/smb.conf
a.Modify this content as follows:

	security = user
	username map = /etc/samba/smbusers

b.Add follow content to avoid wrong decode

 	workgroup = workgroup
	display charset = UTF-8
	unix charset = UTF-8
	dos charset = cp936

c.Add follow part to allow shawn to access:
```
[Share]
comment = Shared Folder with username and password 
path = /home/shawn/samba_share
 # public = yes 
writable = yes
# readyonly =  yes 
valid users = shawn
create mask = 0700
directory mask = 0700
force user = nobody
force group = nogroup
available = yes
browseable = yes 
```
d.Add smaba user shawn and password

	sudo user add shawn
	sudo smbpasswd -a shawn

e.Add network user at /etc/samba/smbusers
	
	shawn = "network username"

f.Restart service 

	sudo service smbd restart
	sudo service nmbd restart

###4.verify service samba
It is very easy to verify service samba between windowns and ubuntu, and what you should to do is to open command windowns with WIN+r.Then type ubuntu's IP address.

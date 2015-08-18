##How to login in the filesystem when you don't know password
This document describes how to solve the situation that you don't know the name and password when you login in the filesystem OPENSUSE or UBUNTU.In my case, the situation may as follow:
<!-- more -->
1. Login any system you can login, and set the root user a password.
For example, login in my desktop ubuntu and set the password for root user.
	
	password root

And then type the password. In my case, root users' password is 123456.
2.Copy information related to password in my desktop ubuntu, and this information in the file /etc/shadow
	
	cat /etc/shadow
	root:$6$HqBYywyR$79C/NySRrPshy8wXFcwHleJ1wHHjjGuQQKm0V2uyLYWXobGUSKbz3Nc5SMhs5VVTMmNpxZYIYlVm8W7Q86n1m1:16479:0:99999:7:::
	daemon:*:16177:0:99999:7:::
	bin:*:16177:0:99999:7:::
	sys:*:16177:0:99999:7:::
	...

What we should do is copying the first line related to root user.
3.Paste the content to corresponding place in the filesystem you don't know the password. In my case the filesystem is OPENSUSE13.1. \Edit the file /etc/shadow IN ***opensuse13.1***,and replace the first line with the content.
Then the root user's password will be 123456 in the system opensuse13.1.

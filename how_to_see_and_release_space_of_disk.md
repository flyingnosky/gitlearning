How to see and release the space of disk
1.command of watch large of disk
	see all the occupy of space: df -hT
	see the space of a filefold: du -sh /opt/
2.how to release the space
	delete the file you want to delete: rm sas
	though you have deleted it, the space is the same and you have to release it
	search the thread that don't release:  lsof | grep delete
	find the pid of those thread which occupy space: kill -9 pid

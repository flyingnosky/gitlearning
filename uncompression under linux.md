This document shows how to compress and uncompress all kinds of compression files under linux.
###1.Format of tar
compression : tar cvf FileName.tar DirName
decompression : tar xvf FileName.tar
###2.Format of gz
compression : gzip FileName
decompression : gzip -d FileName
		gunzip FileName
###3.Format of tar.gz
compression : tar zcvf FileName.tar.gz DirName
decompression : gzip -d FileName.tar.gz
		tar xvf FileName.tar
###4.Format of .bz2
compression : bzip2 -z DirName
decompression : bzip2 -d FileName.bz2
		or bunzip2 FileName.bz2
###5.Format of tar.bz2
compression : tar jcvf FileName.tar.bz2 DirName
decompresson : tar jxvf FileName.tar.bz2
###6.Format of bz
compression : bzip2自动创建bz2格式文件,无法创建bz格式
decompression : bzip2 -d FileName.bz
	        bunzip2 FileName.bz
###7.Format of tar.bz
compression : 同上
decompression : tar jxvf FileName.tar.bz
###8.Format of tar.xz
compression : tar -cvf FileName.tar DirName && xz -z FileName.tar
	 	or tar -Jcvf FileName.tar.xz DirName
decompression : xz -d FileName.tar.xz && tar -xvf FileName.tar
		tar -Jxvf FileName.tar.xz







http://rogerdudler.github.io/git-guide/index.zh.html

##Turn vim into IDE
vim is a very popular editer before installing pulgins.If we want to edit and view our codes more easily, we should
install some plugins about vim after we install vim. We can install vim as follow:

	sudo apt-get update	
	sudo apt-get install vim

###1.Install of Ctags
Ctags tools can locate some tag or symbol more quickly in our codes.There are some steps to install Ctags.
####(1)Download and decompress source code package of ctags
You can download from this website:
	
	http://ctags.sourceforge.net/

####(2)configure && compile && install ctags

	./configure --prefix=/path/to/local/ && make && make install

####(3)modify configure file of vim
if you find that file **~/.vimrc** is not exist, you can copy /etc/vim/vimrc to ~/.vimrc.
Then you modify .vimrc file to add this line:
	
	map <C-F12> :!ctags -R --c++-kinds=+p --fields=+iaS --extra=+q .<CR>

####(4)Use Ctags under our code domain directory
We should excute follow command under our code domain directory:

	ctags -R --c++-kinds=+p --fields=+iaS --extra=+q .

Then we can use ctags tools as we want.
There are some shortcuts to use ctags:
	
    Ctrl-]     jump to the define of symbol 
	Ctrl-t 	   jump back to last location
	

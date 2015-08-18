title: cpio Filesystem
date: 2015-04-17 10:09:00
permalink: abc
categories: Basic
---
##How To Add Services Into cpio Filesystem To Make Cpu and Memory High Load
Sometimes some performances of D02 board should be tested under high cpu and memory load, so we can add some stress services into filesystem to realize the requirements. In my case, we already have cpio filesystem hulk-hip05.cpio.gz and stress service will be added into cpio filesystem as following steps.
<!-- more -->
###(1)What is stress
Stress is a simple workload generator for POSIX system. It imposes a configurable amount of CPU,memory, I/O, and disk stress on the system. We can download it from website:

	http://www.filewatcher.com/m/stress-1.0.1.tar.gz.203343-1.html

###(2)Preparation
a.Download 64bit cross compile toolchain and export the path
we can get this toolchain from the website:

	http://releases.linaro.org/14.09/components/toolchain/binaries/gcc-linaro-aarch64-linux-gnu-4.9-2014.09_linux.tar.xz

b.Decompress cpio filesystem hulk-hip05.cpio.gz
The steps of decompression cpio filesystem hulk-hip05.cpio.gz are as follows:

	cd /path/of/hulk-hip05/
	gunzip hulk-hip05.cpio.gz
	cpio -idmv < hulk-hip05.cpio.gz

After that we get fold named hulk-hip05 which includes files of cpio filesystem.And/path/of/hulk-hip05/ is the location path of hulk-hip05.
***Note:we must decompress and make cpio filesystem with ROOT user, or we will lost some services. This's important.***
###(3)How to compile stress
We can compile service stress as the following step:

	./configure CC=aarch64-linux-gnu-gcc  --host=arm64  --prefix=/path/of/hulk-hip05/usr/local
	make 
	make install

###(4)How to make cpio filesystem
a.Run ###gen_initramfs_list.sh### to create filelist

	./gen_initramfs_list.sh ./hulk-hip05/ > filelist

Note:gen_initramfs_list.sh is the script that creates filelist. 
b.Create gen_init_cpio tool through source code
gen_init_cpio is a tool that trasfer filelist into cpio filesystem.we can create this tool by building the source codes.

	gcc -o gen_init_cpio gen_init_cpio.c

Note:source code gen_init_cpio.c are below.
c.Create cpio filesystem using gen_init_cpio

	gen_init_cpio filelist > rootfs.cpio

d.Compress cpio filesystem into cpio.gz filesystem  

	gzip rootfs.cpio

###script of gen_initramfs_list.sh:
```
#!/bin/sh
# Copyright (C) Martin Schlemmer <azarah@nosferatu.za.org>
# Copyright (C) 2006 Sam Ravnborg <sam@ravnborg.org>
#
# Released under the terms of the GNU GPL
#
# Generate a cpio packed initramfs. It uses gen_init_cpio to generate
# the cpio archive, and then compresses it.
# The script may also be used to generate the inputfile used for gen_init_cpio
# This script assumes that gen_init_cpio is located in usr/ directory
# error out on errors
set -e
usage() {
cat << EOF
Usage:
$0 [-o <file>] [-u <uid>] [-g <gid>] {-d | <cpio_source>} ...
	-o <file>      Create compressed initramfs file named <file> using
		       gen_init_cpio and compressor depending on the extension
	-u <uid>       User ID to map to user ID 0 (root).
		       <uid> is only meaningful if <cpio_source> is a
		       directory.  "squash" forces all files to uid 0.
	-g <gid>       Group ID to map to group ID 0 (root).
		       <gid> is only meaningful if <cpio_source> is a
		       directory.  "squash" forces all files to gid 0.
	<cpio_source>  File list or directory for cpio archive.
		       If <cpio_source> is a .cpio file it will be used
		       as direct input to initramfs.
	-d             Output the default cpio list.
All options except -o and -l may be repeated and are interpreted
sequentially and immediately.  -u and -g states are preserved across
<cpio_source> options so an explicit "-u 0 -g 0" is required
to reset the root/group mapping.
EOF
}
# awk style field access
# $1 - field number; rest is argument string
field() {
	shift $1 ; echo $1
}
list_default_initramfs() {
	# echo usr/kinit/kinit
	:
}
default_initramfs() {
	cat <<-EOF >> ${output}
		# This is a very simple, default initramfs

		dir /dev 0755 0 0
		nod /dev/console 0600 0 0 c 5 1
		dir /root 0700 0 0
		# file /kinit usr/kinit/kinit 0755 0 0
		# slink /init kinit 0755 0 0
	EOF
}
filetype() {
	local argv1="$1"

	# symlink test must come before file test
	if [ -L "${argv1}" ]; then
		echo "slink"
	elif [ -f "${argv1}" ]; then
		echo "file"
	elif [ -d "${argv1}" ]; then
		echo "dir"
	elif [ -b "${argv1}" -o -c "${argv1}" ]; then
		echo "nod"
	elif [ -p "${argv1}" ]; then
		echo "pipe"
	elif [ -S "${argv1}" ]; then
		echo "sock"
	else
		echo "invalid"
	fi
	return 0
}
list_print_mtime() {
	:
}
print_mtime() {
	local my_mtime="0"

	if [ -e "$1" ]; then
		my_mtime=$(find "$1" -printf "%T@\n" | sort -r | head -n 1)
	fi

	echo "# Last modified: ${my_mtime}" >> ${output}
	echo "" >> ${output}
}
list_parse() {
	[ ! -L "$1" ] && echo "$1 \\" || :
}
# for each file print a line in following format
# <filetype> <name> <path to file> <octal mode> <uid> <gid>
# for links, devices etc the format differs. See gen_init_cpio for details
parse() {
	local location="$1"
	local name="/${location#${srcdir}}"
	# change '//' into '/'
	name=$(echo "$name" | sed -e 's://*:/:g')
	local mode="$2"
	local uid="$3"
	local gid="$4"
	local ftype=$(filetype "${location}")
	# remap uid/gid to 0 if necessary
	[ "$root_uid" = "squash" ] && uid=0 || [ "$uid" -eq "$root_uid" ] && uid=0
	[ "$root_gid" = "squash" ] && gid=0 || [ "$gid" -eq "$root_gid" ] && gid=0
	local str="${mode} ${uid} ${gid}"
	[ "${ftype}" = "invalid" ] && return 0
	[ "${location}" = "${srcdir}" ] && return 0
	case "${ftype}" in
		"file")
			str="${ftype} ${name} ${location} ${str}"
			;;
		"nod")
			local dev=`LC_ALL=C ls -l "${location}"`
			local maj=`field 5 ${dev}`
			local min=`field 6 ${dev}`
			maj=${maj%,}
			[ -b "${location}" ] && dev="b" || dev="c"
			str="${ftype} ${name} ${str} ${dev} ${maj} ${min}"
			;;
		"slink")
			local target=`readlink "${location}"`
			str="${ftype} ${name} ${target} ${str}"
			;;
		*)
			str="${ftype} ${name} ${str}"
			;;
	esac
	echo "${str}" >> ${output}
	return 0
}
unknown_option() {
	printf "ERROR: unknown option \"$arg\"\n" >&2
	printf "If the filename validly begins with '-', " >&2
	printf "then it must be prefixed\n" >&2
	printf "by './' so that it won't be interpreted as an option." >&2
	printf "\n" >&2
	usage >&2
	exit 1
}
list_header() {
	:
}
header() {
	printf "\n#####################\n# $1\n" >> ${output}
}
# process one directory (incl sub-directories)
dir_filelist() {
	${dep_list}header "$1"

	srcdir=$(echo "$1" | sed -e 's://*:/:g')
	dirlist=$(find "${srcdir}" -printf "%p %m %U %G\n")

	# If $dirlist is only one line, then the directory is empty
	if [  "$(echo "${dirlist}" | wc -l)" -gt 1 ]; then
		${dep_list}print_mtime "$1"

		echo "${dirlist}" | \
		while read x; do
			${dep_list}parse ${x}
		done
	fi
}

# if only one file is specified and it is .cpio file then use it direct as fs
# if a directory is specified then add all files in given direcotry to fs
# if a regular file is specified assume it is in gen_initramfs format
input_file() {
	source="$1"
	if [ -f "$1" ]; then
		${dep_list}header "$1"
		is_cpio="$(echo "$1" | sed 's/^.*\.cpio\(\..*\)\?/cpio/')"
		if [ $2 -eq 0 -a ${is_cpio} = "cpio" ]; then
			cpio_file=$1
			echo "$1" | grep -q '^.*\.cpio\..*' && is_cpio_compressed="compressed"
			[ ! -z ${dep_list} ] && echo "$1"
			return 0
		fi
		if [ -z ${dep_list} ]; then
			print_mtime "$1" >> ${output}
			cat "$1"         >> ${output}
		else
		        echo "$1 \\"
			cat "$1" | while read type dir file perm ; do
				if [ "$type" = "file" ]; then
					echo "$file \\";
				fi
			done
		fi
	elif [ -d "$1" ]; then
		dir_filelist "$1"
	else
		echo "  ${prog}: Cannot open '$1'" >&2
		exit 1
	fi
}

prog=$0
root_uid=0
root_gid=0
dep_list=
cpio_file=
cpio_list=
output="/dev/stdout"
output_file=""
is_cpio_compressed=
compr="gzip -n -9 -f"

arg="$1"
case "$arg" in
	"-l")	# files included in initramfs - used by kbuild
		dep_list="list_"
		echo "deps_initramfs := $0 \\"
		shift
		;;
	"-o")	# generate compressed cpio image named $1
		shift
		output_file="$1"
		cpio_list="$(mktemp ${TMPDIR:-/tmp}/cpiolist.XXXXXX)"
		output=${cpio_list}
		echo "$output_file" | grep -q "\.gz$" && compr="gzip -n -9 -f"
		echo "$output_file" | grep -q "\.bz2$" && compr="bzip2 -9 -f"
		echo "$output_file" | grep -q "\.lzma$" && compr="lzma -9 -f"
		echo "$output_file" | grep -q "\.xz$" && \
				compr="xz --check=crc32 --lzma2=dict=1MiB"
		echo "$output_file" | grep -q "\.lzo$" && compr="lzop -9 -f"
		echo "$output_file" | grep -q "\.cpio$" && compr="cat"
		shift
		;;
esac
while [ $# -gt 0 ]; do
	arg="$1"
	shift
	case "$arg" in
		"-u")	# map $1 to uid=0 (root)
			root_uid="$1"
			shift
			;;
		"-g")	# map $1 to gid=0 (root)
			root_gid="$1"
			shift
			;;
		"-d")	# display default initramfs list
			default_list="$arg"
			${dep_list}default_initramfs
			;;
		"-h")
			usage
			exit 0
			;;
		*)
			case "$arg" in
				"-"*)
					unknown_option
					;;
				*)	# input file/dir - process it
					input_file "$arg" "$#"
					;;
			esac
			;;
	esac
done

# If output_file is set we will generate cpio archive and compress it
# we are careful to delete tmp files
if [ ! -z ${output_file} ]; then
	if [ -z ${cpio_file} ]; then
		timestamp=
		if test -n "$KBUILD_BUILD_TIMESTAMP"; then
			timestamp="$(date -d"$KBUILD_BUILD_TIMESTAMP" +%s || :)"
			if test -n "$timestamp"; then
				timestamp="-t $timestamp"
			fi
		fi
		cpio_tfile="$(mktemp ${TMPDIR:-/tmp}/cpiofile.XXXXXX)"
		usr/gen_init_cpio $timestamp ${cpio_list} > ${cpio_tfile}
	else
		cpio_tfile=${cpio_file}
	fi
	rm ${cpio_list}
	if [ "${is_cpio_compressed}" = "compressed" ]; then
		cat ${cpio_tfile} > ${output_file}
	else
		(cat ${cpio_tfile} | ${compr}  - > ${output_file}) \
		|| (rm -f ${output_file} ; false)
	fi
	[ -z ${cpio_file} ] && rm ${cpio_tfile}
fi
exit 0
```
###source code of gen_init_cpio.c:
```
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <limits.h>

/*
 * Original work by Jeff Garzik
 *
 * External file lists, symlink, pipe and fifo support by Thayne Harbaugh
 * Hard link support by Luciano Rocha
 */

#define xstr(s) #s
#define str(s) xstr(s)

static unsigned int offset;
static unsigned int ino = 721;
static time_t default_mtime;

struct file_handler {
	const char *type;
	int (*handler)(const char *line);
};

static void push_string(const char *name)
{
	unsigned int name_len = strlen(name) + 1;

	fputs(name, stdout);
	putchar(0);
	offset += name_len;
}

static void push_pad (void)
{
	while (offset & 3) {
		putchar(0);
		offset++;
	}
}

static void push_rest(const char *name)
{
	unsigned int name_len = strlen(name) + 1;
	unsigned int tmp_ofs;

	fputs(name, stdout);
	putchar(0);
	offset += name_len;

	tmp_ofs = name_len + 110;
	while (tmp_ofs & 3) {
		putchar(0);
		offset++;
		tmp_ofs++;
	}
}

static void push_hdr(const char *s)
{
	fputs(s, stdout);
	offset += 110;
}

static void cpio_trailer(void)
{
	char s[256];
	const char name[] = "TRAILER!!!";

	sprintf(s, "%s%08X%08X%08lX%08lX%08X%08lX"
	       "%08X%08X%08X%08X%08X%08X%08X",
		"070701",		/* magic */
		0,			/* ino */
		0,			/* mode */
		(long) 0,		/* uid */
		(long) 0,		/* gid */
		1,			/* nlink */
		(long) 0,		/* mtime */
		0,			/* filesize */
		0,			/* major */
		0,			/* minor */
		0,			/* rmajor */
		0,			/* rminor */
		(unsigned)strlen(name)+1, /* namesize */
		0);			/* chksum */
	push_hdr(s);
	push_rest(name);

	while (offset % 512) {
		putchar(0);
		offset++;
	}
}

static int cpio_mkslink(const char *name, const char *target,
			 unsigned int mode, uid_t uid, gid_t gid)
{
	char s[256];

	if (name[0] == '/')
		name++;
	sprintf(s,"%s%08X%08X%08lX%08lX%08X%08lX"
	       "%08X%08X%08X%08X%08X%08X%08X",
		"070701",		/* magic */
		ino++,			/* ino */
		S_IFLNK | mode,		/* mode */
		(long) uid,		/* uid */
		(long) gid,		/* gid */
		1,			/* nlink */
		(long) default_mtime,	/* mtime */
		(unsigned)strlen(target)+1, /* filesize */
		3,			/* major */
		1,			/* minor */
		0,			/* rmajor */
		0,			/* rminor */
		(unsigned)strlen(name) + 1,/* namesize */
		0);			/* chksum */
	push_hdr(s);
	push_string(name);
	push_pad();
	push_string(target);
	push_pad();
	return 0;
}

static int cpio_mkslink_line(const char *line)
{
	char name[PATH_MAX + 1];
	char target[PATH_MAX + 1];
	unsigned int mode;
	int uid;
	int gid;
	int rc = -1;

	if (5 != sscanf(line, "%" str(PATH_MAX) "s %" str(PATH_MAX) "s %o %d %d", name, target, &mode, &uid, &gid)) {
		fprintf(stderr, "Unrecognized dir format '%s'", line);
		goto fail;
	}
	rc = cpio_mkslink(name, target, mode, uid, gid);
 fail:
	return rc;
}

static int cpio_mkgeneric(const char *name, unsigned int mode,
		       uid_t uid, gid_t gid)
{
	char s[256];

	if (name[0] == '/')
		name++;
	sprintf(s,"%s%08X%08X%08lX%08lX%08X%08lX"
	       "%08X%08X%08X%08X%08X%08X%08X",
		"070701",		/* magic */
		ino++,			/* ino */
		mode,			/* mode */
		(long) uid,		/* uid */
		(long) gid,		/* gid */
		2,			/* nlink */
		(long) default_mtime,	/* mtime */
		0,			/* filesize */
		3,			/* major */
		1,			/* minor */
		0,			/* rmajor */
		0,			/* rminor */
		(unsigned)strlen(name) + 1,/* namesize */
		0);			/* chksum */
	push_hdr(s);
	push_rest(name);
	return 0;
}

enum generic_types {
	GT_DIR,
	GT_PIPE,
	GT_SOCK
};

struct generic_type {
	const char *type;
	mode_t mode;
};

static struct generic_type generic_type_table[] = {
	[GT_DIR] = {
		.type = "dir",
		.mode = S_IFDIR
	},
	[GT_PIPE] = {
		.type = "pipe",
		.mode = S_IFIFO
	},
	[GT_SOCK] = {
		.type = "sock",
		.mode = S_IFSOCK
	}
};

static int cpio_mkgeneric_line(const char *line, enum generic_types gt)
{
	char name[PATH_MAX + 1];
	unsigned int mode;
	int uid;
	int gid;
	int rc = -1;

	if (4 != sscanf(line, "%" str(PATH_MAX) "s %o %d %d", name, &mode, &uid, &gid)) {
		fprintf(stderr, "Unrecognized %s format '%s'",
			line, generic_type_table[gt].type);
		goto fail;
	}
	mode |= generic_type_table[gt].mode;
	rc = cpio_mkgeneric(name, mode, uid, gid);
 fail:
	return rc;
}

static int cpio_mkdir_line(const char *line)
{
	return cpio_mkgeneric_line(line, GT_DIR);
}

static int cpio_mkpipe_line(const char *line)
{
	return cpio_mkgeneric_line(line, GT_PIPE);
}

static int cpio_mksock_line(const char *line)
{
	return cpio_mkgeneric_line(line, GT_SOCK);
}

static int cpio_mknod(const char *name, unsigned int mode,
		       uid_t uid, gid_t gid, char dev_type,
		       unsigned int maj, unsigned int min)
{
	char s[256];

	if (dev_type == 'b')
		mode |= S_IFBLK;
	else
		mode |= S_IFCHR;

	if (name[0] == '/')
		name++;
	sprintf(s,"%s%08X%08X%08lX%08lX%08X%08lX"
	       "%08X%08X%08X%08X%08X%08X%08X",
		"070701",		/* magic */
		ino++,			/* ino */
		mode,			/* mode */
		(long) uid,		/* uid */
		(long) gid,		/* gid */
		1,			/* nlink */
		(long) default_mtime,	/* mtime */
		0,			/* filesize */
		3,			/* major */
		1,			/* minor */
		maj,			/* rmajor */
		min,			/* rminor */
		(unsigned)strlen(name) + 1,/* namesize */
		0);			/* chksum */
	push_hdr(s);
	push_rest(name);
	return 0;
}

static int cpio_mknod_line(const char *line)
{
	char name[PATH_MAX + 1];
	unsigned int mode;
	int uid;
	int gid;
	char dev_type;
	unsigned int maj;
	unsigned int min;
	int rc = -1;

	if (7 != sscanf(line, "%" str(PATH_MAX) "s %o %d %d %c %u %u",
			 name, &mode, &uid, &gid, &dev_type, &maj, &min)) {
		fprintf(stderr, "Unrecognized nod format '%s'", line);
		goto fail;
	}
	rc = cpio_mknod(name, mode, uid, gid, dev_type, maj, min);
 fail:
	return rc;
}

static int cpio_mkfile(const char *name, const char *location,
			unsigned int mode, uid_t uid, gid_t gid,
			unsigned int nlinks)
{
	char s[256];
	char *filebuf = NULL;
	struct stat buf;
	long size;
	int file = -1;
	int retval;
	int rc = -1;
	int namesize;
	int i;

	mode |= S_IFREG;

	file = open (location, O_RDONLY);
	if (file < 0) {
		fprintf (stderr, "File %s could not be opened for reading\n", location);
		goto error;
	}

	retval = fstat(file, &buf);
	if (retval) {
		fprintf(stderr, "File %s could not be stat()'ed\n", location);
		goto error;
	}

	filebuf = malloc(buf.st_size);
	if (!filebuf) {
		fprintf (stderr, "out of memory\n");
		goto error;
	}

	retval = read (file, filebuf, buf.st_size);
	if (retval < 0) {
		fprintf (stderr, "Can not read %s file\n", location);
		goto error;
	}

	size = 0;
	for (i = 1; i <= nlinks; i++) {
		/* data goes on last link */
		if (i == nlinks) size = buf.st_size;

		if (name[0] == '/')
			name++;
		namesize = strlen(name) + 1;
		sprintf(s,"%s%08X%08X%08lX%08lX%08X%08lX"
		       "%08lX%08X%08X%08X%08X%08X%08X",
			"070701",		/* magic */
			ino,			/* ino */
			mode,			/* mode */
			(long) uid,		/* uid */
			(long) gid,		/* gid */
			nlinks,			/* nlink */
			(long) buf.st_mtime,	/* mtime */
			size,			/* filesize */
			3,			/* major */
			1,			/* minor */
			0,			/* rmajor */
			0,			/* rminor */
			namesize,		/* namesize */
			0);			/* chksum */
		push_hdr(s);
		push_string(name);
		push_pad();

		if (size) {
			if (fwrite(filebuf, size, 1, stdout) != 1) {
				fprintf(stderr, "writing filebuf failed\n");
				goto error;
			}
			offset += size;
			push_pad();
		}

		name += namesize;
	}
	ino++;
	rc = 0;
	
error:
	if (filebuf) free(filebuf);
	if (file >= 0) close(file);
	return rc;
}

static char *cpio_replace_env(char *new_location)
{
       char expanded[PATH_MAX + 1];
       char env_var[PATH_MAX + 1];
       char *start;
       char *end;

       for (start = NULL; (start = strstr(new_location, "${")); ) {
               end = strchr(start, '}');
               if (start < end) {
                       *env_var = *expanded = '\0';
                       strncat(env_var, start + 2, end - start - 2);
                       strncat(expanded, new_location, start - new_location);
                       strncat(expanded, getenv(env_var), PATH_MAX);
                       strncat(expanded, end + 1, PATH_MAX);
                       strncpy(new_location, expanded, PATH_MAX);
               } else
                       break;
       }

       return new_location;
}


static int cpio_mkfile_line(const char *line)
{
	char name[PATH_MAX + 1];
	char *dname = NULL; /* malloc'ed buffer for hard links */
	char location[PATH_MAX + 1];
	unsigned int mode;
	int uid;
	int gid;
	int nlinks = 1;
	int end = 0, dname_len = 0;
	int rc = -1;

	if (5 > sscanf(line, "%" str(PATH_MAX) "s %" str(PATH_MAX)
				"s %o %d %d %n",
				name, location, &mode, &uid, &gid, &end)) {
		fprintf(stderr, "Unrecognized file format '%s'", line);
		goto fail;
	}
	if (end && isgraph(line[end])) {
		int len;
		int nend;

		dname = malloc(strlen(line));
		if (!dname) {
			fprintf (stderr, "out of memory (%d)\n", dname_len);
			goto fail;
		}

		dname_len = strlen(name) + 1;
		memcpy(dname, name, dname_len);

		do {
			nend = 0;
			if (sscanf(line + end, "%" str(PATH_MAX) "s %n",
					name, &nend) < 1)
				break;
			len = strlen(name) + 1;
			memcpy(dname + dname_len, name, len);
			dname_len += len;
			nlinks++;
			end += nend;
		} while (isgraph(line[end]));
	} else {
		dname = name;
	}
	rc = cpio_mkfile(dname, cpio_replace_env(location),
	                 mode, uid, gid, nlinks);
 fail:
	if (dname_len) free(dname);
	return rc;
}

static void usage(const char *prog)
{
	fprintf(stderr, "Usage:\n"
		"\t%s [-t <timestamp>] <cpio_list>\n"
		"\n"
		"<cpio_list> is a file containing newline separated entries that\n"
		"describe the files to be included in the initramfs archive:\n"
		"\n"
		"# a comment\n"
		"file <name> <location> <mode> <uid> <gid> [<hard links>]\n"
		"dir <name> <mode> <uid> <gid>\n"
		"nod <name> <mode> <uid> <gid> <dev_type> <maj> <min>\n"
		"slink <name> <target> <mode> <uid> <gid>\n"
		"pipe <name> <mode> <uid> <gid>\n"
		"sock <name> <mode> <uid> <gid>\n"
		"\n"
		"<name>       name of the file/dir/nod/etc in the archive\n"
		"<location>   location of the file in the current filesystem\n"
		"             expands shell variables quoted with ${}\n"
		"<target>     link target\n"
		"<mode>       mode/permissions of the file\n"
		"<uid>        user id (0=root)\n"
		"<gid>        group id (0=root)\n"
		"<dev_type>   device type (b=block, c=character)\n"
		"<maj>        major number of nod\n"
		"<min>        minor number of nod\n"
		"<hard links> space separated list of other links to file\n"
		"\n"
		"example:\n"
		"# A simple initramfs\n"
		"dir /dev 0755 0 0\n"
		"nod /dev/console 0600 0 0 c 5 1\n"
		"dir /root 0700 0 0\n"
		"dir /sbin 0755 0 0\n"
		"file /sbin/kinit /usr/src/klibc/kinit/kinit 0755 0 0\n"
		"\n"
		"<timestamp> is time in seconds since Epoch that will be used\n"
		"as mtime for symlinks, special files and directories. The default\n"
		"is to use the current time for these entries.\n",
		prog);
}

struct file_handler file_handler_table[] = {
	{
		.type    = "file",
		.handler = cpio_mkfile_line,
	}, {
		.type    = "nod",
		.handler = cpio_mknod_line,
	}, {
		.type    = "dir",
		.handler = cpio_mkdir_line,
	}, {
		.type    = "slink",
		.handler = cpio_mkslink_line,
	}, {
		.type    = "pipe",
		.handler = cpio_mkpipe_line,
	}, {
		.type    = "sock",
		.handler = cpio_mksock_line,
	}, {
		.type    = NULL,
		.handler = NULL,
	}
};

#define LINE_SIZE (2 * PATH_MAX + 50)

int main (int argc, char *argv[])
{
	FILE *cpio_list;
	char line[LINE_SIZE];
	char *args, *type;
	int ec = 0;
	int line_nr = 0;
	const char *filename;

	default_mtime = time(NULL);
	while (1) {
		int opt = getopt(argc, argv, "t:h");
		char *invalid;

		if (opt == -1)
			break;
		switch (opt) {
		case 't':
			default_mtime = strtol(optarg, &invalid, 10);
			if (!*optarg || *invalid) {
				fprintf(stderr, "Invalid timestamp: %s\n",
						optarg);
				usage(argv[0]);
				exit(1);
			}
			break;
		case 'h':
		case '?':
			usage(argv[0]);
			exit(opt == 'h' ? 0 : 1);
		}
	}

	if (argc - optind != 1) {
		usage(argv[0]);
		exit(1);
	}
	filename = argv[optind];
	if (!strcmp(filename, "-"))
		cpio_list = stdin;
	else if (!(cpio_list = fopen(filename, "r"))) {
		fprintf(stderr, "ERROR: unable to open '%s': %s\n\n",
			filename, strerror(errno));
		usage(argv[0]);
		exit(1);
	}

	while (fgets(line, LINE_SIZE, cpio_list)) {
		int type_idx;
		size_t slen = strlen(line);

		line_nr++;

		if ('#' == *line) {
			/* comment - skip to next line */
			continue;
		}

		if (! (type = strtok(line, " \t"))) {
			fprintf(stderr,
				"ERROR: incorrect format, could not locate file type line %d: '%s'\n",
				line_nr, line);
			ec = -1;
			break;
		}

		if ('\n' == *type) {
			/* a blank line */
			continue;
		}

		if (slen == strlen(type)) {
			/* must be an empty line */
			continue;
		}

		if (! (args = strtok(NULL, "\n"))) {
			fprintf(stderr,
				"ERROR: incorrect format, newline required line %d: '%s'\n",
				line_nr, line);
			ec = -1;
		}

		for (type_idx = 0; file_handler_table[type_idx].type; type_idx++) {
			int rc;
			if (! strcmp(line, file_handler_table[type_idx].type)) {
				if ((rc = file_handler_table[type_idx].handler(args))) {
					ec = rc;
					fprintf(stderr, " line %d\n", line_nr);
				}
				break;
			}
		}

		if (NULL == file_handler_table[type_idx].type) {
			fprintf(stderr, "unknown file type line %d: '%s'\n",
				line_nr, line);
		}
	}
	if (ec == 0)
		cpio_trailer();

	exit(ec);
}
```

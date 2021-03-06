/*
 * Copyright (c) 2013 Fujitsu Ltd.
 * Author: DAN LI <li.dan@cn.fujitsu.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

/*
 *  DESCRIPTION
 *	Test for feature MS_BIND of mount.
 *	"Perform a bind mount, making a file or a directory subtree visible
 *	 at another point within a file system."
 */

#include <errno.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "test.h"
#include "usctest.h"
#include "safe_macros.h"

static void help(void);
static void setup(void);
static void cleanup(void);

char *TCID = "mount05";
int TST_TOTAL = 1;

#define DIR_MODE	(S_IRWXU | S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP)

static const char *fs_type = "ext2";

static int tflag;
static int dflag;
static char *fstype;
static char *device;
static const char file_src[] = "mnt_src/tstfile";
static const char file_des[] = "mnt_des/tstfile";
static const char mntpoint_src[] = "mnt_src";
static const char mntpoint_des[] = "mnt_des";

static option_t options[] = {
	{"T:", &tflag, &fstype},
	{"D:", &dflag, &device},
	{NULL, NULL, NULL},
};

int main(int argc, char *argv[])
{
	int lc;
	char *msg;

	msg = parse_opts(argc, argv, options, &help);
	if (msg != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	if (tflag)
		fs_type = fstype;

	if (STD_COPIES != 1) {
		tst_resm(TINFO, "-c option has no effect for this testcase - "
			 "%s doesn't allow running more than one instance "
			 "at a time", TCID);
		STD_COPIES = 1;
	}

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		TEST(mount(mntpoint_src, mntpoint_des, fs_type, MS_BIND, NULL));

		if (TEST_RETURN != 0) {
			tst_resm(TFAIL | TTERRNO, "mount(2) failed");
		} else {

			if (open(file_des, O_CREAT | O_EXCL, S_IRWXU) == -1 &&
			    errno == EEXIST)
				tst_resm(TPASS, "bind mount is ok");
			else
				tst_resm(TFAIL, "file %s is not available",
					 file_des);

			TEST(umount(mntpoint_des));
			if (TEST_RETURN != 0)
				tst_brkm(TBROK | TTERRNO, cleanup,
					 "umount(2) failed");
		}
	}

	cleanup();

	tst_exit();
}

void setup(void)
{
	tst_require_root(NULL);

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	tst_tmpdir();

	SAFE_MKDIR(cleanup, mntpoint_src, DIR_MODE);
	SAFE_MKDIR(cleanup, mntpoint_des, DIR_MODE);

	if (dflag) {
		tst_mkfs(NULL, device, fs_type, NULL);

		if (mount(device, mntpoint_src, fs_type, 0, NULL) == -1)
			tst_brkm(TBROK | TERRNO, cleanup, "mount failed");
	}

	SAFE_FILE_PRINTF(cleanup, file_src, "TEST FILE");

	TEST_PAUSE;
}

void cleanup(void)
{
	if (dflag)
		if (umount(mntpoint_src) != 0)
			tst_brkm(TBROK | TTERRNO, NULL, "umount(2) failed");

	TEST_CLEANUP;

	tst_rmdir();
}

void help(void)
{
	printf("-T type	  : specifies the type of filesystem to be mounted. "
	       "Default ext2.\n");
	printf("-D device : device used for mounting.\n");
}

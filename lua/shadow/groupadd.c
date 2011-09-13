/*
 * This file was originally distributed as part of
 * shadow-utils-4.0.17-12.fc6.src.rpm and has been modified
 * in WRLinux for inclusion in rpm.
 */

/*
 * Copyright 1991 - 1993, Julianne Frances Haugh
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of Julianne F. Haugh nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY JULIE HAUGH AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL JULIE HAUGH OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "system.h"
#include "shadow_config.h"

#ident "$Id: groupadd.c,v 1.7 2010/10/18 16:29:16 jbj Exp $"

#include <ctype.h>
#include <fcntl.h>
#include <getopt.h>
#include <grp.h>
#include <stdio.h>
#include <sys/types.h>
#include <setjmp.h>
#ifdef USE_PAM
#include "pam_defs.h"
#include <pwd.h>
#endif				/* USE_PAM */
#include "chkname.h"
#include "defines.h"
#include "getdef.h"
#include "groupio.h"
#include "nscd.h"
#include "prototypes.h"
#ifdef	SHADOWGRP
#include "sgroupio.h"
static int is_shadow_grp;
#endif

/*
 * exit status values
 */
#define E_SUCCESS	0	/* success */
#define E_USAGE		2	/* invalid command syntax */
#define E_BAD_ARG	3	/* invalid argument to option */
#define E_GID_IN_USE	4	/* gid not unique (when -o not used) */
#define E_NAME_IN_USE	9	/* group name not unique */
#define E_GRP_UPDATE	10	/* can't update group file */

#define NO_GETGRENT 1

/*
 * Global variables
 */

static jmp_buf lclack;    /* longjmp() instead of exit()'ing in local functions! */

extern char *optarg;
extern int optind;

static char *group_name;
static gid_t group_id;
static char *empty_list = NULL;

static char *Prog = "rpm:lua:wrs.groupadd";

static int oflg = 0;		/* permit non-unique group ID to be specified with -g */
static int gflg = 0;		/* ID value for the new group */
static int fflg = 0;		/* if group already exists, do nothing and exit(0) */
static int rflg = 0;            /* for adding system accounts (Red Hat) */

/* local function prototypes */
static void usage (void);
static void new_grent (struct group *);

#ifdef SHADOWGRP
static void new_sgent (struct sgrp *);
#endif
static void grp_update (void);
static void find_new_gid (void);
static void check_new_name (void);
static void close_files (void);
static void open_files (void);
static void fail_exit (int);
static gid_t get_gid (const char *gidstr);

/*
 * usage - display usage message and exit
 */
static void usage (void)
{
	fprintf (stderr, _("Usage: groupadd [options] group\n"
			   "\n"
			   "Options:\n"
			   "  -f, --force			force exit with success status if the specified\n"
			   "				group already exists\n"
			   "  -r,                       create system account\n"
			   "  -g, --gid GID			use GID for the new group\n"
			   "  -h, --help			display this help message and exit\n"
			   "  -K, --key KEY=VALUE		overrides /etc/login.defs defaults\n"
			   "  -o, --non-unique		allow create group with duplicate\n"
			   "				(non-unique) GID\n"
			   "\n"));
	fail_exit (E_USAGE);
}

/*
 * new_grent - initialize the values in a group file entry
 *
 *	new_grent() takes all of the values that have been entered and fills
 *	in a (struct group) with them.
 */
static void new_grent (struct group *grent)
{
	memzero (grent, sizeof *grent);
	grent->gr_name = group_name;
	grent->gr_passwd = SHADOW_PASSWD_STRING;	/* XXX warning: const */
	grent->gr_gid = group_id;
	grent->gr_mem = &empty_list;
}

#ifdef	SHADOWGRP
/*
 * new_sgent - initialize the values in a shadow group file entry
 *
 *	new_sgent() takes all of the values that have been entered and fills
 *	in a (struct sgrp) with them.
 */
static void new_sgent (struct sgrp *sgent)
{
	memzero (sgent, sizeof *sgent);
	sgent->sg_name = group_name;
	sgent->sg_passwd = "!";	/* XXX warning: const */
	sgent->sg_adm = &empty_list;
	sgent->sg_mem = &empty_list;
}
#endif				/* SHADOWGRP */

/*
 * grp_update - add new group file entries
 *
 *	grp_update() writes the new records to the group files.
 */
static void grp_update (void)
{
	struct group grp;

#ifdef	SHADOWGRP
	struct sgrp sgrp;
#endif				/* SHADOWGRP */

	/*
	 * Create the initial entries for this new group.
	 */
	new_grent (&grp);
#ifdef	SHADOWGRP
	new_sgent (&sgrp);
#endif				/* SHADOWGRP */

	/*
	 * Write out the new group file entry.
	 */
	if (!gr_update (&grp)) {
		fprintf (stderr, _("%s: error adding new group entry\n"), Prog);
		fail_exit (E_GRP_UPDATE);
	}
#ifdef	SHADOWGRP
	/*
	 * Write out the new shadow group entries as well.
	 */
	if (is_shadow_grp && !sgr_update (&sgrp)) {
		fprintf (stderr, _("%s: error adding new group entry\n"), Prog);
		fail_exit (E_GRP_UPDATE);
	}
#endif				/* SHADOWGRP */
#ifdef WITH_AUDIT
	audit_logger (AUDIT_USER_CHAUTHTOK, Prog, "adding group", group_name,
		      group_id, 1);
#endif
	SYSLOG ((LOG_INFO, "new group: name=%s, GID=%u",
		 group_name, (unsigned int) group_id));
}

/*
 * find_new_gid - find the next available GID
 *
 *	find_new_gid() locates the next highest unused GID in the group
 *	file, or checks the given group ID against the existing ones for
 *	uniqueness.
 */
static void find_new_gid (void)
{
	const struct group *grp;
	gid_t gid_min, gid_max;

#if 1
	if (!rflg) {
		gid_min = 500;
                gid_max = 60000;
        } else {
                gid_min = 1;
                gid_max = 499;
        }
#else
	if (!rflg) {
		gid_min = getdef_unum ("GID_MIN", 500);
                gid_max = getdef_unum ("GID_MAX", 60000);
        } else {
                gid_min = 1;
                gid_max = getdef_unum ("GID_MIN", 500) - 1;
        }
#endif
	/*
	 * Start with some GID value if the user didn't provide us with
	 * one already.
	 */

	if (!gflg)
		group_id = gid_min;

#if 0   /* defeat the file search!! */
        return;
#endif

	/*
	 * Search the entire group file, either looking for this GID (if the
	 * user specified one with -g) or looking for the largest unused
	 * value.
	 */
#ifdef NO_GETGRENT
	gr_rewind ();
	while ((grp = gr_next ())) {
#else
	setgrent ();
	while ((grp = getgrent ())) {
#endif
        printf("Comparing to %s (%d)\n", grp->gr_name, grp->gr_gid);

		if (strcmp (group_name, grp->gr_name) == 0) {
			if (fflg) {
				fail_exit (E_SUCCESS);  /* This should not be possible!! */
			}
			fprintf (stderr, _("%s: name %s is not unique\n"),
				 Prog, group_name);
			fail_exit (E_NAME_IN_USE);
		}
		if (gflg && group_id == grp->gr_gid) {
			if (fflg) {
				/* turn off -g and search again */
				gflg = 0;
#ifdef NO_GETGRENT
				gr_rewind ();
#else
				setgrent ();
#endif
				continue;
			}
			fprintf (stderr, _("%s: GID %u is not unique\n"),
				 Prog, (unsigned int) group_id);
			fail_exit (E_GID_IN_USE);
		}
		if (!gflg && grp->gr_gid >= group_id) {
			if (grp->gr_gid > gid_max)
				continue;
			group_id = grp->gr_gid + 1;
		}
	}
	if (!gflg && group_id == gid_max + 1) {
		for (group_id = gid_min; group_id < gid_max; group_id++) {
#ifdef NO_GETGRENT
			gr_rewind ();
			while ((grp = gr_next ())
			       && grp->gr_gid != group_id);
			if (!grp)
				break;
#else
			if (!getgrgid (group_id))
				break;
#endif
		}
		if (group_id == gid_max) {
			fprintf (stderr, _("%s: can't get unique GID\n"), Prog);
			fail_exit (E_GID_IN_USE);
		}
	}
}

/*
 * check_new_name - check the new name for validity
 *
 *	check_new_name() insures that the new name doesn't contain any
 *	illegal characters.
 */
static void check_new_name (void)
{
#if 0   /* skip this check, for now */
	if (check_group_name (group_name))
#endif
		return;

	/*
	 * All invalid group names land here.
	 */

	fprintf (stderr, _("%s: %s is not a valid group name\n"),
		 Prog, group_name);

	fail_exit (E_BAD_ARG);
}

/*
 * close_files - close all of the files that were opened
 *
 *	close_files() closes all of the files that were opened for this new
 *	group. This causes any modified entries to be written out.
 */
static void close_files (void)
{
	if (!gr_close ()) {
		fprintf (stderr, _("%s: cannot rewrite group file\n"), Prog);
		fail_exit (E_GRP_UPDATE);
	}
	gr_unlock ();
#ifdef	SHADOWGRP
	if (is_shadow_grp && !sgr_close ()) {
		fprintf (stderr,
			 _("%s: cannot rewrite shadow group file\n"), Prog);
		fail_exit (E_GRP_UPDATE);
	}
	if (is_shadow_grp)
		sgr_unlock ();
#endif				/* SHADOWGRP */
}

/*
 * open_files - lock and open the group files
 *
 *	open_files() opens the two group files.
 */
static void open_files (void)
{
	if (!gr_lock ()) {
		fprintf (stderr, _("%s: unable to lock group file\n"), Prog);
#ifdef WITH_AUDIT
		audit_logger (AUDIT_USER_CHAUTHTOK, Prog, "locking group file",
			      group_name, -1, 0);
#endif
		fail_exit (E_GRP_UPDATE);
	}
	if (!gr_open (O_RDWR)) {
		fprintf (stderr, _("%s: unable to open group file\n"), Prog);
#ifdef WITH_AUDIT
		audit_logger (AUDIT_USER_CHAUTHTOK, Prog, "opening group file",
			      group_name, -1, 0);
#endif
		fail_exit (E_GRP_UPDATE);
	}
#ifdef	SHADOWGRP
	if (is_shadow_grp && !sgr_lock ()) {
		fprintf (stderr,
			 _("%s: unable to lock shadow group file\n"), Prog);
		fail_exit (E_GRP_UPDATE);
	}
	if (is_shadow_grp && !sgr_open (O_RDWR)) {
		fprintf (stderr,
			 _("%s: unable to open shadow group file\n"), Prog);
		fail_exit (E_GRP_UPDATE);
	}
#endif				/* SHADOWGRP */
}

/*
 * fail_exit - exit with an error code after unlocking files
 */
static void fail_exit (int code)
{
	(void) gr_unlock ();
#ifdef	SHADOWGRP
	if (is_shadow_grp)
		sgr_unlock ();
#endif
	if (code != E_SUCCESS)
#ifdef WITH_AUDIT
		audit_logger (AUDIT_USER_CHAUTHTOK, Prog, "adding group",
			      group_name, -1, 0);
#endif
	longjmp(lclack,code);
}

/*
 * get_id - validate and get group ID
 */
static gid_t get_gid (const char *gidstr)
{
	long val;
	char *errptr;

	val = strtol (gidstr, &errptr, 10);
	if (*errptr || errno == ERANGE || val < 0) {
		fprintf (stderr, _("%s: invalid numeric argument '%s'\n"), Prog,
			 gidstr);
		fail_exit (E_BAD_ARG);
	}
	return val;
}

/*
 * main - groupadd command
 */

int groupadd_main (int argc, char **argv);
int groupadd_main (int argc, char **argv)
{
	struct stat s;
	int retval;
#ifdef USE_PAM
	pam_handle_t *pamh = NULL;
	struct passwd *pampw;
#endif

/* init various things in case we are called more than once */

 empty_list = NULL;

   oflg = 0;
   gflg = 0;
   fflg = 0;
   rflg = 0;

   optind = 0;


#ifdef WITH_AUDIT
	audit_help_open ();
#endif
	/*
	 * Get my name so that I can use it to report errors.
	 */
#if 0
	Prog = Basename (argv[0]);

	setlocale (LC_ALL, "");
	bindtextdomain (PACKAGE, LOCALEDIR);
	textdomain (PACKAGE);

	OPENLOG ("groupadd");
#endif

       /* field all lower level errors and exit */

       if (setjmp(lclack) != 0)  return -1;

	{
		/*
		 * Parse the command line options.
		 */
		char *cp;
		int option_index = 0;
		int c;
		static struct option long_options[] = {
			{"force", no_argument, NULL, 'f'},
			{"gid", required_argument, NULL, 'g'},
			{"help", no_argument, NULL, 'h'},
			{"key", required_argument, NULL, 'K'},
			{"non-unique", required_argument, NULL, 'o'},
			{NULL, 0, NULL, '\0'}
		};

		while ((c =
			getopt_long (argc, argv, "frg:hK:o", long_options,
				     &option_index)) != -1) {
			switch (c) {
			case 'f':
				/*
				 * "force" - do nothing, just exit(0), if the
				 * specified group already exists. With -g, if
				 * specified gid already exists, choose another
				 * (unique) gid (turn off -g). Based on the RedHat's
				 * patch from shadow-utils-970616-9.
				 */
				fflg++;
				break;
	                case 'r':
	                        /*
	                         * create a system group
	                         */
	                        rflg++;
	                        break;
			case 'g':
				gflg++;
				group_id = get_gid (optarg);
				break;
			case 'h':
				usage ();
				break;
			case 'K':
				/*
				 * override login.defs defaults (-K name=value)
				 * example: -K GID_MIN=100 -K GID_MAX=499
				 * note: -K GID_MIN=10,GID_MAX=499 doesn't work yet
				 */
				cp = strchr (optarg, '=');
				if (!cp) {
					fprintf (stderr,
						 _
						 ("%s: -K requires KEY=VALUE\n"),
						 Prog);
					return (E_BAD_ARG);
				}
				/* terminate name, point to value */
				*cp++ = '\0';
#if 0  /* no login.defs changin' */
				if (putdef_str (optarg, cp) < 0)
					return (E_BAD_ARG);
#endif
				break;
			case 'o':
				oflg++;
				break;
			default:
				usage ();
			}
		}
	}

	if (oflg && !gflg)
		usage ();

	if (optind != argc - 1)
		usage ();

	group_name = argv[argc - 1];
	check_new_name ();

#ifdef USE_PAM
	retval = PAM_SUCCESS;

	pampw = getpwuid (getuid ());
	if (pampw == NULL) {
		retval = PAM_USER_UNKNOWN;
	}

	if (retval == PAM_SUCCESS) {
		retval = pam_start ("groupadd", pampw->pw_name, &conv, &pamh);
	}

	if (retval == PAM_SUCCESS) {
		retval = pam_authenticate (pamh, 0);
		if (retval != PAM_SUCCESS) {
			pam_end (pamh, retval);
		}
	}

	if (retval == PAM_SUCCESS) {
		retval = pam_acct_mgmt (pamh, 0);
		if (retval != PAM_SUCCESS) {
			pam_end (pamh, retval);
		}
	}

	if (retval != PAM_SUCCESS) {
		fprintf (stderr, _("%s: PAM authentication failed\n"), Prog);
		fail_exit (1);
	}
#endif				/* USE_PAM */

	if (stat("/etc/gtmp", &s) == 0) {
        	fputs("vigr lockfile (/etc/gtmp) is present.\n", stderr);
	        fail_exit(1);
	}

#ifdef SHADOWGRP
	is_shadow_grp = sgr_file_present ();
#endif

	/*
	 * Start with a quick check to see if the group exists.
	 */
	if (getgrnam (group_name)) {
		if (fflg) {
			return (E_SUCCESS);
		}
#if 0 /* wrs - suppress this warning */
		fprintf (stderr, _("%s: group %s exists\n"), Prog, group_name);
#endif
#ifdef WITH_AUDIT
		audit_logger (AUDIT_USER_CHAUTHTOK, Prog, "adding group",
			      group_name, -1, 0);
#endif
		fail_exit (E_NAME_IN_USE);
	}

	/*
	 * Do the hard stuff - open the files, create the group entries,
	 * then close and update the files.
	 */
	open_files ();

        if ((retval=setjmp(lclack)) != 0)   /* move longjump() target */
         {
           int r;

           if ((r=setjmp(lclack)) != 0)  return r;  /* failed doing next line */
           close_files();
           return retval;
         }

	if (!gflg || !oflg)
          {
	    find_new_gid ();
          }

	grp_update ();

#if 0   /* no cache flushin' */
	nscd_flush_cache ("group");
        printf("Group cache flushed\n");
#endif

	close_files ();

#ifdef USE_PAM
	if (retval == PAM_SUCCESS)
		pam_end (pamh, PAM_SUCCESS);
#endif				/* USE_PAM */
	return (E_SUCCESS);
	/* NOT REACHED */
}

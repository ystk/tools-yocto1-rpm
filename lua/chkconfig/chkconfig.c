/*
 * This file was originally distributed as part of
 * chkconfig-1.3.34-1.src.rpm and has been modified
 * in WRLinux for inclusion in rpm.
 */

/* Copyright 1997-2006 Red Hat, Inc.
 *
 * This software may be freely redistributed under the terms of the GNU
 * public license.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <glob.h>
#include <libintl.h>
#include <locale.h>
#include <popt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static const char *progname = "rpm:lua:chkconfig";

#define _(String) gettext((String))

#include "leveldb.h"

static void usage(void) {
#if 0
    fprintf(stderr, _("%s version %s - Copyright (C) 1997-2000 Red Hat, Inc.\n"), progname, VERSION);
#endif
    fprintf(stderr, _("This may be freely redistributed under the terms of "
			"the GNU Public License.\n"));
    fprintf(stderr, "\n");
    fprintf(stderr, _("usage:   %s --list [name]\n"), progname);
    fprintf(stderr, _("         %s --add <name>\n"), progname);
    fprintf(stderr, _("         %s --del <name>\n"), progname);
    fprintf(stderr, _("         %s --override <name>\n"), progname);
    fprintf(stderr, _("         %s [--level <levels>] <name> %s\n"), progname, "<on|off|reset|resetpriorities>");

}

static void readServiceError(int rc, char * name) {
    if (rc == 2) {
	fprintf(stderr, _("service %s supports chkconfig, but is not referenced in any runlevel (run 'chkconfig --add %s')\n"), name, name);
    } else if (rc == 1) {
	fprintf(stderr, _("service %s does not support chkconfig\n"), name);
    } else {
	fprintf(stderr, _("error reading information on service %s: %s\n"),
		name, strerror(errno));
    }

    exit(1);
}

static int delServiceOne(char *name, int level) {
    int i, rc;
    glob_t globres;
    struct service s;

    if ((rc = readServiceInfo(name, &s, 0))) {
	readServiceError(rc, name);
	return 1;
    }
    if (s.type == TYPE_XINETD) return 0;


    if (!findServiceEntries(name, level, &globres)) {
	    for (i = 0; i < (int)globres.gl_pathc; i++)
		    unlink(globres.gl_pathv[i]);
	    if (globres.gl_pathc) globfree(&globres);
    }
    return 0;
}

static int delService(char * name) {
    int level, i, rc;
    glob_t globres;
    struct service s;

    if ((rc = readServiceInfo(name, &s, 0))) {
	readServiceError(rc, name);
	return 1;
    }
    if (s.type == TYPE_XINETD) return 0;

    for (level = 0; level < 7; level++) {
	    if (!findServiceEntries(name, level, &globres)) {
		    for (i = 0; i < (int)globres.gl_pathc; i++)
		      unlink(globres.gl_pathv[i]);
		    if (globres.gl_pathc) globfree(&globres);
	    }
    }
    return 0;
}


static inline int laterThan(int i, int j) {
	if (i <= j) {
		i = j+1;
		if (i > 99)
			i = 99;
	}
	return i;
}

static inline int earlierThan(int i, int j) {
	if (i >= j) {
		i = j -1;
		if (i < 0)
			i = 0;
	}
	return i;
}

static int frobOneDependencies(struct service *s, struct service *servs, int numservs, int target) {
	int i, j, k;
	int s0 = s->sPriority;
	int k0 = s->kPriority;

	if (s->sPriority < 0) s->sPriority = 50;
	if (s->kPriority < 0) s->kPriority = 50;
	for (i = 0; i < numservs ; i++) {
		if (s->startDeps) {
			for (j = 0; s->startDeps[j] ; j++) {
				if (!strcmp(s->startDeps[j], servs[i].name))
					s->sPriority = laterThan(s->sPriority, servs[i].sPriority);
				if (servs[i].provides) {
					for (k = 0; servs[i].provides[k]; k++) {
						if (!strcmp(s->startDeps[j], servs[i].provides[k]))
							s->sPriority = laterThan(s->sPriority, servs[i].sPriority);
					}
				}
			}
		}
		if (s->stopDeps) {
			for (j = 0; s->stopDeps[j] ; j++) {
				if (!strcmp(s->stopDeps[j], servs[i].name))
					s->kPriority = earlierThan(s->kPriority, servs[i].kPriority);
				if (servs[i].provides) {
					for (k = 0; servs[i].provides[k]; k++) {
						if (!strcmp(s->stopDeps[j], servs[i].provides[k]))
							s->kPriority = earlierThan(s->kPriority, servs[i].kPriority);
					}
				}
			}
		}
	}

	if (target || ((s0 != s->sPriority) || (k0 != s->kPriority))) {
		for (i = 0; i < 7; i++) {
			if (isConfigured(s->name, i, NULL, NULL)) {
				int on = isOn(s->name, i);
				delServiceOne(s->name,i);
				doSetService(*s, i, on);
			} else if (target) {
				delServiceOne(s->name,i);
				doSetService(*s, i, ((1<<i) & s->levels));
			}
		}
		return 1; /* Resolved something */
	}
	return 0; /* Didn't resolve anything */
}


/* LSB-style dependency frobber. Calculates a usable start priority
 * and stop priority.
 * This algorithm will almost certainly break horribly at some point. */
static void frobDependencies(struct service *s) {
	DIR * dir;
	struct dirent * ent;
	struct stat sb;
	struct service *servs = NULL;
	int numservs = 0;
	char fn[1024];
	int nResolved = 0;

	if (!(dir = opendir(RUNLEVELS "/init.d"))) {
		fprintf(stderr, _("failed to open %s/init.d: %s\n"), RUNLEVELS,
			strerror(errno));
		return;
	}

	while ((ent = readdir(dir))) {
		const char *dn;

		/* Skip any file starting with a . */
		if (ent->d_name[0] == '.')	continue;

		/* Skip files with known bad extensions */
		if ((dn = strrchr(ent->d_name, '.')) != NULL &&
		    (!strcmp(dn, ".rpmsave") || !strcmp(dn, ".rpmnew") || !strcmp(dn, ".rpmorig") || !strcmp(dn, ".swp")))
			continue;

		dn = ent->d_name + strlen(ent->d_name) - 1;
		if (*dn == '~' || *dn == ',')
			continue;

		sprintf(fn, RUNLEVELS "/init.d/%s", ent->d_name);
		if (stat(fn, &sb)) {
			continue;
		}
		if (!S_ISREG(sb.st_mode)) continue;
		if (!strcmp(ent->d_name, s->name)) continue;
		servs = (struct service *) realloc(servs, (numservs+1) * sizeof(struct service));
		if (!readServiceInfo(ent->d_name, servs + numservs, 0))
			numservs++;
	}

	/* Resolve recursively the other dependancies */
	do {
	  	nResolved = 0;
		int i;

		for (i = 0; i < numservs ; i++) {
			if ((servs+i)->isLSB)
				nResolved += frobOneDependencies(servs+i, servs, numservs, 0);
		}
	} while (nResolved);

	/* Resolve our target */
	frobOneDependencies(s, servs, numservs, 1);
}

static int addService(char * name) {
    int i, rc;
    struct service s;

    if ((rc = readServiceInfo(name, &s, 0))) {
	readServiceError(rc, name);
	return 1;
    }

    if (s.type == TYPE_XINETD) return 0;
    if (s.isLSB)
		frobDependencies(&s);
    else
    for (i = 0; i < 7; i++) {
	if (!isConfigured(name, i, NULL, NULL)) {
	    if ((1 << i) & s.levels)
		doSetService(s, i, 1);
	    else
		doSetService(s, i, 0);
	}
    }

    return 0;
}

static int overrideService(char * name) {
    /* Apply overrides if available; no available overrides is no error */
    int level, i, rc;
    glob_t globres;
    struct service s;
    struct service o;
    int priority;
    char type;
    int doChange = 1;
    int configured = 0;
    int thisLevelAdded, thisLevelOn;

    if ((rc = readServiceDifferences(name, &s, &o, 0))) {
	return 0;
    }

    if (s.type == TYPE_XINETD) return 0;

    if ((s.levels == o.levels) &&
        (s.kPriority == o.kPriority) &&
        (s.sPriority == o.sPriority)) {
        /* no relevant changes in the override file */
	return 0;
    }

    if (s.isLSB && (s.sPriority <= -1) && (s.kPriority <= -1))
		frobDependencies(&s);
    if ((s.isLSB || o.isLSB) && (o.sPriority <= -1) && (o.kPriority <= -1))
		frobDependencies(&o);

    /* Apply overrides only if the service has not been changed since
     * being added, and not if the service has never been configured
     * at all.
     */

    for (level = 0; level < 7; level++) {
	thisLevelAdded = isConfigured(name, level, &priority, &type);
        thisLevelOn = s.levels & 1<<level;
        if (thisLevelAdded) {
            configured = 1;
            if (type == 'S') {
                if (priority != s.sPriority || !thisLevelOn) {
                    doChange = 0;
                    break;
                }
            } else if (type == 'K') {
                if (priority != s.kPriority || thisLevelOn) {
                    doChange = 0;
                    break;
                }
            }
	}
    }

    if (configured && doChange) {
        for (level = 0; level < 7; level++) {
            if (!findServiceEntries(name, level, &globres)) {
                for (i = 0; i < (int)globres.gl_pathc; i++)
                    unlink(globres.gl_pathv[i]);
                if (globres.gl_pathc)
                    globfree(&globres);
                if ((1 << level) & o.levels)
                    doSetService(o, level, 1);
                else
                    doSetService(o, level, 0);
            }
        }
    }

    return 0;
}


static int showServiceInfo(char * name, int forgiving) {
    int rc;
    int i;
    struct service s;

    rc = readServiceInfo(name, &s, 0);

    if (!rc && s.type == TYPE_INIT_D) {
	    rc = 2;
	    for (i = 0 ; i < 7 ; i++) {
		    if (isConfigured(name, i, NULL, NULL)) {
			    rc = 0;
			    break;
		    }
	    }
    }

    if (rc) {
	if (!forgiving)
	    readServiceError(rc, name);
	return forgiving ? 0 : 1;
    }

    printf("%-15s", s.name);
    if (s.type == TYPE_XINETD) {
	    printf("\t%s\n", s.levels ? _("on") : _("off"));
	    return 0;
    }

    for (i = 0; i < 7; i++) {
	printf("\t%d:%s", i, isOn(s.name, i) ? _("on") : _("off"));
    }
    printf("\n");

    return 0;
}

static int isXinetdEnabled(void) {
	int i;
	struct service s;

	if (readServiceInfo("xinetd", &s, 0)) {
		return 0;
	}
	for (i = 0; i < 7; i++) {
		if (isOn("xinetd", i))
		  return 1;
	}
	return 0;
}

static int serviceNameCmp(const void * a, const void * b) {
  return strcmp(* (char **)a, * (char **)b);
}

static int xinetdNameCmp(const void * a, const void * b) {
    const struct service * first = (const struct service *) a;
    const struct service * second = (const struct service *) b;

    return strcmp(first->name, second->name);
}



static int listService(char * item) {
    DIR * dir;
    struct dirent * ent;
    struct stat sb;
    char fn[1024];
    char **services;
    int i;
    int numServices = 0;
    int numServicesAlloced;
    int err = 0;

    if (item) return showServiceInfo(item, 0);

    numServicesAlloced = 10;
    services = (char **) malloc(sizeof(*services) * numServicesAlloced);

    if (!(dir = opendir(RUNLEVELS "/init.d"))) {
	fprintf(stderr, _("failed to open %s/init.d: %s\n"), RUNLEVELS,
		strerror(errno));
        return 1;
    }

    while ((ent = readdir(dir))) {
	const char *dn;

	/* Skip any file starting with a . */
	if (ent->d_name[0] == '.')	continue;

	/* Skip files with known bad extensions */
	if ((dn = strrchr(ent->d_name, '.')) != NULL &&
    (!strcmp(dn, ".rpmsave") || !strcmp(dn, ".rpmnew") || !strcmp(dn, ".rpmorig") || !strcmp(dn, ".swp")))
	    continue;

	dn = ent->d_name + strlen(ent->d_name) - 1;
	if (*dn == '~' || *dn == ',')
	    continue;

	sprintf(fn, RUNLEVELS "/init.d/%s", ent->d_name);
	if (stat(fn, &sb)) {
	    fprintf(stderr, _("error reading info for service %s: %s\n"),
		ent->d_name, strerror(errno));
	    continue;
	}
	if (!S_ISREG(sb.st_mode)) continue;

	if (numServices == numServicesAlloced) {
	    numServicesAlloced += 10;
	    services = (char **) realloc(services, numServicesAlloced * sizeof(*services));
        }

	services[numServices] = (char *) alloca(strlen(ent->d_name) + 1);
	strncpy(services[numServices++], ent->d_name, strlen(ent->d_name) + 1);
    }

    qsort(services, numServices, sizeof(*services), serviceNameCmp);

    for (i = 0; i < numServices ; i++) {
	    if (showServiceInfo(services[i], 1)) {
		    free(services);
		    closedir(dir);
		    return 1;
	    }
    }

    free(services);

    closedir(dir);

    if (isXinetdEnabled()) {
	    struct service *s, *t;

	    printf("\n");
	    printf(_("xinetd based services:\n"));
	    if (!(dir = opendir(XINETDDIR))) {
		    fprintf(stderr, _("failed to open directory %s: %s\n"),
			    XINETDDIR, strerror(err));
		    return 1;
	    }
	    numServices = 0;
	    numServicesAlloced = 10;
	    s = (struct service *) malloc(sizeof (*s) * numServicesAlloced);

	    while ((ent = readdir(dir))) {
		    const char *dn;

		    /* Skip any file starting with a . */
		    if (ent->d_name[0] == '.')	continue;

		    /* Skip files with known bad extensions */
		    if ((dn = strrchr(ent->d_name, '.')) != NULL &&
			(!strcmp(dn, ".rpmsave") || !strcmp(dn, ".rpmnew") || !strcmp(dn, ".rpmorig") || !strcmp(dn, ".swp")))
		      continue;

		    dn = ent->d_name + strlen(ent->d_name) - 1;
		    if (*dn == '~' || *dn == ',')
		      continue;

		    if (numServices == numServicesAlloced) {
			    numServicesAlloced += 10;
			    s = (struct service *) realloc(s, numServicesAlloced * sizeof (*s));
		    }
		    if (readXinetdServiceInfo(ent->d_name, s + numServices, 0) != -1)
			    numServices ++;
	    }

	    qsort(s, numServices, sizeof(*s), xinetdNameCmp);
	    t = s;
	    for (i = 0; i < numServices; i++, s++) {
		    char *tmp = (char *) malloc(strlen(s->name) + 5);
		    sprintf(tmp,"%s:",s->name);
		    printf("\t%-15s\t%s\n", tmp,  s->levels ? _("on") : _("off"));
	    }
	    closedir(dir);
	    free(t);
    }
    return 0;
}

static int setService(char * name, int where, int state) {
    int i, rc;
    int what;
    struct service s;

    if (!where && state != -1) {
	/* levels 2, 3, 4, 5 */
	where = (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5);
    } else if (!where) {
	where = (1 << 0) | (1 << 1) | (1 << 2) |
	        (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);
    }

    if ((rc = readServiceInfo(name, &s, 0))) {
	readServiceError(rc, name);
	return 1;
    }

    if (s.type == TYPE_INIT_D) {
	    int rc = 0;

	    if (s.isLSB)
		    frobDependencies(&s);
	    for (i = 0; i < 7; i++) {

		    if (!((1 << i) & where)) continue;

		    if (state == 1 || state == 0)
		      what = state;
		    else if (state == -2)
		      what = isOn(name, i);
		    else if (s.levels & (1 << i))
		      what = 1;
		    else
		      what = 0;
		    rc |= doSetService(s, i, what);
	    }
	    return rc;
    } else if (s.type == TYPE_XINETD) {
	    int xx;
	    if (setXinetdService(s, state)) {
		    return 1;
	    }
	    xx =system("/etc/init.d/xinetd reload >/dev/null 2>&1");
    }

    return 0;
}

int chkconfig_main(int argc, const char ** argv);
int chkconfig_main(int argc, const char ** argv)
{
    int listItem = 0, addItem = 0, delItem = 0, overrideItem = 0;
    int rc, i, x;
    int LSB = 0;
    char * levels = NULL;
    int help=0, version=0;
    struct service s;
    poptContext optCon;
    struct poptOption optionsTable[] = {
	    { "add", '\0', 0, &addItem, 0 },
	    { "del", '\0', 0, &delItem, 0 },
	    { "override", '\0', 0, &overrideItem, 0 },
	    { "list", '\0', 0, &listItem, 0 },
	    { "level", '\0', POPT_ARG_STRING, &levels, 0 },
	    { "levels", '\0', POPT_ARG_STRING, &levels, 0 },
	    { "help", 'h', POPT_ARG_NONE, &help, 0 },
	    { "version", 'v', POPT_ARG_NONE, &version, 0 },
	    { 0, 0, 0, 0, 0 }
    };

#if 0
    if ((progname = strrchr(argv[0], '/')) != NULL)
	progname++;
    else
	progname = argv[0];
    if (!strcmp(progname,"install_initd")) {
	    addItem++;
	    LSB++;
    }
    if (!strcmp(progname,"remove_initd")) {
	    delItem++;
	    LSB++;
    }

    setlocale(LC_ALL, "");
    bindtextdomain("chkconfig","/usr/share/locale");
    textdomain("chkconfig");
#endif

    optCon = poptGetContext("chkconfig", argc, argv, optionsTable, 0);
    poptReadDefaultConfig(optCon, 1);

    if ((rc = poptGetNextOpt(optCon)) < -1) {
	fprintf(stderr, "%s: %s\n",
		poptBadOption(optCon, POPT_BADOPTION_NOALIAS),
		poptStrerror(rc));
	return 1;
    }

    if (version) {
	fprintf(stdout, _("%s version %s\n"), progname, "no version!");
	return 0;
    }

    if (help || argc == 1)  { usage(); return 1; }

    if ((listItem + addItem + delItem + overrideItem) > 1) {
	fprintf(stderr, _("only one of --list, --add, --del, or --override"
                " may be specified\n"));
	return 1;
    }

    if (addItem) {
	char * name = (char *)poptGetArg(optCon);

	if (!name || !*name || poptGetArg(optCon))
	    { usage(); return 1; }

	if (LSB)
		    name = basename(name);
	return addService(name);
    } else if (delItem) {
	char * name = (char *)poptGetArg(optCon);

	if (!name || !*name || poptGetArg(optCon))  { usage(); return 1; }

	if (LSB)
		    name = basename(name);
	return delService(name);
    } else if (overrideItem) {
	char * name = (char *)poptGetArg(optCon);

	if (!name || !*name || poptGetArg(optCon))  { usage(); return 1; }

        name = basename(name);
	return overrideService(name);
    } else if (listItem) {
	char * item = (char *)poptGetArg(optCon);

	if (item && poptGetArg(optCon))  { usage(); return 1; }

	return listService(item);
    } else {
	char * name = (char *)poptGetArg(optCon);
	char * state = (char *)poptGetArg(optCon);
	int where = 0, level = -1;

	if (!name) {
		usage();
                return 1;
	}
	if (levels) {
	    where = parseLevels(levels, 0);
	    if (where == -1)  { usage(); return 1; }
	}

	if (!state) {
	    if (where) {
		rc = x = 0;
		i = where;
		while (i) {
		    if (i & 1) {
			rc++;
			level = x;
		    }
		    i >>= 1;
		    x++;
		}

		if (rc > 1) {
		    fprintf(stderr, _("only one runlevel may be specified for "
			    "a chkconfig query\n"));
		    return 1;
		}
	    }
	    rc = readServiceInfo(name, &s, 0);
	    if (rc)
	       return 1;
	    if (s.type == TYPE_XINETD) {
	       if (isOn("xinetd",level))
		       return !s.levels;
	       else
		       return 1;
	    } else
	       return isOn(name, level) ? 0 : 1;
	} else if (!strcmp(state, "on"))
	    return setService(name, where, 1);
	else if (!strcmp(state, "off"))
	    return setService(name, where, 0);
	else if (!strcmp(state, "reset"))
	    return setService(name, where, -1);
	else if (!strcmp(state, "resetpriorities"))
	    return setService(name, where, -2);
	else
	    { usage(); return 1; }
    }

    usage();

    return 1;
}

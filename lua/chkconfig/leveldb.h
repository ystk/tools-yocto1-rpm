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
#ifndef H_LEVELDB
#define H_LEVELDB

#define RUNLEVELS "/etc/rc.d"
#define XINETDDIR "/etc/xinetd.d"

#include <glob.h>

#define TYPE_INIT_D	0
#define TYPE_XINETD	1

struct service {
    char * name;
    int levels, kPriority, sPriority;
    char * desc;
    char **startDeps;
    char **stopDeps;
    char **provides;
    int type;
    int isLSB;
    int enabled;
};

int parseLevels(char * str, int emptyOk);

/* returns 0 on success, 1 if the service is not chkconfig-able, -1 if an
   I/O error occurs (in which case errno can be checked) */
int readServiceInfo(char * name, struct service * service, int honorHide);
int readServiceDifferences(char * name, struct service * service, struct service * service_overrides, int honorHide);
int parseServiceInfo(int fd, char * name, struct service * service, int honorHide, int partialOk);
int currentRunlevel(void);
int isOn(char * name, int where);
int isConfigured(char * name, int level, int *priority, char *type);
int doSetService(struct service s, int level, int on);
int findServiceEntries(char * name, int level, glob_t * globresptr);
int readXinetdServiceInfo(char *name, struct service *service, int honorHide);
int setXinetdService(struct service s, int on);

#endif

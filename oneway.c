#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <sys/types.h>
#include <sys/prctl.h>
#ifndef PR_SET_NO_NEW_PRIVS
# define PR_SET_NO_NEW_PRIVS 38
#endif

// we might use getpwnam_r 
uid_t name_to_uid(char const *name, gid_t *gid, char** home, char **shell)
{
  if (!name) return -1;
  struct passwd *ptr=getpwnam(name);
  if (0 == ptr) {
      fprintf(stderr, "could not find user [%s]\n", name);
      return -1;
  }
  *gid=ptr->pw_gid;
  *home=strdup(ptr->pw_dir);
  *shell=(ptr->pw_shell);
  return ptr->pw_uid;
}

/*
uid_t name_to_gid(char const *name)
{
  if (!name) return -1;
  struct group *ptr=getgrnam(name);
  if (0 == ptr) return -1;
  return ptr->gr_gid;
}
*/

void usage() {
    printf(
        "oneway [-n|-N] USER COMMAND ARGUMENT...\n"
        "-n no new privileges by setting PR_SET_NO_NEW_PRIVS with prctl\n"
        "-N allow new privileges ex. setuid flag\n"
    );
    exit(-1);
}

int main(int argc, char * argv[]) {
    int is_one_way=0, running_uid, uid, gid;
    char *user_name=NULL;
    char *home=NULL, *shell=NULL;
    if (argc<4) usage();
    if (strcasecmp(argv[1], "-n")) usage();
    is_one_way = (argv[1][1]=='n');
    user_name = argv[2];
    uid = name_to_uid(user_name, &gid, &home, &shell);
    if (uid<0) {
        return -1;
    }
    setenv("HOME", home, 1);
    setenv("SHELL", shell, 1);
    printf("setting uid=%d (%s) gid=%d\n", uid, argv[2], gid);
    running_uid = getuid();
    if (running_uid == 0) {
        /* process is running as root, drop privileges */
        if (setgid(gid) != 0) {
            perror("Unable to drop group privileges");
            return -1;
        }
        int ngroups=256;
        gid_t *list=malloc(sizeof(gid_t)*ngroups);
        getgrouplist(user_name, gid, list, &ngroups);
        if (setgroups(ngroups, list) !=0 ) {
            perror("Unable to drop supplementary groups");
            return -1;
        }
        if (setuid(uid) != 0) {
            perror("Unable to drop user privileges");
            return -1;
        }

    } else {
        printf("already not privileged\n");
        if (running_uid!=uid) {
            fprintf(stderr, "WARNING: running as %d, did not change to %s[%d]\n", running_uid, user_name, uid);
        }
    }
    if (is_one_way && prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0)) {
        perror("failed calling prctl(NO_NEW_PRIVS)");
        return -1;
    }
    return execvp(argv[3], &argv[3]);
}

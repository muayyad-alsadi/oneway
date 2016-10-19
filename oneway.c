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
uid_t name_to_uid(char const *name, char** home, char **shell)
{
  if (!name) return -1;
  struct passwd *ptr=getpwnam(name);
  if (0 == ptr) return -1;
  *home=strdup(ptr->pw_dir);
  *shell=(ptr->pw_shell);
  return ptr->pw_uid;
}

uid_t name_to_gid(char const *name)
{
  if (!name) return -1;
  struct group *ptr=getgrnam(name);
  if (0 == ptr) return -1;
  return ptr->gr_gid;
}


void usage() {
    printf(
        "oneway [-n|-N] USER GROUP COMMAND ARGUMENT...\n"
        "-n no new privileges by setting PR_SET_NO_NEW_PRIVS with prctl\n"
        "-N allow new privileges ex. setuid flag\n"
    );
    exit(-1);
}

int main(int argc, char * argv[]) {
    int is_one_way=0, uid, gid;
    char *home=NULL, *shell=NULL;
    if (argc<5) usage();
    if (strcasecmp(argv[1], "-n")) usage();
    is_one_way = (argv[1][1]=='n');
    uid = name_to_uid(argv[2], &home, &shell);
    gid = name_to_gid(argv[3]);
    setenv("HOME", home, 1);
    setenv("SHELL", shell, 1);
    printf("setting uid=%d (%s) gid=%d (%s)\n", uid, argv[2], gid, argv[3]);
    if (getuid() == 0) {
        /* process is running as root, drop privileges */
        if (setgid(gid) != 0) {
            perror("Unable to drop group privileges");
            return -1;
        }
        gid_t list[1]={gid};
        if (setgroups(1, list) !=0 ) {
            perror("Unable to drop supplementary groups");
            return -1;
        }
        if (setuid(uid) != 0) {
            perror("Unable to drop user privileges");
            return -1;
        }

    }
    if (is_one_way && prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0)) {
        perror("failed calling prctl(NO_NEW_PRIVS)");
        return -1;
    }
    return execvp(argv[4], &argv[4]);
}

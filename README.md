# Oneway - a tool to drop privileges for docker entry-points

## Introduction

Unlike `su` or `sudo` this tool would `exec` to replace the 
current process (so it would receive signals ..etc.)

This process simply calls

* `setgid` to desired group
* `setgroups` to set supplementary groups to only desired group
* `setuid` to desired user
* `prctl` with `PR_SET_NO_NEW_PRIVS` if -n is passed to disallow future privileges

## Usage

```
oneway [-n|-N] USER GROUP COMMAND ARGUMENT...
```

* `-n` no new privileges by setting `PR_SET_NO_NEW_PRIVS` with `prctl`
* `-N` allow new privileges ex. via `su`, `sudo`, `setuid` flag

we have a `test.sh` which display `id` then it pings google (which is a suid binary)

```
./oneway -n alsadi alsadi ./test.sh
```

typically this would give

```
setting uid=1000 (alsadi) gid=1000 (alsadi)
uid=1000(alsadi) gid=1000(alsadi) groups=1000(alsadi) context=unconfined_u:unconfined_r:unconfined_t:s0-s0:c0.c1023
ping: socket: Operation not permitted
ping: socket: Operation not permitted
```

## Docker usage

In your `Dockerfile` you might use Yelp's `dumb-init`

```
ENTRYPOINT [ "/usr/local/bin/dumb-init", "/start.sh" ]
```

and in your `start.sh`

```
# do things as root
chown ...
chmod ...
# run some daemons as root
nohup /usr/local/bin/confd -watch ... >> /data/logs/out.log
# run some daemons as user app
oneway -n app app /app.sh
```

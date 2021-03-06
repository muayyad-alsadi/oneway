# Oneway - a tool to drop privileges for docker entry-points

## Introduction

If you want to drop privileges (as in Docker entry-points) but you don't want
your process to be child process of `su` or `sudo` process.

Unlike `su` or `sudo` this tool would `exec` to replace the 
current process (so it would receive signals ..etc.)

This tool can also set special process flags so disallow any future privileges

This process simply calls

* `setgid` to desired group
* `setgroups` to set supplementary groups to only desired group
* `setuid` to desired user
* `prctl` with `PR_SET_NO_NEW_PRIVS` if -n is passed to disallow future privileges

## Usage

```
oneway [-n|-N] USER COMMAND ARGUMENTS...
```

* `-n` no new privileges by setting `PR_SET_NO_NEW_PRIVS` with `prctl`
* `-N` allow new privileges ex. via `su`, `sudo`, `setuid` flag

we have a `test.sh` which display `id` then it pings google (which is a suid binary)

```
./oneway -n alsadi ./test.sh
```

typically this would give

```
setting uid=1000 (alsadi) gid=1000
uid=1000(alsadi) gid=1000(alsadi) groups=1000(alsadi),10(wheel),135(mock),973(wireshark) context=unconfined_u:unconfined_r:unconfined_t:s0-s0:c0.c1023
ping: socket: Operation not permitted
ping: socket: Operation not permitted
```

## Installation

To build it you only need `gcc`

```
yum install gcc glibc-static
make
```

You can get binaries directly from our github [releases](https://github.com/muayyad-alsadi/oneway/releases)


## Docker usage

In your `Dockerfile` you might use [Yelp's dumb-init](https://github.com/Yelp/dumb-init)


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
exec oneway -n app /app.sh
```

## FAQ

* Q: Why is the benitift of this?
  * drop priviligies in "oneway" manner.
* Q: What is the advantage over `su` / `sudo`
  * they do not drop priviligies they change user.
  * `oneway` make use of kernel `PR_SET_NO_NEW_PRIVS`
  * with `su` / `sudo` one might be able to escelate (ex. using `setuid` binaries)


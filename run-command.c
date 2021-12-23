#define _XOPEN_SOURCE 600	/* For posix_openpt(), grantpt() etc */

#include <glib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>		/* posix_openpt() */
#include <fcntl.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/ioctl.h>

#include "run-command.h"
#include "lists.h"
#include "utils.h"


static void run_child(char *slavename, char *prog, char **argp);
static void run_parent(int fd, GString *prog, GString *host, pid_t pid);


/**
 * Run a command on a host.
 *
 * @ssh command to run to access the host.
 *
 * @ssh_options extra args for the ssh command.
 *
 * @host where to run the following command.
 *
 * @command what to run on the host.
 */
void run_command(GString *ssh,
		 GPtrArray *ssh_options,
		 GString *host,
		 GPtrArray *command,
		 int opt_single,
		 int opt_debug)
{
	pid_t pid;
	/** args is an array of char*, not an array of GString*. */
	GPtrArray *args = g_ptr_array_new();

	int ptfd;
	char *slavename;

	ga(args, g2c(ssh));
	g_ptr_array_add(args, "-n");
	g_ptr_array_add(args, "-q");
	g_ptr_array_add(args, "-t");
	for (int i=0; i<ssh_options->len; i++) {
		g_ptr_array_add(args, "-o");
		ga(args, a2g2c(ssh_options, i));
	}
	ga(args, g2c(host));
	ga(args, "--");
	for (int i=0; i<command->len; i++) {
		ga(args, a2g2c(command, i));
	}
	ga(args, NULL);

	if (opt_debug) {
		//printf("-- rc: args->len=%d\n", args->len);
		//printf("-- rc: args...\n");
		for (int i=0;
		     i<(args->len)-1; /* -1 so we don't print the NULL. */
		     i++) {
			if (i) printf(" ");
			printf("{%s}", a2c(args, i));
		}
		printf("\n");
	}

	GString *gs = g_string_new("");

	ptfd = posix_openpt(O_RDWR);
	grantpt(ptfd);
	unlockpt(ptfd);
	slavename = ptsname(ptfd);
	if (opt_debug) {
		printf("pty = %s\n", slavename);
	}

	pid = fork();
	switch (pid) {
	case -1:
		// Error.
		g_string_printf(gs, "%s # Cannot fork: %s", host->str,
				strerror(errno));
		fprintf(stderr, "%s\n", gs->str);
		failure(gs);
		break;
	case 0:
		// I am the child.
		run_child(slavename, ssh->str, (char **) args->pdata);
		// run_child() does not return.
	default:
		// I am the parent.
		run_parent(ptfd, ssh, host, pid);
		break;
	}

	g_ptr_array_free(args, TRUE);
}


static void run_child(char *slavename, char *prog, char **argp)
{
	int err;
	int fd;

	fd = open(slavename, O_RDWR);
	err = errno;
	if (-1 == fd) {
		fprintf(stderr, "Cannot open %s: %s\n", slavename,
			strerror(err));
		exit(128);
	}
	// Make in, out, err, point to the pty.
	dup2(fd, 2);
	dup2(fd, 1);
	dup2(fd, 0);
	close(fd);
	if (-1 == setsid()) {
		fprintf(stderr, "setsid: %s\n", strerror(errno));
		exit(4);
	}
	if (-1 == ioctl(0, TIOCSCTTY, 1)) {
		fprintf(stderr, "ioctl: %s\n", strerror(errno));
		exit(4);
	}
	execvp(prog, argp);

	// We shouldn't get here under normal circumstances.
	err = errno;
	fprintf(stderr, "Cannot exec %s: %s\n", prog, strerror(err));
	exit(128);
}


void run_parent(int fd, GString *prog, GString *host, pid_t pid)
{
	int readval;
	int writeval;
	int written;
	static ssize_t bufsize = 256;
	unsigned char buf[bufsize];
	unsigned char lastchar = '\0';
	GString *gs;
	int ret;
	pid_t pid2;
	int wstatus;

	fflush(stdout);
	while (-1 != (readval=read(fd, buf, bufsize))) {
		if (0 == readval) {
			break;
		}
		written = 0;
		while (written < readval) {
			writeval = write(1, buf+written, readval-written);
			if (-1 == writeval) {
				exit(5);
			}
			written += writeval;
		}
		lastchar = buf[readval-1];
	}
	if ('\n' != lastchar) {
		write(1, "\n", 1);
	}

	gs = g_string_new("");
	pid2 = wait(&wstatus);
	assert(pid == pid2);
	ret = WEXITSTATUS(wstatus);
	switch (ret) {
	case 0:
		g_string_printf(gs, "%s", host->str);
		success(gs);
		break;
	case 128:
		g_string_printf(gs, "%-*s# Cannot exec %s",
				host_len(), host->str, prog->str);
		failure(gs);
		break;
	default:
		g_string_printf(gs, "%-*s # (%d)",
				host_len(), host->str, ret);
		failure(gs);
		break;
	}
}


/*
 * Run a command on a list of hosts.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <getopt.h>
#include <string.h> // strerror
#include <assert.h> // strerror
#include <errno.h>
#include <glib.h>

#include "lists.h"

#define DD(n) if(opt_debug >= n)

// GString to char*.
#define g2c(p)      ( ((GString*)p)->str )
// GString array index to GString*.
#define a_g(p, i)   (  (GString*)(g_ptr_array_index(p,i)) )
// GString array index to char*.
#define a2g2c(p, i) ( a_g(p,i)->str )
// char* array index to char*.
#define a2c(p, i) ( (char*)(g_ptr_array_index(p,i)) )

// Add pointer to array.
#define ga(a,p)							\
	do {							\
		if (0 && opt_debug) {				\
			printf("adding to %s: %p\n", #a, p);	\
		}						\
		g_ptr_array_add(a,p);				\
	} while (0)

static char *myversion = "0.0.3";

static char *myname;

static void debug_print_flags(void);
static void init(void);
static void do_opts(int argc, char **argv);
static void usage(int longusage, int ret);
static void do_host(int i);
static void run_command(GString *ssh,
			GPtrArray *ssh_options,
			GString *host,
			GPtrArray *command);
static void print_s_f_lists(void);
static void list_hosts(void);
static void list_files(void);


static int         opt_debug = 0;	   /* -D, --debug */
static int         opt_files = 0;	   /* -F, --files */
static int         opt_list_only = 0;	   /* -L, --list-only */
static int         opt_quiet = 0;	   /* -q, --quiet */
static int         opt_single = 0;	   /* -1, --single */
static int         opt_reverse = 0;	   /* -r, --reverse */
static int         opt_sort = 0;	   /* -s, --sort */
static GPtrArray * opt_ssh_options = 0;	   /* -o, --ssh-option */
static GString *   opt_ssh_program = 0;	   /* -S, --ssh-program */
static int         opt_no_tty = 0;	   /* -T, --no-tty */
static GString *   opt_user = 0;	   /* -u, --user */
static GPtrArray * opt_command = 0;	   /* Remote command */
static GPtrArray * success_hosts;	   /* List of successes */
static GPtrArray * failure_hosts;	   /* List of failures */


int main(int argc, char **argv)
{

	init();

	myname = argv[0];
	do_opts(argc, argv);
	process_lists();

	if (opt_debug)
		debug_print_flags();

	if (opt_sort) {
		sort_hosts();
	}

	if (opt_list_only) {
		list_hosts();
	}
	if (opt_files) {
		list_files();
	}
	if (opt_list_only || opt_files) {
		exit(0);
	}

	if (opt_reverse) {
		for (int i=n_hosts()-1; i>=0; i--) {
			do_host(i);
		}
	} else {
		for (int i=0; i<n_hosts(); i++) {
			do_host(i);
		}
	}

	if (0 == opt_command->len) {
		usage(0, 1);
	}
	return 0;
}

/**
 * Run our command on one host.
 *
 * We assume that the hosts are in one list, and just supply the index into
 * that list. Separating this out makes it easier to run the command in any
 * order.
 *
 * @param i the index of the host
 */
static void do_host(int i)
{
	GString *hostname = get_host(i);
	printf("\n-- %s\n", hostname->str);
	run_command(opt_ssh_program,
		    opt_ssh_options,
		    hostname,
		    opt_command);
	if (! opt_quiet) {
		print_s_f_lists();
	}
}


/**
 * List all the hosts on stdout.
 */
static void list_hosts(void)
{
	printf("Hosts:\n");
	for (int i=0; i<n_hosts(); i++) {
		printf("        %s\n", g2c(get_host(i)));
	}
	printf("Not hosts:\n");
	for (int i=0; i<n_not_hosts(); i++) {
		printf("        %s\n", g2c(get_not_host(i)));
	}
}


/**
 * List all the host list files on stdout.
 */
static void list_files(void)
{
	printf("Lists:\n");
	for (int i=0; i<n_host_lists(); i++) {
		printf("        %s\n", g2c(get_host_list(i)));
	}
	printf("Not lists:\n");
	for (int i=0; i<n_not_host_lists(); i++) {
		printf("        %s\n", g2c(get_not_host_list(i)));
	}
}


/**
 * Calculate the field width needed for host names. The width is the smallest
 * multiple of eight that is long enough for every host name.
 */
static int host_len(void)
{
	static int len = 0;
	if (! len) {
		for (int i=0; i<n_hosts(); i++) {
			GString *gs = get_host(i);
			int tl = gs->len;
			if (tl > len)
				len = tl;
		}
		len += 8;
		len -= (len % 8);
	}
	return len;
}


static void print_list(const char *label, GPtrArray *list)
{
	printf("%s:\n", label);
	for (int i=0; i<list->len; i++) {
		printf("\t%s\n", a2g2c(list, i));
	}
}


/**
 * Print the success and failure lists.
 */
static void print_s_f_lists(void)
{
	printf("\n----\n");
	if (success_hosts->len) {
		print_list("Success", success_hosts);
	}
	if (failure_hosts->len) {
		print_list("Failure", failure_hosts);
	}
}


/**
 * Initialise static data.  When we include other modules, their init functions
 * need to be called here.
 */
static void init(void)
{
	init_lists();
	opt_ssh_program = g_string_new("ssh");

	opt_ssh_options = g_ptr_array_new();

	opt_command = g_ptr_array_new();

	success_hosts = g_ptr_array_new();
	failure_hosts = g_ptr_array_new();
}


static const char * const usage_message = "\
Usage: %s [options] [hosts] -- command\n\
       %s [options] [hosts] -L\n\
       %s [options] [hosts] -F\n";
static const char * const long_usage_message = "\
  * means not implemented\n\
    -h|--help       This help\n\
    -V|--version    Print version and exit\n\
  * -1|--single     Output on a single line, with host name\n\
                    Turns on -q\n\
    -F|--files      Show which list files are read\n\
    -H file|--hostlist=file\n\
                    File with list of hosts, one per line\n\
    -L|--list-only  List hosts from files, do not run command - the\n\
                    command is not required here.\n\
    -n h|--not h    Exclude host h\n\
    -N file|--notlist=file\n\
                    Exclude hosts in this list\n\
    -o sshoption    Add \"-o sshoption\" to the ssh command line\n\
  * -q              Quiet (do not print commands and machine names)\n\
    -S prog|--ssh-program=prog\n\
                    Use prog as ssh command (experimental)\n\
    -s|--sort       Sort the host list\n\
    -r              Do the list in reverse\n\
  * -T|--no-tty     Don't allocate a tty\n\
  * -u user         Run commands as user\n\
    [hosts]         Optionally specify hosts to operate on - if none\n\
                    specified and no list specified, defaults to the\n\
                    \"all\" list\n\
    --              \"--\" must be used to separate options, host names,\n\
                    and the command\n\
\n\
 Hosts can be specified as:\n\
   - non-option arguments before the -- separator, or\n\
   - inside a host list file specified with -H.\n\
 Host lists are in . or /home/russells/etc/for-all or /usr/local/etc/for-all.\n\
 Host lists can contain blank lines or comments starting with #.\n\
 Host list defaults to all.\n\
 \"-- command\" must be supplied\n\
";


static void usage(int longusage, int ret)
{
	FILE *f;
	if (ret)
		f = stderr;
	else
		f = stdout;
	fprintf(f, usage_message, myname, myname, myname, myname, myname);
	if (longusage)
		fprintf(f, long_usage_message, myname, myname);
	exit(ret);
}


static const char* const short_options = "-1DFhH:LqsS:u:n:N:rTo:V";
static const struct option long_options[] = {
	{ "debug"       , optional_argument,                0, 'D' },
	{ "files"       ,       no_argument,       &opt_files, 'F' },
	{ "help"        ,       no_argument,                0, 'h' },
	{ "quiet"       ,       no_argument,       &opt_quiet, 'q' },
	{ "host-list"   , required_argument,                0, 'H' },
	{ "list-only"   ,       no_argument,   &opt_list_only, 'L' },
	{ "not"         , required_argument,                0, 'n' },
	{ "not-list"    , required_argument,                0, 'N' },
	{ "single"      ,       no_argument,      &opt_single, '1' },
	{ "ssh-option"  , required_argument,                0, 'o' },
	{ "ssh-program" , required_argument,                0, 'S' },
	{ "sort"        ,       no_argument,        &opt_sort, 's' },
	{ "no-tty"      ,       no_argument,      &opt_no_tty, 'T' },
	{ "user"        , required_argument,                0, 'u' },
	{ "version"     ,       no_argument,                0, 'V' },
	{ 0             ,                 0,                0,  0  }
};


/**
 * Process the command line arguments.
 *
 * @param argc from main()
 * @param argv from main()
 */
static void do_opts(int argc, char **argv)
{
	while (1) {
		//int this_option_optind = optind ? optind : 1;
		int option_index = 0;
		int c = getopt_long(argc, argv, short_options, long_options,
				    &option_index);
		GString *gs;

		if (c == -1)
			break;
		switch (c) {
		case 1:
			// case 1 is for non-option arguments, up to "--".
			// That is specified by the leading '-' in
			// short_options.
			gs = g_string_new(optarg);
			add_host(gs);
			break;
		case '1':
			opt_single = 1;
			break;
		case 'D':
			opt_debug ++;
			break;
		case 'F':
			opt_files = 1;
			break;
		case 'h':
			usage(1, 0);
			break;
		case 'H':
			gs = g_string_new(optarg);
			add_list(gs);
			break;
		case 'L':
			opt_list_only = 'L';
			break;
		case 'n':
			gs = g_string_new(optarg);
			add_not_host(gs);
			break;
		case 'N':
			gs = g_string_new(optarg);
			add_not_list(gs);
			break;
		case 'o':
			gs = g_string_new(optarg);
			ga(opt_ssh_options, gs);
			break;
		case 'q':
			opt_quiet = 'q';
			break;
		case 'r':
			opt_reverse = 'r';
			break;
		case 's':
			opt_sort = 's';
			break;
		case 'S':
			opt_ssh_program = g_string_new(optarg);
			break;
		case 'T':
			opt_no_tty = 'T';
			break;
		case 'u':
			opt_user = g_string_new(optarg);
			break;
		case 'V':
			printf("%s\n", myversion);
			exit(0);
			break;
		}
	}
	// Now we're into the args after "--".  All those are the command and
	// args for the remote host.
	for (int i=optind; i<argc; i++) {
		ga(opt_command, g_string_new(argv[i]));
	}
}


/**
 * Debug output printing.
 *
 * @todo Rationalise the debug output.
 */
static void debug_print_flags(void)
{
	DD(1) if (opt_single) {
		printf("opt_single\n");
	}
	DD(1) if (opt_files) {
		printf("opt_files\n");
	}
	DD(1) if (n_hosts()) {
		for (int j=0; j<n_hosts(); j++) {
			printf("host: %s\n", g2c(get_host(j)));
		}
	}
	DD(2) if (n_host_lists()) {
		for (int j=0; j<n_host_lists(); j++) {
			printf("host list: %s\n", g2c(get_host_list(j)));
		}
	}
	DD(2) if (opt_list_only) {
		printf("opt_list_only\n");
	}
	DD(2) if (n_not_hosts()) {
		for (int j=0; j<n_not_hosts(); j++) {
			printf("not: %s\n", g2c(get_not_host(j)));
		}
	}
	DD(2) if (n_not_host_lists()) {
		for (int j=0; j<n_not_host_lists(); j++) {
			printf("not list: %s\n", g2c(get_not_host_list(j)));
		}
	}
	DD(1) if (opt_quiet) {
		printf("opt_quiet\n");
	}
	DD(1) if (opt_reverse) {
		printf("opt_reverse\n");
	}
	DD(1) if (opt_single) {
		printf("opt_single\n");
	}
	DD(1) if (opt_sort) {
		printf("opt_sort\n");
	}
	DD(1) if (opt_ssh_options->len) {
		for (int j=0; j<opt_ssh_options->len; j++) {
			printf("ssh option: %s\n",
			       ((GString *)g_ptr_array_index(opt_ssh_options, j))->str);
		}
	}
	DD(1) if (opt_ssh_program) {
		printf("opt_ssh_program: %s\n", opt_ssh_program->str);
	}
	DD(1) if (opt_no_tty) {
		printf("opt_no_tty\n");
	}
	DD(1) if (opt_user) {
		printf("opt_user: %s\n", opt_user->str);
	}
	DD(1) if (opt_command->len) {
		for (int i=0; i<opt_command->len; i++) {
			if (i<0) {
				printf(" ");
			}
			printf("{%s}",
			       ((GString *)g_ptr_array_index(opt_command, i))->str);
		}
		printf("\n");
	}
}


/**
 * A host command run failed.  Record that on a list.
 *
 * @todo Should the failure and success lists be maintained here, or in the
 * lists module?
 */
static void failure(GString *s)
{
	g_ptr_array_add(failure_hosts, s);
}

/**
 * A host command run succeeded.  Record that on a list.
 *
 * @see failure()
 */
static void success(GString *s)
{
	g_ptr_array_add(success_hosts, s);
}


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
static void run_command(GString *ssh,
			GPtrArray *ssh_options,
			GString *host,
			GPtrArray *command)
{
	int ret;
	int wstatus;
	pid_t pid, pid2;
	/** args is an array of char*, not an array of GString*. */
	GPtrArray *args = g_ptr_array_new();

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
		execvp(ssh->str, (char * const *) args->pdata);
		fprintf(stderr, "Cannot exec %s: %s\n", ssh->str,
			strerror(errno));
		exit(128);
	default:
		// I am the parent.
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
					host_len(), host->str, ssh->str);
			failure(gs);
			break;
		default:
			g_string_printf(gs, "%-*s # (%d)",
					host_len(), host->str, ret);
			failure(gs);
			break;
		}
	}

	g_ptr_array_free(args, TRUE);
}

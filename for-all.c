/*
 * Run a command on a list of hosts.
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <glib.h>

static char *myversion = "0.0.2";

static char *myname;

static void debug_print_flags(void);
static void setup(void);
static void do_opts(int argc, char **argv);
static void usage(int longusage, int ret);
static int run_command(GString *command);

#define N_NOT_LISTS 10

static int         opt_debug = 0;	   /* -D, --debug */
static int         opt_files = 0;	   /* -F, --files */
static int         opt_list_only = 0;	   /* -L, --list-only */
static GPtrArray * opt_hosts;		   /* non-option arguments */
static GPtrArray * opt_host_lists;	   /* -H, --host-list */
static GPtrArray * opt_not;		   /* -n, --not */
static GPtrArray * opt_not_lists;	   /* -N, --not-list */
static int         opt_quiet = 0;	   /* -q, --quiet */
static int         opt_single = 0;	   /* -1, --single */
static int         opt_reverse = 0;	   /* -r, --reverse */
static int         opt_sort = 0;	   /* -s, --sort */
static GPtrArray * opt_ssh_options = 0;	   /* -o, --ssh-option */
static GString *   opt_ssh_program = 0;	   /* -S, --ssh-program */
static int         opt_no_tty = 0;	   /* -T, --no-tty */
static GString *   opt_user = 0;	   /* -u, --user */
static GString *   opt_command = 0;	   /* Remote command */

int main(int argc, char **argv)
{

	setup();

	myname = argv[0];
	do_opts(argc, argv);

	if (opt_debug)
		debug_print_flags();

	for (int i=0; i<opt_hosts->len; i++) {
		GString *cmd = g_string_new(opt_ssh_program->str);
		for (int i=0; i<opt_ssh_options->len; i++) {
			g_string_append_printf
				(cmd, " %s",
				 ((GString*) g_ptr_array_index(opt_ssh_options, i))->str);
		}
		g_string_append_printf
			(cmd, " %s -- %s",
			 ((GString*) g_ptr_array_index(opt_hosts, i))->str,
			 opt_command->str);
		printf("%s\n", cmd->str);
		run_command(cmd);
		g_string_free(cmd, TRUE);
	}
	if (0 == opt_command->len) {
		usage(0, 1);
	}
	return 0;
}

static void setup(void)
{
	opt_hosts = g_ptr_array_new();
	opt_host_lists = g_ptr_array_new();
	opt_not = g_ptr_array_new();
	opt_not_lists = g_ptr_array_new();
	opt_ssh_program = g_string_new("ssh");

	GString *gs = g_string_new("-q");
	opt_ssh_options = g_ptr_array_new();
	g_ptr_array_add(opt_ssh_options, gs);

	opt_command = g_string_new(0);
}


static const char * const usage_message = "\
Usage: %s [options] [hosts] -- command\n\
       %s [options] [hosts] -L\n\
       %s [options] [hosts] -F\n";
static const char * const long_usage_message = "\
    -h|--help       This help\n\
    -V|--version    Print version and exit\n\
    -1|--single     Output on a single line, with host name\n\
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
    -q              Quiet (do not print commands and machine names)\n\
    -S prog|--ssh-program=prog\n\
                    Use prog as ssh command (experimental)\n\
    -s|--sort       Sort the host list\n\
    -r              Do the list in reverse\n\
    -T|--no-tty     Don't allocate a tty\n\
    -u user         Run commands as user\n\
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
	{ "debug"       ,       no_argument,       &opt_debug, 'D' },
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


static void do_opts(int argc, char **argv)
{
	if (opt_debug) {
		for (int i=0; i<argc; i++) {
			printf("arg %d: %s\n", i, argv[i]);
		}
	}
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
			gs = g_string_new(optarg);
			g_ptr_array_add(opt_hosts, gs);
			break;
		case '1':
			opt_single = 1;
			break;
		case 'D':
			opt_debug = 'D';
			break;
		case 'F':
			opt_files = 1;
			break;
		case 'h':
			usage(1, 0);
			break;
		case 'H':
			gs = g_string_new(optarg);
			g_ptr_array_add(opt_host_lists, gs);
			break;
		case 'L':
			opt_list_only = 'L';
			break;
		case 'n':
			gs = g_string_new(optarg);
			g_ptr_array_add(opt_not, gs);
			break;
		case 'N':
			gs = g_string_new(optarg);
			g_ptr_array_add(opt_not_lists, gs);
			break;
		case 'o':
			gs = g_string_new(optarg);
			g_ptr_array_add(opt_ssh_options, gs);
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
	for (int i=optind; i<argc; i++) {
		if (opt_debug)
			printf("leftover arg %d: %s\n", i, argv[i]);
		if (i > optind) {
			g_string_append_c(opt_command, ' ');
		}
		g_string_append(opt_command, argv[i]);
	}
}


static void debug_print_flags(void)
{
	if (opt_single) {
		printf("opt_single\n");
	}
	if (opt_files) {
		printf("opt_files\n");
	}
	if (opt_hosts->len) {
		for (int j=0; j<opt_hosts->len; j++) {
			printf("host: %s\n",
			       ((GString *)g_ptr_array_index(opt_hosts, j))->str);
		}
	}
	if (opt_host_lists->len) {
		for (int j=0; j<opt_host_lists->len; j++) {
			printf("host list: %s\n",
			       ((GString *)g_ptr_array_index(opt_host_lists, j))->str);
		}
	}
	if (opt_list_only) {
		printf("opt_list_only\n");
	}
	if (opt_not->len) {
		for (int j=0; j<opt_not->len; j++) {
			printf("not: %s\n",
			       ((GString *)g_ptr_array_index(opt_not, j))->str);
		}
	}
	if (opt_not_lists->len) {
		for (int j=0; j<opt_not_lists->len; j++) {
			printf("not list: %s\n",
			       ((GString *)g_ptr_array_index(opt_not_lists, j))->str);
		}
	}
	if (opt_quiet) {
		printf("opt_quiet\n");
	}
	if (opt_reverse) {
		printf("opt_reverse\n");
	}
	if (opt_single) {
		printf("opt_single\n");
	}
	if (opt_sort) {
		printf("opt_sort\n");
	}
	if (opt_ssh_options->len) {
		for (int j=0; j<opt_ssh_options->len; j++) {
			printf("ssh option: %s\n",
			       ((GString *)g_ptr_array_index(opt_ssh_options, j))->str);
		}
	}
	if (opt_ssh_program) {
		printf("opt_ssh_program: %s\n", opt_ssh_program->str);
	}
	if (opt_no_tty) {
		printf("opt_no_tty\n");
	}
	if (opt_user) {
		printf("opt_user: %s\n", opt_user->str);
	}
	printf("command: %s\n", opt_command->str);
}


/**
 * Run a command with system(3).
 *
 * TODO: This should use fork/exec and wait.  That will require using an argv
 * array, rather than a single string.
 *
 * @command What to do
 *
 * @return exit code of program.
 */
static int run_command(GString *command)
{
	int ret = 0;
	ret = system(command->str);
	return ret;
}

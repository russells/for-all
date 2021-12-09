/*
 * Run a command on a list of hosts.
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>


static char *myname;


static int    opt_files = 0;	   /* 'F', "--files" */
static int    opt_list_only = 0;   /* 'L', "--list-only" */
static int    opt_quiet = 0;	   /* 'q', "--quiet" */
static int    opt_sort = 0;	   /* 'S', "--sort" */
static int    opt_ssh_program = 0; /* 's', "--ssh-program" */
//static char * opt_user = 0;	   /* 'u', "--user" */

static const char* const short_options = "1FhH:Lqs:Su:n:N:rTo:V";
static const struct option long_options[] = {
	{ "files"       ,       no_argument,       &opt_files, 'F' },
	{ "help"        ,       no_argument,                0, 'h' },
	{ "quiet"       ,       no_argument,       &opt_quiet, 'q' },
	{ "host-list"   , required_argument,                0, 'H' },
	{ "list-only"   ,       no_argument,   &opt_list_only, 'L' },
	{ "not"         , required_argument,                0, 'n' },
	{ "not-list"    , required_argument,                0, 'N' },
	{ "single"      ,       no_argument,                0, '1' },
	{ "ssh-program" , required_argument, &opt_ssh_program, 's' },
	{ "sort"        ,       no_argument,        &opt_sort, 'S' },
	{ "user"        , required_argument,                0, 'u' },
	{ "version"     ,       no_argument,                0, 'V' },
	{ 0             ,                 0,                0,  0  }
};


static const char * const usage_message = "\
Usage: %s [options] [hosts] -- command\n\
       %s [options] [hosts] -L\n\
       %s [options] [hosts] -F\n\
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
    -s prog|--ssh-program=prog\n\
                    Use prog as ssh command (experimental)\n\
    -S|--sort       Sort the host list\n\
    -r              Do the list in reverse\n\
    -T              Don't allocate a tty\n\
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


static void do_opts(int argc, char **argv)
{
	getopt_long(argc, argv, short_options, long_options, 0);
}


static void usage(int ret)
{
	FILE *f;
	if (ret)
		f = stderr;
	else
		f = stdout;
	fprintf(f, usage_message, myname, myname, myname, myname, myname);
	exit(ret);
}


int main(int argc, char **argv)
{
	myname = argv[0];
	do_opts(argc, argv);
	usage(1);
}

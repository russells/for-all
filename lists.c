#include "lists.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <regex.h>


GPtrArray *hosts;
GPtrArray *nots;

GPtrArray *host_lists;
GPtrArray *not_host_lists;


/**
 * Called when we remove a string from the hosts list.
 *
 * The hosts list owns the GStrings, so we want to deallocate them.  This is
 * called after we process all the host names and lists, so we can remove hosts
 * from the list of hosts to contact.
 *
 * x the GString that is being removed.
 */
static void hosts_remove(gpointer x);


static GString *line_to_host(char *line);
static int line_host_match(char *line, char **name, int *namelen);
static int in_list(GPtrArray *a, GString *h);



void init_lists(void)
{
	hosts = g_ptr_array_new();
	g_ptr_array_set_free_func(hosts, hosts_remove);
	nots = g_ptr_array_new();
	host_lists = g_ptr_array_new();
	not_host_lists = g_ptr_array_new();
}


int n_hosts(void) { return hosts->len; }
int n_not_hosts(void) { return nots->len; }
int n_host_lists(void) { return host_lists->len; }
int n_not_host_lists(void) { return not_host_lists->len; }


GString *get_host(int i)
{
	assert(i >= 0);
	assert(i < hosts->len);
	return (GString*) g_ptr_array_index(hosts, i);
}


GString *get_not_host(int i)
{
	assert(i >= 0);
	assert(i < nots->len);
	return (GString*) g_ptr_array_index(nots, i);
}


GString *get_host_list(int i)
{
	assert(i >= 0);
	assert(i < host_lists->len);
	return (GString*) g_ptr_array_index(host_lists, i);
}


GString *get_not_host_list(int i)
{
	assert(i >= 0);
	assert(i < not_host_lists->len);
	return (GString*) g_ptr_array_index(not_host_lists, i);
}


void add_host(GString *host)
{
	g_ptr_array_add(hosts, host);
}


void add_not_host(GString *host)
{
	g_ptr_array_add(nots, host);
}


/**
 * Given a line of text, see if it matches a host name specification.
 *
 * @param line the text to scan for a host name.
 *
 * @return a newly allocated GString if a host name is found, or NULL if not
 * host name is found.
 */
static GString *line_to_host(char *line)
{
	GString *gs = 0;
	int matched;
	char *name;
	int namelen;

	matched = line_host_match(line, &name, &namelen);
	if (matched) {
		gs = g_string_new_len(name, namelen);
	}
	return gs;
}


/**
 * Match a line with a regex, looking for a host name.
 *
 * A line has
 * - optional white space
 * - optional host name
 * - optional white space
 * - optional # with any other characters following
 * - end of line.
 *
 * @param line the text of the line to match
 * @param name pointer to a pointer in which to save the start of the name
 * @param namelen pointer to where to save the length of the name
 *
 * @return true if a match was found, 0 otherwise.
 */
static int line_host_match(char *line, char **name, int *namelen)
{
	static int compiled = 0;
	static regex_t reg;
	int regret;
	regmatch_t matches[3];
	regoff_t off, len;

	static char lineregex[]
		= "^\\s*([[:alnum:]]+[[:alnum:]\\.-]*)\\s*(#.*)?";

	if (! compiled) {
		regret = regcomp(&reg, lineregex,
				 REG_EXTENDED | REG_NEWLINE);
		if (regret) {
			char errbuf[256];
			regerror(regret, &reg, errbuf, 256);
			fprintf(stderr, "Cannot compile \"%s\": %s\n",
				lineregex, errbuf);
			exit(3);
		}
		compiled = TRUE;
	}
	regret = regexec(&reg, line, 3, matches, 0);
	if (regret) {
		return 0;
	}

	if (matches[1].rm_so == -1) {
		return 0;
	}
	off = matches[1].rm_so;
	len = matches[1].rm_eo;
	*name = line + off;
	*namelen = len;
	return TRUE;
}


static void read_one_list(GPtrArray *list, GString *listname)
{
	FILE *f;
	char *data = 0;
	size_t size = 0;

	f = fopen(listname->str, "r");
	if (0 == f) {
		fprintf(stderr, "Cannot open %s: %s\n", listname->str,
			strerror(errno));
		return;
	}

	data = malloc(256);
	size = 256;
	while (1) {
		ssize_t ret;
		ret = getline(&data, &size, f);
		if (-1 == ret) {
			int err = errno;
			if (0 != err) {
				fprintf(stderr, "error reading %s: %s\n",
					listname->str, strerror(err));
			}
			break;
		}
		//printf("Line: <<<%s>>>\n", data);
		GString *gs = line_to_host(data);
		if (gs) {
			// gs is newly allocated, and the host list takes over
			// ownership.
			g_ptr_array_add(list, gs);
		}
	}
	free(data);
	fclose(f);
}


void add_list(GString *listname)
{
	read_one_list(hosts, listname);
}


void add_not_list(GString *listname)
{
	read_one_list(nots, listname);
}


/**
 * Sort the lists in place.
 */
void sort_hosts(void)
{
}


void process_lists(void)
{
	for (int i=0; i<hosts->len; i++) {
		if (in_list(nots, (GString*) g_ptr_array_index(hosts, i))) {
			g_ptr_array_remove_index(hosts, i);
			i--;
		}
	}
}


static void hosts_remove(gpointer x)
{
	GString *gs = (GString *) x;
	g_string_free(gs, TRUE);
}


/**
 * Is a host in an array?
 */
static int in_list(GPtrArray *a, GString *h)
{
	for (int i=0; i<a->len; i++) {
		if (g_string_equal(h, (GString*)g_ptr_array_index(a,i))) {
			return TRUE;
		}
	}
	return FALSE;
}

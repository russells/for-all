#ifndef lists_h_INCLUDED
#define lists_h_INCLUDED


#include <glib.h>


// lists.c
void init_lists(void);
void read_list(GString *listname);
void read_nots(GString *listname);
void add_host(GString *host);
void add_not_host(GString *host);
void add_list(GString *list);
void add_not_list(GString *list);
int host_len(void);
int n_hosts(void);
int n_not_hosts(void);
int n_host_lists(void);
int n_not_host_lists(void);
void success(GString *s);
void failure(GString *s);
GString *get_success(int i);
GString *get_failure(int i);
int n_successes(void);
int n_failures(void);
GString *get_host(int i);
GString *get_not_host(int i);
GString *get_host_list(int i);
GString *get_not_host_list(int i);
void sort_hosts(void);
void process_lists(void);
int hosts_name_length(void);


#endif // lists_h_INCLUDED

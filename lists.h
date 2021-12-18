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
int n_hosts(void);
int n_not_hosts(void);
int n_host_lists(void);
int n_not_host_lists(void);
GString *get_host(int i);
GString *get_not_host(int i);
GString *get_host_list(int i);
GString *get_not_host_list(int i);


#endif // lists_h_INCLUDED

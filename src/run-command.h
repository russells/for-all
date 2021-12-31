#ifndef run_command_h_INCLUDED
#define run_command_h_INCLUDED

#include <glib.h>
#include <stdio.h>



void run_command(GString *ssh,
		 GPtrArray *ssh_options,
		 GString *host,
		 GPtrArray *command,
		 int opt_single,
		 int opt_debug);



#endif // run_command_h_INCLUDED

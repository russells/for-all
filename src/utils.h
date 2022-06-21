#ifndef utils_h_INCLUDED
#define utils_h_INCLUDED

/**
 * Various utility macros used in different places.
 */

#include <glib.h>


// GString to char*.
#define g2c(p)      ( ((GString*)p)->str )
// GString array index to GString*.
#define a_g(p, i)   (  (GString*)(g_ptr_array_index(p,i)) )
// GString array index to char*.
#define a2g2c(p, i) ( a_g(p,i)->str )
// char* array index to char*.
#define a2c(p, i) ( (char*)(g_ptr_array_index(p,i)) )

// Add pointer to array.
#define ga(a,p)								\
	do {								\
		if (0 && opt_debug) {					\
			printf("adding to %s: %p\n", #a, (void*)p);	\
		}							\
		g_ptr_array_add(a,p);					\
	} while (0)


#endif // utils_h_INCLUDED

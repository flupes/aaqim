#ifndef AAQIM_UTILS_H
#define AAQIM_UTILS_H

#if defined(AAQIM_DEBUG)
#define dbg_printf(args...) printf(args)
#else
#define dbg_printf(...)
#endif

#endif

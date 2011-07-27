//#define _DEBUG_
#ifndef _DEBUG_
#define debug_msg(fmt, args...)
#else
#define debug_msg(fmt, args...) printf(fmt, ##args)
#endif

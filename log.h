
#ifndef _LOG_H_
#define _LOG_H_

enum log_level {
	LOG_DEBUG = 0x10,
	LOG_SIGNAL = 0x20,
};

#ifdef DEBUG
#if (DEBUG > 0)
#define logf(lvl, ...) do { if(DEBUG | lvl) { fprintf(stderr, __VA_ARGS__); } } while(0)
#endif
#else
#define logf(lvl, ...)
#endif

#endif


#ifndef __COMMON_H__
#define __COMMON_H__

#include <assert.h>

#define DEBUG

#ifdef DEBUG
#define Assert(...) \
  do { \
    assert(__VA_ARGS__);\
  } while (0)
#else
#define Assert(...) \
  do { \
    if(__VA_ARGS__);\
  } while (0)
#endif

#define min(x, y) (x < y ? x: y)

#endif
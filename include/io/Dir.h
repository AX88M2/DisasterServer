#ifndef DIR_H
#define DIR_H
#if defined(_WIN32)
#define WIN32_LEARN_AND_MEAN
#include <direct.h>
#define mkdir(dir, mode) _mkdir(dir)
#elif defined(__unix) || defined(__unix__)
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#endif
#endif
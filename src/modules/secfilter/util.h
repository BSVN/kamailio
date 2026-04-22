#ifndef _SECF_UTIL_H_
#define _SECF_UTIL_H_

#include "../../core/str.h"
#include "../../core/dprint.h"
#include "../../core/ut.h"

void secf_trim_quotes(str *input);

int parse_name(char *_s, int _len, str *name);

#endif
#include "util.h"

void secf_trim_quotes(str *input)
{
    if (!input || input->len < 2 || input->s == NULL) {
        return;
    }

    if (input->s[0] == '"' && input->s[input->len - 1] == '"') {
        input->s++;
        input->len -= 2;
    }
}

int parse_name(char *_s, int _len, str *name)
{
    if (_s == NULL || _len <= 0) {
        if (name) {
            name->s = NULL;
            name->len = 0;
        }
        LM_DBG("no name data found in contact header (empty)\n");
        return 0;
    }

    name->s = _s;
    name->len = _len;

    char *begin;
    int len;

    trim_len(len, begin, *name);
    name->s = begin;
    name->len = len;

    secf_trim_quotes(name);

    trim_len(len, begin, *name);
    name->s = begin;
    name->len = len;

    LM_DBG("parsed and trimmed name: %.*s\n", 
            name->len, name->s);

    return 0;
}
#ifndef LEXER_H
#define LEXER_H

#include "token.h"
#include "keywordMap.h"
#include "buffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

token get_next_token();

#endif

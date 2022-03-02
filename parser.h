#ifndef PARSER_H
#define PARSER_H

#include "FirstAndFollow.h"
#include "parserDef.h"
#include "globalDef.h"

void printRule(FILE* fp, GrammarRule gRule);

void printParseTableToFile();

void populateRules();

void createParseTable();

void writeParseTableToFile();

#endif


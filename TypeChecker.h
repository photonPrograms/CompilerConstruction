/*
    Group 18

    Team Members:
    1. Madhav Gupta (2019A7PS0063P)
    2. Meenal Gupta (2019A7PS0243P)
    3. Pratham Gupta (2019A7PS0051P)
    4. Sankha Das (2019A7PS0029P)
    5. Yash Gupta (2019A7PS1138P)
*/

# ifndef TYPE_CHECKER_HEADER
# define TYPE_CHECKER_HEADER

# include <stdio.h>
# include "globalDef.h"
//# include "FirstAndFollow.h"
# include "astGenerator.h"
# include "astDef.h"
# include "parserDef.h"
# include "lexerDef.h"
# include "parser.h"
# include "typing.h"
# include "symbolTableDef.h"
# include "symbolTable.h"


typedef struct VariableVisitedNode {
	char* lexeme;
	boolean visited;
	struct VariableVisitedNode* next;
} VariableVisitedNode;

boolean checkVariableChanges(astNode* root, VariableVisitedNode* toVisitLL);

boolean markVariableChanges(astNode* root, VariableVisitedNode* toVisitLL);

VariableVisitedNode* extractVariablesFromBoolean(astNode* root, VariableVisitedNode* toVisitLL);
struct TypeArrayElement* findType(astNode* root, 
		SymbolTable* localTable, SymbolTable* baseTable);

struct TypeArrayElement* findTypeField(astNode* root, 
		struct Field* fieldLL);

struct Field* searchInFieldLL(char* fieldLexeme,
		struct Field* fieldLL);

int typeCheck(astNode* root, SymbolTable* baseTable);

struct TypeArrayElement* checkTypeEquality(struct TypeArrayElement* t1, 
		struct TypeArrayElement* t2);

int findLengthFormal(FunctionParameter* head);

int findLengthActual(astNode* head);

# endif

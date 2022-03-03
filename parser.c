#include <stdio.h>
#include <string.h>
#include "parserDef.h"
#include "globalDef.h"
#include "lexerDef.h"
#include "lexer.h"
#include "FirstAndFollow.h"

char* parseTableFile = "parseTable.csv";

tokenTag syncTokens[] = {
    TK_END,
    TK_ENDWHILE,
    TK_ENDIF,
    TK_ENDUNION,
    TK_ENDRECORD,
    TK_SEM
};

int numSyncTokens = 6;

void printRule(FILE* fp, GrammarRule gRule){
    NonTerm A = NonTerms[gRule.head];
    tnt* alpha = gRule.body;

    fprintf(fp, "%s ===> ", A.symbol);
    for(int i = 0; i < gRule.bodyLength; i++){
        if(alpha[i].isEpsilon){
            fprintf(fp, " epsilon ");
        }else if(alpha[i].isTerminal){
            fprintf(fp, " %s ", tokenNames[alpha[i].terminal]);
        } else {
            fprintf(fp, " %s ", FirstAndFollowList[alpha[i].nonTermIndex].symbol);   
        }    
    }
    fprintf(fp, ",");
}

void printParseTableToFile(){
    FILE* fp = fopen(parseTableFile, "w+");
    fprintf(fp, ",");
    for(int terminalIndex = 0; terminalIndex < 58; terminalIndex++){
        fprintf(fp, "%s,", tokenNames[terminalIndex]);
    }
    fprintf(fp, "\n");
    for(int nonTerminalIndex = 0; nonTerminalIndex < numNonTerminals; nonTerminalIndex++){
        fprintf(fp, "%s,", FirstAndFollowList[nonTerminalIndex].symbol);
        for(int terminalIndex = 0; terminalIndex < NUMBER_OF_TOKENS; terminalIndex++){
            if(parseTable[nonTerminalIndex][terminalIndex] == -1){
                fprintf(fp, "Error,");
            } else if(parseTable[nonTerminalIndex][terminalIndex] == -2) {
                fprintf(fp, "Synch,");
            }
            else {
                printRule(fp, grammarRules[parseTable[nonTerminalIndex][terminalIndex]]);
            }
        }
        fprintf(fp, "\n");
    }
}

void populateRules(){
    // open the grammar file
    FILE *fp = fopen(GRAMMAR_FILE,"r");

    if (!fp) {
		printf("ERROR: could not open the file for populating rules!\n");
		exit(0);
	}

    char currLine[MAX_PROD_LEN * MAX_LEN];
    
    int grammarRulesIndex = 0;

    while(fgets(currLine, 500, fp) != NULL) {
        char head[MAX_LEN];
        char body[MAX_LEN * MAX_PROD_LEN];

        // get the current line from the file input stream
		// dividing it into head and body
		sscanf(currLine, "%s ===> %[^\n\t]", head, body);
        // printf("HEAD: %s ----> TAIL: %s\n",head,body);
        grammarRules[grammarRulesIndex].head = whichNonTerminal(head);
        
        char *token = strtok(body," ");
        int tailLength = 0;
        while(token != NULL) {
            //if it is a terminal
            int nonTermTailIndex = whichNonTerminal(token);
            // printf("\t\tnonTerminalIndex: %d string: %s\n",nonTermTailIndex,token);
            if(strcmp(epsilon,token) == 0) {
                //printf("\t\tepsilon pushed.\n");
                grammarRules[grammarRulesIndex].body[tailLength].isEpsilon = TRUE;
                grammarRules[grammarRulesIndex].body[tailLength].isTerminal = FALSE;
                grammarRules[grammarRulesIndex].body[tailLength].terminal = -1;
                grammarRules[grammarRulesIndex].body[tailLength].nonTermIndex = -1;
            }
            else if(nonTermTailIndex < 0) {
                //printf("\t\tToken number for string : %s\n",token);
                grammarRules[grammarRulesIndex].body[tailLength].isEpsilon = FALSE;
                grammarRules[grammarRulesIndex].body[tailLength].isTerminal = TRUE;
                grammarRules[grammarRulesIndex].body[tailLength].terminal = tokstrToToken(token);
                grammarRules[grammarRulesIndex].body[tailLength].nonTermIndex = -1;
            } else {
                grammarRules[grammarRulesIndex].body[tailLength].isEpsilon = FALSE;
                grammarRules[grammarRulesIndex].body[tailLength].isTerminal = FALSE;
                grammarRules[grammarRulesIndex].body[tailLength].nonTermIndex = nonTermTailIndex;
                grammarRules[grammarRulesIndex].body[tailLength].terminal = -1;
            }
            token = strtok(NULL," ");
            tailLength++;
        }
        grammarRules[grammarRulesIndex].bodyLength = tailLength;
        grammarRulesIndex++;    
    }
    return;
}

void createParseTable(FirstAndFollow FirstAndFollowList){
    memset(parseTable, -1, sizeof(parseTable));

    for(int i = 0; i < numSyncTokens; i++) {
        for(int j = 0; j < numNonTerminals; j++) {
            parseTable[j][syncTokens[i]] = -2;
        }
    }

    for(int ruleIndex = 0; ruleIndex < numRules; ruleIndex++){
        GrammarRule* prodRule = &grammarRules[ruleIndex];

        printf("Body Length: %d\n", prodRule->bodyLength);

        int nonTerminalIndex = prodRule->head;
        printf("%d. ", nonTerminalIndex);

        FirstAndFollowElement fnf = FirstAndFollowList[nonTerminalIndex];
        printf("%s ===> ", fnf.symbol);

        tnt* alpha = prodRule->body;
        
        for(int j = 0; j < prodRule->bodyLength; j++){
            tnt var = alpha[j];
            
            if (var.isEpsilon) {
                printf("epsilon");
                for(int k = 0; k < fnf.followLen; k++){
                    int terminalIndex = fnf.follow[k];
                    parseTable[nonTerminalIndex][terminalIndex] = ruleIndex;
                }

                if (fnf.dollarInFollow){
                    parseTable[nonTerminalIndex][TK_EOF] = ruleIndex;
                }
            }else if(var.isTerminal){
                printf("%s ", tokenNames[var.terminal]);
                // first of a terminal is the terminal itself 
                int terminalIndex = var.terminal;
                parseTable[nonTerminalIndex][terminalIndex] = ruleIndex;
            } else {
                // var is a non-terminal
                FirstAndFollowElement nonTerm = FirstAndFollowList[var.nonTermIndex];
                printf("%s ", nonTerm.symbol);

                for(int k = 0; k < nonTerm.firstLen; k++){
                    int terminalIndex = nonTerm.first[k];
                    printf("%d; ", terminalIndex);
                    parseTable[nonTerminalIndex][terminalIndex] = ruleIndex;
                }

                if (nonTerm.nullable) {
                    // if the first set of the non-terminal contains epsilon
                    for(int k = 0; k < fnf.followLen; k++){
                        int terminalIndex = fnf.follow[k];
                        parseTable[nonTerminalIndex][terminalIndex] = ruleIndex;
                    }     

                    if (fnf.dollarInFollow){
                        parseTable[nonTerminalIndex][TK_EOF] = ruleIndex;
                    }
                }
            } 
        }
        printf("\n");
    }
}

Stack* initiateStack(){
    Stack* inputStack = (Stack*) malloc(sizeof(Stack));
    inputStack->stackPointer = 0;
    
    tnt* bottomOfStack = (tnt*) malloc(sizeof(tnt));
    bottomOfStack->isTerminal = TRUE;
    bottomOfStack->terminal = TK_EOF;
    bottomOfStack->nonTermIndex = -1;
    bottomOfStack->isEpsilon = FALSE;

    inputStack->stackElements[inputStack->stackPointer] = bottomOfStack;

    return inputStack;
}

void pop(Stack* inputStack){
    if(inputStack->stackPointer == -1){
        return;
    }

    inputStack->stackElements[inputStack->stackPointer] = NULL;
    (inputStack->stackPointer)--;
}

void push(Stack* inputStack, tnt* var){
    if(inputStack->stackPointer == (STACK_SIZE - 1)){
        return;
    }

    (inputStack->stackPointer)++;
    inputStack->stackElements[inputStack->stackPointer] = var;
}

tnt* top(Stack* inputStack){
    return inputStack->stackElements[inputStack->stackPointer];
}

void insertRuleBodyIntoStack(Stack* inputStack, GrammarRule gRule){
    tnt* alpha = gRule.body;
    for(int i = gRule.bodyLength - 1; i >= 0; i--){
        push(inputStack, &alpha[i]);
    }
}


//1: correctly displayed this subtree, -1: could not display this subtree
int inorderTraversalParseTree (FILE* fp, ParseTreeNode *root) {
    
    if(root->isLeafNode) {
        return fprintf(fp,"%s\t\t%d\t\t%s\t\t----\t\t%s\t\tYes\t\t----",root->terminal.lexeme,root->terminal.linenumber,tokenNames[root->terminal.type]
        ,FirstAndFollowList[root->parent->nonTermIndex].symbol);
    }

    int i;
    int result = 1;
    for(i = 0; i < ( root->numberOfChildren - 1 ); i++) {
        result *= inorderTraversalParseTree(fp,root->children[i]);
    }

    fprintf(fp,"----\t\t----\t\t----\t\t----\t\t%s\t\tNo\t\t%s",FirstAndFollowList[root->parent->nonTermIndex].symbol, FirstAndFollowList[root->nonTermIndex].symbol);

    result *= inorderTraversalParseTree(fp,root->children[i]);

    return result;
}

ParseTreeNode* newParseTreeNode() {
    ParseTreeNode* newNode = (ParseTreeNode*)malloc(sizeof(ParseTreeNode));
    newNode->parent = NULL;
    newNode->isLeafNode = FALSE;
    token temp;
    temp.linenumber = 0;
    strcpy(temp.lexeme,"");
    temp.type = -1;
    newNode->numberStatus = 0;
    newNode->terminal = temp;
    newNode->nonTermIndex = -1;
    newNode->integerValue = 69;
    newNode->floatValue = 69.69;
    memset(newNode->children,NULL,MAX_PROD_LEN);
    newNode->nextSibling = NULL;
    newNode->numberOfChildren = 0;
    return newNode;
}

int printParseTree(ParseTreeNode* root, char* filename) {

    FILE* fp = fopen(filename,"w");

    if(!fp) {
        printf("Could not open file for printing tree.\n");
        return -1;
    }

    return inorderTraversalParseTree(fp,root);
}

tnt* createStackElement(token t){
    tnt* termOrNonTerm = (tnt*) malloc(sizeof(tnt));

    termOrNonTerm->isTerminal = TRUE;
    termOrNonTerm->isEpsilon = FALSE;
    termOrNonTerm->terminal = t.type;
    termOrNonTerm->nonTermIndex = -1;

    return termOrNonTerm;
}


ParseTreeNode* parseInputSourceCode(twinBuffer* buffer){
    // initialize stack
    Stack* inputStack = initiateStack();

    // pushing '$' to stack
    tnt* stackBottom = (tnt*) malloc(sizeof(tnt));
    stackBottom->isTerminal = TRUE;
    stackBottom->terminal = TK_EOF;
    stackBottom->isEpsilon = FALSE;
    stackBottom->nonTermIndex = -1;
    push(inputStack, stackBottom);

    // pushing <program> to stack
    stackBottom = (tnt*) malloc(sizeof(tnt));
    stackBottom->isTerminal = FALSE;
    stackBottom->terminal = -1;
    stackBottom->isEpsilon = FALSE;
    stackBottom->nonTermIndex = 0;
    push(inputStack, stackBottom);

    // initialize parseTree
    ParseTreeNode* root = newParseTreeNode();
    ParseTreeNode* current = root;
    
    tnt* topOfStack;

    while(TRUE){
        tnt* stackElement = createStackElement(get_next_token(buffer));

        topOfStack = top(inputStack);
    }

}
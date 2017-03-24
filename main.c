/* front.c - a lexical analyzer system for simple
arithmetic expressions */
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

/* Global declarations */
/* Variables */
int charClass;
char lexeme [100];
char expression [100]; /*holds the expression so far*/

char nextChar;
int lexLen;
int exprLen = 0;
int token;
int nextToken;
int prevToken; /*IDENT and IDENT, IDENT and INT_LIT, INT_LIT and INT_LIT etc cannot be side by side*/
int errorCode = 0; /*if this is anything other than 0, cease the function*/
int newline_found;

char lastCharacter;
 
size_t length = 0;
ssize_t read;
char* target_line = NULL;
int indexl;




FILE *in_fp, *fopen();

/* Function declarations */
void addChar();
void getChar();
void getNonBlank();
void addToExpression();
void clearExpression();

int lex();

void getLine();
void addLine();

void expr();
void term();
void factor();
void error1();
void error2();
void error3();


/* Character classes */
#define LETTER 0
#define DIGIT 1
#define UNKNOWN 99
/* Token codes */
#define NEWLINE 15
#define INT_LIT 10
#define IDENT 11
#define ASSIGN_OP 20
#define ADD_OP 21
#define SUB_OP 22
#define MULT_OP 23
#define DIV_OP 24
#define LEFT_PAREN 25
#define RIGHT_PAREN 26

// <expr> → <term> {(+ | -) <term>}
// <term> → <factor> {(* | /) <factor>}
// <factor> → id | int_constant | ( <expr> )


/******************************************************/
/* main driver */
/* main driver */

int main(int argc, char* argv[]) {
  /* Open the input data file and process its contents */

  /* If the file Name is not provided, exit! */
  if (argc != 2) {
    printf("No file specified :(\n");
    return 0;
  }

  if ((in_fp = fopen(argv[1], "r")) == NULL)
    printf("ERROR - cannot open front.in \n");
  else { 
    while ((read = getline(&target_line, &length, in_fp)) != -1) {
      printf("\n\n\n");
      indexl = 0;
      getChar();
      if(target_line != NULL){
        do {
          lex();  
          expr();
        } while (nextToken != EOF);
      }
    } 
  }
  return 0;
}

/*****************************************************/
/* lookup - a function to lookup operators and parentheses
and return the token */
int lookup(char ch) {
  switch (ch) {
  case '(':
  addChar();
  prevToken = nextToken;
  nextToken = LEFT_PAREN;
  break;

  case ')':
    addChar();
    prevToken = nextToken;
    nextToken = RIGHT_PAREN;
    break;
  
  case '+':
    addChar();
    prevToken = nextToken;
    nextToken = ADD_OP;
    break;
  
  case '-':
    addChar();
    prevToken = nextToken;  
    nextToken = SUB_OP;
    break;
  
  case '*':
    addChar();
    prevToken = nextToken;
    nextToken = MULT_OP;
    break;
  
  case '/':
    addChar();
    prevToken = nextToken;
    nextToken = DIV_OP;
    break;

  default:
    addChar();
    prevToken = nextToken;
    nextToken = EOF;
    break;
  }
  return nextToken;
}
/*****************************************************/
/* addChar - a function to add nextChar to lexeme */
void addChar() {
  if (lexLen <= 98) {
    lexeme[lexLen++] = nextChar;
    lexeme[lexLen] = 0;
  }
  else
    printf("Error - lexeme is too long \n");
}


/*adds all lexemes to an expression*/
void addToExpression() {
  if (exprLen <= 98) {
    /*add each character in current lexeme to expression*/
    for (int i = 0; i < lexLen; ++i) {
      expression[exprLen++] = lexeme[i];
      expression[exprLen] = 0;
    }
    
  }
}

void clearExpression() {
  for (int i = 0; i < exprLen; ++i) {
    expression[i] = 0;
  }
  exprLen = 0;
}

void clearLexeme() {
  for (int i = 0; i < lexLen; ++i) {
    lexeme[i] = 0;
  }
  lexLen = 0;
}

/*****************************************************/
/* getChar - a function to get the next character of
input and determine its character class */
void getChar() {
  if ((nextChar = getc(in_fp)) != EOF) {
    if (isalpha(nextChar))
      charClass = LETTER;
    else if (isdigit(nextChar))
      charClass = DIGIT;
    else if (nextChar == '\n') {
      charClass = NEWLINE;
    }
    else charClass = UNKNOWN;  
  }
  else
  charClass = EOF;
}

/*****************************************************/
/* getNonBlank - a function to call getChar until it
returns a non-
whitespace
character */
void getNonBlank() {
  while (isspace(nextChar))
    getChar();
}

/*****************************************************/
/* lex - a simple lexical analyzer for arithmetic
expressions */
int lex() {
  lexLen = 0;
  getNonBlank();
  switch (charClass) {
    /* Parse identifiers */
    case LETTER:
      addChar();
      getChar();
      while (charClass == LETTER || charClass == DIGIT) {
        addChar();
        getChar();
      }
      prevToken = nextToken;
      nextToken = IDENT;
      break;

    /* Parse integer literals */
    case DIGIT:
      addChar();
      getChar();
      while (charClass == DIGIT) {
        addChar();
        getChar();
      }
      prevToken = nextToken;
      nextToken = INT_LIT;
      break;

    /* Parentheses and operators */  
    case UNKNOWN:
      lookup(nextChar);
      getChar();
      break;

    case NEWLINE:
      prevToken = nextToken;
      nextToken = NEWLINE;
      printf("I'm a new line\n");
      lexeme[1] = 'N';
      lexeme[2] = 'L';
      lexeme[3] = 0;
      break;

    /* EOF */
    case EOF:
      prevToken = nextToken;
      nextToken = EOF;
      lexeme[0] = 'E';
      lexeme[1] = 'O';
      lexeme[2] = 'F';
      lexeme[3] = 0;
      break;
  } /* End of switch */

  if (errorCode == 0) {
    printf("\nPrevious token is: %d, Next token is: %d, Next lexeme is %s\n",
      prevToken, nextToken, lexeme);
    addToExpression();
    printf("%s \n", expression);
  }

  return nextToken;
} /* End of function lex */



/* expr
Parses strings in the language generated by the rule:
<expr> -> <term> {(+ | -) <term>}
*/
void expr() {
  //printf("Enter <expr>\n");
  /* Parse the first term */
  term();

  /* As long as the next token is + or -, get
  the next token and parse the next term */
  while (nextToken == ADD_OP || nextToken == SUB_OP) {
    lex();
    term();
  }
  // printf("Exit <expr>\n");
} /* End of function expr */

/* term
Parses strings in the language generated by the rule:
<term> -> <factor> {(* | /) <factor>)
*/
void term() {
  // printf("Enter <term>\n");
  /* Parse the first factor */
  factor();

  /* As long as the next token is * or /, get the
  next token and parse the next factor */
  while (nextToken == MULT_OP || nextToken == DIV_OP) {
    lex();
    factor();
  }
  //printf("Exit <term>\n");
} /* End of function term */

/* factor
Parses strings in the language generated by the rule:
<factor> -> id | int_constant | ( <expr )
*/
void factor() {
  printf("Enter <factor>\n");
  /* Determine which RHS */
  if (nextToken == IDENT || nextToken == INT_LIT)
    /* Get the next token */
    lex();
  /* If the RHS is ( <expr>), call lex to pass over the
  left parenthesis, call expr, and check for the right
  parenthesis */
  else {
    if (nextToken == LEFT_PAREN) {
      lex();
      expr();
      if (nextToken == RIGHT_PAREN)
        lex();
      else
        error1();
    } /* End of if (nextToken == ... */
    /* It was not an id, an integer literal, or a left
    parenthesis */
    else
      error2();
  } /* End of else */
  printf("Exit <factor>\n");;
} /* End of function factor */ 

// <expr> → <term> {(+ | -) <term>}
// <term> → <factor> {(* | /) <factor>}
// <factor> → id | int_constant | ( <expr> )

//+ and - must be followed by a term or factor
//* 

void error1() {
  printf("AN ERROR OCCURRED: Unclosed parenthesis.\n");
  errorCode = 1;
}

void error2() { 
  printf("AN ERROR OCCURRED: Expected an id, an integer literal, or a left parenthesis, received '%s'\n", lexeme);
  errorCode = 2;
}

void error3() {
  printf("AN ERROR OCCURRED: Expected an operator, but received '%s'\n", lexeme);
  errorCode = 3;
}
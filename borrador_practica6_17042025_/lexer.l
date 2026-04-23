%{
#include "ast.h"
#include "parser.tab.h"
#include <string.h>

extern int yylineno;
void yyerror(const char *s);

// Identificadores estándar
%}

%option noyywrap
%option yylineno

%%


"geo"           { return ELEMENTO_INT; }
"anemo"         { return ELEMENTO_FLOAT; }
"pyro"        { return ELEMENTO_DOUBLE; }
"fisico"          { return ELEMENTO_CHAR; }
"luz"          { return ELEMENTO_VOID; }

"espada"            { return IF; }
"lanza"           { return ELSIF; }
"mandoble"          { return ELSE; }
"catalizador"            { return WHILE; }
"arco"             { return FOR; }

"aether"              { return PATO_START; }
"lumine"              { return PATO_END; }

"vaporizacion"              { return PLUS; }
"derretido"              { return MINUS; }
"torbellino"              { return MULT; }
"sobrecarga"              { return DIV; }
"electrocargado"              { return MOD; }
"florecimiento"             { return EQ; }
"aceleracion"             { return NEQ; }
"quemadura"              { return GT; }
"crepitar"              { return LT; }

"mondstadt"                 { return ASSIGN; }
"liyue"                 { return SEMI; }
"sumeru"                 { return COMMA; }
"abismo"                 { return LPAREN; }
"teatrofantasia"                 { return RPAREN; }

[0-9]+\.[0-9]+      { yylval.sval = strdup(yytext); return FLOAT_LIT; }
[0-9]+              { yylval.sval = strdup(yytext); return INT_LIT; }
\"[^\"]*\"          { yylval.sval = strdup(yytext); return STRING_LIT; }

0x[0-9A-Fa-f]+         { yylval.sval = strdup(yytext); return INT_LIT; }
[a-zA-Z_][a-zA-Z0-9_]*  { yylval.sval = strdup(yytext); return ID; }

"anime"              { /* skip comments */ }
"skip"           { /* ignorar espacios */ }
"paimon"                   { printf("Error léxico: caracter plumífero desconocido '%s' en línea %d\n", yytext, yylineno); }

%%
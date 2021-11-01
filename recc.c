#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

//トークンの種類
typedef enum {
  TK_RESERVED,      //記号
  TK_NUM,           //整数
  TK_EOF,           //入力の終わりを表すトークン
} TokenKind;

typedef struct Token Token;

struct Token {
  TokenKind kind;   //トークンの型
  Token *next;      //次の入力トークン
  int val;          //kindがTK_NUMの場合、その数値
  char *str;        //トークン文字列
};

//現在着目しているトークン
Token *token;

//入力された文字列全体を指す
char *user_input;

void error_at(char *loc, char *fmt, ...);
bool consume(char op);
void expect(char op);
int expect_number();
bool at_eof();
Token *new_token(TokenKind kind, Token *cur, char *str);
Token *tokenize();

int main(int argc,char **argv) {
  if(argc != 2){
    fprintf(stderr,"引数の数が正しくありません\n");
    return 1;
  }

  user_input = argv[1];
  //トークナイズする
  token = tokenize();

  //アセンブリの前半部分を出力
  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");

  //式の最初は数字でなければならないので、それをチェック
  //最初のmov命令を出力
  printf("  mov rax,%d\n",expect_number());

  //'+ <数>'あるいは'- <数>'というトークンの並びを消費しつつ、アセンブリを出力
  while(!at_eof()) {
    //次のトークンの先頭が＋だったらaddする
    if(consume('+')) {
      printf("  add rax,%d\n",expect_number());
      continue;
    }

    expect('-');
    printf("  sub rax,%d\n",expect_number());
  }

  printf("  ret\n");
  return 0;
}


//入力文字列pをトークナイズしてそれを返す
Token *tokenize() {
  char *p = user_input;
  //ダミーのhead、入力の先頭の前に繋げる
  //headという構造体作成、その構造体のnextはNULL
  Token head;
  head.next = NULL;
  //Tokenを直接触らないためのcur
  Token *cur = &head;

  while(*p) {

    //空白文字をスキップ
    if(isspace(*p)) {
      p++;
      continue;
    }

    if(*p == '+' || *p == '-') {
      //引数は後置インクリメントなので、関数に渡されるのはpの値
      //curに関数の結果が渡された後にインクリメントする
      cur = new_token(TK_RESERVED,cur,p++);
      continue;
    }

    if(isdigit(*p)) {
      cur = new_token(TK_NUM,cur,p);
      cur -> val = strtol(p,&p,10);
      continue;
    }

    error_at(p,"expected a number");
  }
  new_token(TK_EOF,cur,p);

  //headの次は入力文字列の先頭
  return head.next;
}

//次のトークンが数値の場合、トークンを1つ読み進めてその数値を返す
int expect_number() {
  if(token -> kind != TK_NUM){
    error_at(token -> str,"expected a number");
  }
  int val = token -> val;
  //次のトークンを見る
  token = token -> next;
  return val;
}

//新しいトークンを作成してcurに繋げる
Token *new_token(TokenKind kind,Token *cur,char *str) {
  //mallocと違い、あり当てられたメモリをゼロクリアする必要がない
  Token *tok = calloc(1,sizeof(Token));
  tok -> kind = kind;
  tok -> str = str;
  //ここのcurは一つ前のトークン。一つ前のトークンのnextが新しく作成したトークンを指す
  cur -> next = tok;
  return tok;
}

//次のトークンが期待している記号のときは、トークンを1つ読み進める
void expect(char op) {
  if(token -> kind != TK_RESERVED || token -> str[0] != op){
    error_at(token -> str,"expected '%c'",op);
  }
  token = token -> next;
}

//次のトークンが期待している記号のときは、トークンを1つ読み進める
bool consume(char op) {
  if (token -> kind != TK_RESERVED || token -> str[0] != op){
    return false;
  }
  token = token -> next;
  return true;
}

//エラーを報告するための関数
//printfと同じ引数を取る
void error_at(char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap,fmt);

  //入力文字列全体のアドレス？からエラー箇所のアドレス？を引くと、入力文字列の何バイト目にエラーがあるかわかる
  //ポインタとポインタの差分を取る
  //ポインタの減算は、内部でアドレスを引いた後にそのポインタが指している変数の型
  //のバイト数(sizeof(変数の型))で割った結果を求めるようにコンパイラは動く
  //この場合はcharなので/1している
  //初めに定義した変数の方が、割り当てられるメモリのアドレス番号が小さいから？
  int pos = loc - user_input;
  //printf("pos:%d\n",pos);
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, "");      //pos個の空白を出力
  fprintf(stderr, "^ ");
  vfprintf(stderr,fmt,ap);
  fprintf(stderr,"\n");
  exit(1);
}

//指している部分が末尾であればtrue
bool at_eof() {
  return token -> kind == TK_EOF;
}

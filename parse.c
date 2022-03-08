#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include "recc.h"


bool startswith(char *p,char *q) {
  return memcmp(p,q,strlen(q)) == 0;
}


//新しいトークンを作成してcurに繋げる
Token *new_token(TokenKind kind,Token *cur,char *str,int len) {
  //mallocと違い、あり当てられたメモリをゼロクリアする必要がない
  Token *tok = calloc(1,sizeof(Token));
  tok -> kind = kind;
  tok -> str = str;
  tok -> len = len;
  //ここのcurは一つ前のトークン。一つ前のトークンのnextが新しく作成したトークンを指す
  cur -> next = tok;
  return tok;
}


//エラーを報告するための関数
//printfと同じ引数を取る
//第二引数は可変長
void error_at(char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap,fmt);

  //エラー箇所のアドレスから入力文字列の先頭のアドレスを引くと、入力文字列の何バイト目にエラーがあるかわかる
  //ポインタとポインタの差分（アドレス番号の引き算）
  //locは、入力文字列（user_input）のエラーがある場所のアドレスを指しているため
  //必ずlocが指すアドレスのほうがuser_inputが指すアドレスより大きくなる
  int pos = loc - user_input;
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

    if(startswith(p,"==") || startswith(p,"!=") || startswith(p,"<=") || startswith(p,">=")) {
      cur = new_token(TK_RESERVED,cur,p,2);
      p += 2;
      continue;
    }

    if(startswith(p,"=")) {
      cur = new_token(TK_RESERVED,cur,p++,1);
      continue;
    }


    if(strchr("+-*/()<>;",*p)) {
      //引数は後置インクリメントなので、関数に渡されるのはpの値
      //curに関数の結果が渡された後にインクリメントする
      //長さは１
      cur = new_token(TK_RESERVED,cur,p++,1);
      continue;
    }

    if(isdigit(*p)) {
      cur = new_token(TK_NUM,cur,p,0);
      char *q = p;
      //第２引数&pは、pに変換不可能な値があった場合の格納先
      //変換可能：数値、変換不可能：文字
      cur -> val = strtol(p,&p,10);
      cur -> len = p - q;
      continue;
    }

    //アルファベット1文字a〜zならば、TK_IDENT型のトークンを作る
    if('a' <= *p && *p <= 'z') {
      cur = new_token(TK_IDENT,cur,p++,1);
      continue;
    }

    error_at(p,"invalid token");
  }
  new_token(TK_EOF,cur,p,0);

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


//次のトークンが期待している記号のときは、トークンを1つ読み進める
void expect(char *op) {
  if(token -> kind != TK_RESERVED || strlen(op) != token -> len || memcmp(token -> str,op,token -> len)){
    error_at(token -> str,"expected \"%s\"",op);
  }
  token = token -> next;
}


//次のトークンが期待している記号のときは、トークンを1つ読み進める
bool consume(char *op) {
  if (token -> kind != TK_RESERVED || strlen(op) != token -> len || memcmp(token -> str,op,token -> len)){
    return false;
  }
  token = token -> next;
  return true;
}

//抽象構文木の新しいノードを作成
//左辺と右辺を受け取る2項演算子
Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = calloc(1,sizeof(Node));
  node -> kind = kind;
  node -> lhs = lhs;
  node -> rhs = rhs;
  return node;
}


//抽象構文木の新しいノードを作成
//数値（構文木の終端）
Node *new_node_num(int val) {
  Node *node = calloc(1,sizeof(Node));
  node -> kind = ND_NUM;
  node -> val = val;
  return node;
}

//program = stmt*
void program() {
  int i = 0;

  while(!at_eof()) {
    code[i++] = stmt();
  }
  code[i] = NULL;
}

//stmt = expr ";"
Node *stmt() {
  Node *node = expr();
  expect(";");

  return node;
}

//expr = assign
Node *expr() {
  return assign();
}

//assign = equality ("=" assign)?
Node *assign() {
  Node *node = equality();

  if(consume("=")) {
    node = new_node(ND_ASSIGN,node,assign());
  }

  return node;
}

//equality = relational ("==" relational | "!=" relational)*
Node *equality() {
  Node *node = relational();

  for(;;) {
    if(consume("==")) {
      node = new_node(ND_EQ,node,relational());
    }else if (consume("!=")) {
      node = new_node(ND_NE,node,relational());
    }else {
      return node;
    }
  }
}


//relational = add ("<" add | "<=" add | ">" add | ">=" add)*
Node *relational() {
  Node *node = add();

  for(;;) {
    if(consume("<")) {
      node = new_node(ND_LT,node,add());
    }else if(consume("<=")) {
      node = new_node(ND_LE,node,add());
    }else if(consume(">")) {
      //addは木の左に展開される
      //こういう構成はよくあるらしい
      node = new_node(ND_LT,add(),node);
    }else if(consume(">=")) {
      node = new_node(ND_LE,add(),node);
    }else {
      return node;
    }
  }
}


//add = mul ("+" mul | "-" mul)*
Node *add() {
  Node *node = mul();

  for(;;) {
    if(consume("+")) {
      node = new_node(ND_ADD,node,mul());
    }else if(consume("-")) {
      node = new_node(ND_SUB,node,mul());
    }else {
      //ここで返される抽象構文木は、演算子は左結合（返されるノードの左側の枝のほうが深くなる）
      return node;
    }
  }
}


//mul = unary ("*" unary | "/" unary)*
Node *mul() {
  Node *node = unary();

  for(;;) {
    if(consume("*")) {
      node = new_node(ND_MUL,node,unary());
    }else if(consume("/")) {
      node = new_node(ND_DIV,node,unary());
    }else {
      //ここで返される抽象構文木は、演算子は左結合（返されるノードの左側の枝のほうが深くなる）
      return node;
    }
  }
}


//unary = ("+" | "-")? primary
Node *unary(){
  if(consume("+")) {
    //+XをXと置き換える
    return unary();
  }else if(consume("-")) {
    //-Xを0-Xと置き換える
    return new_node(ND_SUB,new_node_num(0),unary());
  }

  //＋もーもついていなければprimaryを呼び出す
  //＋、ーがついててもついてなくても適応する
  return primary();
}


//primary = num | ident | "(" expr ")"
Node *primary() {
  Token *tok;

  if(token -> kind == TK_IDENT){
    tok = token;
    token = token -> next;
  }else{
    tok = NULL;
  }

  if(tok) {
    Node *node = calloc(1,sizeof(Node));
    node -> kind = ND_LVAR;
    node -> offset = (tok -> str[0] - 'a' + 1) * 8;
    return node;
  }

  //次のトークンが"("なら、"(" expr ")"のはず
  if(consume("(")) {
    Node *node = expr();
    expect(")");
    return node;
  }

  //そうでなければ数値のはず
  return new_node_num(expect_number());
}

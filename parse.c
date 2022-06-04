#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include "recc.h"

LVar *locals;


//指している部分が末尾であればtrue
bool at_eof() {
  return token -> kind == TK_EOF;
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

//stmt = expr ";" | "return" expr ";" | "if" "(" expr ")" stmt ("else" stmt)?
//       | "while" "(" expr ")" stmt
//       | "for" "(" expr? ";" expr? ";" expr? ")" stmt
// "if" ( cond ) then "else" els
// "for" ( init; cond; inc ) body
// "while" ( cond ) body
// "do" body "while" ( cond )
// "switch" ( cond ) body
// "case" val ":" body
Node *stmt() {
  Node *node;

  fprintf(stderr, "token is %d\n",token -> kind);

  if(token -> kind == TK_RETURN) {
    node = calloc(1,sizeof(Node));
    node -> kind = ND_RETURN;
    token = token -> next;
    node -> lhs = expr();

  }else if(token -> kind == TK_IF) {
    node = calloc(1,sizeof(Node));
    node -> kind = ND_IF;
    token = token -> next;
    expect("(");
    node -> cond = expr();
    expect(")");

    node -> then = stmt();

    fprintf(stderr, "if no naka token is %d\n",token -> kind);

    //Token *tmp = token -> next;
  }else if(token -> kind == TK_ELSE) {
    fprintf(stderr, "KOYAMA\n");
    node = calloc(1,sizeof(Node));
    node -> kind = ND_IFELSE;
    token = token -> next;
    node -> els = stmt();
  }else {
    node = expr();
  }

  fprintf(stderr, "if DETA token is %d\n",token -> kind);

  if(!consume(";")) {
    error_at(token -> str,"';'ではないトークンです");
  }
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

  tok = consume_ident(tok);

  if(tok) {
    Node *node = calloc(1,sizeof(Node));
    node -> kind = ND_LVAR;

    LVar *lvar = find_lvar(tok);

    if(lvar) {
      node -> offset = lvar -> offset;
    }else {
      //localsの初期化
      if(locals == NULL) {
        locals = calloc(1,sizeof(LVar));
        //リストの最後はNULL
        locals -> next = NULL;
      }

      lvar = calloc(1,sizeof(LVar));
      //リストの先頭に追加していく
      lvar -> next = locals;
      lvar -> name = tok -> str;
      lvar -> len = tok -> len;
      lvar -> offset = locals -> offset + 8;
      node -> offset = lvar -> offset;
      locals = lvar;
    }
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

LVar *find_lvar(Token *tok) {
  for(LVar *var = locals;var;var = var -> next) {
    if(var -> len == tok -> len && !memcmp(tok -> str,var -> name,var -> len)){
      return var;
    }
  }
  return NULL;
}

//変数かそうでないか判定
Token *consume_ident(Token *tok) {
  if(token -> kind == TK_IDENT){
    tok = token;
    token = token -> next;
    return tok;
  }else{
    return NULL;
  }
}

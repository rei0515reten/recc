#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

//トークナイザー（トークンの種類）
typedef enum {
  TK_RESERVED,      //記号
  TK_NUM,           //整数
  TK_EOF,           //入力の終わりを表すトークン
} TokenKind;

typedef struct Token Token;

//抽象構文木（ノードの種類）
typedef enum {
  ND_ADD,           // +
  ND_SUB,           // -
  ND_MUL,           // *
  ND_DIV,           // /
  ND_NUM,           // 整数
} NodeKind;

typedef struct Node Node;

struct Node {
  NodeKind kind;    //ノードの型
  Node *lhs;        //左辺
  Node *rhs;        //右辺
  int val;          //kindがND_NUMの場合のみ使う
};

//Token型のnextは、リストの次の構造体を示す
//示している構造体はToken型なので、それを指すポインタもToken型でなければいけない
//その変数の型が訳わからなくなったら、その変数が何を指しているか、何を代入しようとしているのかを考える
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
Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int val);
Node *expr();
Node *mul();
Node *primary();
void gen(Node *node);


//トークナイズ　➔　抽象構文木　➔　コード生成（スタック）
int main(int argc,char **argv) {
  if(argc != 2){
    fprintf(stderr,"引数の数が正しくありません\n");
    return 1;
  }

  //トークナイズしてバースする
  user_input = argv[1];
  token = tokenize();
  //返ってくるnodeは抽象構文木
  //expr,mul,primaryでトークン列をパースする
  Node *node = expr();

  //アセンブリの前半部分を出力
  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");

  //抽象構文木を下りながらコード生成
  gen(node);

  //スタックトップに式全体の値が残っているはずなので
  //それをRAXにロードして関数からの返り値とする
  printf("  pop rax\n");
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

    if(strchr("+-*/()",*p)) {
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

    error_at(p,"invalid token");
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

//expr = mul ("+" mul | "-" mul)*
Node *expr() {
  Node *node = mul();

  for(;;) {
    if(consume('+')) {
      node = new_node(ND_ADD,node,mul());
    }else if(consume('-')) {
      node = new_node(ND_SUB,node,mul());
    }else {
      //ここで返される抽象構文木は、演算子は左結合（返されるノードの左側の枝のほうが深くなる）
      return node;
    }
  }
}

//mul = primary ("*" primary | "/" primary)*
Node *mul() {
  Node *node = primary();

  for(;;) {
    if(consume('*')) {
      node = new_node(ND_MUL,node,primary());
    }else if(consume('/')) {
      node = new_node(ND_DIV,node,primary());
    }else {
      //ここで返される抽象構文木は、演算子は左結合（返されるノードの左側の枝のほうが深くなる）
      return node;
    }
  }
}

//primary = "(" expr ")" | num
Node *primary() {
  //次のトークンが"("なら、"(" expr ")"のはず
  if(consume('(')) {
    Node *node = expr();
    expect(')');
    return node;
  }

  //そうでなければ数値のはず
  return new_node_num(expect_number());
}

//x86-64のスタック操作命令
void gen(Node *node){
  if(node -> kind == ND_NUM) {
    printf("  push %d\n",node -> val);
    return;
  }

  //左から下る
  gen(node -> lhs);
  gen(node -> rhs);

  //スタックトップから順にRDI,RAXにpop
  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch(node -> kind) {
    case ND_ADD:
      printf("  add rax, rdi\n");
      break;
    case ND_SUB:
      printf("  sub rax, rdi\n");
      break;
    case ND_MUL:
      printf("  imul rax, rdi\n");
      break;
    case ND_DIV:
      //idiv ➔ 符号あり除算を行う命令
      //x86-64のidivの仕様
      //暗黙のうちに、RDXとRAXを取り、その2つを合わせたものを128ビット整数とみなす。
      //その整数を引数のレジスタ（今回はRDI）の64ビットの値で割り、商をRAX、余りをRAXにセットする。
      //という仕様。
      //RDXとRAX(128bit) / RDI(64bit) = RAX(商)  RAX(余り)
      //cqo命令を使うと、RAXに入っている64ビットの値を128ビットに伸ばしてRDXとRAXにセットする
      printf("  cqo\n");
      //RDIは割られる値
      printf("  idiv rdi\n");
      break;
  }

  //RAXの値（演算の結果）をスタックにpush
  printf("  push rax\n");
}

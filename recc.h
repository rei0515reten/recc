#ifndef RECC_H
#define RECC_H

//トークナイザー（トークンの種類）
typedef enum {
  TK_RESERVED,      //記号
  TK_NUM,           //整数
  TK_EOF,           //入力の終わりを表すトークン
} TokenKind;

//抽象構文木（ノードの種類）
typedef enum {
  ND_ADD,           // +
  ND_SUB,           // -
  ND_MUL,           // *
  ND_DIV,           // /
  ND_EQ,            //==
  ND_NE,            //!=
  ND_LT,            //<
  ND_LE,            //<=
  ND_NUM,           // 整数
} NodeKind;

typedef struct Node Node;

struct Node {
  NodeKind kind;    //ノードの型
  Node *lhs;        //左辺
  Node *rhs;        //右辺
  int val;          //kindがND_NUMの場合のみ使う
};

typedef struct Token Token;

//Token型のnextは、リストの次の構造体を示す
//示している構造体はToken型なので、それを指すポインタもToken型でなければいけない
//その変数の型が訳わからなくなったら、その変数が何を指しているか、何を代入しようとしているのかを考える
struct Token {
  TokenKind kind;   //トークンの型
  Token *next;      //次の入力トークン
  int val;          //kindがTK_NUMの場合、その数値
  char *str;        //トークン文字列
  int len;          //トークンの長さ
};

//現在着目しているトークン
extern Token *token;
//入力された文字列全体を指す
extern char *user_input;

void error_at(char *loc, char *fmt, ...);
bool consume(char *op);
void expect(char *op);
int expect_number();
Token *new_token(TokenKind kind, Token *cur, char *str,int len);
Token *tokenize();
Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int val);
bool startswith(char *p,char *q);
Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();
void gen(Node *node);


#endif

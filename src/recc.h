#ifndef RECC_H    /* 二重インクルード防止 */
#define RECC_H

/* トークナイザ（トークンの種類）*/
typedef enum 
{
  TK_RESERVED,   //記号
  TK_IDENT,      //識別子
  TK_NUM,        //整数
  TK_EOF,        //入力の終わりを表すトークン
  TK_RETURN,
  TK_IF,
  TK_ELSE,
  TK_WHILE,
  TK_FOR,
} enu_TokenKind;

/* 抽象構文木（ノードの種類）*/
typedef enum 
{
  ND_ADD,           // +
  ND_SUB,           // -
  ND_MUL,           // *
  ND_DIV,           // /
  ND_ASSIGN,        //=
  ND_EQ,            //==
  ND_NE,            //!=
  ND_LT,            //<
  ND_LE,            //<=
  ND_LVAR,          //ローカル変数
  ND_NUM,           // 整数
  ND_RETURN,        //return
  ND_IF,
  ND_WHILE,
  ND_FOR,
} enu_NodeKind;

typedef struct st_Node st_Node;
struct st_Node          /* 抽象構文木のノード */
{
  enu_NodeKind kind;    //ノードの型
  st_Node*     lhs;     //左辺
  st_Node*     rhs;     //右辺
  int          val;     //kindがND_NUMの場合のみ使う
  int          offset;  //kindがND_LVARの場合のみ使う

  st_Node*     cond;
  st_Node*     then;
  st_Node*     els;
};

typedef struct st_Token st_Token;
struct st_Token         /* トークン */
{
  enu_TokenKind kind;   //トークンの型
  st_Token*     next;   //次の入力トークン(リストの次の構造体を示す。st_Token型構造体を示すst_Token型のポインタ)
  int           val;    //kindがTK_NUMの場合、その数値
  char*         str;    //トークン文字列
  int           len;    //トークンの長さ
};

typedef struct st_LVar st_LVar;
struct st_LVar 
{
  st_LVar* next;       //次の変数かNULL
  char*    name;       //変数の名前
  int      len;        //名前の長さ
  int      offset;     //RBPからのオフセット
};

extern st_Token* currToken;     /* 現在着目しているトークン */
extern char*     user_input;    /* 入力された文字列全体を指す */
extern st_Node*  code[100];  /* パース結果のノードを順にストアする配列(最後はNULLを埋めてどこが最後なのかわかるようにする) */
extern st_LVar*  locals;     /* ローカル変数のリスト */

void error_at(char *loc, char *fmt, ...);
bool consume(char *op);
void expect(char *op);
int expect_number();
st_Token* new_token(enu_TokenKind kind, st_Token* cur, char* str, int len);
st_Token* tokenize();
st_Node* new_node(enu_NodeKind kind, st_Node* lhs, st_Node* rhs);
st_Node* new_node_num(int val);
bool compare_str(char* p, char* q);
void parse();
st_Node* stmt();
st_Node* expr();
st_Node* assign();
st_Node* equality();
st_Node* relational();
st_Node* add();
st_Node* mul();
st_Node* unary();
st_Node* primary();
st_LVar* find_lvar(st_Token* tok);
st_Token* consume_ident(st_Token* tok);
int is_alnum(char c);
void gen(st_Node* node);

#endif

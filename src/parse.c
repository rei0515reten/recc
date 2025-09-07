#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include "recc.h"
#include <assert.h>

st_LVar *locals;     /* ローカル変数のリスト */

/* 現在のトークンがEOFであるか */
bool at_eof()
{
  return currToken->kind == TK_EOF;
}

//次のトークンが数値の場合、トークンを1つ読み進めてその数値を返す
int expect_number() 
{
  if(currToken->kind != TK_NUM)
  {
    error_at(currToken->str, "expected a number");
  }
  int val = currToken->val;
  //次のトークンを見る
  currToken = currToken->next;
  return val;
}


//次のトークンが期待している記号のときは、トークンを1つ読み進める
void expect(char* op) 
{
  if(currToken->kind != TK_RESERVED || strlen(op) != currToken->len || memcmp(currToken->str, op, currToken->len))
  {
    error_at(currToken->str, "expected \"%s\"", op);
  }
  currToken = currToken->next;
}


//次のトークンが期待している記号のときは、トークンを1つ読み進める
bool consume(char* op) 
{
  if (currToken->kind != TK_RESERVED || strlen(op) != currToken->len || memcmp(currToken->str, op, currToken->len))
  {
    return false;
  }
  currToken = currToken->next;
  return true;
}

//抽象構文木の新しいノードを作成
//左辺と右辺を受け取る2項演算子
st_Node *new_node(enu_NodeKind kind, st_Node* lhs, st_Node* rhs) 
{
  st_Node* node = calloc(1, sizeof(st_Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}


//抽象構文木の新しいノードを作成
//数値（構文木の終端）
st_Node* new_node_num(int val) 
{
  st_Node* node = calloc(1, sizeof(st_Node));
  node->kind = ND_NUM;
  node->val = val;
  return node;
}

/* パースメイン関数(parse = stmt*) */
void parse() 
{
  int i = 0;

  while(!at_eof()) 
  {
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
st_Node* stmt() 
{
  st_Node* node;

  if(currToken->kind == TK_RETURN) 
  {
    node = calloc(1, sizeof(st_Node));
    node->kind = ND_RETURN;
    currToken = currToken->next;
    node->lhs = expr();
  }
  else if(currToken->kind == TK_IF) 
  {
    node = calloc(1, sizeof(st_Node));
    node->kind = ND_IF;
    currToken = currToken->next;
    expect("(");
    node->cond = expr();
    expect(")");
    node->then = stmt();

    if(currToken->kind == TK_ELSE) 
    {
      currToken = currToken->next;
      node->els = stmt();
    }
    return node;
  }
  else if(currToken->kind == TK_WHILE) 
  {
    node = calloc(1, sizeof(st_Node));
    node->kind = ND_WHILE;
    currToken = currToken->next;
    expect("(");
    node->cond = expr();
    expect(")");
    node->then = stmt();

    return node;
  }
  else
  {
    node = expr();
  }

  if(!consume(";")) 
  {
    error_at(currToken->str, "';'ではないトークンです");
  }
  return node;
}

//expr = assign
st_Node* expr() 
{
  return assign();
}

//assign = equality ("=" assign)?
st_Node* assign() 
{
  st_Node* node = equality();

  if(consume("=")) 
  {
    node = new_node(ND_ASSIGN, node, assign());
  }

  return node;
}

//equality = relational ("==" relational | "!=" relational)*
st_Node* equality() 
{
  st_Node* node = relational();

  for(;;) 
  {
    if(consume("==")) 
    {
      node = new_node(ND_EQ, node, relational());
    }
    else if (consume("!=")) 
    {
      node = new_node(ND_NE, node, relational());
    }
    else 
    {
      return node;
    }
  }
}


//relational = add ("<" add | "<=" add | ">" add | ">=" add)*
st_Node* relational() 
{
  st_Node* node = add();

  for(;;) 
  {
    if(consume("<")) 
    {
      node = new_node(ND_LT, node, add());
    }
    else if(consume("<=")) 
    {
      node = new_node(ND_LE, node, add());
    }
    else if(consume(">")) 
    {
      //addは木の左に展開される
      //こういう構成はよくあるらしい
      node = new_node(ND_LT, add(), node);
    }
    else if(consume(">=")) 
    {
      node = new_node(ND_LE, add(), node);
    }
    else 
    {
      return node;
    }
  }
}


//add = mul ("+" mul | "-" mul)*
st_Node* add() 
{
  st_Node* node = mul();

  for(;;) 
  {
    if(consume("+")) 
    {
      node = new_node(ND_ADD, node, mul());
    }
    else if(consume("-")) 
    {
      node = new_node(ND_SUB, node, mul());
    }
    else 
    {
      //ここで返される抽象構文木は、演算子は左結合（返されるノードの左側の枝のほうが深くなる）
      return node;
    }
  }
}


//mul = unary ("*" unary | "/" unary)*
st_Node* mul() 
{
  st_Node* node = unary();

  for(;;) 
  {
    if(consume("*")) 
    {
      node = new_node(ND_MUL, node, unary());
    }
    else if(consume("/")) 
    {
      node = new_node(ND_DIV, node, unary());
    }
    else 
    {
      //ここで返される抽象構文木は、演算子は左結合（返されるノードの左側の枝のほうが深くなる）
      return node;
    }
  }
}


//unary = ("+" | "-")? primary
st_Node* unary()
{
  if(consume("+")) 
  {
    //+XをXと置き換える
    return unary();
  }
  else if(consume("-")) 
  {
    //-Xを0-Xと置き換える
    return new_node(ND_SUB, new_node_num(0), unary());
  }

  //＋もーもついていなければprimaryを呼び出す
  //＋、ーがついててもついてなくても適応する
  return primary();
}


//primary = num | ident | "(" expr ")"
st_Node* primary() 
{
  st_Token* tok;

  tok = consume_ident(tok);

  if(tok) 
  {
    st_Node* node = calloc(1, sizeof(st_Node));
    node->kind = ND_LVAR;

    st_LVar* lvar = find_lvar(tok);

    if(lvar) 
    {
      node->offset = lvar->offset;
    }
    else 
    {
      //localsの初期化
      if(locals == NULL) 
      {
        locals = calloc(1, sizeof(st_LVar));
        //リストの最後はNULL
        locals->next = NULL;
      }

      lvar = calloc(1, sizeof(st_LVar));
      //リストの先頭に追加していく
      lvar->next = locals;
      lvar->name = tok->str;
      lvar->len = tok->len;
      lvar->offset = locals->offset + 8;
      node->offset = lvar->offset;
      locals = lvar;
    }
    return node;
  }

  //次のトークンが"("なら、"(" expr ")"のはず
  if(consume("(")) 
  {
    st_Node* node = expr();
    expect(")");
    return node;
  }

  //そうでなければ数値のはず
  return new_node_num(expect_number());
}

st_LVar* find_lvar(st_Token* tok) 
{
  for(st_LVar* var = locals; var; var = var->next) 
  {
    if(var->len == tok->len && !memcmp(tok->str, var->name, var->len))
    {
      return var;
    }
  }
  return NULL;
}

//変数かそうでないか判定
st_Token* consume_ident(st_Token* tok) 
{
  if(currToken->kind == TK_IDENT)
  {
    tok = currToken;
    currToken = currToken->next;
    return tok;
  }
  else
  {
    return NULL;
  }
}

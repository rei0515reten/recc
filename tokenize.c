#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include "recc.h"

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

    if(strncmp(p,"if",2) == 0) {
      cur = new_token(TK_IF,cur,p,2);
      p += 2;

      continue;
    }


    if(strncmp(p,"return",6) == 0 && !is_alnum(p[6])) {
      cur = new_token(TK_RETURN,cur,p,6);
      p += 6;

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

    //ローカル変数をトークナイズ
    if(isalpha(*p) || *p == '_') {
      char *q = p;

      while(*p != ' ' && *p != ';'){
        //foo+bar;(ok)
        if(strchr("+-*/()<>=",*p)) {
          break;
        }

        //foo@;(ng)
        if(strchr("@:$#&",*p)) {
          error_at(p,"Illegal variable");
        }
        p++;
      }
      cur = new_token(TK_IDENT,cur,q,p - q);

      continue;
    }

    error_at(p,"invalid token");
  }
  new_token(TK_EOF,cur,p,0);

  //headの次は入力文字列の先頭
  return head.next;
}

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

//returnの後ろに来てはいけない文字であるか
int is_alnum(char c) {
  return ('a' <= c && c <= 'z') || ('A' <= c && c <= '9') || (c == '_');
}

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include "recc.h"

/* tokenize.c メイン関数 */
/* Tokenize the input string p and return the tokens */
st_Token* tokenize() 
{
  char* inptr = user_input; /* 入力文字列の先頭 */
  st_Token head;            //ダミーのhead、入力の先頭の前に繋げる
  head.next = NULL;
  st_Token* cur = &head;    //Tokenを直接触らないためのcur

  while(*inptr)              /* NULLまで回す */
  {
    if(isspace(*inptr))      //空白文字をスキップ
    {
      inptr++;
      continue;
    }

    if(compare_str(inptr, "==") || compare_str(inptr, "!=") ||   /* 条件・比較 */
    compare_str(inptr, "<=") || compare_str(inptr, ">=")) 
    {
      cur = new_token(TK_RESERVED, cur, inptr, 2);
      inptr += 2;
      continue;
    }

    if(compare_str(inptr, "="))    /* 代入演算子*/
    {
      cur = new_token(TK_RESERVED, cur, inptr, 1);
      inptr++;
      continue;
    }

    if(strchr("+-*/()<>;", *inptr))   /*入力ポインタの先頭が演算子・括弧・セミコロンであるか */
    {
      cur = new_token(TK_RESERVED, cur, inptr++, 1);
      continue;
    }

    if(strncmp(inptr, "if", 2) == 0)         /* if(第一引数と第二引数を第三引数バイト分比較する) */
    {
      cur = new_token(TK_IF, cur, inptr, 2);
      inptr += 2;
      continue;
    }

    if(strncmp(inptr, "else", 4) == 0)       /* else(第一引数と第二引数を第三引数バイト分比較する) */
    {
      cur = new_token(TK_ELSE, cur, inptr, 4);
      inptr += 4;
      continue;
    }

    if(strncmp(inptr, "while", 5) == 0)      /* while(第一引数と第二引数を第三引数バイト分比較する) */
    {
      cur = new_token(TK_WHILE, cur, inptr, 5);
      inptr += 5;
      continue;
    }

    if(strncmp(inptr, "return", 6) == 0 && !is_alnum(inptr[6])) /* return(第一引数と第二引数を第三引数バイト分比較＆禁止文字チェック) */
    {
      cur = new_token(TK_RETURN, cur, inptr, 6);
      inptr += 6;
      continue;
    }

    if(isdigit(*inptr))  /* 数字ならばトークナイズ */
    {
      cur = new_token(TK_NUM, cur, inptr, 0);
      char *tmpptr = inptr;                   /*ダブルポインタ */
      cur->val = strtol(inptr, &inptr, 10);   /* 数値(long型)に変換 */
                                              /* 第２引数&inptrはinptrに変換不可能な値があった場合の格納先(変換可能：数値、変換不可能：文字) */
      cur->len = inptr - tmpptr;              /* 数字の長さ */
      continue;
    }

    if(isalpha(*inptr) || *inptr == '_')  /* ローカル変数名の先頭は英字またはアンダースコアである必要がある */
    {
      char* q = inptr;

      while(*inptr != ' ' && *inptr != ';')
      {
        //foo+bar;(ok)
        if(strchr("+-*/()<>=!", *inptr))    /*入力ポインタの先頭が演算子・括弧・セミコロンであるか */
        {
          break;
        }

        //foo@;(ng)
        if(strchr("@:$#&", *inptr))    /*入力ポインタの先頭が無効記号であるか */
        {
          error_at(inptr, "Illegal variable");
        }
        inptr++;
      }
      cur = new_token(TK_IDENT, cur, q, inptr - q);
      continue;
    }

    error_at(inptr, "invalid token");    /* どれにも当てはまらなかったらエラー */
  }
  new_token(TK_EOF, cur, inptr, 0);      /* 最後にEOFトークンを追加 */

  return head.next;   /* head.nextは最初のトークンを指しているので、最初のトークン情報を返す */
}

/* 文字列が先頭から一致するか */
bool compare_str(char* inptr,char* str) 
{
  return memcmp(inptr, str, strlen(str)) == 0;
}

/* 新しいトークンを作成 */
/* 新しく作成したトークンを返却*/
st_Token* new_token(enu_TokenKind kind,st_Token* currTok,char* str,int len) 
{
  st_Token* newTok = calloc(1, sizeof(st_Token));    /* callocはmallocと違い、割り当てられたメモリをゼロクリアする必要がない */
  newTok->kind = kind;
  newTok->str = str;
  newTok->len = len;
  currTok->next = newTok;      /* 現在のトークンのnextが新しく作成したトークンを指す */
  return newTok;
}

/* エラー報告関数(printfと同じ引数を取る,第二引数は可変長) */
void error_at(char* loc, char* fmt, ...) 
{
  va_list ap;
  va_start(ap, fmt);

  //エラー箇所のアドレスから入力文字列の先頭のアドレスを引くと、入力文字列の何バイト目にエラーがあるかわかる
  //locは、入力文字列（user_input）のエラーがある場所のアドレスを指しているため,必ずlocが指すアドレスのほうがuser_inputが指すアドレスより大きくなる
  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, "");      //pos個の空白を出力
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

/* returnの後ろにあってはいけない文字があるか */
int is_alnum(char c) 
{
  return ('a' <= c && c <= 'z') || ('A' <= c && c <= '9') || (c == '_');
}

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include "recc.h"
#include <assert.h>

st_Token* currToken;  /* 現在着目しているトークン */
char*     user_input; /* 入力された文字列全体を指す */
st_Node*  code[100];  /* パース結果のノードを順にストアする配列(最後はNULLを埋めてどこが最後なのかわかるようにする) */

/* recc 処理概要 */
/* トークナイズ　➔　抽象構文木　➔　コードジェネレータ（スタック） */

/* main.c メイン関数 */
int main(int argc, char** argv) 
{
  if(argc != 2)     /* 引数チェック */
  {
    fprintf(stderr,"引数の数が正しくありません\n");
    return 1;
  }

  user_input = argv[1];       /* 入力された文字列をuser_inputに保存 */
  currToken = tokenize();     /* トークナイズ(返ってくるトークンは抽象構文木で、最初のトークンが返却される) */
  parse();                    /* トークン列をパース */

  /* アセンブリの前半部分を出力 */
  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");

  /* プロローグ */
  /* rbpをスタックに保存して、rspをrbpに設定 */
  /* rspをrbpに設定して、スタックの領域を確保 */
  /* ここでは、ローカル変数の領域を確保するため */
  /* 208バイト確保しているので、26個の8バイトの */
  /* ローカル変数を扱えるようにしている */
  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  printf("  sub rsp, 208\n");

  for(int i = 0; code[i]; i++)  /* codeの配列の;区切りの式を先頭から順にコード生成 */
  {
    gen(code[i]);               /* コードジェネレータ */
    /* 式の評価結果としてスタックに一つの値が残っているはずなので*/
    /* スタックが溢れないようにポップしておく */
    printf("  pop rax\n");
  }

  /* エピローグ */
  /* 最後の式の結果がRAXに残っているのでそれが戻り値になる */
  printf("  mov rsp, rbp\n");
  printf("  pop rbp\n");
  printf("  ret\n");

  return 0;
}

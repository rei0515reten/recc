#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include "recc.h"

//トークナイズ　➔　抽象構文木　➔　コードジェネレータ（スタック）
int main(int argc,char **argv) {
  if(argc != 2){
    fprintf(stderr,"引数の数が正しくありません\n");
    return 1;
  }

  //トークナイズしてバースする
  user_input = argv[1];
  token = tokenize();
  //返ってくるnodeは抽象構文木
  //トークン列をパースする
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

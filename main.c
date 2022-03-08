#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include "recc.h"

Token *token;
//入力された文字列全体を指す
char *user_input;

Node *code[100];

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
  program();

  //アセンブリの前半部分を出力
  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");

  //プロローグ
  //変数26個分の領域を確保
  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  printf("  sub rsp, 208\n");

  //codeの配列の;区切りの式を先頭から順にコード生成
  for(int i = 0;code[i];i++){
    gen(code[i]);

    // 式の評価結果としてスタックに一つの値が残っているはずなので、スタックが溢れないようにポップしておく
    printf("  pop rax\n");
  }

  // エピローグ
  // 最後の式の結果がRAXに残っているのでそれが返り値になる
  printf("  mov rsp, rbp\n");
  printf("  pop rbp\n");
  printf("  ret\n");

  return 0;
}

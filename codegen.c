#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include "recc.h"

//式を左辺値として評価する
void gen_lval(Node *node) {
  if(node -> kind != ND_LVAR) {
    fprintf(stderr,"代入の左辺値が変数ではありません");
    exit(1);
  }

  printf("  mov rax, rbp\n");
  printf("  sub rax, %d\n",node -> offset);
  printf("  push rax\n");
}

//x86-64のスタック操作命令
void gen(Node *node){
  if(node -> kind == ND_RETURN) {
    gen(node -> lhs);
    printf("  pop rax\n");
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    return;
  }

  switch (node -> kind) {
    case ND_NUM:
      printf("  push %d\n",node -> val);
      return;

    case ND_LVAR:
      gen_lval(node);
      printf("  pop rax\n");
      printf("  mov rax, [rax]\n");
      printf("  push rax\n");
      return;

    case ND_ASSIGN:
      gen_lval(node -> lhs);
      gen(node -> rhs);

      printf("  pop rdi\n");
      printf("  pop rax\n");
      printf("  mov [rax], rdi\n");
      printf("  push rdi\n");
      return;
    }

    if(node -> kind == ND_IF || node -> kind == ND_IFELSE) {
      gen(node -> cond);
      gen(node -> then);
      gen(node -> els);
    } else {
      //左から下る
      gen(node -> lhs);
      gen(node -> rhs);
    }

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
    case ND_EQ:
      //cmp命令
      //スタックから２つの整数をポップして比較を行う
      //x86-64のcmp命令の結果はフラグレジスタにセットされる。
      //cmp命令はフラグレジスタの全フィールド(ZFやOFなど)を更新する。
      //更新するフィールドは命令によって違う。
      printf("  cmp rax, rdi\n");
      //sete命令
      //ZFの値が１のとき(0であるとき)にALに1をセットする。
      //それ以外の場合は0をセットする。
      printf("  sete al\n");
      //movzb命令
      //seteがALに値をセットすると自動的にRAXもこうしんされることになるが、RAX全体に0か1を
      //セットしたい場合は上位56ビットをゼロクリアしなければならない。
      //それを行うのがこの命令。
      //ALはRAXの下位8ビット。
      printf("  movzb rax, al\n");
      break;
    case ND_NE:
      printf("  cmp rax, rdi\n");
      printf("  setne al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_LT:
      printf("  cmp rax, rdi\n");
      printf("  setl al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_LE:
      printf("  cmp rax, rdi\n");
      printf("  setle al\n");
      printf("  movzb rax, al\n");
      break;

    case ND_IFELSE:
      if(node -> kind == ND_IFELSE) {
        printf("  je .Lelese001\n");
        printf("  jmp .Lend0002\n");
        printf(".Lelse001\n");
        printf(".Lend002\n");
      }else {
        printf("  je .Lend001\n");
        printf(".Lend001\n");
      }

    case ND_IF:
      printf("  pop rax\n");
      printf("  cmp rax, 0");
      break;

  }


  //RAXの値（演算の結果）をスタックにpush
  printf("  push rax\n");
}

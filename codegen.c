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

      printf("  pop rax\n");
      printf("  pop rdi\n");
      printf("  mov [rdi], rax\n");
      printf("  push rax\n");
      return;
    }

    if(node -> kind != ND_IF) {
      gen(node -> lhs);
      gen(node -> rhs);

      //スタックトップから順にRDI,RAXにpop
      printf("  pop rdi\n");
      printf("  pop rax\n");
    }



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
    case ND_IF:
      gen(node -> cond);          //genでは(node->cond)->kindを見る
      printf("  cmp rax, 0\n");   //raxが0のときelse

      if(node -> els) {
        printf("  je els\n");
      }else{
        printf("  je end\n");
      }

      gen(node -> then);

      if(node -> els){
        printf("  jmp end\n");
        printf("els:\n");
        gen(node -> els);
      }
      printf("end:\n");
      break;
    case ND_WHILE:
      printf("start:\n");
      gen(node -> cond);
      printf("  cmp rax, 0\n");   //raxが0のときelse
      printf("  je end\n");

      gen(node -> then);

      printf("  jmp start:\n");
      printf("end:\n");
      break;
  }

  //RAXの値（演算の結果）をスタックにpush
  printf("  push rax\n");
}

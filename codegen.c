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

static int count() {
  static int i = 1;
  return i++;
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
      printf("  cqo\n");
      printf("  idiv rdi\n");       //RDIは割られる値
      break;
    case ND_EQ:
      printf("  cmp rax, rdi\n");
      printf("  sete al\n");        //ZFの値が１のとき(値が0であるとき)にALに1をセットする。
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
      int c = count();
      gen(node -> cond);          //genでは(node->cond)->kindを見る
      printf("  cmp rax, 0\n");   //raxが0のときelse

      if(node -> els) {
        printf("  je els.%d\n",c);
      }else{
        printf("  je end.%d\n",c);
      }

      gen(node -> then);

      if(node -> els){
        printf("  jmp end.%d\n",c);
        printf("els.%d:\n",c);
        gen(node -> els);
      }
      printf("end.%d:\n",c);
      break;
    case ND_WHILE:
      c = count();
      printf("start.%d:\n",c);
      gen(node -> cond);
      printf("  cmp rax, 0\n");   //raxが0のときelse
      printf("  je end.%d\n",c);

      gen(node -> then);

      printf("  jmp start.%d:\n",c);
      printf("end.%d:\n",c);
      break;
  }

  //RAXの値（演算の結果）をスタックにpush
  printf("  push rax\n");
}

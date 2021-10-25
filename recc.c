#include <stdio.h>
#include <stdlib.h>

int main(int argc,char **argv) {
  if(argc != 2){
    fprintf(stderr,"引数の数が正しくありません\n");
    return 1;
  }

  char *p = argv[1];

  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");
  //文字列を10進の数値に変換（&pには読み込んだ次の文字列のアドレス)
  printf("  mov rax,%ld\n",strtol(p,&p,10));

  //指し示す部分は上のstrtolでずれているため、ここでは文字列の二番目を指している
  while(*p) {
    if(*p == '+') {
      p++;
      printf("  add rax,%ld\n",strtol(p,&p,10));
      continue;
    }

    if(*p == '-') {
      p++;
      printf("  sub rax,%ld\n",strtol(p,&p,10));
      continue;
    }

    fprintf(stderr,"予期しない文字：%c\n",*p);
  }

  printf("  ret\n");
  return 0;
}

.intel_syntax noprefix

.global main

.text
main:
//x = 1;while(x != 5) x = x + 1;

	mov rax, 1
	push rax
	pop rax
	mov rbx, 5
start:
	cmp rax, rbx
	jnz end
	add rax, 1
	jmp start
end:


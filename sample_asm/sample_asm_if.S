.intel_syntax noprefix

.global main

.text
main:
//if(x != 0) return 0 else return 1

	mov rax, 0
	push rax
	pop rax
	mov rbx, 0
	cmp rax ,rbx
	jnz els
	mov rax ,0
	ret
	jmp end
els:
	mov rax ,1
	ret
end:	

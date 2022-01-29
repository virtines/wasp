[bits 16]
[org 0x8000]
[section .text]

global _start
_start:
	mov esp, 0x8000
	mov     esi, 20
	call    fib
	hlt


fib:
	cmp esi, 2
	jge .fib_sum
	mov eax, esi
	ret

.fib_sum:

	;# stack: [  acc    ] [  arg    ]
	;#        esp + 0     esp+4

	sub esp, 8
	mov dword [esp + 4], esi
	mov dword [esp + 0], 0

	sub esi, 2
	call fib
	add dword [esp + 0], eax

	mov esi, dword [esp + 4]
	sub esi, 1
	call fib
	add dword [esp + 0], eax
	mov eax, dword [esp + 0]
	add esp, 8
	ret



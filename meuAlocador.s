.section .data

    topoInicialHeap: .quad 0
    
    erroFinaliza: .string "Erro ao restaurar heap.\n"
    erroNumBytes: .string "Número de bytes inválido.\n"
    erroAloca: .string "Erro ao alocar bloco.\n"

    aux: .quad 0

.section .text
.globl _start

iniciaAlocador:
    movq $12, %rax                      # escolhe o tipo de syscall
    movq $0, %rdi                       # para retornar o valor atual 
    syscall
    movq %rax, topoInicialHeap
    ret
    
finalizaAlocador:
    movq $12, %rax                      # escolhe o tipo de syscall (brk)
    movq topoInicialHeap, %rdi          # define novo valor de brk
    syscall
    movq topoInicialHeap, %rax          
    
    cmpq $0, %rax                       # if ret syscall = 0
    jne fim_if_fin
    
    movq $1, %rax                       # write    
    movq $1, %rdi                       # stdout
    movq $erroFinaliza, %rsi            # inicio do buffer
    movq $25, %rdx                      # tam do buffer
    syscall
fim_if_fin:
    ret
    
alocaMem:
    pushq %rbp
    movq %rsp, %rbp

    movq 16(%rbp), %rax
    cmpq $0, %rax                       # if num_bytes <= 0
    jg fim_if_num0
    
    movq $1, %rax                       # write  
    movq $1, %rdi                       # stdout                                
    movq $erroNumBytes, %rsi            # inicio do buffer                      
    movq $28, %rdx                      # tam do buffer                         
    syscall 
   
    movq $0, %rax
    popq %rbp
    ret 
fim_if_num0:
    subq $32, %rsp                      # aloca espaço p/ variaveis locais
    movq topoInicialHeap, %rax          
    movq %rax, -8(%rbp)                 # posAtualHeap em -8(%rbp)   
    
    movq $12, %rax                      # escolhe o tipo de syscall (brk)
    movq $0, %rdi                       # para retornar o valor atual 
    syscall  
    movq %rax, -16(%rbp)                # topoAtualHeap em -16(%rbp) 
    
    movq $0, -24(%rbp)                  # blocoLivre = NULL em -24(%rbp)
    movq $4096, -32(%rbp)               # menorTamanho = 4096 em -32(%rbp)    

while:
    movq -8(%rbp), %rax                 
    cmpq %rax, -16(%rbp)                # while posAtualHeap < topoAtualHeap
	jge fim_while_aloca
    
    movq -8(%rbp), %rbx
    movq (%rbx), %r10                   # %r10 = estado
    addq $8, %rbx
    movq (%rbx), %r11                   # %r11 = tamanho

    cmpq $0, %r10
    jne fim_if_while                    # if estado == 0
    movq 16(%rbp), %rax
    cmpq %r11, %rax
    jl fim_if_while                     # && tamanho >= num_bytes
    cmpq %r11, -32(%rbp)
    jge fim_if_while                    # && tamanho < menorTamanho
    
    movq -8(%rbp), %rax
    movq %rax, -24(%rbp)                # blocoLivre = posAtualHeap
    movq %r11, -32(%rbp)                # menorTamanho = tamanho 

fim_if_while:
    shr $12, %r11                       # tamanho = tamanho / 4096
    addq $1, %r11                       # tamanho++
    shl $12, %r11                       # tamanho * 4096 
    addq $16, %r11                      # tamanho + 16
    addq %r11, -8(%rbp)                 # posAtualHeap = tamanho                 

fim_while_aloca:
    cmpq $0, -24(%rbp)                  
    je else_bloco_livre                 # if blocoLivre
    
    movq -24(%rbp), %rbx
    movq $1, (%rbx)                     # marca estado como ocupado
    
    addq $16, -24(%rbp)
    movq -24(%rbp), %rax                # retorno = blocoLivre + 16
    
    addq $32, %rsp                      # desaloca variaveis locais
    popq %rbp                           # restaura %rbp
    ret    
    
else_bloco_livre: 
    movq 16(%rbp), %r11
    addq $4095, %r11                    # novoTamanho = %r11 = num_bytes + 4095
    shr $12, %r11                       # %r11 / 4096
    shl $12, %r11                       # %r11 * 4096
    
    movq -16(%rbp), %r10                # %r10 = topoAtalHeap
    addq %r11, %r10                     # %r10 = topoAtualHeap + novoTamanho
    addq $16, %r10                      # %r10 += 16
    movq $12, %rax                      # define o tipo de syscall (brk)
    movq %r10, %rdi                     # define o novo topo da heap
    syscall
    
    cmpq $0, %rax
    jne alocou                           # if retorno de brk != 0
    
    movq $1, %rax                       # write                                 
    movq $1, %rdi                       # stdout                                
    movq $erroAloca, %rsi               # inicio do buffer                      
    movq $28, %rdx                      # tam do buffer                         
    syscall

    addq $32, %rsp                      # desaloca variaveis
    movq $0, %rax                       # retorno = NULL
    popq %rbp                           # restaura %rbp
    ret

alocou:
    movq -16(%rbp), %r10                # %r10 = novoBloco
    movq $1, (%r10)                     # estado do novoBloco = ocupado
    addq $8, %r10                       # %r10 = novoBloco + 8
    movq 16(%rbp), %rax
    movq %rax, (%r10)                   # novoBloco + 8 = num_bytes
    
    addq $8, %r10                       # %r10 = novoBloco + 8 + 8
    movq %r10, %rax                     # retorno = novoBloco + 16
    addq $32, %rsp                      # desaloca variaveis
    popq %rbp                           # restaura %rbp
    ret

liberaMem:
    pushq %rbp
    movq %rsp, %rbp                     
    
    cmpq $0, 16(%rbp)                   # if bloco
    jne tem_bloco           
    popq %rbp                           # restaura %rbp
    movq $1, %rax                       # retorno = 1
    ret

tem_bloco:
    movq 16(%rbp), %r11
    subq $16, %r11                      # %r11 = bloco-16
    movq (%r11), %r10                   # %r10 = estado do bloco
    
    cmpq $0, %r10                       # if estado == 0
    jne ta_ocupado                      
    popq %rbp                           # restaura %rbp
    movq $2, %rax                       # retorno = 2
    ret

ta_ocupado:
    movq $0, (%r11)                     # marca como livre

    popq %rbp                           # restaura %rbp 
    movq $0, %rax                       # retorno = 0
    ret
    
_start:
    pushq %rbp
    movq %rsp, %rbp    

    subq $8, %rsp                       # void* bloco1 em -8(%rbp)  
    subq $8, %rsp                       # void* bloco2 em -16(%rbp)

    call iniciaAlocador
    
    movq $500, %rax                     
    pushq %rax                          # empilha parametro
    call alocaMem
    addq $8, %rsp                       # libera espaço do parametro
    movq %rax, -8(%rbp)                 # retorno da função no bloco 1
   
    movq $400, %rax                     
    pushq %rax                          # empilha parametro
    call alocaMem
    addq $8, %rsp                       # libera espaço do parametro
    movq %rax, -16(%rbp)                # retorno da função no bloco 2
      
    movq -8(%rbp), %rax                     
    pushq %rax                          # empilha parametro
    call liberaMem
    addq $8, %rsp                       # libera espaço do parametro

    movq -16(%rbp), %rax                     
    pushq %rax                          # empilha parametro
    call liberaMem
    addq $8, %rsp                       # libera espaço do parametro
    
    call finalizaAlocador

    addq $16, %rsp
    movq $60, %rax
    syscall

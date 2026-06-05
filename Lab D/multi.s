section .rodata
    format_hex: db "%02hhx", 0       ; Format for printing a single byte
    format_newline: db 10, 0         ; Newline character

section .data
    ; Default variables required for Part 4 (No-arguments mode)
    global x_struct
    global y_struct
    x_struct: db 5
    x_num: db 0xaa, 1, 2, 0x44, 0x4f
    y_struct: db 6
    y_num: db 0xaa, 1, 2, 3, 0x44, 0x4f

    ; Variables for LFSR Pseudo-Random Number Generator
    STATE: dw 0xACE1                 ; 16-bit LFSR initialized state
    MASK:  dw 0xB400                 ; 16-bit LFSR mask

section .bss
    ; Buffers for reading input strings (Part 1.B)
    in_buf: resb 600

section .text
    global main
    global print_multi
    global getmulti
    global add_multi
    global PRmulti

    extern printf
    extern fgets
    extern stdin
    extern malloc

main:
    push ebp
    mov ebp, esp
    push ebx
    push esi
    push edi

    mov ebx, [ebp + 8]   ; ebx = argc
    mov edx, [ebp + 12]  ; edx = argv

    cmp ebx, 1
    je .default_mode     ; If argc == 1, no arguments provided

    ; Parse argv[1]
    mov esi, [edx + 4]   ; esi = pointer to argv[1] string
    mov ax, word [esi]   ; Load the first 2 bytes of the string (-I or -R)

    cmp ax, 0x492D       ; '-I' in Little Endian (0x2D is '-', 0x49 is 'I')
    je .stdin_mode
    
    cmp ax, 0x522D       ; '-R' in Little Endian (0x2D is '-', 0x52 is 'R')
    je .random_mode

.default_mode:
    mov esi, x_struct
    mov edi, y_struct
    jmp .calculate_and_print

.stdin_mode:
    call getmulti
    mov esi, eax         ; Save pointer to 1st struct in ESI
    call getmulti
    mov edi, eax         ; Save pointer to 2nd struct in EDI
    jmp .calculate_and_print

.random_mode:
    call PRmulti
    mov esi, eax
    call PRmulti
    mov edi, eax

.calculate_and_print:
    ; Print first number
    push esi
    call print_multi
    add esp, 4

    ; Print second number
    push edi
    call print_multi
    add esp, 4

    ; Add them together
    push edi
    push esi
    call add_multi
    add esp, 8
    
    ; add_multi returns pointer to result in EAX
    ; Print the result
    push eax
    call print_multi
    add esp, 4

    ; Epilogue
    mov eax, 0           ; Return 0 (Success)
    pop edi
    pop esi
    pop ebx
    mov esp, ebp
    pop ebp
    ret

; PART 1.A: Print Multi-precision Integer
print_multi:
    push ebp
    mov ebp, esp
    push ebx
    push esi

    mov esi, [ebp + 8]       ; Pointer to struct
    movzx ebx, byte [esi]    ; ebx = array size

    test ebx, ebx
    jz .end_print

    ; Start from the highest index (size) down to 1 for correct Endianness
    mov ecx, ebx             

.print_multi_loop:
    movzx eax, byte [esi + ecx] 

    ; Print single byte
    push ecx                 ; Backup caller-saved ECX before printf
    push eax
    push format_hex
    call printf
    add esp, 8
    pop ecx

    dec ecx
    jnz .print_multi_loop

    push format_newline
    call printf
    add esp, 4

.end_print:
    pop esi
    pop ebx
    mov esp, ebp
    pop ebp
    ret

; PART 1.B: Read Multi-precision Integer
getmulti:
    push ebp
    mov ebp, esp
    push ebx
    push esi
    push edi

    ; Plant a '0' at index 0 for odd-length strings
    mov byte [in_buf], '0'

    mov eax, [stdin]
    push eax
    push 500
    push in_buf + 1
    call fgets
    add esp, 12

    ; Find actual string length
    mov ecx, 0
.find_len:
    mov al, byte [in_buf + 1 + ecx]
    cmp al, 10
    je .found_end
    cmp al, 0
    je .found_end
    inc ecx
    jmp .find_len

.found_end:
    mov byte [in_buf + 1 + ecx], 0

    ; Adjust pointer for odd lengths
    mov edx, ecx
    and edx, 1               
    mov esi, in_buf + 1
    sub esi, edx             

    ; Calculate needed bytes
    mov eax, ecx
    add eax, edx             
    shr eax, 1               ; EAX = size in bytes (n)

    push eax                 ; Preserve 'n'
    mov edx, eax
    inc edx                  ; Size to allocate = n + 1
    
    push edx
    call malloc
    add esp, 4
    
    mov edi, eax             ; EDI = pointer to newly allocated struct
    pop eax                  ; Restore 'n' into EAX

    mov byte [edi], al       ; Set the size byte
    push edi                 ; Save base pointer to return at the end

    inc edi                  ; Move EDI to start of the num array
    ; --------------------------------------------------------

    mov ebx, eax
    shl ebx, 1
    sub ebx, 2

    mov ecx, eax
    test ecx, ecx
    jz .end_getmulti

.convert_loop:
    ; High nibble
    mov al, byte [esi + ebx]
    call hex_char_to_val
    shl al, 4
    mov dl, al

    ; Low nibble
    mov al, byte [esi + ebx + 1]
    call hex_char_to_val
    or al, dl

    mov byte [edi], al
    inc edi

    sub ebx, 2
    dec ecx
    jnz .convert_loop

.end_getmulti:
    pop eax                  ; EAX = base pointer of the new struct (Return value)

    pop edi
    pop esi
    pop ebx
    mov esp, ebp
    pop ebp
    ret

; Helper for getmulti
hex_char_to_val:
    cmp al, '9'
    jle .is_digit
    and al, 0xDF             ; Convert lowercase to uppercase
    sub al, 'A' - 10
    ret
.is_digit:
    sub al, '0'
    ret

; PART 2.A: Get MaxMin (Not CDECL)
; Inputs: EAX = ptr1, EBX = ptr2. Outputs: EAX = max_ptr, EBX = min_ptr
get_max_min:
    movzx ecx, byte [eax]    
    movzx edx, byte [ebx]    
    cmp ecx, edx
    jge .already_ordered     
    xchg eax, ebx            ; Swap if ptr2 is longer
.already_ordered:
    ret

; PART 2.B: Add Multi-precision Integers
add_multi:
    push ebp
    mov ebp, esp
    push ebx
    push esi
    push edi

    mov eax, [ebp + 8]
    mov ebx, [ebp + 12]
    call get_max_min         ; Now EAX = max, EBX = min
    
    mov edi, eax             ; EDI = max_ptr
    mov esi, ebx             ; ESI = min_ptr

    ; Calculate needed allocation size
    movzx eax, byte [edi]    ; eax = max_len
    cmp eax, 255
    je .set_alloc_size       
    add eax, 1               ; new_len = max_len + 1 (for final carry)
    
.set_alloc_size:
    mov edx, eax
    add edx, 1               ; Allocating space for the size byte as well

    push eax                 ; Preserve calculated new_len
    push edx
    call malloc
    add esp, 4
    
    mov ebx, eax             ; EBX = new struct pointer
    pop eax                  
    mov byte [ebx], al       ; Set size of new array

    ; CHALLENGE: Preserving the Carry Flag.
    ; Loop instructions (cmp, inc) overwrite CPU flags.
    ; Solution: We manually save the carry state in DL after each addition.
    mov ecx, 1               
    mov dl, 0                ; DL = Manual Carry Flag

.add_min_loop:
    movzx eax, byte [esi]
    cmp ecx, eax
    jg .end_add_min_loop

    mov al, byte [edi + ecx] 
    add al, dl               ; Add previous carry
    mov dl, 0                
    jnc .no_carry_1
    mov dl, 1                ; Save carry 
.no_carry_1:

    add al, byte [esi + ecx] ; Add min array byte
    jnc .no_carry_2
    mov dl, 1                ; Save carry
.no_carry_2:

    mov byte [ebx + ecx], al 
    inc ecx                  
    jmp .add_min_loop

.end_add_min_loop:

.add_max_loop:
    movzx eax, byte [edi]
    cmp ecx, eax
    jg .end_add_max_loop

    mov al, byte [edi + ecx]
    add al, dl               ; Add previous carry (no min array left)
    mov dl, 0                
    jnc .no_carry_3
    mov dl, 1
.no_carry_3:

    mov byte [ebx + ecx], al 
    inc ecx                  
    jmp .add_max_loop

.end_add_max_loop:

    ; Final carry placement
    movzx eax, byte [edi]    
    cmp eax, 255
    je .skip_final_carry
    mov byte [ebx + ecx], dl ; Place remaining carry at the end

.skip_final_carry:
    mov eax, ebx             ; Return new struct pointer
    pop edi
    pop esi
    pop ebx
    mov esp, ebp
    pop ebp
    ret

; PART 3.A: rand_num
; Generates a pseudo-random byte using 16-bit LFSR.
rand_num:
    push ebx
    push ecx
    push edx

    mov ecx, 8               ; Generate 8 bits for a byte
    xor ebx, ebx

.generate_bit:
    mov ax, word [STATE]
    mov dx, ax
    and dx, word [MASK]      ; Apply LFSR mask

    ; CHALLENGE: 16-bit Parity. x86 Parity Flag only works on lower 8 bits.
    ; Solution: XOR high byte and low byte into AL to get full 16-bit parity.
    mov al, dh
    xor al, dl          
    test al, al
    jpo .odd_parity     
    
.even_parity:
    clc                 
    jmp .shift_state

.odd_parity:
    stc                 

.shift_state:
    ; RCR pushes the Carry Flag (the parity bit) into the MSB
    mov ax, word [STATE]
    rcr ax, 1           
    mov word [STATE], ax

    ; The old LSB fell into the Carry Flag. Push it into our result byte.
    rcr bl, 1           

    dec ecx
    jnz .generate_bit

    mov al, bl               ; Return generated byte in AL
    pop edx
    pop ecx
    pop ebx
    ret

; PART 3.B: PRmulti
; Allocates and fills a struct with random bytes.
PRmulti:
    push ebp
    mov ebp, esp
    push ebx
    push esi
    push edi

.get_length:
    call rand_num       
    test al, al         
    jz .get_length           ; Length cannot be 0, try again

    movzx ebx, al            ; EBX = Array length (n)

    mov eax, ebx
    inc eax                  
    push eax
    call malloc              ; Allocate n+1 bytes
    add esp, 4
    
    mov edi, eax        
    mov byte [edi], bl       ; Set struct size
    mov ecx, 1          

.fill_array:
    cmp ecx, ebx
    jg .end_prmulti

    call rand_num            ; Generate random data byte
    mov byte [edi + ecx], al 
    
    inc ecx
    jmp .fill_array

.end_prmulti:
    mov eax, edi             ; Return pointer to random struct
    pop edi
    pop esi
    pop ebx
    mov esp, ebp
    pop ebp
    ret
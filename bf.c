#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX_MEMORY ((size_t)30000)

#define INC     '+'
#define DEC     '-'
#define PTR_INC '>'
#define PTR_DEC '<'
#define OP_LOOP '['
#define CL_LOOP ']'
#define OUT     '.'
#define IN      ','

typedef u_int8_t u8;
typedef size_t usize;

typedef struct Stack {
    usize* data;
    usize size;
} Stack;

void push(Stack* stack, usize value) {
    usize* tmp;
    if ( !stack || stack->size == 0 ) {
        tmp = malloc( sizeof(usize) );
    } else {
        tmp = realloc (
            stack->data,
            (stack->size + 1) * sizeof(usize)
        );
    }

    if (!tmp) {
        perror("No available memory\n");
        exit(EXIT_FAILURE);
    }

    stack->data = tmp;
    stack->size++;
    stack->data[stack->size - 1] = value;
}

void pop(Stack* stack) {
    if ( stack->size == 0 )
        return;

    stack->size--;

    if (stack->size == 0) {
        free(stack->data);
        stack->data = NULL;
        return;
    }

    usize *tmp = realloc(stack->data, stack->size * sizeof *stack->data);
    if (tmp)
        stack->data = tmp;
}

usize getCurrentLoopIdx(Stack* stack) {
    return stack->data[stack->size - 1];
}

typedef struct Memory {
    u8* bytes;
    usize size;
    usize ptr;
} Memory;

void allocateMem(Memory* memory) {
    u8* tmp = realloc ( 
        memory->bytes,
        (memory->size + 1) * sizeof(u8)
    );

    if ( !tmp ) { 
        perror("No available memory\n"); 
        exit(EXIT_FAILURE);
    }

    memory->size++;
    memory->bytes = tmp;
    memory->bytes[memory->size - 1] = 0;
}

u8 getCurrentMemory(Memory* memory) {
    return memory->bytes[memory->ptr];
}

typedef struct Program {
    char* instructions;
    usize length;
    usize ptr;
} Program;

bool E_O_F(Program* program) {
    return program->ptr == program->length;
}

char getCurrentInstruction(Program* program) {
    return program->instructions[program->ptr];
}

int main(int argc, char **argsv) {
    if ( argc < 2 ) {
        fprintf(stderr, "No file path provided\n");
        return 1;
    }

    FILE* file = fopen( argsv[1], "rb" );
    if ( !file ) { 
        perror("No such file directory\n"); 
        exit(EXIT_FAILURE);
    }

    fseek(file, 0, SEEK_END);
    usize file_size = (usize)ftell(file);
    rewind(file);

    Program program;
    program.ptr = 0;
    program.length = file_size;

    program.instructions = malloc( program.length * sizeof(char) );
    if ( !program.instructions ) { 
        perror("No available memory\n"); 
        exit(EXIT_FAILURE);
    }

    fread(program.instructions, 1, file_size, file);
    fclose(file);

    Memory memory;
    memory.ptr = 0;
    memory.size = 1;

    memory.bytes = malloc( sizeof(u8) );
    if ( !memory.bytes ) { 
        perror("No available memory\n"); 
        exit(EXIT_FAILURE);
    }
    memory.bytes[0] = 0;

    Stack loops_stack;
    loops_stack.data = NULL;
    loops_stack.size = 0;

    while ( !E_O_F(&program) ) {
        if ( memory.ptr == memory.size ) { allocateMem(&memory); }

        switch ( getCurrentInstruction(&program) ) {
        case INC:
            if ( getCurrentMemory(&memory) != 255 ) { memory.bytes[memory.ptr]++; }
            break;
        case DEC:
            if ( getCurrentMemory(&memory) != 0 ) { memory.bytes[memory.ptr]--; }
            break;
        case PTR_INC:
            if ( memory.ptr + 1 < MAX_MEMORY ) { memory.ptr++; }
            break;
        case PTR_DEC:
            if ( memory.ptr != 0 ) { memory.ptr--; }
            break;
        case OP_LOOP:
            if ( getCurrentMemory(&memory) != 0 ) {
                push(&loops_stack, program.ptr);
                break;
            }

            int num_of_new_loops = 0;
            program.ptr ++;
            while ( !E_O_F(&program) && getCurrentInstruction(&program) != CL_LOOP) {
                if ( getCurrentInstruction(&program) == OP_LOOP ) { num_of_new_loops++; }
                if ( 
                    program.ptr != program.length - 1 && 
                    program.instructions[program.ptr + 1] == CL_LOOP && 
                    num_of_new_loops != 0
                ) { program.ptr++; }
                program.ptr++;
            } 

            break;
        case CL_LOOP:
            if ( getCurrentMemory(&memory) != 0 ) {
                program.ptr = getCurrentLoopIdx(&loops_stack);
                break;
            }
            pop(&loops_stack);
            break;
        case OUT:
            printf("%c", getCurrentMemory(&memory));
            break;
        case IN:
            break;
        }

        program.ptr += 1;
    }

    free(memory.bytes);
    free(program.instructions);
    free(loops_stack.data);
}

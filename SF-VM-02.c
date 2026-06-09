#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

typedef int(*opcode_function_t)(unsigned char, unsigned char);

#define OPCODE_ADD  0
#define OPCODE_SUB  1
#define OPCODE_MUL  2
#define OPCODE_DIV  3
#define OPCODE_MOD  4
#define OPCODE_STP  5
#define OPCODE_LDI  6
#define OPCODE_ADR  7
#define OPCODE_SUR  8
#define OPCODE_INC  9
#define OPCODE_DEC  10
#define OPCODE_JMP  11
#define OPCODE_CMP  12

#define OPCODE_JE   13
#define OPCODE_JZ   13

#define OPCODE_JL   14
#define OPCODE_JG   15
#define OPCODE_JNE  16
#define OPCODE_JLE  17
#define OPCODE_JGE  18

#define OPCODE_LDM  19
#define OPCODE_STI  20
#define OPCODE_STR  21
#define OPCODE_ADM  22
#define OPCODE_SBM  23
#define OPCODE_MLM  24
#define OPCODE_MDM  25
#define OPCODE_DVM  26
#define OPCODE_NDM  27
#define OPCODE_ORM  28
#define OPCODE_XOM  29
#define OPCODE_SLM  30
#define OPCODE_SRM  31
#define OPCODE_NTM  32
#define OPCODE_PSH  33
#define OPCODE_POP  34
#define OPCODE_CAL  35
#define OPCODE_RET  36

#define DATA_SIZE             10
#define CODE_AMOUNT           9
#define STACK_SIZE            10
#define PROGRAM_SIZE          sizeof(memory)
#define INSTRUCTIONS_COUNT    37
#define INSTRUCTION_SIZE      3

#define LEFT_OPERAND  IP + 1
#define RIGHT_OPERAND IP + 2

#define RX_COUNT 4


static unsigned char memory [] = {

  /*Data segment*/
  /* 00 */  0,
  /* 01 */  0,
  /* 02 */  0,
  /* 03 */  0,
  /* 04 */  0,
  /* 05 */  0,
  /* 06 */  0,
  /* 07 */  0,
  /* 08 */  0,
  /* 09 */  0,
  /*Data segment*/

  /*Code segment*/
  /* 10 */  0,  9,   7,    /* ADD  9,   7   */
  /* 13 */  1,  8,   4,    /* SUB  8,   4   */
  /* 12 */  2,  2,   5,    /* MUL  2,   5   */
  /* 13 */  22, 5,   10,    /* MUL  2,   5   */
  /* 14 */  23, 5,   4,    /* MUL  2,   5   */
  /* 15 */  24, 5,   2,    /* MUL  2,   5   */
  /* 16 */  25, 5,   2,    /* MUL  2,   5   */
  /* 13 */  22, 5,   10,    /* MUL  2,   5   */
  /* 13 */  26, 5,   2,    /* MUL  2,   5   */
  /*Code segment*/

  /*Stack segment*/
  /* 01 */  0,
  /* 02 */  0,
  /* 03 */  0,
  /* 04 */  0,
  /* 05 */  0,
  /* 06 */  0,
  /* 07 */  0,
  /* 08 */  0,
  /* 09 */  0,
  /* 10 */  0,
  /*Stack segment*/

};



/* Registers */
static int SP = DATA_SIZE + (CODE_AMOUNT*INSTRUCTION_SIZE) + STACK_SIZE;
static int IP = DATA_SIZE;
static int LR = 0;
static unsigned char IR[INSTRUCTION_SIZE] = {0, 0, 0};
static int OUTPUT = 0;

static unsigned char FLAGS = 0;

#define FLAG_POSITIVE 0
#define FLAG_ZERO     1
#define FLAG_NEGATIVE 2

/* 0000 0000 */
/* xxxx xxNZ */

static unsigned char R0 = 0;
static unsigned char R1 = 0;
static unsigned char R2 = 0;
static unsigned char R3 = 0;


static opcode_function_t current_instruction = NULL;

int opcode_add(unsigned char left_operand, unsigned char right_operand){
    return left_operand + right_operand;
}

int opcode_sub(unsigned char left_operand, unsigned char right_operand){
    return left_operand - right_operand;
}

int opcode_mul(unsigned char left_operand, unsigned char right_operand){
    return left_operand * right_operand;
}

int opcode_div(unsigned char left_operand, unsigned char right_operand){
    if(right_operand == 0){
                    printf("Exception: divide by zero\n");
                    return false;
                }
    return left_operand / right_operand;
}

int opcode_mod(unsigned char left_operand, unsigned char right_operand){
    return left_operand % right_operand;
}

static unsigned char get_rx_value(unsigned char operand){
     switch(operand){
        case 0: return R0;
        case 1: return R1;
        case 2: return R2;
        case 3: return R3;
        // skip default case
    }

    return 255;
}

static void set_rx_value(unsigned char operand, unsigned char value){
     switch(operand){
        case 0: R0 = value; return;
        case 1: R1 = value; return;
        case 2: R2 = value; return;
        case 3: R3 = value; return;
        // skip default case
    }
}

int opcode_ldi(unsigned char left_operand, unsigned char right_operand){
    switch(left_operand){
        case 0: R0 = right_operand; break;
        case 1: R1 = right_operand; break;
        case 2: R2 = right_operand; break;
        case 3: R3 = right_operand; break;
        default:
            printf("Invalid register address\n");
            exit(0);
    }

    return right_operand;
}

int opcode_stp(unsigned char left_operand, unsigned char right_operand){
    OUTPUT = left_operand;
    exit(left_operand);
}

int opcode_adr(unsigned char left_operand, unsigned char right_operand){
    if(left_operand >= RX_COUNT){
        printf("Invlaid Rx register address\n");
        exit(0);
    }

    unsigned char Rx = get_rx_value(left_operand) + right_operand;
    set_rx_value(left_operand, Rx);
    return Rx;
}

int opcode_sur(unsigned char left_operand, unsigned char right_operand){
    if(left_operand >= RX_COUNT){
        printf("Invlaid Rx register address\n");
        exit(0);
    }

    unsigned char Rx = get_rx_value(left_operand) - right_operand;
    set_rx_value(left_operand, Rx);
    return Rx;
}


int opcode_inc(unsigned char left_operand, unsigned char right_operand){
    if(left_operand >= RX_COUNT){
        printf("Invlaid Rx register address\n");
        exit(0);
    }

    unsigned char Rx = get_rx_value(left_operand);
    Rx++;
    set_rx_value(left_operand, Rx);
    return Rx;
}

int opcode_dec(unsigned char left_operand, unsigned char right_operand){
    if(left_operand >= RX_COUNT){
        printf("Invlaid Rx register address\n");
        exit(0);
    }

    unsigned char Rx = get_rx_value(left_operand);
    Rx--;
    set_rx_value(left_operand, Rx);
    return Rx;
}

int opcode_jmp(unsigned char left_operand, unsigned char right_operand){
    IP = left_operand;
    return left_operand;
}

int opcode_cmp(unsigned char left_operand, unsigned char right_operand){
    unsigned char Rx = get_rx_value(left_operand);
    unsigned char Ry = get_rx_value(right_operand);

    OUTPUT = Rx - Ry;
    FLAGS = FLAG_POSITIVE;

    if(OUTPUT == 0){
        FLAGS = FLAG_ZERO;
    }else if (OUTPUT < 0){
        FLAGS = FLAG_NEGATIVE;
    }else if (OUTPUT > 0){
        FLAGS = FLAG_POSITIVE;
    }

    return FLAGS;
}

int opcode_je(unsigned char left_operand, unsigned char right_operand){

    if(FLAGS == FLAG_ZERO){
        IP = left_operand;
        return left_operand;
    }
    
    return 255;
}

int opcode_jne(unsigned char left_operand, unsigned char right_operand){

    if(FLAGS != FLAG_ZERO){
        IP = left_operand;
        return left_operand;
    }
    
    return 255;
}

int opcode_jl(unsigned char left_operand, unsigned char right_operand){

    if(FLAGS == FLAG_NEGATIVE){
        IP = left_operand;
        return left_operand;
    }
    
    return 255;
}

int opcode_jg(unsigned char left_operand, unsigned char right_operand){

    if(FLAGS == FLAG_POSITIVE){
        IP = left_operand;
        return left_operand;
    }
    
    return 255;
}

int opcode_jle(unsigned char left_operand, unsigned char right_operand){

    if( FLAGS == FLAG_NEGATIVE | FLAGS == FLAG_ZERO){
        IP = left_operand;
        return left_operand;
    }
    
    return 255;
}

int opcode_jge(unsigned char left_operand, unsigned char right_operand){

    if( FLAGS == FLAG_POSITIVE|| FLAGS == FLAG_ZERO){
        IP = left_operand;
        return left_operand;
    }
    
    return 255;
}

int opcode_ldm(unsigned char left_operand, unsigned char right_operand){
    if(right_operand >= DATA_SIZE){
        printf("error: this data doesn't exist\n");
        exit(0);
    }
    switch(left_operand){
        case 0: R0 = memory[right_operand]; break;
        case 1: R1 = memory[right_operand]; break;
        case 2: R2 = memory[right_operand]; break;
        case 3: R3 = memory[right_operand]; break;
        default:
            printf("Invalid register address\n");
            exit(0);
    }

    return right_operand;
}

int opcode_str(unsigned char left_operand, unsigned char right_operand){
    if(left_operand >= DATA_SIZE){
        printf("error: this data doesn't exist\n");
        exit(0);
    }
    if(right_operand >= RX_COUNT){
        printf("Invlaid Rx register address\n");
        exit(0);
    }

    unsigned char Rx = get_rx_value(right_operand);

    memory[left_operand] = Rx;

    return memory[left_operand];
}

int opcode_sti(unsigned char left_operand, unsigned char right_operand){
    if(left_operand >= DATA_SIZE){
        printf("error: this data doesn't exist\n");
        exit(0);
    }

    memory[left_operand] = right_operand;

    return right_operand;
}

int opcode_adm(unsigned char left_operand, unsigned char right_operand){
    if(left_operand >= DATA_SIZE){
        printf("error: this data doesn't exist\n");
        exit(0);
    }

    memory[left_operand] = memory[left_operand] + right_operand;

    return memory[left_operand];
}

int opcode_sbm(unsigned char left_operand, unsigned char right_operand){
    if(left_operand >= DATA_SIZE){
        printf("error: this data doesn't exist\n");
        exit(0);
    }

    memory[left_operand] = memory[left_operand] - right_operand;

    return memory[left_operand];
}

int opcode_mlm(unsigned char left_operand, unsigned char right_operand){
    if(left_operand >= DATA_SIZE){
        printf("error: this data doesn't exist\n");
        exit(0);
    }

    memory[left_operand] = memory[left_operand] * right_operand;

    return memory[left_operand];
}

int opcode_mdm(unsigned char left_operand, unsigned char right_operand){
    if(left_operand >= DATA_SIZE){
        printf("error: this data doesn't exist\n");
        exit(0);
    }

    memory[left_operand] = memory[left_operand] % right_operand;

    return memory[left_operand];
}

int opcode_dvm(unsigned char left_operand, unsigned char right_operand){
    if(left_operand >= DATA_SIZE){
        printf("error: this data doesn't exist\n");
        exit(0);
    }
    if(right_operand == 0){
        printf("Exception: divide by zero\n");
        return false;
    }

    memory[left_operand] = memory[left_operand] / right_operand;

    return memory[left_operand];
}

int opcode_ndm(unsigned char left_operand, unsigned char right_operand){
    if(left_operand >= DATA_SIZE){
        printf("error: this data doesn't exist\n");
        exit(0);
    }

    memory[left_operand] = memory[left_operand] & right_operand;

    return memory[left_operand];
}

int opcode_orm(unsigned char left_operand, unsigned char right_operand){
    if(left_operand >= DATA_SIZE){
        printf("error: this data doesn't exist\n");
        exit(0);
    }

    memory[left_operand] = memory[left_operand] | right_operand;

    return memory[left_operand];
}

int opcode_xom(unsigned char left_operand, unsigned char right_operand){
    if(left_operand >= DATA_SIZE){
        printf("error: this data doesn't exist\n");
        exit(0);
    }

    memory[left_operand] = memory[left_operand] ^ right_operand;

    return memory[left_operand];
}

int opcode_slm(unsigned char left_operand, unsigned char right_operand){
    if(left_operand >= DATA_SIZE){
        printf("error: this data doesn't exist\n");
        exit(0);
    }

    memory[left_operand] = memory[left_operand] << right_operand;

    return memory[left_operand];
}

int opcode_srm(unsigned char left_operand, unsigned char right_operand){
    if(left_operand >= DATA_SIZE){
        printf("error: this data doesn't exist\n");
        exit(0);
    }

    memory[left_operand] = memory[left_operand] >> right_operand;

    return memory[left_operand];
}

int opcode_ntm(unsigned char left_operand, unsigned char right_operand){
    if(left_operand >= DATA_SIZE){
        printf("error: this data doesn't exist\n");
        exit(0);
    }

    memory[left_operand] = ~memory[left_operand];

    return memory[left_operand];
}

int opcode_psh(unsigned char left_operand, unsigned char right_operand){
    if(SP <= DATA_SIZE + (CODE_AMOUNT * 3)){
        printf("erorr\n");
         exit(0);
    }
    memory[SP] = left_operand;

    SP = SP - 1;

    return memory[SP + 1];
}

int opcode_pop(unsigned char left_operand, unsigned char right_operand){
    if(SP > DATA_SIZE + (CODE_AMOUNT*INSTRUCTION_SIZE) + STACK_SIZE){
        printf("erorr\n");
         exit(0);
    }
    SP = SP + 1;

    return memory[SP];
}

int opcode_cal(unsigned char left_operand, unsigned char right_operand){
    LR = IP;
    IP = left_operand;
    return left_operand;
}

int opcode_ret(unsigned char left_operand, unsigned char right_operand){
    IP = LR;
    return IP;
}

static const opcode_function_t opcode_functions[INSTRUCTIONS_COUNT] = {
        opcode_add, opcode_sub, opcode_mul,
        opcode_div, opcode_mod, opcode_stp,
        opcode_ldi, opcode_adr, opcode_sur,
        opcode_inc, opcode_dec, opcode_jmp,
        opcode_cmp, opcode_je , opcode_jl ,
        opcode_jg , opcode_jne, opcode_jle,
        opcode_jge, opcode_ldm, opcode_sti, 
        opcode_str, opcode_adm, opcode_sbm, 
        opcode_mlm, opcode_mdm, opcode_dvm,
        opcode_ndm, opcode_orm, opcode_xom,
        opcode_slm, opcode_srm, opcode_ntm,
        opcode_psh, opcode_pop, opcode_cal,
        opcode_ret,
};


static bool cpu_fetch(void){
    int byte = 0;
    if(IP >= PROGRAM_SIZE){
        /* HALT */
        exit(0);
    }

    /* Fetch */
    IR[0] = memory[IP];
    IR[1] = memory[LEFT_OPERAND];
    IR[2] = memory[RIGHT_OPERAND];

    /* Move to the next insturction */
    IP += INSTRUCTION_SIZE;
    
    return true;
}

static bool cpu_decode(void){
    if(IR[0] >= INSTRUCTIONS_COUNT){
        printf("Invalid instruction\n");
        // IR[0] = 10; // rewrite
        IR[0] = IR[1] = IR[2] = 0;
        exit(0);
    }

    current_instruction = opcode_functions[IR[0]];

    return true;
}

static bool cpu_execute(void){
    OUTPUT = current_instruction(IR[1], IR[2]);
    return true;
}

int main(void){
    // reset
    IP = DATA_SIZE;
    IR[0] = IR[1] = IR[2] = 0;

    while(true){
        cpu_fetch();
        cpu_decode();
        cpu_execute();
        printf("output -> %d\n", OUTPUT);
    }

    return 0;
}
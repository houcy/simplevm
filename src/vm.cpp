/*******************************************************************************
 * filename    : vm.c
 * author      : Zac Brown <zac@zacbrown.org>
 * date        : 11.17.2009
 * description : vm main class
 ******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <math.h>
#include "include/vm.h"
#include "include/stack.h"

using namespace std;

static simplevm_stack_t _data_stack;
static simplevm_stack_p data_stack = &_data_stack;
static vector<operation_p> lines;
static vector<label_p> jump_list;

#define PUSH(data) simplevm_stack_push(data_stack, data)
#define PEEK() simplevm_stack_peek(data_stack)
#define POP() simplevm_stack_pop(data_stack)
#define STACK_SIZE() simplevm_stack_size(data_stack)

int main(int argc, char* args[])
{
    if (argc < 2) { usage(); return -2; }
    if (simplevm_stack_init(data_stack) == -1)
    {
        printf("ERROR: simplevm_stack_init failed\n");
        return -1;
    }

    parse_file(args[1]);
    execute_goto();

    return 0;
}

void execute_goto(void)
{
    int lines_len = lines.size();
    for (int i = 0; i < lines_len; i++)
    {
        operation_p cur_op = lines[i];

        if (strcmp(cur_op->instr_str, "IADD") == 0) { cur_op->instr = &&iadd; }
        if (strcmp(cur_op->instr_str, "FADD") == 0) { cur_op->instr = &&fadd; }
        if (strcmp(cur_op->instr_str, "ISUB") == 0) { cur_op->instr = &&isub; }
        if (strcmp(cur_op->instr_str, "FSUB") == 0) { cur_op->instr = &&fsub; }
        if (strcmp(cur_op->instr_str, "IMUL") == 0) { cur_op->instr = &&imul; }
        if (strcmp(cur_op->instr_str, "FMUL") == 0) { cur_op->instr = &&fmul; }
        if (strcmp(cur_op->instr_str, "IDIV") == 0) { cur_op->instr = &&idiv; }
        if (strcmp(cur_op->instr_str, "FDIV") == 0) { cur_op->instr = &&fdiv; }
        if (strcmp(cur_op->instr_str, "MOD") == 0) { cur_op->instr = &&mod; }
        if (strcmp(cur_op->instr_str, "IPOW") == 0) { cur_op->instr = &&ipow; }
        if (strcmp(cur_op->instr_str, "FPOW") == 0) { cur_op->instr = &&fpow; }
        if (strcmp(cur_op->instr_str, "I2F") == 0) { cur_op->instr = &&i2f; }
        if (strcmp(cur_op->instr_str, "F2I") == 0) { cur_op->instr = &&f2i; }
        if (strcmp(cur_op->instr_str, "INEG") == 0) { cur_op->instr = &&ineg; }
        if (strcmp(cur_op->instr_str, "FNEG") == 0) { cur_op->instr = &&fneg; }
        if (strcmp(cur_op->instr_str, "ICMP") == 0) { cur_op->instr = &&icmp; }
        if (strcmp(cur_op->instr_str, "FCMP") == 0) { cur_op->instr = &&fcmp; }
        if (strcmp(cur_op->instr_str, "CCMP") == 0) { cur_op->instr = &&ccmp; }
        if (strcmp(cur_op->instr_str, "SCMP") == 0) { cur_op->instr = &&scmp; }
        if (strcmp(cur_op->instr_str, "JMP") == 0) { cur_op->instr = &&jmp; }
        if (strcmp(cur_op->instr_str, "JMPZ") == 0) { cur_op->instr = &&jmpz; }
        if (strcmp(cur_op->instr_str, "IPUSH") == 0) { cur_op->instr = &&ipush; }
        if (strcmp(cur_op->instr_str, "FPUSH") == 0) { cur_op->instr = &&fpush; }
        if (strcmp(cur_op->instr_str, "CPUSH") == 0) { cur_op->instr = &&cpush; }
        if (strcmp(cur_op->instr_str, "SPUSH") == 0) { cur_op->instr = &&spush; }
        if (strcmp(cur_op->instr_str, "DROP") == 0) { cur_op->instr = &&drop; }
        if (strcmp(cur_op->instr_str, "DUP") == 0) { cur_op->instr = &&dup; }
        if (strcmp(cur_op->instr_str, "DUP2") == 0) { cur_op->instr = &&dup2; }
        if (strcmp(cur_op->instr_str, "SWAP") == 0) { cur_op->instr = &&swap; }
        if (strcmp(cur_op->instr_str, "ILOAD") == 0) { cur_op->instr = &&iload; }
        if (strcmp(cur_op->instr_str, "FLOAD") == 0) { cur_op->instr = &&fload; }
        if (strcmp(cur_op->instr_str, "CLOAD") == 0) { cur_op->instr = &&cload; }
        if (strcmp(cur_op->instr_str, "SLOAD") == 0) { cur_op->instr = &&sload; }
        if (strcmp(cur_op->instr_str, "ISTOR") == 0) { cur_op->instr = &&istor; }
        if (strcmp(cur_op->instr_str, "FSTOR") == 0) { cur_op->instr = &&fstor; }
        if (strcmp(cur_op->instr_str, "CSTOR") == 0) { cur_op->instr = &&cstor; }
        if (strcmp(cur_op->instr_str, "SSTOR") == 0) { cur_op->instr = &&sstor; }
        if (strcmp(cur_op->instr_str, "HALT") == 0) { cur_op->instr = &&halt; }
        if (strcmp(cur_op->instr_str, "PRINT") == 0) { cur_op->instr = &&print; }
        if (strcmp(cur_op->instr_str, "DUMP") == 0) { cur_op->instr = &&dump; }
    }

    stack_element_p elem, e1, e2, res;
    float temp_f;
    int  temp_i;
    unsigned int cur_line = 0;
    operation_p cur_op;
  top_exec:
    if (cur_line >= lines.size()) exit(0);
    cur_op = lines[cur_line++];
    goto *cur_op->instr;

  iadd:
    e1 = POP();
    e2 = POP();
    res = (stack_element_p) malloc(sizeof(stack_element_t));
    res->data.dat_i = e1->data.dat_i + e2->data.dat_i;
    res->type = 0x00001;
    free(e1); free(e2);
    PUSH(res);
    goto top_exec;

  fadd:
    e1 = POP();
    e2 = POP();
    res = (stack_element_p) malloc(sizeof(stack_element_t));
    res->data.dat_f = e1->data.dat_f + e2->data.dat_f;
    res->type = 0x00010;
    free(e1); free(e2);
    PUSH(res);
    goto top_exec;

  isub:
    e1 = POP();
    e2 = POP();
    res = (stack_element_p) malloc(sizeof(stack_element_t));
    res->data.dat_i = e2->data.dat_i - e1->data.dat_i;
    res->type = 0x00001;
    free(e1); free(e2);
    PUSH(res);
    goto top_exec;

  fsub:
    e1 = POP();
    e2 = POP();
    res = (stack_element_p) malloc(sizeof(stack_element_t));
    res->data.dat_f = e2->data.dat_f - e1->data.dat_f;
    res->type = 0x00010;
    free(e1); free(e2);
    PUSH(res);
    goto top_exec;

  imul:
    e1 = POP();
    e2 = POP();
    res = (stack_element_p) malloc(sizeof(stack_element_t));
    res->data.dat_i = e1->data.dat_i * e2->data.dat_i;
    res->type = 0x00001;
    free(e1); free(e2);
    PUSH(res);
    goto top_exec;

  fmul:
    e1 = POP();
    e2 = POP();
    res = (stack_element_p) malloc(sizeof(stack_element_t));
    res->data.dat_f = e1->data.dat_f * e2->data.dat_f;
    res->type = 0x00010;
    free(e1); free(e2);
    PUSH(res);
    goto top_exec;

  idiv:
    e1 = POP();
    e2 = POP();
    res = (stack_element_p) malloc(sizeof(stack_element_t));
    res->data.dat_i = e2->data.dat_i / e1->data.dat_i;
    res->type = 0x00001;
    free(e1); free(e2);
    PUSH(res);
    goto top_exec;

  fdiv:
    e1 = POP();
    e2 = POP();
    res = (stack_element_p) malloc(sizeof(stack_element_t));
    res->data.dat_f = e2->data.dat_f / e1->data.dat_f;
    res->type = 0x00010;
    free(e1); free(e2);
    PUSH(res);
    goto top_exec;

  mod:
    e1 = POP();
    e2 = POP();
    res = (stack_element_p) malloc(sizeof(stack_element_t));
    res->data.dat_i = e2->data.dat_i % e1->data.dat_i;
    res->type = 0x00001;
    free(e1); free(e2);
    PUSH(res);
    goto top_exec;

  ipow:
    e1 = POP(); /* exponent */
    e2 = POP(); /* base */
    elem = (stack_element_p) malloc(sizeof(stack_element_t));
    elem->type = 0x00001;
    elem->data.dat_i = (int) pow(e2->data.dat_i, e1->data.dat_i);
    free(e1); free(e2);
    PUSH(elem);
    goto top_exec;

  fpow:
    e1 = POP(); /* exponent */
    e2 = POP(); /* base */
    elem = (stack_element_p) malloc(sizeof(stack_element_t));
    elem->type = 0x00010;
    elem->data.dat_f = (float) pow(e2->data.dat_f, e1->data.dat_f);
    free(e1); free(e2);
    PUSH(elem);
    goto top_exec;

  i2f:
    elem = PEEK();
    temp_f = (float) elem->data.dat_i;
    memset(&elem->data, 0, sizeof(data_t));
    elem->data.dat_f = temp_f;
    elem->type = 0x00010;
    goto top_exec;

  f2i:
    elem = PEEK();
    temp_i = (int) elem->data.dat_f;
    memset(&elem->data, 0, sizeof(data_t));
    elem->data.dat_i = temp_i;
    elem->type = 0x00001;
    goto top_exec;

  ineg:
    elem = PEEK();
    elem->data.dat_i *= -1;
    goto top_exec;

  fneg:
    elem = PEEK();
    elem->data.dat_f *= -1.0;
    goto top_exec;

  icmp:
    e1 = POP();
    e2 = POP();
    res = (stack_element_p) malloc(sizeof(stack_element_t));
    res->type = 0x00001;
    if (e1->data.dat_i < e2->data.dat_i) res->data.dat_i = -1;
    else if (e1->data.dat_i == e2->data.dat_i) res->data.dat_i = 0;
    else res->data.dat_i = 1;
    free(e1); free(e2);
    PUSH(res);
    goto top_exec;

  fcmp:
    e1 = POP();
    e2 = POP();
    res = (stack_element_p) malloc(sizeof(stack_element_t));
    res->type = 0x00001;
    if (e1->data.dat_f < e2->data.dat_f) res->data.dat_i = -1;
    else if (e1->data.dat_f == e2->data.dat_f) res->data.dat_i = 0;
    else res->data.dat_i = 1;
    free(e1); free(e2);
    PUSH(res);
    goto top_exec;

  ccmp:
    e1 = POP();
    e2 = POP();
    res = (stack_element_p) malloc(sizeof(stack_element_t));
    res->type = 0x00001;
    if (e1->data.dat_c < e2->data.dat_c) res->data.dat_i = -1;
    else if (e1->data.dat_c == e2->data.dat_c) res->data.dat_i = 0;
    else res->data.dat_i = 1;
    free(e1); free(e2);
    PUSH(res);
    goto top_exec;

  scmp:
    e1 = POP();
    e2 = POP();
    res = (stack_element_p) malloc(sizeof(stack_element_t));
    res->type = 0x00001;
    res->data.dat_i = strcmp(e1->data.dat_s, e2->data.dat_s);
    free(e1); free(e2);
    PUSH(res);
    goto top_exec;

  jmp:
    cur_line = get_label_loc(cur_op->data.dat_s);
    goto top_exec;

  jmpz:
    elem = POP();
    if (elem->data.dat_i == 0) cur_line = get_label_loc(cur_op->data.dat_s);
    free(elem);
    goto top_exec;

  ipush:
    elem = (stack_element_p) malloc(sizeof(stack_element_t));
    elem->type = 0x00001;
    elem->data.dat_i = cur_op->data.dat_i;
    PUSH(elem);
    goto top_exec;

  fpush:
    elem = (stack_element_p) malloc(sizeof(stack_element_t));
    elem->type = 0x00010;
    elem->data.dat_f = cur_op->data.dat_f;
    PUSH(elem);
    goto top_exec;

  cpush:
    elem = (stack_element_p) malloc(sizeof(stack_element_t));
    elem->type = 0x00100;
    elem->data.dat_c = cur_op->data.dat_c;
    PUSH(elem);
    goto top_exec;

  spush:
    elem = (stack_element_p) malloc(sizeof(stack_element_t));
    elem->type = 0x01000;
    strcpy(elem->data.dat_s, cur_op->data.dat_s);
    PUSH(elem);
    goto top_exec;

  drop:
    free(POP());
    goto top_exec;

  dup:
    elem = (stack_element_p) malloc(sizeof(stack_element_t));
    memcpy(elem, PEEK(), sizeof(stack_element_t));
    PUSH(elem);
    goto top_exec;

  dup2:
    elem = POP();
    e1 = PEEK();
    e2 = (stack_element_p) malloc(sizeof(stack_element_t));
    memcpy(e2, e1, sizeof(stack_element_t));
    PUSH(elem);
    PUSH(e2);
    e1 = (stack_element_p) malloc(sizeof(stack_element_t));
    memcpy(e1, elem, sizeof(stack_element_t));
    PUSH(e1);
    goto top_exec;

  swap:
    e1 = POP();
    e2 = POP();
    PUSH(e1);
    PUSH(e2);
    goto top_exec;

  iload:
  fload:
  cload:
  sload:
  istor:
  fstor:
  cstor:
  sstor:
  halt:
    elem = POP();
    exit(elem->data.dat_i);

  print:
  dump:
    printf("--- data stack:\n");
    stack_element_p ptr = PEEK();
    while(ptr != data_stack->base)
    {
        if (ptr->type == 0x00001) printf("int:%d\n", ptr->data.dat_i);
        else if (ptr->type == 0x00010) printf("float:%f\n", ptr->data.dat_f);
        else if (ptr->type == 0x00100) printf("char:%c\n", ptr->data.dat_c);
        else if (ptr->type == 0x01000) printf("string\"%s\"\n", ptr->data.dat_s);
        else printf("unknown\n");
        ptr = ptr->next;
    }
    printf("\n");
    goto top_exec;
}

int parse_file(char* filename)
{
    ifstream source_file_h(filename);

    if (source_file_h.fail())
    {
        printf("ERROR: file \"%s\" not found, quitting.\n", filename);
        return -1;
    }

    while (!source_file_h.eof())
    {
        char line[MAX_OP_LEN];
        operation_p op_obj = (operation_p)malloc(sizeof(operation_t));

        CHECK_NULL(op_obj,
            "ERROR:parse_file(%d): malloc failed\n", __LINE__, -1);

        string op, val;
        source_file_h.getline(line, MAX_OP_LEN);
        string str_line(line);
        stringstream line_stream(str_line);
        line_stream >> op; line_stream >> val;

        /* comment line */
        if (op.c_str()[0] == ';' || op.compare("") == 0) { continue; }

        /* label declaration line */
        int op_str_len = strlen(op.c_str());
        if (op.c_str()[op_str_len - 1] == ':')
        {
            char label[op_str_len - 1];
            strncpy(label, op.c_str(), op_str_len - 1);
            label_p new_label = (label_p) malloc(sizeof(label_t));
            new_label->label = (char*) malloc((op_str_len - 1) * sizeof(char));
            new_label->line = lines.size();
            strcpy(new_label->label, label);
            jump_list.push_back(new_label);
            continue;
        }

        /* op instruction line */
        for (int op_test = IADD; op_test != LAST_NOOP_INSTR; op_test++)
        {
            if (strcmp(op.c_str(), instr_strings[op_test]) == 0)
            {
                strcpy(op_obj->instr_str, instr_strings[op_test]);
                if (strcmp(val.c_str(), "") != 0)
                {
                    stringstream val_stream(val);
                    string s;
                    switch (instr_strings[op_test][0])
                    {
                        case 'I':
                            val_stream >> op_obj->data.dat_i;
                            break;
                        case 'F':
                            val_stream >> op_obj->data.dat_f;
                            break;
                        case 'C':
                            val_stream >> op_obj->data.dat_c;
                            break;
                        case 'J':
                        case 'S':
                            val_stream >> s;
                            strncpy(op_obj->data.dat_s, s.c_str(), MAX_STR_LEN);
                            break;
                        default:
                            printf("WARNING: malformed data \"%s\"\n",
                                val.c_str());
                    }
                }
            }
        }

        lines.push_back(op_obj);
    }

    return 0;
}

int get_label_loc(char* label)
{
    int jump_list_len = jump_list.size();
    for (int i = 0; i < jump_list_len; i++)
    {
        label_p cur_label = jump_list[i];
        if (strcmp(cur_label->label, label) == 0)
        {
            return cur_label->line;
        }
    }

    return -1;
}

void usage(void)
{
    printf("./simplevm <assembly file>\n");
}

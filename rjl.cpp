// [DONE] needs a linear form so that programs can be loaded from files
// - needs a foreign function interface so that C libraries can be wrapped
// [DONE] needs some unit tests
// - needs garbage collection, currently there's no garbage collection
// [DONE] implement assignment
// [DONE] implement exceptions
// [DONE] implement prototype lookup in objects
// - implement slot lookup and assignment semantics for self and dynamic scope
// - test exceptions
// - write buitin functions for fixnums
// - write a loop test
//
// - test the symbol table first, it doesn't work

#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

typedef size_t Fixnum;

struct Object;
struct Buffer;

struct ObjectPair {
    Object *key;
    Object *value;
};

struct Object {
    Fixnum      length;
    Fixnum      occupied;
    Object     *proto;
    ObjectPair *table;
    Buffer     *buffer;
};

struct Buffer {
    Fixnum      length;
    char        data[0];
};

struct ArrayBuffer {
    Fixnum  length_bytes;
    Fixnum  tail;
    Object* data[0];
};

#define INST_POP      0x1
#define INST_SEND     0x2
#define INST_CALL     0x3
#define INST_PUSH     0x4
#define INST_JMP      0x5
#define INST_RET      0x6
#define INST_TERM     0x7
#define INST_DUP      0x8
#define INST_ASSIGN   0x9
#define INST_RAISE    0x10
#define INST_NEW      0x11

Object* new_object(Object *proto = 0);
Object* new_string(char const *str, int length = -1);
Object* new_block();
Object* new_array();
Object* new_symbol_table();
ArrayBuffer* allocate_array(Fixnum length);
Object* sym(char const *str, int length = -1);
Object* new_instr_table();

Object* Array   = new_object();
Object* String  = new_object();
Object* Integer = new_object();
Object* Block   = new_object();

Object* SymbolTable = new_symbol_table();
// the value of slot 1 is the count of symbols in the table
// otherwise it just stores strings

Object* Instr   = new_instr_table();
// do not init until after symbol table because this uses the symbol table

Object* SYM_STACK   = sym("stack");
Object* SYM_PC      = sym("pc");
Object* SYM_CODE    = sym("code");
Object* SYM_BLOCK   = sym("block");
Object* SYM_CLOSURE = sym("closure");
Object* SYM_SELF    = sym("self");
Object* SYM_RET     = sym("ret");
Object* SYM_CATCH   = sym("catch");
Object* SYM_FRAME   = sym("frame");
Object* SYM_PARENT  = sym("parent");
Object* SYM_ARG     = sym("arg");

Object* SYM_LEXICAL_FRAME = sym("lexical_frame");
Object* SYM_PARENT_FRAME = sym("parent_frame");
Object* SYM_LOCAL = sym("local");

bool is_fixnum(Object *obj) {
    return ((Fixnum)obj & 0x1) == 0x1;
}

bool is_sym(Object *obj) {
    return ((Fixnum)obj & 0x2) == 0x2;
}

bool is_object(Object *obj) {
    return obj != 0 && ((Fixnum)obj & 0x3) == 0;
}

bool is_nil(Object *obj) {
    return obj == 0;
}

Fixnum fixnum(Object *obj) {
    Fixnum sign_bit = (Fixnum)obj & (0x1L << ((sizeof(Fixnum) * 8) - 1));
    return ((Fixnum)obj >> 2) | sign_bit;
}

Object *object(Fixnum fixnum) {
    Fixnum sign_bit = (Fixnum)fixnum & (0x1L << ((sizeof(Fixnum) * 8) - 1));
    return (Object *)(((Fixnum)fixnum << 2) | sign_bit | 0x1);
}

Object *sym_object(Fixnum fixnum) {
    Fixnum sign_bit = (Fixnum)fixnum & (0x1L << ((sizeof(Fixnum) * 8) - 1));
    return (Object *)(((Fixnum)fixnum << 2) | sign_bit | 0x2);
}

Fixnum sym_fixnum(Object *obj) {
    Fixnum sign_bit = (Fixnum)obj & (0x1L << ((sizeof(Fixnum) * 8) - 1));
    return ((Fixnum)obj >> 2) | sign_bit;
}

bool fixnum_equals(Object *s, Object *t) {
    return is_fixnum(s) && is_fixnum(t) && s == t;
}

bool sym_fixnum_equals(Object *s, Object *t) {
    return is_sym(s) && is_sym(t) && s == t;
}

bool is_string(Object *s) {
    return is_object(s) && s->proto == String;
}

bool has_buffer(Object *s) {
    return is_object(s) && s->buffer != 0;
}

bool string_equals(Object *s, Object *t) {
    return is_object(s) && is_object(t) && has_buffer(s) && has_buffer(t) &&
        ( s->buffer->length == t->buffer->length ) &&
        ( memcmp(s->buffer->data, t->buffer->data, s->buffer->length) == 0 );
}

Fixnum string_hash(Object *s) {
    if ( ! has_buffer(s) ) {
        return 0;
    }
    else {
        Fixnum result  = 0;
        Fixnum length  = s->buffer->length / sizeof(result);
        Fixnum residue = s->buffer->length % sizeof(result);
        Fixnum *buf    = (Fixnum *)s->buffer->data;
        Fixnum i, j;
        for(i=0; i<length; ++i) {
            result ^= buf[i];
        }
        for(j=0; j<residue; ++j) {
            result ^= ((Fixnum)s->buffer->data[i * sizeof(result)]) << (8 * j);
        }
    }
}

void set_proto(Object *obj, Object *proto) {
    obj->proto = proto;
}

void *allocate(Fixnum size) {
    void *ptr = malloc(size);
    memset(ptr, 0, size);
}

void deallocate(void *ptr) {
    free(ptr);
}

Object* new_object(Object *proto) {
    Object *obj   = (Object*)allocate(sizeof(Object));
    obj->length   = 2;
    obj->occupied = 0;
    obj->proto    = proto;
    obj->table    = (ObjectPair*)allocate(sizeof(ObjectPair)*obj->length);
    obj->buffer   = 0;
    return obj;
};

void grow_object_table(Object *obj);

// only stores primitive keys based on fixnum or buffer
void set_slot(Object *obj, Object *key, Object *value) {
    Object *candidate;
    Fixnum index;

    if ( is_fixnum(key) ) {
        index = fixnum(key);
        candidate = obj->table[index % obj->length].key;
        while( candidate != 0 && ! fixnum_equals(candidate, key) ) {
            index += 1;
            candidate = obj->table[index % obj->length].key;
        }
    }
    if ( is_sym(key) ) {
        index = sym_fixnum(key);
        candidate = obj->table[index % obj->length].key;
        while( candidate != 0 && ! sym_fixnum_equals(candidate, key) ) {
            index += 1;
            candidate = obj->table[index % obj->length].key;
        }
    }
    else if ( is_string(key) ) {
        index = string_hash(key);
        candidate  = obj->table[index % obj->length].key;
        while( candidate != 0 && ! string_equals(candidate, key) ) {
            index += 1;
            candidate = obj->table[index % obj->length].key;
        }
    }
    else {
        // other types are not supported presently
        return;
    }

    obj->table[index % obj->length].value = value;
    if ( candidate == 0 ) {
        obj->table[index % obj->length].key = key;
        obj->occupied++;
        if ( obj->occupied * 4 > obj->length * 3 ) {
            grow_object_table(obj);
        }
    }
}

Object* get_slot(Object *obj, Object *key) 
{
    Object *candidate = 0;
    Fixnum index;

    if ( is_fixnum(key) ) {
        index = fixnum(key);
        candidate = obj->table[index % obj->length].key;
        while( candidate != 0 && ! fixnum_equals(candidate, key) ) {
            index += 1;
            candidate = obj->table[index % obj->length].key;
        }
    }
    if ( is_sym(key) ) {
        index = sym_fixnum(key);
        candidate = obj->table[index % obj->length].key;
        while( candidate != 0 && ! sym_fixnum_equals(candidate, key) ) {
            index += 1;
            candidate = obj->table[index % obj->length].key;
        }
    }
    else if ( is_string(key) ) {
        index = string_hash(key);
        candidate  = obj->table[index % obj->length].key;
        while( candidate != 0 && ! string_equals(candidate, key) ) {
            index += 1;
            candidate = obj->table[index % obj->length].key;
        }
    }
    else {
        // other types are not supported presently
    }
    if ( candidate != 0 ) {
        return obj->table[index % obj->length].value;
    }
    // perform lookup in parent object
    return is_object(obj) && is_object(obj->proto) ? get_slot(obj->proto, key) : 0;
}

void grow_object_table(Object *obj)
{
    Object new_obj;
    memcpy(&new_obj, obj, sizeof(*obj));
    new_obj.length *= 2;
    new_obj.table = (ObjectPair*)allocate(sizeof(ObjectPair)*new_obj.length);
    for(Fixnum i = 0; i<obj->length; ++i) {
        ObjectPair *curr = obj->table + i;
        if ( curr->key != 0 ) {
            set_slot(&new_obj, curr->key, curr->value);
        }
    }
    deallocate(obj->table);
    obj->length = new_obj.length;
    obj->table  = new_obj.table;
}

Object* sym(char const *str, int length) {
    Object *key   = new_string(str, length);
    Object *value = get_slot(SymbolTable, key);
    if ( value == 0 ) {
        Fixnum index = 
            sym_fixnum(get_slot(SymbolTable, sym_object(1)))+1;
        value = sym_object(index);
        set_slot(SymbolTable, sym_object(1), value);
        set_slot(SymbolTable, key, value);
    }
    return value;
}

Object* new_symbol_table() {
    Object *obj = new_object();
    set_slot(obj, sym_object(1), sym_object(2));
    return obj;
}

Object *new_instr_table() {
    Object *instr_table = new_object();
    set_slot(instr_table, sym("pop"),    sym_object(INST_POP));
    set_slot(instr_table, sym("send"),   sym_object(INST_SEND));
    set_slot(instr_table, sym("call"),   sym_object(INST_CALL));
    set_slot(instr_table, sym("push"),   sym_object(INST_PUSH));
    set_slot(instr_table, sym("jmp"),    sym_object(INST_JMP));
    set_slot(instr_table, sym("ret"),    sym_object(INST_RET));
    set_slot(instr_table, sym("term"),   sym_object(INST_TERM));
    set_slot(instr_table, sym("dup"),    sym_object(INST_DUP));
    set_slot(instr_table, sym("assign"), sym_object(INST_ASSIGN));
    set_slot(instr_table, sym("raise"),  sym_object(INST_RAISE));
    set_slot(instr_table, sym("new"),    sym_object(INST_NEW));
    return instr_table;
};

Buffer* new_buffer(Fixnum length, char const* data = 0) {
    Buffer *buffer = (Buffer *)allocate(sizeof(Buffer)+length);
    buffer->length = length;
    if ( data != 0 ) {
        memcpy(buffer->data, data, length);
    }
    return buffer;
}

Object* new_string(char const *str, int length) {
    Object *obj = new_object(String);
    if ( length == -1 ) {
        length = strlen(str);
    }
    obj->buffer = new_buffer(length, str);
    return obj;
}

Object* new_array() {
    Object *obj = new_object(Array);
    obj->buffer = (Buffer *)allocate_array(4);
    return obj;
}

Object* new_block() {
    Object *obj = new_array();
    set_proto(obj, Block);
    return obj;
}

Object* get_code(Object *frame) {
    return frame;
        
}

bool is_array(Object *obj) {
    return is_object(obj) && ( obj->proto == Array || obj->proto == Block );
}

bool is_block(Object *obj) {
    return is_object(obj) && ( obj->proto == Block );
}

ArrayBuffer* allocate_array(Fixnum length)
{
    Fixnum length_bytes = sizeof(Fixnum)+(length*sizeof(Object *));
    ArrayBuffer *buffer = (ArrayBuffer *)allocate(length_bytes+sizeof(Buffer));
    buffer->length_bytes = length_bytes;
    buffer->tail = 0;
    return buffer;
};

ArrayBuffer* resize_array(Object *obj) {
    ArrayBuffer* old_array = (ArrayBuffer *)obj->buffer;
    ArrayBuffer* new_array = allocate_array((old_array->tail)*2);
    memcpy(new_array->data, old_array->data, 
        old_array->length_bytes - sizeof(old_array->tail));
    new_array->tail = old_array->tail;
    deallocate(obj->buffer);
    obj->buffer = (Buffer *)new_array;
    return new_array;
}

Fixnum array_length(Object *obj) 
{
    if ( is_array(obj) ) {
        ArrayBuffer *array = (ArrayBuffer *)obj->buffer;
        return array->tail;
    }
    return 0;
}

Object *array_parent(Object *obj) {
    while( ! is_array(obj) && is_object(obj) ) {
        obj = obj->proto;
    }
    return obj;
}

Fixnum array_capacity(Object *obj)
{
    if ( is_array(obj) ) {
        if ( obj->buffer == 0 ) {
            return 0;
        }
        return obj->buffer->length / sizeof(Object *);
    }
    return 0;
};

void push(Object *obj, Object *value) {
    obj = array_parent(obj);
    if ( is_array(obj) ) {
        if (obj->buffer == 0) {
            obj->buffer = (Buffer *)allocate_array(4);
        }
        ArrayBuffer* array = (ArrayBuffer *)obj->buffer;
        if ( (char *)(array->data + array->tail) >= 
            (char *)(obj->buffer->data + obj->buffer->length)
        ) {
            array = resize_array(obj);
        }
        array->data[array->tail++] = value;
    }
}

Object *pop(Object *obj) {
    obj = array_parent(obj);
    if ( is_array(obj) ) {
        ArrayBuffer* array = (ArrayBuffer *)obj->buffer;
        if ( array->tail > 0 ) {
            return array->data[--array->tail];
        }
        else {
            return 0;
        }
    }
    else {
        return 0;
    }
}

Object *peek(Object *stack) {
    Object *obj = array_parent(stack);
    if ( is_array(obj) ) {
        ArrayBuffer* array = (ArrayBuffer *)obj->buffer;
        if ( array->tail > 0 ) {
            return array->data[array->tail-1];
        }
        else {
            return 0;
        }
    }
    else {
        return 0;
    }
}

Fixnum stack_length(Object *stack) {
    return array_length(array_parent(stack));
}

Object *get_at(Object *obj, Fixnum index) {
    obj = array_parent(obj);
    if ( is_array(obj) ) {
        ArrayBuffer* array = (ArrayBuffer *)obj->buffer;
        if ( index < array->tail ) {
            return array->data[index];
        }
        else {
            return 0;
        }
    }
    else {
        return 0;
    }
}

void set_at(Object *obj, Fixnum index, Object *value) {
    if ( is_array(obj) ) {
        while ( index >= array_capacity(obj) ) {
            resize_array(obj);
        }
        ArrayBuffer* array = (ArrayBuffer *)obj->buffer;
        array->data[index] = value;
    }
}

Object *new_closure(Object *block, Object *self, Object *lexical_frame) {
    Object *new_closure = new_object();
    new_closure->proto = block;
    set_slot(new_closure, SYM_LEXICAL_FRAME, lexical_frame);
    set_slot(new_closure, SYM_SELF, self);
    return new_closure;
}

Object *new_frame(Object *parent_frame, Object *closure) {
    Object *new_frame = new_object();
    new_frame->proto = closure;
    set_slot(new_frame, SYM_STACK,         new_array());
    set_slot(new_frame, SYM_LOCAL,         new_object());
    set_slot(new_frame, SYM_PARENT_FRAME,  parent_frame);
    set_slot(new_frame, SYM_PC,            0);
    return new_frame;
}

Object *object_resolve(Object *object, Object *slot_name) {
    if ( object == 0 ) {
        return 0;
    }
    return get_slot(object, slot_name);
}

Object *frame_resolve(Object *frame, Object *slot_name) {
    if ( frame == 0 ) {
        return 0;
    }
    Object *result = get_slot(get_slot(frame, SYM_LOCAL), slot_name);
    if ( result == 0 ) {
        result = frame_resolve(get_slot(frame, SYM_LEXICAL_FRAME), slot_name);
    }
    if ( result == 0 ) {
        result = object_resolve(get_slot(frame, SYM_SELF), slot_name);
    }
    return result;
}

bool is_closure(Object *closure) 
{
    while(is_object(closure)) {
        if ( is_block(closure) ) {
            return true;
        }
        closure = closure->proto;
    }
}

Object* new_exception_frame(Object *frame, Object *exception) {
    // FIXME
    return 0;
};

Object* new_exception(char const* str) {
    // FIXME
    return 0;
};

Object* object_send(Object *object, Object *frame, Object *stack) {
    if ( stack_length(stack) == 0 ) {
        return new_exception_frame(frame, new_exception("Missing argument"));
    }
    Object *slot_name = pop(stack);
    Object *slot_value = object_resolve(object, slot_name);
    if ( is_closure(slot_value) ) {
        set_slot(slot_value, SYM_SELF, object);
    }
    push(stack, slot_value);
    return frame;
}

Object* closure_send(Object *closure, Object *frame, Object *stack) {
    Object *closure_frame = new_frame(frame, closure); 
    Object *arg_slot      = get_slot(closure, SYM_ARG);
    if ( arg_slot != 0 ) {
        if ( stack_length(stack) == 0 ) {
            return new_exception_frame(frame, new_exception("Missing argument"));
        }
        else {
            Object *arg_value = pop(stack);
            arg_value = frame_resolve(frame, arg_value);
            set_slot(get_slot(closure_frame, SYM_LOCAL), arg_slot, arg_value);
            return closure_frame;
        }
    }
}

Object* send(Object *frame, Object *stack) 
{
    Object *target   = pop(stack);
    if ( is_sym(target) ) {
        target = frame_resolve(frame, target);
    }
    if ( is_closure(target) ) {
        return closure_send(target, frame, stack);
    }
    else {
        return object_send(target, frame, stack);
    }
};

Object *get_instr(Object *code, Fixnum pc)
{
    return get_at(code, pc);
}

void interpret(Object *frame) {

    Fixnum pc         = fixnum(get_slot(frame, SYM_PC));
    Object *stack     = get_slot(frame, SYM_STACK);
    Object *code      = get_code(frame);

    Object *top       = 0;
    Object *jmp_pos   = 0;
    Object *proto     = 0;
    Object *new_frame = 0;

    bool   terminate = false;
  
    while ( ! terminate ) {
        Object *instr  = get_instr(code, pc);
        switch(sym_fixnum(instr)) {
            case INST_POP:
                top = pop(stack);
                set_slot(frame, SYM_PC, object(++pc));
                break;
            case INST_PUSH:
                // a push instruction must be followed by an object
                // for string object we probably want to call dup on them
                // but that is a matter for the thing generating the code
                push(stack, get_at(code, ++pc));
                set_slot(frame, SYM_PC, object(++pc));
                break;
            case INST_JMP:
                // jumps to the second position on the stack 
                // if the top of the stack is not nil
                top = pop(stack);
                jmp_pos = get_at(code, ++pc);
                if ( top != 0 ) {
                    pc = fixnum(jmp_pos);
                }
                set_slot(frame, SYM_PC, object(pc));
                break;
            case INST_SEND:
                new_frame = send(frame, stack);
                if ( 
                    frame == new_frame      && 
                    stack_length(stack) < 2 && 
                    ! is_closure(peek(stack)) 
                ) {
                    set_slot(frame, SYM_PC, object(++pc));
                }
                frame = new_frame;
                break;
            case INST_RET:
                top   = pop(stack);
                frame = get_slot(frame, SYM_PARENT_FRAME);
                stack = get_slot(frame, SYM_STACK);
                push(stack, top);
                code  = get_code(frame);
                pc    = fixnum(get_slot(frame, SYM_PC));
                break;
            case INST_TERM:
                terminate = true;
                break;
            case INST_NEW:
                proto = pop(stack);
                push(stack, new_object(proto));
                set_slot(frame, SYM_PC, object(++pc));
                break;
        }
    }
  
};

#include <stdio.h>

bool assert_true(bool value, char const* message) {
    if ( ! value ) {
        fprintf(stderr, "ASSERT FAILURE: %s\n", message);
        exit(-1);
    }
}

void test_push_pop() {
    Object *self = 0;
    Object *ret  = 0;
    Object *block = new_block();
    Object *frame = new_frame(ret, block);
    push(block, sym_object(INST_PUSH));
    push(block, object(1));
    push(block, sym_object(INST_PUSH));
    push(block, object(2));
    push(block, sym_object(INST_POP));
    push(block, sym_object(INST_POP));
    push(block, sym_object(INST_TERM));
    interpret(frame);
    assert_true(array_length(get_slot(frame, SYM_STACK)) == 0, "Assert stack empty");
};

void test_send() {
    Object *self = 0;
    Object *ret  = 0;
    Object *block = new_block();
    Object *frame = new_frame(ret, block);
    Object *obj   = new_object();
    Object *one   = sym("one");
    set_slot(obj, one, object(1));

    push(block, sym_object(INST_PUSH));
    push(block, one);
    push(block, sym_object(INST_PUSH));
    push(block, obj);
    push(block, sym_object(INST_SEND));
    push(block, sym_object(INST_TERM));
    interpret(frame);
    assert_true(array_length(get_slot(frame, SYM_STACK)) == 1, "One return value");
    assert_true(peek(get_slot(frame, SYM_STACK)) == object(1), "Stack contains result");
};

void test_array() 
{
    Object *a = sym("a");
    Object *b = sym("b");
    Object *c = sym("c");
    Object *arr = new_array();
    push(arr, a);
    assert_true(a == get_at(arr, 0), "first element is a");
    push(arr, b);
    assert_true(b == get_at(arr, 1), "second element is b");
    push(arr, c);
    assert_true(c == get_at(arr, 2), "third element is c");
    assert_true(array_length(arr) == 3, "length of the array is 3");
    push(arr, a);
    assert_true(a == get_at(arr, 3), "at 3 is a");
    push(arr, b);
    assert_true(b == get_at(arr, 4), "at 4 is b");
    push(arr, c);
    assert_true(c == get_at(arr, 5), "at 5 is c");
    assert_true(a == get_at(arr, 0), "at 0 is a");
    assert_true(b == get_at(arr, 1), "at 1 is b");
    assert_true(c == pop(arr), "pop c 6");
    assert_true(b == pop(arr), "pop b 5");
    assert_true(a == pop(arr), "pop a 4");
    assert_true(array_length(arr) == 3, "length of the array is 3");
    assert_true(c == pop(arr), "pop c 3");
    assert_true(b == pop(arr), "pop b 2");
    assert_true(a == pop(arr), "pop a 1");
    assert_true(array_length(arr) == 0, "length of the array is 0");
}

int read_token(char const* line, int line_len, char *value, int value_len, int offset) 
{
    Fixnum pos = 0;

    // skip leading white space
    while( offset < line_len && line[offset] && pos + 1 < value_len && 
        isspace(line[offset]) ) {
        offset++;
    }

    // read the string
    while( offset < line_len && line[offset] && pos + 1 < value_len ) {
        value[pos] = line[offset];
        if ( *value == '"' && pos > 0 ) { 
            if ( line[offset] == '"' ) {
                if ( pos > 1 ) {
                    if ( value[pos-1] != '\\' ) {
                        value[pos] = line[offset];
                        break;
                    }
                }
            }
            value[pos] = line[offset];
        }
        else {
            if ( isspace(line[offset]) ) {
                break;
            }
            value[pos] = line[offset];
        }
        pos++; offset++;
    }
    value[pos] = 0;

    return offset;
}

void set_array_length(Object *obj, Fixnum length) 
{
    if ( is_array(obj) ) {
        while ( length > array_capacity(obj) ) {
            resize_array(obj);
        }
        ArrayBuffer* array = (ArrayBuffer *)obj->buffer;
        array->tail = length;
    }
}

Object* replace_labels(Object *block, Object *labels) 
{
    for(Fixnum i = 0;i < array_length(labels); ++i) {
        Fixnum block_index = fixnum(get_at(labels, i));
        Object *pc = get_slot(labels, get_at(block, block_index));
        set_at(block, block_index, pc);
    }
    set_array_length(labels, 0);
}

Fixnum read_fixnum(char *s, Fixnum length)
{
    Fixnum result = 0;
    Fixnum i = 0;
    while( i < length && isdigit(s[i]) ) {
        result *= 10;
        result += s[i] - '0';
        i += 1;
    }
    return result;
};

Object* load(FILE *input) 
{
    char line[4096];
    char instr_token[4096];
    char arg_token[4096];
    Fixnum offset;

    Object *block     = new_block();
    Object *blocks    = new_object();
    Object *instr_sym = 0;
    Object *labels    = new_array();
    Object *instr     = 0;


    // - implement a mechanism to represent blocks and symbols
    while ( fgets(line, sizeof(line), input) ) {
        offset = 0;
        offset = read_token(
            line, sizeof(line), instr_token, sizeof(instr_token), offset
        );
        offset = read_token(
            line, sizeof(line), arg_token, sizeof(arg_token), offset
        );
        if ( *instr_token == ':' ) {
            replace_labels(block, labels);
            block = new_block();
            labels = new_array();
            set_slot(blocks, sym(instr_token+1), block);
        }
        else if ( *instr_token == '.' ) {
            Fixnum label = array_length(block);
            set_slot(labels, sym(instr_token+1), object(label));
        }
        else {
            instr_sym = sym(instr_token);
            instr = get_slot(Instr, instr_sym);
            if ( instr ) {
                push(block, instr);
                if ( *arg_token == '#' ) {
                    push(block, sym(arg_token+1));
                }
                else if ( *arg_token == ':' ) {
                    push(block, get_slot(blocks, sym(arg_token+1)));
                }
                else if ( *arg_token == '.' ) {
                    push(labels, object(array_length(block)));
                    push(block, sym(arg_token+1));
                }
                else if ( *arg_token == '"' ) {
                    push(block, new_string(arg_token+1, strlen(arg_token)-1));
                }
                else if ( isdigit(*arg_token) ) {
                    push(block, object(read_fixnum(arg_token, sizeof(arg_token))));
                }
                else if ( *arg_token == 'n' && strcmp(arg_token, "nil") == 0 ) {
                    push(block, 0);
                }
            }
        }
    }

    replace_labels(block, labels);
    return block;
};


int main(int argc, char **argv) {
    if ( argc < 2 ) {
        test_array();
        test_push_pop();
        test_send();
    }
    else {
        FILE *input = fopen(argv[1], "r");
        Object *block = load(input);
        interpret(new_frame(0, block));
    }
};






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
//--------
// Test 6 fails because the function returns a string and no method is invoked on 
// the string so the code throws an exception, ignoring the final value in an expression
// should be easier

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
ArrayBuffer* allocate_array(Fixnum length);
Object* sym(char const *str, int length = -1);
Object* sym(Object *str);
Object* new_symbol(Object *str);
Object* new_symbol(char const *str, int length = -1);
Object* new_instr_table();

Object* Array   = new_object();
Object* String  = new_object();
Object* Symbol  = new_object();
Object* Integer = new_object();
Object* Block   = new_object();

Object* SymbolTable = new_object();
Object* SetterTable = new_object();

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
Object* SYM_PARENT_FRAME  = sym("parent_frame");
Object* SYM_LOCAL         = sym("local");

Object* SYM_ASSIGNMENT = sym("assignment");
Object* SYM_TRUE       = sym("true");
Object* SYM_FALSE      = sym("false");

Object* SYM_TYPE       = sym("type");
Object* SYM_VALUE      = sym("type");
Object* SYM_OPEN       = sym("open");
Object* SYM_READ_LINE  = sym("readline");

bool is_fixnum(Object *obj) {
    return ((Fixnum)obj & 0x1) == 0x1;
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

bool fixnum_equals(Object *s, Object *t) {
    return is_fixnum(s) && is_fixnum(t) && s == t;
}

bool is_string(Object *s) {
    return is_object(s) && s->proto == String;
}

bool is_sym(Object *s) {
    return is_object(s) && s->proto == Symbol;
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
      index = (Fixnum)key;
      candidate = obj->table[index % obj->length].key;
      while( candidate != 0 && candidate != key ) {
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
      index = (Fixnum)key;
        candidate = obj->table[index % obj->length].key;
        while( candidate != 0 && candidate != key ) {
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

int is_setter_string(Object *str) {
  return str->buffer && str->buffer->length > 2 && 
    ( str->buffer->data[str->buffer->length - 1] == ':' );
}

int is_setter(Object *sym) {
  get_slot(SetterTable, sym) != 0;
}

Object* create_slot_from_setter(Object *str) {
  Object *slot_string = new_string(str->buffer->data, str->buffer->length - 1);
  return sym(slot_string);
}

Object* setter_slot(Object *setter_sym) {
  return get_slot(SetterTable, setter_sym);
}

Object* sym(char const *str, int length) {
  Object *key   = new_string(str, length);
  return sym(key);
}

Object* sym(Object *key) {
    Object *value = get_slot(SymbolTable, key);
    if ( value == 0 ) {
      value = new_symbol(key);
      set_slot(SymbolTable, key, value);
      if ( is_setter_string(key) ) {
	set_slot(SetterTable, key, create_slot_from_setter(key));
      }
    }
    return value;
}

Object *new_instr_table() {
    Object *instr_table = new_object();
    set_slot(instr_table, sym("pop"),    object(INST_POP));
    set_slot(instr_table, sym("send"),   object(INST_SEND));
    set_slot(instr_table, sym("call"),   object(INST_CALL));
    set_slot(instr_table, sym("push"),   object(INST_PUSH));
    set_slot(instr_table, sym("jmp"),    object(INST_JMP));
    set_slot(instr_table, sym("ret"),    object(INST_RET));
    set_slot(instr_table, sym("term"),   object(INST_TERM));
    set_slot(instr_table, sym("dup"),    object(INST_DUP));
    set_slot(instr_table, sym("assign"), object(INST_ASSIGN));
    set_slot(instr_table, sym("raise"),  object(INST_RAISE));
    set_slot(instr_table, sym("new"),    object(INST_NEW));
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

Object* new_symbol(char const *str, int length) {
    Object *obj = new_object(Symbol);
    if ( length == -1 ) {
        length = strlen(str);
    }
    obj->buffer = new_buffer(length, str);
    return obj;
}

Object* new_symbol(Object *key) {
  if ( is_string(key) ) {
    Object *obj = new_object(Symbol);
    obj->buffer = new_buffer(key->buffer->length, key->buffer->data);
  }
  else {
    abort();
  }
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

Object* new_exception_frame(Object *frame, char const* str) {
    // FIXME
    return 0;
};

Object* new_exception(char const* str) {
    // FIXME
    return 0;
};

Object* setter_send(Object *target, Object *slot, Object *frame, Object *stack) {
  if ( stack_length(stack) == 0 ) {
    return new_exception_frame(frame, new_exception("Missing argument"));
  }
  else {
    Object *value = pop(stack);
    set_slot(frame, SYM_ASSIGNMENT, SYM_TRUE);
    set_slot(target, setter_slot(slot), value);
    push(stack, value);
  }
  return frame;
} 

Object* object_send(Object *target, Object *frame, Object *stack) {
    if ( stack_length(stack) == 0 ) {
      push(stack, target);
      return frame;
    }
    Object *slot_name  = pop(stack);
    Object *slot_value = object_resolve(target, slot_name);
    if ( slot_value == 0 && is_setter(slot_name) ) {
      return setter_send(target, slot_name, frame, stack);
    }
    else {
      if ( is_closure(slot_value) ) {
	set_slot(slot_value, SYM_SELF, target);
      }
      push(stack, slot_value);
      return frame;
    }
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
  else {
    return closure_frame;
  }
}

Object* send(Object *frame, Object *stack) 
{
    set_slot(frame, SYM_ASSIGNMENT, SYM_FALSE);
    Object *target   = pop(stack);
    if ( is_sym(target) ) {
      Object *slot = target;
      target = frame_resolve(frame, slot);
      if ( target == 0 && is_setter(slot) ) {
	return setter_send(get_slot(frame, SYM_LOCAL), slot, frame, stack);
      }
      else {
	push(stack, target);
	return frame;
      }
    }
    if ( is_closure(target) ) {
      return closure_send(target, frame, stack);
    }
    else if ( target != 0 ) {
      return object_send(target, frame, stack);
    }
    else {
      return frame;
    }
}

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
        switch(fixnum(instr)) {
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
                  (! is_closure(peek(stack)) || 
		     get_slot(frame, SYM_ASSIGNMENT) == SYM_TRUE) 
              ) {
                  set_slot(frame, SYM_PC, object(++pc));
              }
	      else if ( frame != new_frame ) {
		frame = new_frame;
                stack = get_slot(frame, SYM_STACK);
                code  = get_code(frame);
                pc    = fixnum(get_slot(frame, SYM_PC));
	      }
              break;
            case INST_RET:
                top   = pop(stack);
                frame = get_slot(frame, SYM_PARENT_FRAME);
                stack = get_slot(frame, SYM_STACK);
                code  = get_code(frame);
                pc    = fixnum(get_slot(frame, SYM_PC));
                push(stack, top);
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
    push(block, object(INST_PUSH));
    push(block, object(1));
    push(block, object(INST_PUSH));
    push(block, object(2));
    push(block, object(INST_POP));
    push(block, object(INST_POP));
    push(block, object(INST_TERM));
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

    push(block, object(INST_PUSH));
    push(block, one);
    push(block, object(INST_PUSH));
    push(block, obj);
    push(block, object(INST_SEND));
    push(block, object(INST_TERM));
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

void dump_stack(Object *object, int indent = 0);

void dump_buffer(Object *object) {
  Fixnum i;
  for(i=0;i<object->buffer->length;++i) {
    putchar(object->buffer->data[i]);
  }
}

void dump(Object *object, int indent = 0) {
  char indent_string[indent+1];
  indent_string[indent]=0;
  memset(indent_string, ' ', indent);
  if ( object == 0 ) {
    printf("%s0\n", indent_string);
    return;
  }
  else if ( is_object(object) ) {
    printf("%sproto: %p\n", indent_string, object->proto);
    printf("%sbuffer: %p\n", indent_string, object->buffer);
    for(Fixnum i=0;i<object->length;++i) {
      ObjectPair *pair = object->table + i;
      if ( pair->key != 0 ) {
	if ( is_sym(pair->key) ) {
	  printf("%s:", indent_string);
	  dump_buffer(pair->key);
	  printf("\n");
	  dump(pair->value, indent+1);
	}
	else {
	  printf("%snot a sym\n", indent_string);
	}
      }
    }
    if ( array_parent(object) != 0 ) {
      printf("%sarray: \n", indent_string);
      dump_stack(object, indent);
    }
  }
  else if ( is_fixnum(object) ) {
    printf("%sfixnum: %d\n", indent_string, fixnum(object));
  }
  else if ( is_sym(object) ) {
    printf("%ssym:", indent_string);
    dump_buffer(object);
    printf("\n");
  }
  else if ( is_string(object) ) {
    printf("%sstring:", indent_string);
    dump_buffer(object);
    printf("\n");
  }
}

void dump_stack(Object *object, int indent) {
  char indent_string[indent+1];
  indent_string[indent]=0;
  memset(indent_string, ' ', indent);
  for(Fixnum i=0; i<stack_length(object); ++i) {
    printf("%s%d:", indent_string, i);
    dump(get_at(array_parent(object), i), indent+1);
  }
}

// ----------------------------------------------------------------------
// Parser
// ----------------------------------------------------------------------

Object* TOK_EOL = sym("TOK_EOL");
Object* TOK_IDENT = sym("TOK_IDENT");
Object* TOK_OPEN_MAP = sym("TOK_OPEN_MAP");
Object* TOK_OPEN_ARRAY = sym("TOK_OPEN_ARRAY");
Object* TOK_OPEN_GROUP = sym("TOK_OPEN_GROUP");
Object* TOK_OPEN_BLOCK = sym("TOK_OPEN_BLOCK");
Object* TOK_CLOSE_MAP = sym("TOK_CLOSE_MAP");
Object* TOK_CLOSE_ARRAY = sym("TOK_CLOSE_ARRAY");
Object* TOK_CLOSE_GROUP = sym("TOK_CLOSE_GROUP");
Object* TOK_CLOSE_BLOCK = sym("TOK_CLOSE_BLOCK");
Object* TOK_STRING = sym("TOK_STRING");
Object* TOK_UNTERM_STRING = sym("TOK_UNTERM_STRING");
Object* TOK_INT = sym("TOK_INT");
Object* TOK_FLOAT = sym("TOK_FLOAT");
Object* TOK_PIPE = sym("TOK_PIPE");
Object* TOK_EQUALS = sym("TOK_EQUALS");

bool is_space(char c) {
  return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f';
}

bool is_digit(char c) {
  return c >= '0' && c <= '9';
}

bool is_ident_start(char c) {
  return ! is_space(c);
}

bool is_ident_continue(char c) {
  return ! is_space(c) && c != '[' && c != ']' && c != '{' && c != '}' && c != '|' && c != '(' && c != ')';
};

Fixnum read_tok(char const* line, Fixnum line_len, Fixnum offset, Object *tok) {
  Fixnum i = offset;

  // skip any white space
  while ( is_space(line[i]) ) ++i; 

  if ( i >= line_len || line[i] == 0 ) {
    set_slot(tok, SYM_TYPE, TOK_EOL);
    return i+1;
  }

  switch(line[i]) {
  case 0:
    set_slot(tok, SYM_TYPE, TOK_EOL);
    return i+1;
  case '{':
    set_slot(tok, SYM_TYPE, TOK_OPEN_BLOCK);
    return i+1;
  case '}':
    set_slot(tok, SYM_TYPE, TOK_CLOSE_BLOCK);
    return i+1;
  case '(':
    set_slot(tok, SYM_TYPE, TOK_OPEN_GROUP);
    return i+1;
  case ')':
    set_slot(tok, SYM_TYPE, TOK_CLOSE_GROUP);
    return i+1;
  case '[':
    if ( i+1 < line_len && line[i+1] == '|' ) {
      set_slot(tok, SYM_TYPE, TOK_OPEN_ARRAY);
    }
    else {
      set_slot(tok, SYM_TYPE, TOK_OPEN_MAP);
    }
    return i+1;
  case '|':
    if ( i+1 < line_len && line[i+1] == ']' ) {
      set_slot(tok, SYM_TYPE, TOK_CLOSE_ARRAY);
    }
    else {
      set_slot(tok, SYM_TYPE, TOK_PIPE);
    }
    return i+1;
  case ']':
    set_slot(tok, SYM_TYPE, TOK_OPEN_MAP);
    return i+1;
  case '=':
    if ( i+1 >= line_len || isspace(line[i+1]) ) {
      set_slot(tok, SYM_TYPE, TOK_EQUALS);
      return i+1;
    }
  }

  Fixnum start_index = i;

  if ( is_digit(line[i]) || ( line[i] == '-' && i+1 < line_len && is_digit(line[i+1]) ) ) {
    set_slot(tok, SYM_TYPE, TOK_INT);
    while( i < line_len && is_digit(line[i]) ) ++i;
    if ( i+1 < line_len && line[i+1] == '.' ) {
      set_slot(tok, SYM_TYPE, TOK_FLOAT);
      ++i;
      while( i < line_len && is_digit(line[i]) ) ++i;
      if ( i < line_len && line[i] == 'e' ) {
	++i;
	if ( i < line_len && line[i] == '-' || is_digit(line[i]) ) {
	  ++i;
	  while ( i < line_len && is_digit(line[i]) ) ++i;
	}
      }
    }
  }
  else if ( line[i] == '"' ) {
    set_slot(tok, SYM_TYPE, TOK_STRING);
    ++i;
    while ( i < line_len && line[i] != '"' ) {
      if ( line[i] == '\\' ) ++i;
      ++i;
    }
    if ( i >= line_len ) {
      set_slot(tok, SYM_TYPE, TOK_UNTERM_STRING);
    }
    else {
      ++i;
    }
  }
  else if ( is_ident_start(line[i]) ) {
    set_slot(tok, SYM_TYPE, TOK_IDENT);
    while( is_ident_continue(line[i]) ) ++i;
  }

  set_slot(tok, SYM_VALUE, new_string(line + start_index, i - start_index));
  return i;
}

int print_tokens(FILE *input) {
  Fixnum offset;
  char   line[4096];
  while ( fgets(line, sizeof(line), input) ) {
    offset = 0;
    while ( offset < sizeof(line) && line[offset] != 0 ) {
      Object *tok = new_object();
      offset = read_tok(line, sizeof(line), offset, tok);
      printf("token: ");
      dump_buffer(get_slot(tok, SYM_TYPE));
      printf("\n");
    }
  }
}

Object *File = new_object();
Object *Function = new_object();

typedef Object* (*native_function_ptr)(Object *frame);

Object *new_function(native_function_ptr f) {
  Object *obj = new_object(Function);
  obj->buffer = new_buffer(sizeof(f), (char *)&f);
};

Object* file_open(Object *frame) {
  Object *stack = get_slot(frame, SYM_STACK);
  Object *arg = pop(stack);
  if ( ! is_string(arg) ) {
    return new_exception_frame(frame, "Type error: expected a string argument");
  }
  if ( arg->buffer->length >= 4096 ) {
    return new_exception_frame(frame, "Argument error: expected string less than 4096 characters");
  }
  else {
    char fname[4096];
    memcpy(fname, arg->buffer->data, arg->buffer->length);
    fname[arg->buffer->length] = 0;
    FILE *file = fopen(fname, "r");
    Object *result = new_object(File);
    result->buffer = new_buffer(sizeof(file), (char *)&file);
    push(stack, result);
  }
}

Object *file_read_line(Object *frame) {
  Object *self = get_slot(frame, SYM_SELF);
  if ( ! is_object(self) && self->proto == File ) {
    return new_exception_frame(frame, "Expected a file object as self");
  }
  else {
    char line[4096];
    if ( fgets(line, sizeof(line), *(FILE**)self->buffer->data) ) {
      Object *stack = get_slot(frame, SYM_STACK);
      push(stack, new_string(line));
    }
    else {
      push(get_slot(frame, SYM_STACK), 0);
    }
    return frame;
  }
}

// ----------------------------------------------------------------------
// Main
// ----------------------------------------------------------------------

int main(int argc, char **argv) {
  set_slot(File, SYM_OPEN, new_function(&file_open));
  set_slot(File, SYM_READ_LINE, new_function(&file_read_line));

    if ( argc < 2 ) {
        test_array();
        test_push_pop();
        test_send();
    }
    else if ( argc < 3 ) {
        FILE *input = fopen(argv[1], "r");
        Object *block = load(input);
	Object *frame = new_frame(0, block);
        interpret(frame);
	dump(frame, 0);
	dump_stack(get_slot(frame, SYM_STACK));
    }
    else {
      FILE *input = fopen(argv[2], "r");
      print_tokens(input);
    }
}




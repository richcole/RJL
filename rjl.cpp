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
// Test 6 fails because the function returns a string and no method is
// invoked on the string so the code throws an exception, ignoring the
// final value in an expression should be easier

#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <math.h>

typedef ssize_t Fixnum;

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

typedef Object* (*native_function_ptr)(Object *frame);

#define INST_POP      0x1
#define INST_SEND     0x2
#define INST_CALL     0x3
#define INST_PUSH     0x4
#define INST_JMP      0x5
#define INST_JMPZ     0x6
#define INST_JMPNZ    0x7
#define INST_RET      0x8
#define INST_TERM     0x9
#define INST_DUP      0x10
#define INST_ASSIGN   0x11
#define INST_RAISE    0x12
#define INST_NEW      0x13

Object* new_object(Object *proto = 0);
Object* new_string(char const *str, int length = -1);
Object* new_string_of_length(Fixnum length);
Object* new_block();
Object* new_array();
ArrayBuffer* allocate_array(Fixnum length);
Object* sym(char const *str, int length = -1);
Object* sym(Object *str);
Object* new_symbol(Object *str);
Object* new_symbol(char const *str, int length = -1);
Object* new_instr_table();
Fixnum is_function(Object *obj);

Object* Array   = new_object();
Object* String  = new_object();
Object* Symbol  = new_object();
Object* Integer = new_object();
Object* Block   = new_object();
Object* Double  = new_object();

Object* SymbolTable = new_object();
Object* SetterTable = new_object();
Object* ExisterTable = new_object();

Object* SendMarker  = new_object();

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
Object* SYM_ARGS    = sym("args");

Object* SYM_LEXICAL_FRAME = sym("lexical_frame");
Object* SYM_PARENT_FRAME  = sym("parent_frame");
Object* SYM_LOCAL         = sym("local");

Object* SYM_ASSIGNMENT = sym("assignment");
Object* SYM_TRUE       = sym("true");
Object* SYM_FALSE      = sym("false");

Object* SYM_TYPE       = sym("type");
Object* SYM_VALUE      = sym("value");
Object* SYM_OPEN       = sym("open");
Object* SYM_READ_LINE  = sym("readline");

Object* SYM_LE         = sym("<");
Object* SYM_GT         = sym(">");
Object* SYM_LEQ        = sym("<=");
Object* SYM_GEQ        = sym(">=");
Object* SYM_PLUS       = sym("+");
Object* SYM_MINUS      = sym("-");

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
    return result;
  }
}

void set_proto(Object *obj, Object *proto) {
    obj->proto = proto;
}

void *allocate(Fixnum size) {
    void *ptr = malloc(size);
    memset(ptr, 0, size);
    return ptr;
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
      // other types are based on object location, same as symbols
      index = (Fixnum)key;
      candidate = obj->table[index % obj->length].key;
      while( candidate != 0 && candidate != key ) {
        index += 1;
        candidate = obj->table[index % obj->length].key;
      }
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
    else { // otherwise based in object location, same code as symbols
      index = (Fixnum)key;
      candidate = obj->table[index % obj->length].key;
      while( candidate != 0 && candidate != key ) {
	index += 1;
	candidate = obj->table[index % obj->length].key;
      }
    }
    if ( candidate != 0 ) {
        return obj->table[index % obj->length].value;
    }
    // perform lookup in parent object
    if ( is_object(obj) && is_object(obj->proto) ) {
      return get_slot(obj->proto, key);
    }
    else {
      return 0;
    }
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

Object *string_append(Object *s, char const* str, int str_len = -1) {
  if ( str_len == -1 ) {
    str_len = strlen(str);
  }
  Object *result = new_string_of_length(s->buffer->length + str_len);
  memcpy(result->buffer->data, s->buffer->data, s->buffer->length);
  memcpy(result->buffer->data + s->buffer->length, str, str_len);
  return result;
}

int is_setter_string(Object *str) {
  return str->buffer && str->buffer->length >= 1 && 
    ( str->buffer->data[str->buffer->length - 1] == '=' );
}

int is_setter(Object *sym) {
  return get_slot(SetterTable, sym) != 0;
}

Object* create_slot_from_setter(Object *str) {
  Object *slot_string = new_string(str->buffer->data, str->buffer->length - 1);
  return sym(slot_string);
}

Object* setter_slot(Object *setter_sym) {
  return get_slot(SetterTable, setter_sym);
}

Object* setter_sym(Object *symbol) {
  Object *setter_string = string_append(symbol, "=", 1);
  return sym(setter_string);
};

int is_exister_string(Object *str) {
  return str->buffer && str->buffer->length > 2 && 
    ( str->buffer->data[str->buffer->length - 1] == ':' );
}

int is_exister(Object *sym) {
  return get_slot(ExisterTable, sym) != 0;
}

Object* create_slot_from_exister(Object *str) {
  Object *slot_string = new_string(str->buffer->data, str->buffer->length - 1);
  return sym(slot_string);
}

Object* exister_slot(Object *exister_sym) {
  return get_slot(ExisterTable, exister_sym);
}

Object* exister_sym(Object *symbol) {
  Object *exister_string = string_append(symbol, ":", 1);
  return sym(exister_string);
};

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
        set_slot(SetterTable, value, create_slot_from_setter(key));
      }
      if ( is_exister_string(key) ) {
        set_slot(ExisterTable, value, create_slot_from_exister(key));
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
    set_slot(instr_table, sym("jmpz"),   object(INST_JMPZ));
    set_slot(instr_table, sym("jmpnz"),  object(INST_JMPNZ));
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

Buffer* new_buffer_of_length(Fixnum length) {
  Buffer *buffer = (Buffer *)allocate(sizeof(Buffer)+length);
  buffer->length = length;
  return buffer;
};

Object* new_string(char const *str, Fixnum length) {
  Object *obj = new_object(String);
  if ( length == -1 ) {
    length = strlen(str);
  }
  obj->buffer = new_buffer(length, str);
  return obj;
}

Object* new_double(double value) {
  Object *obj = new_object(Double);
  obj->buffer = new_buffer(sizeof(double), (char *)&value);
  return obj;
}

Object* new_integer(int value) {
  Object *obj = new_object(Double);
  obj->buffer = new_buffer(sizeof(double), (char *)&value);
  return obj;
}

Fixnum is_integer(Object *obj) {
  return is_object(obj) && obj->proto == Integer;
}

Fixnum is_double(Object *obj) {
  return is_object(obj) && obj->proto == Double;
}

Object* new_string_of_length(Fixnum length) {
  Object *obj = new_object(String);
  obj->buffer = new_buffer_of_length(length);
  return obj;
}

void string_to_buffer(Object *s, char *buf, int buf_len) {
  if ( s->buffer->length + 1 <= buf_len ) {
    memcpy(buf, s->buffer->data, s->buffer->length);
    buf[s->buffer->length] = 0;
  }
  else {
    memcpy(buf, s->buffer->data, buf_len - 1);
    buf[buf_len-1] = 0;
  }
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
    return obj;
  }
  else {
    abort();
    return 0;
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
  return false;
}

Object* new_exception_frame(Object *frame, Object *exception) {
  // FIXME
  abort();
  return 0;
};

Object* new_exception_frame(Object *frame, char const* str) {
  // FIXME
  abort();
  return 0;
};

Object* new_exception(char const* str) {
  // FIXME
  abort();
  return 0;
};

// ----------------------------------------------------------------------
// Send
//

Object* object_send(Object *target, Object *frame, Object *stack);
Object* setter_send(Object *target, Object *slot, Object *frame, Object *stack);
Object* exister_send(Object *target, Object *slot, Object *frame, Object *stack);

Object* setter_send(Object *target, Object *slot, Object *frame, Object *stack) {
  if ( stack_length(stack) == 0 ) {
    return new_exception_frame(frame, new_exception("Missing argument"));
  }
  else {
    Object *value = pop(stack);
    set_slot(frame, SYM_ASSIGNMENT, SYM_TRUE);
    set_slot(target, setter_slot(slot), value);
  }
  return frame;
} 

Object* exister_send(Object *target, Object *slot, Object *frame, Object *stack) {
  Object *eslot = exister_slot(slot);
  Object *obj = get_slot(target, eslot);
  if ( obj == 0 ) {
    obj = new_object();
    set_slot(target, eslot, obj);
  }
  push(stack, obj);
  return frame;
}

Object* fixnum_send(Object *target, Object *frame, Object *stack) {
  if ( array_length(stack) == 0 ) {
    push(stack, target);
    return frame;
  }
  else {
    Object *slot_name = pop(stack);
    if ( slot_name == SYM_LE && is_fixnum(peek(stack)) ) {
      if ( fixnum(target) < fixnum(pop(stack)) ) {
	push(stack, frame_resolve(frame, SYM_TRUE));
      }
      else {
	push(stack, frame_resolve(frame, SYM_FALSE));
      }
      return frame;
    }
    if ( slot_name == SYM_LEQ && is_fixnum(peek(stack)) ) {
      if ( fixnum(target) <= fixnum(pop(stack)) ) {
	push(stack, frame_resolve(frame, SYM_TRUE));
      }
      else {
	push(stack, frame_resolve(frame, SYM_FALSE));
      }
      return frame;
    }
    if ( slot_name == SYM_GT && is_fixnum(peek(stack)) ) {
      if ( fixnum(target) > fixnum(pop(stack)) ) {
	push(stack, frame_resolve(frame, SYM_TRUE));
      }
      else {
	push(stack, frame_resolve(frame, SYM_FALSE));
      }
      return frame;
    }
    if ( slot_name == SYM_GEQ && is_fixnum(peek(stack)) ) {
      if ( fixnum(target) >= fixnum(pop(stack)) ) {
	push(stack, frame_resolve(frame, SYM_TRUE));
      }
      else {
	push(stack, frame_resolve(frame, SYM_FALSE));
      }
      return frame;
    }
    else if ( slot_name == SYM_PLUS && is_fixnum(peek(stack)) ) {
      push(stack, object(fixnum(target) + fixnum(pop(stack))));
      return frame;
    }
    else if ( slot_name == SYM_MINUS && is_fixnum(peek(stack)) ) {
      push(stack, object(fixnum(target) - fixnum(pop(stack))));
      return frame;
    }
    else {
      Object *slot = pop(stack);
      push(stack, target);
      push(stack, slot);
      target = frame_resolve(frame, sym("Integer"));
      return object_send(target, frame, stack);
    }
  }
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
  if ( slot_value == 0 && is_exister(slot_name) ) {
    return exister_send(target, slot_name, frame, stack);
  }
  else {
    if ( is_closure(slot_value) ) {
      set_slot(slot_value, SYM_SELF, target);
    }
    push(stack, slot_value);
    return frame;
  }
}

Object* function_send(Object *target, Object *frame, Object *stack) {
  native_function_ptr func_ptr = *(native_function_ptr*)target->buffer->data;
  return (*func_ptr)(frame);
}

Object* closure_send(Object *closure, Object *frame, Object *stack) {
  Object *closure_frame = new_frame(frame, closure); 
  Object *arg_slots     = get_slot(closure, SYM_ARGS);
  if ( arg_slots != 0 ) {
    Fixnum i;
    for(i=0; i<array_length(arg_slots); ++i) {
      Object *arg_slot = get_at(arg_slots, i);
      if ( stack_length(stack) == 0 ) {
        return new_exception_frame(frame, new_exception("Missing argument"));
      }
      else {
        Object *arg_value = pop(stack);
        if ( is_sym(arg_value) ) {
          arg_value = frame_resolve(frame, arg_value);
        }
        set_slot(get_slot(closure_frame, SYM_LOCAL), arg_slot, arg_value);
      }
    }
    return closure_frame;
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
      if ( target == 0 && is_exister(slot) ) {
        return exister_send(get_slot(frame, SYM_LOCAL), slot, frame, stack);
      }
      else {
        push(stack, target);
        return frame;
      }
    }
    if ( is_fixnum(target) ) {
      return fixnum_send(target, frame, stack);
    }
    else if ( is_closure(target) ) {
      return closure_send(target, frame, stack);
    }
    else if ( is_function(target) ) {
      return function_send(target, frame, stack);
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
                // jump to a location
                jmp_pos = get_at(code, ++pc);
		pc = fixnum(jmp_pos);
                set_slot(frame, SYM_PC, object(pc));
                break;
            case INST_JMPZ:
                // jumps to the second position on the stack 
                // if the top of the stack is nil
                top = pop(stack);
                jmp_pos = get_at(code, ++pc);
                if ( top == 0 ) {
                    pc = fixnum(jmp_pos);
                }
		else {
		  ++pc;
		}
                set_slot(frame, SYM_PC, object(pc));
                break;
            case INST_JMPNZ:
                // jumps to the second position on the stack 
                // if the top of the stack is not nil
                top = pop(stack);
                jmp_pos = get_at(code, ++pc);
                if ( top != 0 ) {
                    pc = fixnum(jmp_pos);
                }
		else {
		  ++pc;
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

void assert_true(bool value, char const* message) {
    if ( ! value ) {
        fprintf(stderr, "ASSERT FAILURE: %s\n", message);
        exit(-1);
    }
}

void test_push_pop() {
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
    return block;
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

void dump_object(Object *object, int indent, Object *visited);

void dump_buffer(Object *object, int indent, int newline, Object *visited) {
  Fixnum i;
  char indent_string[indent+1];
  indent_string[indent]=0;
  memset(indent_string, ' ', indent);
  if ( is_array(object) ) {
    printf("[\n");
    for(i=0;i<array_length(object);++i) {
      Object *child_object = get_at(object, i);
      if ( child_object ) {
        printf("%s %03d: ", indent_string, i);
        dump_object(child_object, indent + 1, visited);
      }
      else {
        printf("%s %03d: 0\n", indent_string, i);
      }
    }
    printf("%s]", indent_string);
  }
  else if ( is_fixnum(object) ) {
    printf("%d", fixnum(object));
  }
  else if ( is_integer(object) ) {
    dump_object(object, 0, 0);
  }
  else if ( is_double(object) ) {
    dump_object(object, 0, 0);
  }
  else if ( object->buffer ) {
    for(i=0;i<object->buffer->length;++i) {
      if ( object->buffer->data[i] == '\n' ) {
	putchar('\\');
	putchar('n');
      }
      else if ( object->buffer->data[i] == '\t' ) {
	putchar('\\');
	putchar('t');
      }
      else if ( object->buffer->data[i] == '\r' ) {
	putchar('\\');
	putchar('r');
      }
      else if ( object->buffer->data[i] == '\f' ) {
	putchar('\\');
	putchar('f');
      }
      else {
	putchar(object->buffer->data[i]);
      }
    }
  }
  else {
    printf("undefined");
  }
  if ( newline ) {
    printf("\n");
  }
}

void dump_object(Object *obj, int indent, Object *visited) {
  char indent_string[indent+1];
  indent_string[indent]=0;
  memset(indent_string, ' ', indent);
  if ( visited == 0 ) {
    visited = new_object();
  }
  if ( obj == 0 ) {
    printf("0\n");
    return;
  }
  else if ( is_fixnum(obj) ) {
    printf("%d\n", fixnum(obj));
  }
  else if ( is_sym(obj) ) {
    printf("#");
    dump_buffer(obj, indent+1, 1, visited);
  }
  else if ( is_string(obj) ) {
    printf("\""); 
    dump_buffer(obj, 0, 0, visited); 
    printf("\"\n");
  }
  else if ( is_double(obj) ) {
    printf("%lf\n", *(double *)obj->buffer->data);
  }
  else if ( is_integer(obj) ) {
    printf("%d\n", *(int *)obj->buffer->data);
  }
  else if ( is_object(obj) ) {
    if ( get_slot(visited, obj) != 0 ) {
      printf("^%p\n", obj);
      return;
    }
    set_slot(visited, obj, object(1));
    printf("{ %p\n", obj);
    if ( obj->proto ) {
      printf("%s proto: %p\n", indent_string, obj->proto);
    }
    if ( obj->buffer ) {
      printf("%s buffer: ", indent_string);
      dump_buffer(obj, indent+1, 1, visited);
    }
    for(Fixnum i=0;i<obj->length;++i) {
      ObjectPair *pair = obj->table + i;
      if ( pair->key != 0 ) {
        if ( is_sym(pair->key) ) {
          printf("%s ", indent_string);
          dump_buffer(pair->key, indent+1, 0, visited);
          printf(": ");
          dump_object(pair->value, indent+1, visited);
        }
        else {
          printf("%snot a sym\n", indent_string);
        }
      }
    }
    printf("%s}\n", indent_string);
  }
}

// ----------------------------------------------------------------------
// Parser
// ----------------------------------------------------------------------

Object *advance(Object *parse_cxt);
Object *parse_string(char const* str, int str_len);

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
Object* TOK_ASSIGN = sym("TOK_ASSIGN");
Object* TOK_SEMI = sym("TOK_SEMI");
Object* TOK_IF = sym("TOK_IF");
Object* TOK_ELSE = sym("TOK_ELSE");
Object* TOK_ELSIF = sym("TOK_ELSIF");
Object* TOK_WHILE = sym("TOK_WHILE");

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
  return ! is_space(c) && c != '[' && c != ']' && c != '{' && c != '}' && c != '|' && c != '(' && c != ')' && c != ';' ;
};

Object *parse_string(char const* str, int str_len) {
  Object *obj = new_string_of_length(str_len);
  Fixnum i, j, k, buf_i, value;
  for(i=0,j=0;i<str_len;i++) {
    if ( str[i] == '\\' && i+1 < str_len ) {
      if ( isdigit(str[i+1]) ) {
        value = 0;
        for(k=i+1, buf_i=0;k<str_len&&isdigit(str[k]);++k) {
          value *= 10; value += str[k] - '0';
        }
        obj->buffer->data[j++] = (unsigned char)value;
        i = k - 1;
      }
      else {
        switch(str[i+1]) {
        case 'n':
          obj->buffer->data[j] = '\n';
	  break;
        case 'f':
          obj->buffer->data[j] = '\f';
	  break;
        case 'r':
          obj->buffer->data[j] = '\r';
	  break;
        case 't':
          obj->buffer->data[j] = '\t';
	  break;
        default:
          obj->buffer->data[j] = str[i+1]; 
	  break;
	}
        ++i;
	++j;
      }
    }
    else {
      obj->buffer->data[j++] = str[i];
    }
  }
  return new_string(obj->buffer->data, j);
}

Fixnum read_tok(char const* line, Fixnum line_len, Fixnum offset, Object *tok) {
  Fixnum i = offset;

  // skip any white space
  while ( is_space(line[i]) ) ++i; 

  if ( i >= line_len || line[i] == 0 ) {
    set_slot(tok, SYM_TYPE, TOK_EOL);
    return i;
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
  case ';':
    set_slot(tok, SYM_TYPE, TOK_SEMI);
    return i+1;
  case '=':
    if ( i+1 >= line_len || isspace(line[i+1]) ) {
      set_slot(tok, SYM_TYPE, TOK_ASSIGN);
      return i+1;
    }
  }

  Fixnum start_index = i;
  Fixnum int_value   = 0;
  Fixnum int_sign    = 1;
  double float_sign = 1.0;
  double float_value = 0.0;
  double float_mult  = 1.0;
  double float_mant  = 0.0;
  double float_mant_sign = 1.0;

  if ( is_digit(line[i]) || 
       ( line[i] == '-' && i+1 < line_len && is_digit(line[i+1]) ) 
  ) {
    set_slot(tok, SYM_TYPE, TOK_INT);
    if ( line[i] == '-' ) {
      ++i;
      int_sign = -1;
      float_sign = -1.0;
    }
    while( i < line_len && is_digit(line[i]) ) {
      int_value *= 10;
      int_value += line[i] - '0';
      float_value *= 10;
      float_value += line[i] - '0';
      ++i;
    }
    if ( i+1 < line_len && line[i+1] == '.' ) {
      set_slot(tok, SYM_TYPE, TOK_FLOAT);
      ++i;
      while( i < line_len && is_digit(line[i]) ) {
	float_mult *= 0.1; 
	float_value += ( (line[i] - '0') * float_mult) ;
	++i;
      }
      if ( i < line_len && line[i] == 'e' ) {
        ++i;
        if ( i < line_len && ((line[i] == '-') || is_digit(line[i])) ) {
	  if ( line[i] == '-' ) {
	    float_mant_sign = -1.0;
	  }
          ++i;
          while ( i < line_len && is_digit(line[i]) ) {
	    float_mant *= 10;
	    float_mant += line[i] - '0';
	    ++i;
	  }
        }
      }
      double value = float_sign * float_value * pow(10, float_mant * float_mant_sign);
      set_slot(tok, SYM_VALUE, new_double(value));
    }
    else {
      set_slot(tok, SYM_VALUE, object(int_value * int_sign));
    }
    return i;
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

  Fixnum tok_len = i - start_index;
  if ( tok_len == 2 && strncmp("if", line + start_index, tok_len) == 0 ) {
    set_slot(tok, SYM_TYPE, TOK_IF);
  }
  if ( tok_len == 4 && strncmp("else", line + start_index, tok_len) == 0 ) {
    set_slot(tok, SYM_TYPE, TOK_ELSE);
  }
  if ( tok_len == 5 && strncmp("elsif", line + start_index, tok_len) == 0 ) {
    set_slot(tok, SYM_TYPE, TOK_ELSIF);
  }
  if ( tok_len == 5 && strncmp("while", line + start_index, tok_len) == 0 ) {
    set_slot(tok, SYM_TYPE, TOK_WHILE);
  }

  Object *value;
  if ( get_slot(tok, SYM_TYPE) == TOK_STRING ) {
    value = parse_string(line + start_index + 1, i - (start_index + 2));
  }
  else {
   value = new_string(line + start_index, tok_len);
  }
  set_slot(tok, SYM_VALUE, value);

  return i;
}

void print_tokens(Object *tokens) {
  for(Fixnum i=0;i<array_length(tokens);++i) {
    Object *tok = get_at(tokens, i);
    Object *tok_type = get_slot(tok, SYM_TYPE);
    Object *tok_value = get_slot(tok, SYM_VALUE);
    printf("token: ");
    if ( tok_value ) {
      dump_buffer(tok_type, 0, 0, 0);
      printf(":");
      dump_buffer(tok_value, 0, 1, 0);
    }
    else {
      dump_buffer(get_slot(tok, SYM_TYPE), 0, 1, 0);
    }
  }
}

Object *token_stack(FILE *input) {
  Object *tokens = new_array();
  Fixnum offset;
  char   line[4096];
  while ( fgets(line, sizeof(line), input) ) {
    offset = 0;
    while ( offset < (Fixnum)sizeof(line) && line[offset] != 0 ) {
      Object *tok = new_object();
      offset = read_tok(line, sizeof(line), offset, tok);
      push(tokens, tok);
    }
  }
  return tokens;
}

Object *ParseError = new_object();

void parse_error(Object *parse_cxt, char const* str) {
  Object *parse_errors = get_slot(parse_cxt, sym("errors"));
  Object *error        = new_object();
  set_slot(error, sym("message"), new_string(str));
  push(parse_errors, error);
};

Object *new_parse_cxt(Object *tokens) {
  Object *parse_cxt = new_object();
  set_slot(parse_cxt, sym("pos"),    object(0));
  set_slot(parse_cxt, sym("tokens"), tokens);
  set_slot(parse_cxt, sym("errors"), new_array());

  Object *begin_set = 0;

  begin_set = new_array();
  push(begin_set, TOK_OPEN_MAP);
  push(begin_set, TOK_OPEN_BLOCK);
  push(begin_set, TOK_IDENT);
  push(begin_set, TOK_ASSIGN);
  push(begin_set, TOK_INT);
  set_slot(parse_cxt, sym("expr_or_assign_begin_set"), begin_set);

  begin_set = new_array();
  set_slot(parse_cxt, sym("stmt_begin_set"), begin_set);
  push(begin_set, TOK_IF);
  push(begin_set, TOK_WHILE);
  push(begin_set, TOK_OPEN_MAP);
  push(begin_set, TOK_OPEN_BLOCK);
  push(begin_set, TOK_IDENT);
  push(begin_set, TOK_INT);

  begin_set = new_array();
  set_slot(parse_cxt, sym("expr_begin_set"), begin_set);
  push(begin_set, TOK_IF);
  push(begin_set, TOK_WHILE);
  push(begin_set, TOK_OPEN_MAP);
  push(begin_set, TOK_OPEN_BLOCK);
  push(begin_set, TOK_IDENT);

  return parse_cxt;
};

Object *curr(Object *parse_cxt) {
  Fixnum pos = fixnum(get_slot(parse_cxt, sym("pos")));
  Object *tokens = get_slot(parse_cxt, sym("tokens"));
  if ( pos < array_length(tokens) ) {
    Object *tok = get_at(tokens, pos);
    return tok;
  }
  else {
    return 0;
  }
}

int have(Object *parse_cxt, Object *type) {
  Fixnum pos = fixnum(get_slot(parse_cxt, sym("pos")));
  Object *tokens = get_slot(parse_cxt, sym("tokens"));
  if ( pos < array_length(tokens) ) {
    Object *tok = get_at(tokens, pos);
    if ( get_slot(tok, sym("type")) == type ) {
      return 1;
    }
  }
  return 0;
};

int have_k(Object *parse_cxt, Object *type, Fixnum k) {
  Fixnum pos = fixnum(get_slot(parse_cxt, sym("pos")))+k;
  Object *tokens = get_slot(parse_cxt, sym("tokens"));
  if ( pos < array_length(tokens) ) {
    Object *tok = get_at(tokens, pos);
    Object *tok_type = get_slot(tok, sym("type"));
    if ( tok_type == type ) {
      return 1;
    }
  }
  return 0;
};

int have_set(Object *parse_cxt, Object* type_set) {
  Fixnum i;
  for(i=0;i<array_length(type_set);++i) {
    if ( have(parse_cxt, get_at(type_set, i)) ) {
      return 1;
    }
  }
  return 0;
};

int mustbe(Object *parse_cxt, Object *type) {
  Fixnum pos = fixnum(get_slot(parse_cxt, sym("pos")));
  Object *tokens = get_slot(parse_cxt, sym("tokens"));
  if ( pos >= array_length(tokens) ) {
    parse_error(parse_cxt, "Ran into the end of input stream");
    return 0;
  }

  Object *tok = get_at(tokens, pos);
  if ( get_slot(tok, SYM_TYPE) != type ) {
    parse_error(parse_cxt, "Unexpected token");
    return 0;
  }

  advance(parse_cxt);
  return 1;
};

Object *advance_one(Object *parse_cxt) {
  Fixnum pos = fixnum(get_slot(parse_cxt, sym("pos")));
  Object *obj = get_at(get_slot(parse_cxt, sym("tokens")), pos);
  set_slot(parse_cxt, sym("pos"), object(pos+1));
  return obj;
};

Object *advance(Object *parse_cxt) {
  Object *curr = advance_one(parse_cxt);
  while ( have(parse_cxt, TOK_EOL) ) {
    advance_one(parse_cxt);
  }
  return curr;
}

Object* new_parse_node(Object *type) {
  Object *node = new_array();
  set_slot(node, sym("type"), type);
  return node;
};

// ----------------------------------------------------------------------
// Parser

Object* parse_block(Object *parse_cxt);
Object* parse_program(Object *parse_cxt);
Object* parse_stmt_list(Object *parse_cxt);
Object* parse_stmt(Object* parse_cxt);
Fixnum  have_arg_list(Object *parse_cxt);
Object* parse_arg_list(Object *parse_cxt);
Object* parse_expr(Object *parse_cxt, Object *expr);
Object* parse_expr_group(Object *parse_cxt);
Object* parse_if_expr(Object *parse_cxt);
Object* parse_while_expr(Object *parse_cxt);
Object* parse_expr(Object *parse_cxt);
Object *parse_group_expr(Object *parse_cxt);

Object *parse_program(Object *parse_cxt) {
  Object *program    = new_parse_node(sym("program"));
  set_slot(program, sym("block"), parse_block(parse_cxt));
  return program;
}

Fixnum have_arg_list(Object *parse_cxt) {
  Fixnum k = 0;
  while(have_k(parse_cxt, TOK_IDENT, k)) k++;
  if ( have_k(parse_cxt, TOK_PIPE, k) ) {
    return 1;
  }
  else {
    return 0;
  }
}

Object* parse_arg_list(Object *parse_cxt) {
  Object *arg_list = new_parse_node(sym("arg_list"));
  while ( have(parse_cxt, TOK_IDENT) ) {
    push(arg_list, advance(parse_cxt));
  };
  mustbe(parse_cxt, TOK_PIPE);
  return arg_list;
}

Object* parse_block(Object *parse_cxt) {
  Object *block = new_parse_node(sym("block"));
  if ( ! mustbe(parse_cxt, TOK_OPEN_BLOCK) ) {
    return block;
  }
  if ( have_arg_list(parse_cxt) ) {
    set_slot(block, sym("arg_list"), parse_arg_list(parse_cxt));
  }
  set_slot(block, sym("stmt_list"), parse_stmt_list(parse_cxt));
  if ( ! mustbe(parse_cxt, TOK_CLOSE_BLOCK) ) {
    return block;
  }
  return block;
};

Object* parse_stmt_list(Object *parse_cxt) {
  Object *stmt_list = new_parse_node(sym("stmt_list"));
  Fixnum have_semi = true;
  while ( have_semi && 
          have_set(parse_cxt, get_slot(parse_cxt, sym("stmt_begin_set"))) 
  ) {
    push(stmt_list, parse_stmt(parse_cxt));
    if ( have(parse_cxt, TOK_SEMI) ) {
      advance(parse_cxt);
    } 
    else {
      have_semi = false;
    }
  };
  return stmt_list;
}

Object* parse_stmt(Object* parse_cxt) {
  if ( have(parse_cxt, TOK_IF) ) {
    return parse_if_expr(parse_cxt);
  }
  if ( have(parse_cxt, TOK_WHILE) ) {
    return parse_while_expr(parse_cxt);
  }
  else {
    return parse_expr(parse_cxt, new_parse_node(sym("expr")));
  }
}

Object* parse_while_expr(Object* parse_cxt) {
  Object *while_expr = new_parse_node(sym("while_expr"));
  if ( ! mustbe(parse_cxt, TOK_WHILE) ) {
    return while_expr;
  }
  set_slot(while_expr, sym("cond"), parse_group_expr(parse_cxt));
  if ( ! have(parse_cxt, TOK_OPEN_BLOCK) ) {
    parse_error(parse_cxt, "expected block");
    return while_expr;
  }
  set_slot(while_expr, sym("block"), parse_block(parse_cxt));
  return while_expr;
}

Object* parse_if_expr(Object* parse_cxt) {
  Object *if_expr = new_parse_node(sym("if_expr"));
  if ( ! mustbe(parse_cxt, TOK_IF) ) {
    return if_expr;
  }
  set_slot(if_expr, sym("cond"), parse_group_expr(parse_cxt));
  if ( ! have(parse_cxt, TOK_OPEN_BLOCK) ) {
    parse_error(parse_cxt, "expected block");
    return if_expr;
  }
  set_slot(if_expr, sym("block"), parse_block(parse_cxt));
  if ( have(parse_cxt, TOK_ELSE) ) {
    advance(parse_cxt);
    if ( ! have(parse_cxt, TOK_OPEN_BLOCK) ) {
      parse_error(parse_cxt, "expected block");
      return if_expr;
    }
    set_slot(if_expr, sym("else_block"), parse_block(parse_cxt));
  }
  return if_expr;
}

Object *parse_group_expr(Object *parse_cxt) {
  Object *expr = new_parse_node(sym("group_expr"));
  mustbe(parse_cxt, TOK_OPEN_GROUP);
  parse_expr(parse_cxt, expr);
  mustbe(parse_cxt, TOK_CLOSE_GROUP);
  return expr;
}

Object *parse_expr(Object *parse_cxt, Object *expr) {
  while ( true ) {
    if ( have(parse_cxt, TOK_IDENT) ) {
      push(expr, advance(parse_cxt));
    }
    else if ( have(parse_cxt, TOK_OPEN_GROUP) ) {
      push(expr, parse_group_expr(parse_cxt));
    }
    else if ( have(parse_cxt, TOK_OPEN_BLOCK) ) {
      push(expr, parse_block(parse_cxt));
    }
    else if ( have(parse_cxt, TOK_ASSIGN) ) {
      push(expr, advance(parse_cxt));
    }
    else if ( have(parse_cxt, TOK_STRING) ) {
      push(expr, advance(parse_cxt));
    }
    else if ( have(parse_cxt, TOK_INT) ) {
      push(expr, advance(parse_cxt));
    }
    else {
      break;
    }
  }
  return expr;
}

// ----------------------------------------------------------------------
// Code Gen

Object *code_gen_error(Object *cxt, char const* reason);
Object *new_code_gen_cxt();
Fixnum is_type(Object *cand, Object *type);
Object *code_gen_program(Object *cxt, Object *program);
Object *code_gen_block(Object *cxt, Object *block_node, Object *block, Fixnum include_ret);
Object *code_gen_stmt_list(Object *cxt, Object *block, Object *stmt_list);
Object *code_gen_expr(Object *cxt, Object *block, Object *expr);
Object *code_gen_if_expr(Object *cxt, Object *block, Object *if_expr);
Object *code_gen_while_expr(Object *cxt, Object *block, Object *if_expr);

Object *new_code_gen_cxt() {
  Object *code_gen_cxt = new_object();
  set_slot(code_gen_cxt, sym("errors"), new_array());
  return code_gen_cxt;
}

void code_gen_error(Object *cxt, Object *node, char const* reason) {
  Object *error = new_object();
  set_slot(error, sym("node"), node);
  set_slot(error, sym("text"), new_string(reason));
  push(get_slot(cxt, sym("errors")), error);
}

Fixnum is_type(Object *cand, Object *type) {
  if ( cand != 0 && is_object(cand) ) {
    Object *cand_type = get_slot(cand, sym("type"));
    if ( cand_type == type ) {
      return 1;
    }
    else {
      return 0;
    }
  }
  else {
    return 0;
  }
}

Object *code_gen_program(Object *cxt, Object *program) {
  Object *block_node = get_slot(program, sym("block"));
  if ( is_type(block_node, sym("block")) ) {
    return code_gen_block(cxt, block_node, 0, 1);
  }
  else {
    return 0;
  }
}

Object *code_gen_arg_list(Object *cxt, Object *block, Object *arg_list)
{
  Object *args = new_array();
  if ( arg_list != 0 && is_type(arg_list, sym("arg_list")) ) {
    for(int i=0; i<array_length(arg_list); ++i) {
      Object *arg = get_at(arg_list, i);
      if ( is_type(arg, TOK_IDENT) ) {
        push(args, sym(get_slot(arg, sym("value"))));
      }
    }
  }
  set_slot(block, SYM_ARGS, args);
  return block;
}

Object *code_gen_block(Object *cxt, Object *block_node, Object *block, Fixnum include_ret) {
  if ( block == 0 ) {
    block = new_block();
    Object *arg_list = get_slot(block_node, sym("arg_list"));
    code_gen_arg_list(cxt, block, arg_list);
  }
  Object *stmt_list_node = get_slot(block_node, sym("stmt_list"));
  if ( is_type(stmt_list_node, sym("stmt_list")) ) {
    code_gen_stmt_list(cxt, block, stmt_list_node);
  }
  if ( include_ret ) {
    push(block, object(INST_RET));
  }
  return block;
}

Object *code_gen_stmt_list(Object *cxt, Object *block, Object *stmt_list) {
  for(int i=0; i<array_length(stmt_list); ++i) {
    Object *expr = get_at(stmt_list, i);
    if ( is_type(expr, sym("expr")) ) {
      code_gen_expr(cxt, block, expr);
    }
    else if ( is_type(expr, sym("if_expr")) ) {
      code_gen_if_expr(cxt, block, expr);
    }
    else if ( is_type(expr, sym("while_expr")) ) {
      code_gen_while_expr(cxt, block, expr);
    }
  }
  return block;
}

void code_gen_truthiness(Object *cxt, Object *block)
{
  // test for truthiness
  push(block, object(INST_PUSH));
  push(block, sym("is_true"));
  push(block, object(INST_SEND));
};

Object *code_gen_if_expr(Object *cxt, Object *block, Object *if_expr) {
  Object *cond       = get_slot(if_expr, sym("cond"));
  Object *if_block   = get_slot(if_expr, sym("block"));
  Object *else_block = get_slot(if_expr, sym("else_block"));
  
  Fixnum else_jmp_index;
  Fixnum if_jmp_index;

  code_gen_expr(cxt, block, cond);
  code_gen_truthiness(cxt, block);

  // jump to else block if zero
  push(block, object(INST_JMPZ));
  if_jmp_index = array_length(block);
  push(block, 0);   // place holder

  // otherwise perform the if block
  code_gen_block(cxt, if_block, block, 0);

  // skip the else block
  if ( else_block ) {
    push(block, object(INST_JMP));
    else_jmp_index = array_length(block);
    push(block, 0); // place holder
  
    set_at(block, if_jmp_index, object(array_length(block)));

    code_gen_block(cxt, else_block, block, 0);

    set_at(block, else_jmp_index, object(array_length(block)));
  }
  else {
    set_at(block, if_jmp_index, object(array_length(block)));
  }

  return block;
}

Object *code_gen_while_expr(Object *cxt, Object *block, Object *while_expr) {
  Object *cond        = get_slot(while_expr, sym("cond"));
  Object *while_block = get_slot(while_expr, sym("block"));
  
  Fixnum while_jmp_index;
  Fixnum while_cond_index;

  while_cond_index = array_length(block);

  code_gen_expr(cxt, block, cond);
  code_gen_truthiness(cxt, block);

  // jump to tail if zero
  push(block, object(INST_JMPZ));
  while_jmp_index = array_length(block);
  push(block, 0);   // place holder

  // otherwise perform the while block
  code_gen_block(cxt, while_block, block, 0);

  push(block, object(INST_JMP));
  push(block, object(while_cond_index));

  // set the break condition index
  set_at(block, while_jmp_index, object(array_length(block)));

  return block;
}

Object *code_gen_expr(Object *cxt, Object *block, Object *expr) {
  Fixnum i, j;
  Object *stack = new_array();
  Fixnum num_assignments = 0;
  for(i=0; i<array_length(expr); ++i) {
    Object *elem = get_at(expr, i);
    if ( is_type(elem, TOK_IDENT) ) {
      push(stack, sym(get_slot(elem, sym("value"))));
    }
    else if ( is_type(elem, TOK_STRING) ) {
      push(stack, get_slot(elem, sym("value")));
    }
    else if ( is_type(elem, TOK_INT) ) {
      push(stack, get_slot(elem, sym("value")));
    }
    else if ( is_type(elem, sym("group_expr")) ) {
      push(stack, code_gen_expr(cxt, new_block(), elem));
    }
    else if ( is_type(elem, TOK_ASSIGN) ) {
      if ( num_assignments > 0 ) {
	code_gen_error(cxt, expr, "Only a single assignment is permitted in an expr");
	return block;
      }
      // make the previous ident a setter and those before it existers
      j = array_length(stack) - 1;
      num_assignments += 1;
      if ( j >= 0 ) {
	if ( is_sym(get_at(stack, j)) ) {
	  set_at(stack, j, setter_sym(get_at(stack, j)));
	}
	else {
	  // insert code here to perform assignment where slot name is an expr
	  abort();
	}
	j--;
      }
      while ( j >= 0 ) {
	if ( is_sym(get_at(stack, j)) ) {
	  set_at(stack, j, exister_sym(get_at(stack, j)));
	}
	else {
	  // insert code here to perform existence where slot name is an expr
	  abort();
	}
	--j;
      }
      push(stack, SendMarker);
    }
  }
  for(i=array_length(stack)-1;i>=0;--i) {
    Object *elem = get_at(stack, i);
    if ( is_block(elem) ) {
      for(j=0;j<array_length(elem);++j) {
	push(block, get_at(elem, j));
      }
    }
    else if ( elem == SendMarker ) {
      push(block, object(INST_SEND));
    }
    else {
      push(block, object(INST_PUSH));
      push(block, elem);
    }
  }
  push(block, object(INST_SEND));
  return block;
};

// ----------------------------------------------------------------------
// Native function interface
// ----------------------------------------------------------------------

Object *File = new_object();
Object *Function = new_object();

Object *new_function(native_function_ptr f) {
  Object *obj = new_object(Function);
  obj->buffer = new_buffer(sizeof(f), (char *)&f);
  return obj;
};

Fixnum is_function(Object *obj) {
  return is_object(obj) && obj->proto == Function ? 1 : 0;
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
    return result;
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

Object *native_sys_print(Object *frame) {
  Object *stack = get_slot(frame, SYM_STACK);
  Object *arg = pop(stack);
  if ( is_sym(arg) ) {
    arg = frame_resolve(frame, arg);
  }
  if ( is_string(arg) ) {
    fwrite(arg->buffer->data, arg->buffer->length, 1, stdout);
    return frame;
  }
  else {
    return new_exception_frame(frame, "Expected string argument");
  }
}

Object *native_not(Object *frame) {
  Object *stack       = get_slot(frame, SYM_STACK);
  Object *arg         = pop(stack);
  if ( is_sym(arg) ) {
    arg = frame_resolve(frame, arg);
  }
  if ( arg == 0 || arg == frame_resolve(frame, SYM_FALSE) ) {
    push(stack, frame_resolve(frame, SYM_TRUE));
  }
  else {
    push(stack, 0);
  }
  return frame;
}

Object *native_is_true(Object *frame) {
  Object *stack       = get_slot(frame, SYM_STACK);
  Object *arg         = pop(stack);
  if ( is_sym(arg) ) {
    arg = frame_resolve(frame, arg);
  }
  if ( arg == 0 || arg == frame_resolve(frame, SYM_FALSE) ) {
    push(stack, 0);
  }
  else {
    push(stack, frame_resolve(frame, SYM_TRUE));
  }
  return frame;
}

// ----------------------------------------------------------------------
// Sys

Object *new_sys() {
  Object *sys = new_object();
  set_slot(sys, sym("print"), new_function(&native_sys_print));
  return sys;
}

Object* run_program(Object *block) {
  Object *parent_block = new_block();
  Object *parent_frame = new_frame(0, parent_block);
  Object *parent_stack = get_slot(parent_frame, SYM_STACK);
  Object *sys = new_sys();
  Object *parent_local = get_slot(parent_frame, SYM_LOCAL);

  set_slot(block, SYM_LEXICAL_FRAME, parent_frame);
  set_slot(parent_local, sym("not"), new_function(&native_not));
  set_slot(parent_local, sym("is_true"), new_function(&native_is_true));
  set_slot(parent_local, SYM_TRUE,  new_object());
  set_slot(parent_local, SYM_FALSE, new_object());

  push(parent_stack, sys);
  push(parent_stack, block);
  push(parent_block, object(INST_SEND));
  push(parent_block, object(INST_TERM));
  interpret(parent_frame);
  return parent_frame;
}

// ----------------------------------------------------------------------
// Main

int main(int argc, char **argv) {
  set_slot(File, SYM_OPEN, new_function(&file_open));
  set_slot(File, SYM_READ_LINE, new_function(&file_read_line));

  Fixnum i, j, k;
  Object *options = new_object();
  Object *value   = 0;

  for(i=1;i<argc; ++i) {
    if ( argv[i][0] == '-' ) {
      for(j=0;argv[i][j]=='-';++j);
      for(k=j;argv[i][k]!=0&&argv[i][k]!='=';++k);
      Object *key   = new_string(argv[i] + j, k-j);
      Object *value = new_string(argv[i] + k + 1);
      set_slot(options, sym(key), value);
    }
  }

  if ( (value = get_slot(options, sym("test"))) != 0 ) {
    test_array();
    test_push_pop();
    test_send();
  }
  else if ( (value = get_slot(options, sym("interpret"))) != 0 ) {
    char buf[4096];
    string_to_buffer(value, buf, sizeof(buf));
    FILE *input = fopen(buf, "r");
    Object *block = load(input);
    Object *frame = new_frame(0, block);
    interpret(frame);
    dump_object(frame, 0, 0);
  }
  else if ( (value = get_slot(options, sym("lex"))) != 0 ) {
    char buf[4096];
    string_to_buffer(value, buf, sizeof(buf));
    FILE *input = fopen(buf, "r");
    print_tokens(token_stack(input));
  }
  else if ( (value = get_slot(options, sym("parse"))) != 0 ) {
    char buf[4096];
    string_to_buffer(value, buf, sizeof(buf));
    FILE *input = fopen(buf, "r");
    Object *result = parse_program(new_parse_cxt(token_stack(input)));
    dump_object(result, 0, 0);
  }
  else if ( (value = get_slot(options, sym("code_gen"))) ) {
    char buf[4096];
    string_to_buffer(value, buf, sizeof(buf));
    FILE *input = fopen(buf, "r");
    Object *parse_tree = parse_program(new_parse_cxt(token_stack(input)));
    Object *code = code_gen_program(new_code_gen_cxt(), parse_tree);
    dump_object(code, 0, 0);
  }
  else if ( (value = get_slot(options, sym("run"))) ) {
    char buf[4096];
    string_to_buffer(value, buf, sizeof(buf));
    FILE *input = fopen(buf, "r");
    Object *parse_tree = parse_program(new_parse_cxt(token_stack(input)));
    Object *code = code_gen_program(new_code_gen_cxt(), parse_tree);
    run_program(code);
  }
  else {
    printf("Nothing to do!\n");
  }
}




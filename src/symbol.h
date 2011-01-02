Object* new_sym(char const* s) {
  Object *obj = new_object();
  obj->buffer = (Buffer *) new_string_buffer(s);
  return obj;
};

#define SYM(name) \
  Object *const name = new_sym(#name)

SYM(True);
SYM(False);
SYM(Push);
SYM(Pc);
SYM(Send);
SYM(Jmp);
SYM(JmpZ);
SYM(JmpNZ);
SYM(Code);
SYM(Stack);
SYM(Ret);
SYM(Term);
SYM(Block);
SYM(Parent);
SYM(Frame);
SYM(Local);
SYM(Self);
SYM(LexicalParent);
SYM(Print);
SYM(Arg);
SYM(Sys);



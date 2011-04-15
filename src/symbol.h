#define SYM(name) \
  Object *const name = new_object();

SYM(Push);
SYM(PushFrame);
SYM(PushBlock);
SYM(Pc);
SYM(Send);
SYM(Jmp);
SYM(JmpTrue);
SYM(JmpNotTrue);
SYM(Code);
SYM(Stack);
SYM(Return);
SYM(LocalReturn);
SYM(Term);
SYM(Block);
SYM(Frame);
SYM(Local);
SYM(Self);
SYM(Arg);
SYM(Sys);
SYM(Reason);
SYM(Catch);
Object *True = new_object();
Object *False = new_object();

void init_general_symbols() {
  add_sym(Push,      "push");
  add_sym(PushFrame, "push_frame");
  add_sym(PushBlock, "push_block");
  add_sym(Pc,        "pc");
  add_sym(Send,    "send");
  add_sym(Jmp,     "jmp");
  add_sym(JmpTrue, "jmptrue");
  add_sym(JmpNotTrue,   "jmpnottrue");
  add_sym(Code,    "code");
  add_sym(Stack,   "stack");
  add_sym(Return,  "return");
  add_sym(LocalReturn,  "local_return");
  add_sym(Term,    "term");
  add_sym(Block,   "block");
  add_sym(Frame,   "frame");
  add_sym(Local,   "local");
  add_sym(Self,    "self");
  add_sym(Arg,       "arg");
  add_sym(Sys,       "sys");
  add_sym(Nil,       "nil");
  add_sym(Undefined, "undefined");
  add_sym(Dirty,     "dirty");
  add_sym(Reason,    "reason");
  add_sym(Catch,     "catch");
  add_sym(True,   "True");
  add_sym(False,  "False");
  add_sym(Parent, "parent");
}


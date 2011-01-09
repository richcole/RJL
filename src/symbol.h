#define SYM(name) \
  Object *const name = new_object();

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
SYM(Frame);
SYM(Local);
SYM(Self);
SYM(LexicalParent);
SYM(Print);
SYM(Arg);
SYM(Sys);
SYM(Reason);

void init_general_symbols() {
  add_sym(Push,    "push");
  add_sym(Pc,      "pc");
  add_sym(Send,    "send");
  add_sym(Jmp,     "jmp");
  add_sym(JmpZ,    "jmpz");
  add_sym(JmpNZ,   "jmpnz");
  add_sym(Code,    "code");
  add_sym(Stack,   "stack");
  add_sym(Ret,     "ret");
  add_sym(Term,    "term");
  add_sym(Block,   "block");
  add_sym(Frame,   "frame");
  add_sym(Local,   "local");
  add_sym(Self,    "self");
  add_sym(LexicalParent, "lexical_parent");
  add_sym(Print,   "print");
  add_sym(Arg,     "arg");
  add_sym(Sys,     "sys");
	add_sym(Nil,     "nil");
	add_sym(Dirty,   "dirty");
	add_sym(Reason,  "reason");
}


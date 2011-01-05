#define SYM(name) \
  Object *const name = new_object;

void init_symbols() {
  add_sym(True,    "True");
  add_sym(False,   "False");
  add_sym(Push,    "push");
  add_sym(Pc,      "pc");
  add_sym(Send,    "send");
  add_sym(Jmp,     "jmp");
  add_sym(JmpZ,    "jmpz");
  add_sym(JmpNZ,   "jmpz");
  add_sym(Code,    "code");
  add_sym(Stack,   "stack");
  add_sym(Ret,     "ret");
  add_sym(Term,    "term");
  add_sym(Block,   "block");
  add_sym(Parent,  "parent");
  add_sym(Frame,   "frame");
  add_sym(Local,   "local");
  add_sym(Self,    "self");
  add_sym(LexicalParent, "lexical_parent");
  add_sym(Print,   "print");
  add_sym(Arg,     "arg");
  add_sym(Sys,     "sys");
}


program = block_expr ;

block = BLOCK_OPEN [ ident_list ] stmt_list BLOCK_CLOSE ;

ident_list = IDENT* PIPE

stmt_list = [ stmt ] [ SEMI stmt_list ]

stmt = if_stmt | while_stmt | expr

expr = block | ident_expr | assign_expr

ident_expr  = IDENT expr

assign_expr = ASSIGN expr






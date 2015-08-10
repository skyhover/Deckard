goal:  compilation_unit
		{}
;

/* 19.3 Productions from 3: Lexical structure  */
literal:
	INT_LIT_TK
|	FP_LIT_TK
|	BOOL_LIT_TK
|	CHAR_LIT_TK
|	STRING_LIT_TK
|	NULL_TK
;

/* 19.4 Productions from 4: Types, Values and Variables  */
type:
	primitive_type
|	reference_type
;

primitive_type:
	integral
|	float
|	BOOLEAN_TK
;

integral:
	BYTE_TK
|	SHORT_TK
|	INT_TK
|	LONG_TK
|	CHAR_TK
|	INTEGRAL_TK
;

float:
	FLOAT_TK
|	DOUBLE_TK
|	FP_TK
;

reference_type:
	class_or_interface_type
|	array_type
|	class_or_interface_array_type
		{ /* TODO */ }
;

class_or_interface_type:
	name
|	name type_list
		{ /* TODO */ }
;

class_type:
	class_or_interface_type	/* Default rule */
;

interface_type:
	class_or_interface_type
;

array_type:
	primitive_type dims
		{
		  int osb = pop_current_osb (ctxp);
		  tree t = build_java_array_type (($1), -1);
		  while (--osb)
		    t = build_unresolved_array_type (t);
		  $$ = t;
		}
|	name dims
		{
		  int osb = pop_current_osb (ctxp);
		  tree t = $1;
		  while (osb--)
		    t = build_unresolved_array_type (t);
		  $$ = t;
		}
;

class_or_interface_array_type:
	class_or_interface_type dims
		{ /* TODO */ }
;

/* 19.5 Productions from 6: Names  */
name:
	simple_name		/* Default rule */
|	qualified_name		/* Default rule */
;

simple_name:
	identifier		/* Default rule */
;

qualified_name:
	name DOT_TK identifier
		{ $$ = make_qualified_name ($1, $3, $2.location); }
;

identifier:
	ID_TK
;

/* 19.6: Production from 7: Packages  */
compilation_unit:
		{$$ = NULL;}
|	package_declaration
|	import_declarations
|	type_declarations
|	package_declaration import_declarations
|	package_declaration type_declarations
|	import_declarations type_declarations
|	package_declaration import_declarations type_declarations
;

import_declarations:
	import_declaration
		{
		  $$ = NULL;
		}
|	import_declarations import_declaration
		{
		  $$ = NULL;
		}
;

type_declarations:
	type_declaration
| 	type_declarations type_declaration
;

package_declaration:
	PACKAGE_TK name SC_TK
		{
		  ctxp->package = EXPR_WFL_NODE ($2);
		  register_package (ctxp->package);
		}
|	annotations PACKAGE_TK name SC_TK
		{ /* TODO */ }
|	PACKAGE_TK error
		{yyerror ("Missing name"); RECOVER;}
|	PACKAGE_TK name error
		{yyerror ("';' expected"); RECOVER;}
|	annotations PACKAGE_TK error
		{yyerror ("Missing name"); RECOVER;}
|	annotations PACKAGE_TK name error
		{yyerror ("';' expected"); RECOVER;}
;

import_declaration:
	single_type_import_declaration
|	type_import_on_demand_declaration
;

single_type_import_declaration:
	IMPORT_TK name SC_TK
		{
		  tree name = EXPR_WFL_NODE ($2), last_name;
		  int   i = IDENTIFIER_LENGTH (name)-1;
		  const char *last = &IDENTIFIER_POINTER (name)[i];
		  while (last != IDENTIFIER_POINTER (name))
		    {
		      if (last [0] == '.')
			break;
		      last--;
		    }
		  last_name = get_identifier (++last);
		  if (IS_A_SINGLE_IMPORT_CLASSFILE_NAME_P (last_name))
		    {
		      tree err = find_name_in_single_imports (last_name);
		      if (err && err != name)
			parse_error_context
			  ($2, "Ambiguous class: `%s' and `%s'",
			   IDENTIFIER_POINTER (name),
			   IDENTIFIER_POINTER (err));
		      else
			REGISTER_IMPORT ($2, last_name);
		    }
		  else
		    REGISTER_IMPORT ($2, last_name);
		}
|	IMPORT_TK STATIC_TK name SC_TK
		{ /* TODO */ }
|	IMPORT_TK error
		{yyerror ("Missing name"); RECOVER;}
|	IMPORT_TK name error
		{yyerror ("';' expected"); RECOVER;}
|	IMPORT_TK STATIC_TK error
		{yyerror ("Missing name"); RECOVER;}
|	IMPORT_TK STATIC_TK name error
		{yyerror ("';' expected"); RECOVER;}
;

type_import_on_demand_declaration:
	IMPORT_TK name DOT_TK MULT_TK SC_TK
		{
		  tree name = EXPR_WFL_NODE ($2);
		  tree it;
		  /* Search for duplicates. */
		  for (it = ctxp->import_demand_list; it; it = TREE_CHAIN (it))
		    if (EXPR_WFL_NODE (TREE_PURPOSE (it)) == name)
		      break;
		  /* Don't import the same thing more than once, just ignore
		     duplicates (7.5.2) */
		  if (! it)
		    {
		      read_import_dir ($2);
		      ctxp->import_demand_list =
			chainon (ctxp->import_demand_list,
				 build_tree_list ($2, NULL_TREE));
		    }
		}
|	IMPORT_TK STATIC_TK name DOT_TK MULT_TK SC_TK
		{ /* TODO */ }
|	IMPORT_TK name DOT_TK error
		{yyerror ("'*' expected"); RECOVER;}
|	IMPORT_TK name DOT_TK MULT_TK error
		{yyerror ("';' expected"); RECOVER;}
|	IMPORT_TK STATIC_TK name DOT_TK error
		{yyerror ("'*' expected"); RECOVER;}
|	IMPORT_TK STATIC_TK name DOT_TK MULT_TK error
		{yyerror ("';' expected"); RECOVER;}
;

annotations:
	annotation
		{ /* TODO */ }
|	annotations annotation
		{ /* TODO */ }
;

annotation:
	AT_TK annotation_name
		{ /* TODO */ }
|	AT_TK annotation_name OP_TK element_value CP_TK
		{ /* TODO */ }
|	AT_TK annotation_name OP_TK element_value_pairs CP_TK
		{ /* TODO */ }
|	AT_TK error
		{yyerror ("Annotation name expected"); RECOVER;}
|	AT_TK annotation_name OP_TK error
		{yyerror ("')' expected"); RECOVER;}
;

annotation_name:
	name
		{ /* TODO */ }
;

element_value_pairs:
	element_value_pair
		{ /* TODO */ }
|	element_value_pairs C_TK element_value_pair
		{ /* TODO */ }
|	element_value_pairs C_TK error
		{yyerror ("Key-value pair expected"); RECOVER;}
;

element_value_pair:
	identifier EQ_TK element_value
		{ /* TODO */ }
;

element_value:
	expression
		{ /* TODO */ }
|	annotation
		{ /* TODO */ }
|	element_value_array_initializer
		{ /* TODO */ }
;

element_value_array_initializer:
	OCB_TK element_value_pairs C_TK CCB_TK
		{ /* TODO */ }
|	OCB_TK element_value_pairs CCB_TK
		{ /* TODO */ }
|	OCB_TK CCB_TK
		{ /* TODO */ }
;

annotation_type_declaration:
	AT_TK INTERFACE_TK identifier annotation_type_body
		{ /* TODO */ }
;

annotation_type_body:
	OCB_TK annotation_type_element_declarations CCB_TK
		{ /* TODO */ }
	OCB_TK SC_TK CCB_TK
		{ /* TODO */ }
	OCB_TK annotation_type_element_declarations error
		{yyerror ("'}' expected"); RECOVER;}
;

annotation_type_element_declarations:
	annotation_type_element_declaration
		{ /* TODO */ }
|	annotation_type_element_declarations annotation_type_element_declaration
		{ /* TODO */ }
;

annotation_type_element_declaration:
	annotation_type_element_rest
		{ /* TODO */ }
|	modifiers annotation_type_element_rest
		{ /* TODO */ }
;

annotation_type_element_rest:
	type annotation_method_or_constant_rest SC_TK
		{ /* TODO */ }
|	annotation_type_element_non_delimited SC_TK
		{ /* TODO */ }
|	annotation_type_element_non_delimited
		{ /* TODO */ }
;

annotation_type_element_non_delimited:
	class_declaration
		{ /* TODO */ }
|	interface_declaration
		{ /* TODO */ }
|	enum_declaration
		{ /* TODO */ }
|	annotation_type_declaration
		{ /* TODO */ }
;

annotation_method_or_constant_rest:
	annotation_method_rest
		{ /* TODO */ }
|	annotation_constant_rest
		{ /* TODO */ }
;

annotation_method_rest:
	identifier OP_TK CP_TK default_value
		{ /* TODO */ }
;

annotation_constant_rest:
	variable_declarators
		{ /* TODO */ }
;

default_value:
	DEFAULT_TK element_value
		{ /* TODO */ }
;

type_declaration:
	class_declaration
		{ end_class_declaration (0); }
|	interface_declaration
		{ end_class_declaration (0); }
|	enum_declaration
		{ /* TODO */ }
|	annotation_type_declaration
		{ /* TODO */ }
|	empty_statement
|	error
		{
		  YYERROR_NOW;
		  yyerror ("Class or interface declaration expected");
		}
;

/* 19.7 Shortened from the original:
   modifiers: modifier | modifiers modifier
   modifier: any of public...  */
modifiers:
	 modifier
		{
		  $$ = (1 << $1);
		}
|	modifiers modifier
		{
		  int acc = (1 << $2);
		  if ($$ & acc)
		    parse_error_context
		      (ctxp->modifier_ctx [$2], "Modifier `%s' declared twice",
		       java_accstring_lookup (acc));
		  else
		    {
		      $$ |= acc;
		    }
		}
;
modifier:
	PUBLIC_TK
|	PRIVATE_TK
|	PROTECTED_TK
|	STATIC_TK
|	FINAL_TK
|	SYNCHRONIZED_TK
|	VOLATILE_TK
|	TRANSIENT_TK
|	NATIVE_TK
|	PAD_TK
|	ABSTRACT_TK
|	STRICT_TK
|	CONST_TK
|	MODIFIER_TK
|	annotation
		{ /* TODO */ }
;

/* 19.8.1 Production from $8.1: Class Declaration */
class_declaration:
	modifiers CLASS_TK identifier super interfaces
		{ create_class ($1, $3, $4, $5); }
	class_body
		{;}
|	CLASS_TK identifier super interfaces
		{ create_class (0, $2, $3, $4); }
	class_body
		{;}
|	modifiers CLASS_TK identifier type_list super interfaces
		{ /* TODO */ }
	class_body
		{ /* TODO */ }
|	CLASS_TK identifier type_list super interfaces
		{ /* TODO */ }
	class_body
		{ /* TODO */ }
|	modifiers CLASS_TK error
		{ yyerror ("Missing class name"); RECOVER; }
|	CLASS_TK error
		{ yyerror ("Missing class name"); RECOVER; }
|	CLASS_TK identifier error
		{
		  if (!ctxp->class_err) yyerror (" expected");
		  DRECOVER(class1);
		}
|	modifiers CLASS_TK identifier error
		{ if (!ctxp->class_err) yyerror (" expected"); RECOVER; }
;

type_list:
	LT_TK type_parameters GT_TK
		{ /* TODO */ }
|	LT_TK types GT_TK
		{ /* TODO */ }
|	LT_TK GT_TK
		{ /* TODO */ }
;

types:
	type
		{ /* TODO */ }
|	types C_TK type
		{ /* TODO */ }
;

type_parameters:
	type_parameter
		{ /* TODO */ }
|	type_parameters C_TK type_parameter
		{ /* TODO */ }
;

type_parameter:
	identifier
		{ /* TODO */ }
|	identifier EXTENDS_TK type_bound
		{ /* TODO */ }
|	identifier EXTENDS_TK error
		{ yyerror ("Type expected"); RECOVER; }
;

type_bound:
	type
		{ /* TODO */ }
|	type_bound AND_TK type
		{ /* TODO */ }
;

super:
		{ $$ = NULL; }
|	EXTENDS_TK class_type
		{ $$ = $2; }
|	EXTENDS_TK class_type error
		{yyerror (" expected"); ctxp->class_err=1;}
|	EXTENDS_TK error
		{yyerror ("Missing super class name"); ctxp->class_err=1;}
;

interfaces:
		{ $$ = NULL_TREE; }
|	IMPLEMENTS_TK interface_type_list
		{ $$ = $2; }
|	IMPLEMENTS_TK error
		{
		  ctxp->class_err=1;
		  yyerror ("Missing interface name");
		}
;

interface_type_list:
	interface_type
		{
		  ctxp->interface_number = 1;
		  $$ = build_tree_list ($1, NULL_TREE);
		}
|	interface_type_list C_TK interface_type
		{
		  ctxp->interface_number++;
		  $$ = chainon ($1, build_tree_list ($3, NULL_TREE));
		}
|	interface_type_list C_TK error
		{yyerror ("Missing interface name"); RECOVER;}
;

class_body:
	OCB_TK CCB_TK
		{
		  /* Store the location of the  when doing xrefs */
		  if (flag_emit_xref)
		    DECL_END_SOURCE_LINE (GET_CPC ()) =
		      EXPR_WFL_ADD_COL ($2.location, 1);
		  $$ = GET_CPC ();
		}
|	OCB_TK class_body_declarations CCB_TK
		{
		  /* Store the location of the  when doing xrefs */
		  if (flag_emit_xref)
		    DECL_END_SOURCE_LINE (GET_CPC ()) =
		      EXPR_WFL_ADD_COL ($3.location, 1);
		  $$ = GET_CPC ();
		}
;

class_body_declarations:
	class_body_declaration
|	class_body_declarations class_body_declaration
;

class_body_declaration:
	class_member_declaration
|	static_initializer
|	constructor_declaration
|	generic_constructor_declaration
|	block			/* Added, JDK1.1, instance initializer */
		{
		  if ($1 != empty_stmt_node)
		    {
		      TREE_CHAIN ($1) = CPC_INSTANCE_INITIALIZER_STMT (ctxp);
		      SET_CPC_INSTANCE_INITIALIZER_STMT (ctxp, $1);
		    }
		}
;

class_member_declaration:
	field_declaration
|	method_declaration
|	generic_method_declaration
|	class_declaration	/* Added, JDK1.1 inner classes */
		{ end_class_declaration (1); }
|	interface_declaration	/* Added, JDK1.1 inner interfaces */
		{ end_class_declaration (1); }
|	enum_declaration
		{ /* TODO */ }
|	empty_statement
;

/* 19.8.2 Productions from 8.3: Field Declarations  */
field_declaration:
	type variable_declarators SC_TK
		{ register_fields (0, $1, $2); }
|	modifiers type variable_declarators SC_TK
		{
		  check_modifiers
		    ("Illegal modifier `%s' for field declaration",
		     $1, FIELD_MODIFIERS);
		  check_modifiers_consistency ($1);
		  register_fields ($1, $2, $3);
		}
;

variable_declarators:
	/* Should we use build_decl_list () instead ? FIXME */
	variable_declarator	/* Default rule */
|	variable_declarators C_TK variable_declarator
		{ $$ = chainon ($1, $3); }
|	variable_declarators C_TK error
		{yyerror ("Missing term"); RECOVER;}
;

variable_declarator:
	variable_declarator_id
		{ $$ = build_tree_list ($1, NULL_TREE); }
|	variable_declarator_id ASSIGN_TK variable_initializer
		{
		  if (java_error_count)
		    $3 = NULL_TREE;
		  $$ = build_tree_list
		    ($1, build_assignment ($2.token, $2.location, $1, $3));
		}
|	variable_declarator_id ASSIGN_TK error
		{
		  yyerror ("Missing variable initializer");
		  $$ = build_tree_list ($1, NULL_TREE);
		  RECOVER;
		}
|	variable_declarator_id ASSIGN_TK variable_initializer error
		{
		  yyerror ("';' expected");
		  $$ = build_tree_list ($1, NULL_TREE);
		  RECOVER;
		}
;

variable_declarator_id:
	identifier
|	variable_declarator_id OSB_TK CSB_TK
		{ $$ = build_unresolved_array_type ($1); }
|	identifier error
		{yyerror ("Invalid declaration"); DRECOVER(vdi);}
|	variable_declarator_id OSB_TK error
		{
		  yyerror ("']' expected");
		  DRECOVER(vdi);
		}
|	variable_declarator_id CSB_TK error
		{yyerror ("Unbalanced ']'"); DRECOVER(vdi);}
;

variable_initializer:
	expression
|	array_initializer
;

/* 19.8.3 Productions from 8.4: Method Declarations  */
method_declaration:
	method_header
		{
		  current_function_decl = $1;
		  if (current_function_decl
		      && TREE_CODE (current_function_decl) == FUNCTION_DECL)
		    source_start_java_method (current_function_decl);
		  else
		    current_function_decl = NULL_TREE;
		}
	method_body
		{ finish_method_declaration ($3); }
|	method_header error
		{YYNOT_TWICE yyerror (" expected"); RECOVER;}
;

generic_method_declaration:
	type_list method_declaration
		{ /* TODO */ }
;

method_header:
	type method_declarator throws
		{ $$ = method_header (0, $1, $2, $3); }
|	VOID_TK method_declarator throws
		{ $$ = method_header (0, void_type_node, $2, $3); }
|	modifiers type method_declarator throws
		{ $$ = method_header ($1, $2, $3, $4); }
|	modifiers VOID_TK method_declarator throws
		{ $$ = method_header ($1, void_type_node, $3, $4); }
|	type error
		{
		  yyerror ("Invalid method declaration, method name required");
		  RECOVER;
		}
|	modifiers type error
		{
		  yyerror ("Identifier expected");
		  RECOVER;
		}
|	VOID_TK error
		{
		  yyerror ("Identifier expected");
		  RECOVER;
		}
|	modifiers VOID_TK error
		{
		  yyerror ("Identifier expected");
		  RECOVER;
		}
|	modifiers error
		{
		  yyerror ("Invalid method declaration, return type required");
		  RECOVER;
		}
;

method_declarator:
	identifier OP_TK CP_TK
		{
		  ctxp->formal_parameter_number = 0;
		  $$ = method_declarator ($1, NULL_TREE);
		}
|	identifier OP_TK formal_parameter_list CP_TK
		{ $$ = method_declarator ($1, $3); }
|	method_declarator OSB_TK CSB_TK
		{
		  EXPR_WFL_LINECOL (wfl_operator) = $2.location;
		  TREE_PURPOSE ($1) =
		    build_unresolved_array_type (TREE_PURPOSE ($1));
		  parse_warning_context
		    (wfl_operator,
		     "Discouraged form of returned type specification");
		}
|	identifier OP_TK error
		{yyerror ("')' expected"); DRECOVER(method_declarator);}
|	method_declarator OSB_TK error
		{yyerror ("']' expected"); RECOVER;}
;

formal_parameter_list:
	formal_parameter
		{
		  ctxp->formal_parameter_number = 1;
		}
|	last_formal_parameter
		{ /* TODO */ }
|	formal_parameter_list C_TK formal_parameter
		{
		  ctxp->formal_parameter_number += 1;
		  $$ = chainon ($1, $3);
		}
|	formal_parameter_list C_TK last_formal_parameter
		{ /* TODO */ }
|	formal_parameter_list C_TK error
		{ yyerror ("Missing formal parameter term"); RECOVER; }
;

last_formal_parameter:
	type ELLIPSIS_TK variable_declarator_id
		{ /* TODO */ }
;

formal_parameter:
	type variable_declarator_id
		{
		  $$ = build_tree_list ($2, $1);
		}
|	final type variable_declarator_id /* Added, JDK1.1 final parms */
		{
		  $$ = build_tree_list ($3, $2);
		  ARG_FINAL_P ($$) = 1;
		}
|	type error
		{
		  yyerror ("Missing identifier"); RECOVER;
		  $$ = NULL_TREE;
		}
|	final type error
		{
		  yyerror ("Missing identifier"); RECOVER;
		  $$ = NULL_TREE;
		}
;

final:
	modifiers
		{
		  check_modifiers ("Illegal modifier `%s'. Only `final' was expected here",
				   $1, ACC_FINAL);
		  if ($1 != ACC_FINAL)
		    MODIFIER_WFL (FINAL_TK) = build_wfl_node (NULL_TREE);
		}
;

throws:
		{ $$ = NULL_TREE; }
|	THROWS_TK class_type_list
		{ $$ = $2; }
|	THROWS_TK error
		{yyerror ("Missing class type term"); RECOVER;}
;

class_type_list:
	class_type
		{ $$ = build_tree_list ($1, $1); }
|	class_type_list C_TK class_type
		{ $$ = tree_cons ($3, $3, $1); }
|	class_type_list C_TK error
		{yyerror ("Missing class type term"); RECOVER;}
;

method_body:
	block
|	SC_TK { $$ = NULL_TREE; }
;

/* 19.8.4 Productions from 8.5: Static Initializers  */
static_initializer:
	static block
		{
		  TREE_CHAIN ($2) = CPC_STATIC_INITIALIZER_STMT (ctxp);
		  SET_CPC_STATIC_INITIALIZER_STMT (ctxp, $2);
		  current_static_block = NULL_TREE;
		}
;

static:				/* Test lval.sub_token here */
	modifiers
		{
		  check_modifiers ("Illegal modifier `%s' for static initializer", $1, ACC_STATIC);
		  /* Can't have a static initializer in an innerclass */
		  if ($1 | ACC_STATIC &&
		      GET_CPC_LIST () && !TOPLEVEL_CLASS_DECL_P (GET_CPC ()))
		    parse_error_context
		      (MODIFIER_WFL (STATIC_TK),
		       "Can't define static initializer in class `%s'. Static initializer can only be defined in top-level classes",
		       IDENTIFIER_POINTER (DECL_NAME (GET_CPC ())));
		  SOURCE_FRONTEND_DEBUG (("Modifiers: %d", $1));
		}
;

/* 19.8.5 Productions from 8.6: Constructor Declarations  */
constructor_declaration:
	constructor_header
		{
		  current_function_decl = $1;
		  source_start_java_method (current_function_decl);
		}
	constructor_body
		{ finish_method_declaration ($3); }
;

generic_constructor_declaration:
	type_list constructor_declaration
		{ /* TODO */ }
;

constructor_header:
	constructor_declarator throws
		{ $$ = method_header (0, NULL_TREE, $1, $2); }
|	modifiers constructor_declarator throws
		{ $$ = method_header ($1, NULL_TREE, $2, $3); }
;

constructor_declarator:
	simple_name OP_TK CP_TK
		{
		  ctxp->formal_parameter_number = 0;
		  $$ = method_declarator ($1, NULL_TREE);
		}
|	simple_name OP_TK formal_parameter_list CP_TK
		{ $$ = method_declarator ($1, $3); }
;

constructor_body:
	/* Unlike regular method, we always need a complete (empty)
	   body so we can safely perform all the required code
	   addition (super invocation and field initialization) */
	block_begin constructor_block_end
		{
		  BLOCK_EXPR_BODY ($2) = empty_stmt_node;
		  $$ = $2;
		}
|	block_begin explicit_constructor_invocation constructor_block_end
		{ $$ = $3; }
|	block_begin block_statements constructor_block_end
		{ $$ = $3; }
|       block_begin explicit_constructor_invocation block_statements constructor_block_end
		{ $$ = $4; }
;

constructor_block_end:
	block_end
;

/* Error recovery for that rule moved down expression_statement: rule.  */
explicit_constructor_invocation:
	this_or_super OP_TK CP_TK SC_TK
		{
		  $$ = build_method_invocation ($1, NULL_TREE);
		  $$ = build_debugable_stmt (EXPR_WFL_LINECOL ($1), $$);
		  $$ = java_method_add_stmt (current_function_decl, $$);
		}
|	this_or_super OP_TK argument_list CP_TK SC_TK
		{
		  $$ = build_method_invocation ($1, $3);
		  $$ = build_debugable_stmt (EXPR_WFL_LINECOL ($1), $$);
		  $$ = java_method_add_stmt (current_function_decl, $$);
		}
        /* Added, JDK1.1 inner classes. Modified because the rule
	   'primary' couldn't work.  */
|	name DOT_TK SUPER_TK OP_TK argument_list CP_TK SC_TK
		{$$ = parse_jdk1_1_error ("explicit constructor invocation"); }
|	name DOT_TK SUPER_TK OP_TK CP_TK SC_TK
		{$$ = parse_jdk1_1_error ("explicit constructor invocation"); }
;

this_or_super:			/* Added, simplifies error diagnostics */
	THIS_TK
		{
		  tree wfl = build_wfl_node (this_identifier_node);
		  EXPR_WFL_LINECOL (wfl) = $1.location;
		  $$ = wfl;
		}
|	SUPER_TK
		{
		  tree wfl = build_wfl_node (super_identifier_node);
		  EXPR_WFL_LINECOL (wfl) = $1.location;
		  $$ = wfl;
		}
;

/* 19.9 Productions from 9: Interfaces  */
/* 19.9.1 Productions from 9.1: Interfaces Declarations  */
interface_declaration:
	INTERFACE_TK identifier
		{ create_interface (0, $2, NULL_TREE); }
	interface_body
		{ ; }
|	modifiers INTERFACE_TK identifier
		{ create_interface ($1, $3, NULL_TREE); }
	interface_body
		{ ; }
|	INTERFACE_TK identifier extends_interfaces
		{ create_interface (0, $2, $3);	}
	interface_body
		{ ; }
|	INTERFACE_TK identifier type_list
		{ /* TODO */ }
	interface_body
		{ /* TODO */ }
|	modifiers INTERFACE_TK identifier type_list
		{ /* TODO */ }
	interface_body
		{ /* TODO */ }
|	INTERFACE_TK identifier type_list extends_interfaces
		{ /* TODO */ }
	interface_body
		{ /* TODO */ }
|	modifiers INTERFACE_TK identifier extends_interfaces
		{ create_interface ($1, $3, $4); }
	interface_body
		{ ; }
|	INTERFACE_TK identifier error
		{ yyerror (" expected"); RECOVER; }
|	modifiers INTERFACE_TK identifier error
		{ yyerror (" expected"); RECOVER; }
;

extends_interfaces:
	EXTENDS_TK interface_type
		{
		  ctxp->interface_number = 1;
		  $$ = build_tree_list ($2, NULL_TREE);
		}
|	extends_interfaces C_TK interface_type
		{
		  ctxp->interface_number++;
		  $$ = chainon ($1, build_tree_list ($3, NULL_TREE));
		}
|	EXTENDS_TK error
		{yyerror ("Invalid interface type"); RECOVER;}
|	extends_interfaces C_TK error
		{yyerror ("Missing term"); RECOVER;}
;

interface_body:
	OCB_TK CCB_TK
		{ $$ = NULL_TREE; }
|	OCB_TK interface_member_declarations CCB_TK
		{ $$ = NULL_TREE; }
;

interface_member_declarations:
	interface_member_declaration
|	interface_member_declarations interface_member_declaration
;

interface_member_declaration:
	constant_declaration
|	abstract_method_declaration
|	generic_abstract_method_declaration
|	class_declaration	/* Added, JDK1.1 inner classes */
		{ end_class_declaration (1); }
|	interface_declaration	/* Added, JDK1.1 inner interfaces */
		{ end_class_declaration (1); }
|	enum_declaration
		{ /* TODO */ }
;

constant_declaration:
	field_declaration
;

abstract_method_declaration:
	method_header SC_TK
		{
		  check_abstract_method_header ($1);
		  current_function_decl = NULL_TREE; /* FIXME ? */
		}
|	method_header error
		{yyerror ("';' expected"); RECOVER;}
;

generic_abstract_method_declaration:
	type_list abstract_method_declaration
		{ /* TODO */ }
;

enum_declaration:
	ENUM_TK identifier interfaces OCB_TK enum_constants CCB_TK
		{ /* TODO */ }
|	ENUM_TK identifier interfaces OCB_TK CCB_TK
		{ /* TODO */ }
|	ENUM_TK error
		{yyerror ("Invalid enum declaration"); RECOVER;}
|	ENUM_TK identifier error
		{yyerror ("Invalid enum declaration"); RECOVER;}
|	ENUM_TK identifier interfaces error
		{yyerror ("Invalid enum declaration"); RECOVER;}
|	ENUM_TK identifier interfaces OCB_TK error
		{yyerror ("Invalid enum declaration"); RECOVER;}
|	ENUM_TK identifier interfaces OCB_TK enum_constants error
		{yyerror ("Invalid enum declaration"); RECOVER;}
;

enum_constants:
	enum_constant
		{ /* TODO */ }
|	enum_constants C_TK enum_constant
		{ /* TODO */ }
|	enum_constants C_TK error
		{yyerror ("Missing enum constant"); RECOVER;}
;

enum_constant:
	identifier		/* Doesn't support enum args, but we hope those are rare */
		{ /* TODO */ }
|	annotations identifier
		{ /* TODO */ }
;

/* 19.10 Productions from 10: Arrays  */
array_initializer:
	OCB_TK CCB_TK
		{ $$ = build_new_array_init ($1.location, NULL_TREE); }
|	OCB_TK C_TK CCB_TK
		{ $$ = build_new_array_init ($1.location, NULL_TREE); }
|	OCB_TK variable_initializers CCB_TK
		{ $$ = build_new_array_init ($1.location, $2); }
|	OCB_TK variable_initializers C_TK CCB_TK
		{ $$ = build_new_array_init ($1.location, $2); }
;

variable_initializers:
	variable_initializer
		{
		  $$ = tree_cons (maybe_build_array_element_wfl ($1),
				  $1, NULL_TREE);
		}
|	variable_initializers C_TK variable_initializer
		{
		  $$ = tree_cons (maybe_build_array_element_wfl ($3), $3, $1);
		}
|	variable_initializers C_TK error
		{yyerror ("Missing term"); RECOVER;}
;

/* 19.11 Production from 14: Blocks and Statements  */
block:
	block_begin block_end
		{ $$ = $2; }
|	block_begin block_statements block_end
		{ $$ = $3; }
;

block_begin:
	OCB_TK
		{ enter_block (); }
;

block_end:
	CCB_TK
		{
		  maybe_absorb_scoping_blocks ();
		  /* Store the location of the  when doing xrefs */
		  if (current_function_decl && flag_emit_xref)
		    DECL_END_SOURCE_LINE (current_function_decl) =
		      EXPR_WFL_ADD_COL ($1.location, 1);
		  $$ = exit_block ();
		  if (!BLOCK_SUBBLOCKS ($$))
		    BLOCK_SUBBLOCKS ($$) = empty_stmt_node;
		}
;

block_statements:
	block_statement
|	block_statements block_statement
;

block_statement:
	local_variable_declaration_statement
|	statement
		{ java_method_add_stmt (current_function_decl, $1); }
|	class_declaration	/* Added, JDK1.1 local classes */
		{
		  LOCAL_CLASS_P (TREE_TYPE (GET_CPC ())) = 1;
		  end_class_declaration (1);
		}
;

local_variable_declaration_statement:
	local_variable_declaration SC_TK /* Can't catch missing ';' here */
;

local_variable_declaration:
	type variable_declarators
		{ declare_local_variables (0, $1, $2); }
|	final type variable_declarators /* Added, JDK1.1 final locals */
		{ declare_local_variables ($1, $2, $3); }
;

statement:
	statement_without_trailing_substatement
|	labeled_statement
|	if_then_statement
|	if_then_else_statement
|	while_statement
|	for_statement
		{ $$ = exit_block (); }
;

statement_nsi:
	statement_without_trailing_substatement
|	labeled_statement_nsi
|	if_then_else_statement_nsi
|	while_statement_nsi
|	for_statement_nsi
		{ $$ = exit_block (); }
;

statement_without_trailing_substatement:
	block
|	empty_statement
|	expression_statement
|	switch_statement
|	do_statement
|	break_statement
|	continue_statement
|	return_statement
|	synchronized_statement
|	throw_statement
|	try_statement
|	assert_statement
;

empty_statement:
	SC_TK
		{
		  if (flag_extraneous_semicolon
		      && ! current_static_block
		      && (! current_function_decl ||
			  /* Verify we're not in a inner class declaration */
			  (GET_CPC () != TYPE_NAME
			   (DECL_CONTEXT (current_function_decl)))))

		    {
		      EXPR_WFL_SET_LINECOL (wfl_operator, input_line, -1);
		      parse_warning_context (wfl_operator, "An empty declaration is a deprecated feature that should not be used");
		    }
		  $$ = empty_stmt_node;
		}
;

label_decl:
	identifier REL_CL_TK
		{
		  $$ = build_labeled_block (EXPR_WFL_LINECOL ($1),
					    EXPR_WFL_NODE ($1));
		  pushlevel (2);
		  push_labeled_block ($$);
		  PUSH_LABELED_BLOCK ($$);
		}
;

labeled_statement:
	label_decl statement
		{ $$ = finish_labeled_statement ($1, $2); }
|	identifier error
		{yyerror ("':' expected"); RECOVER;}
;

labeled_statement_nsi:
	label_decl statement_nsi
		{ $$ = finish_labeled_statement ($1, $2); }
;

/* We concentrate here a bunch of error handling rules that we couldn't write
   earlier, because expression_statement catches a missing ';'.  */
expression_statement:
	statement_expression SC_TK
		{
		  /* We have a statement. Generate a WFL around it so
		     we can debug it */
		  $$ = build_expr_wfl ($1, input_filename, input_line, 0);
		  /* We know we have a statement, so set the debug
                     info to be eventually generate here. */
		  $$ = JAVA_MAYBE_GENERATE_DEBUG_INFO ($$);
		}
|	error SC_TK
		{
		  YYNOT_TWICE yyerror ("Invalid expression statement");
		  DRECOVER (expr_stmt);
		}
|	error OCB_TK
		{
		  YYNOT_TWICE yyerror ("Invalid expression statement");
		  DRECOVER (expr_stmt);
		}
|	error CCB_TK
		{
		  YYNOT_TWICE yyerror ("Invalid expression statement");
		  DRECOVER (expr_stmt);
		}
|       this_or_super OP_TK error
		{yyerror ("')' expected"); RECOVER;}
|       this_or_super OP_TK CP_TK error
		{
		  parse_ctor_invocation_error ();
		  RECOVER;
		}
|       this_or_super OP_TK argument_list error
		{yyerror ("')' expected"); RECOVER;}
|       this_or_super OP_TK argument_list CP_TK error
		{
		  parse_ctor_invocation_error ();
		  RECOVER;
		}
|	name DOT_TK SUPER_TK error
		{yyerror ("'(' expected"); RECOVER;}
|	name DOT_TK SUPER_TK OP_TK error
		{yyerror ("')' expected"); RECOVER;}
|	name DOT_TK SUPER_TK OP_TK argument_list error
		{yyerror ("')' expected"); RECOVER;}
|	name DOT_TK SUPER_TK OP_TK argument_list CP_TK error
		{yyerror ("';' expected"); RECOVER;}
|	name DOT_TK SUPER_TK OP_TK CP_TK error
		{yyerror ("';' expected"); RECOVER;}
;

statement_expression:
	assignment
|	pre_increment_expression
|	pre_decrement_expression
|	post_increment_expression
|	post_decrement_expression
|	method_invocation
|	class_instance_creation_expression
;

if_then_statement:
	IF_TK OP_TK expression CP_TK statement
		{
		  $$ = build_if_else_statement ($2.location, $3,
						$5, NULL_TREE);
		}
|	IF_TK error
		{yyerror ("'(' expected"); RECOVER;}
|	IF_TK OP_TK error
		{yyerror ("Missing term"); RECOVER;}
|	IF_TK OP_TK expression error
		{yyerror ("')' expected"); RECOVER;}
;

if_then_else_statement:
	IF_TK OP_TK expression CP_TK statement_nsi ELSE_TK statement
		{ $$ = build_if_else_statement ($2.location, $3, $5, $7); }
;

if_then_else_statement_nsi:
	IF_TK OP_TK expression CP_TK statement_nsi ELSE_TK statement_nsi
		{ $$ = build_if_else_statement ($2.location, $3, $5, $7); }
;

switch_statement:
	switch_expression
		{
		  enter_block ();
		}
	switch_block
		{
		  /* Make into "proper list" of COMPOUND_EXPRs.
		     I.e. make the last statement also have its own
		     COMPOUND_EXPR. */
		  maybe_absorb_scoping_blocks ();
		  TREE_OPERAND ($1, 1) = exit_block ();
		  $$ = build_debugable_stmt (EXPR_WFL_LINECOL ($1), $1);
		}
;

switch_expression:
	SWITCH_TK OP_TK expression CP_TK
		{
		  $$ = build (SWITCH_EXPR, NULL_TREE, $3, NULL_TREE);
		  EXPR_WFL_LINECOL ($$) = $2.location;
		}
|	SWITCH_TK error
		{yyerror ("'(' expected"); RECOVER;}
|	SWITCH_TK OP_TK error
		{yyerror ("Missing term or ')'"); DRECOVER(switch_statement);}
|	SWITCH_TK OP_TK expression CP_TK error
		{yyerror (" expected"); RECOVER;}
;

/* Default assignment is there to avoid type node on switch_block
   node. */

switch_block:
	OCB_TK CCB_TK
		{ $$ = NULL_TREE; }
|	OCB_TK switch_labels CCB_TK
		{ $$ = NULL_TREE; }
|	OCB_TK switch_block_statement_groups CCB_TK
		{ $$ = NULL_TREE; }
|	OCB_TK switch_block_statement_groups switch_labels CCB_TK
		{ $$ = NULL_TREE; }
;

switch_block_statement_groups:
	switch_block_statement_group
|	switch_block_statement_groups switch_block_statement_group
;

switch_block_statement_group:
	switch_labels block_statements
;

switch_labels:
	switch_label
|	switch_labels switch_label
;

switch_label:
	CASE_TK constant_expression REL_CL_TK
		{
		  tree lab = build1 (CASE_EXPR, NULL_TREE, $2);
		  EXPR_WFL_LINECOL (lab) = $1.location;
		  java_method_add_stmt (current_function_decl, lab);
		}
|	DEFAULT_TK REL_CL_TK
		{
		  tree lab = build (DEFAULT_EXPR, NULL_TREE, NULL_TREE);
		  EXPR_WFL_LINECOL (lab) = $1.location;
		  java_method_add_stmt (current_function_decl, lab);
		}
|	CASE_TK error
		{yyerror ("Missing or invalid constant expression"); RECOVER;}
|	CASE_TK constant_expression error
		{yyerror ("':' expected"); RECOVER;}
|	DEFAULT_TK error
		{yyerror ("':' expected"); RECOVER;}
;

while_expression:
	WHILE_TK OP_TK expression CP_TK
		{
		  tree body = build_loop_body ($2.location, $3, 0);
		  $$ = build_new_loop (body);
		}
;

while_statement:
	while_expression statement
		{ $$ = finish_loop_body (0, NULL_TREE, $2, 0); }
|	WHILE_TK error
		{YYERROR_NOW; yyerror ("'(' expected"); RECOVER;}
|	WHILE_TK OP_TK error
		{yyerror ("Missing term and ')' expected"); RECOVER;}
|	WHILE_TK OP_TK expression error
		{yyerror ("')' expected"); RECOVER;}
;

while_statement_nsi:
	while_expression statement_nsi
		{ $$ = finish_loop_body (0, NULL_TREE, $2, 0); }
;

do_statement_begin:
	DO_TK
		{
		  tree body = build_loop_body (0, NULL_TREE, 1);
		  $$ = build_new_loop (body);
		}
	/* Need error handing here. FIXME */
;

do_statement:
	do_statement_begin statement WHILE_TK OP_TK expression CP_TK SC_TK
		{ $$ = finish_loop_body ($4.location, $5, $2, 1); }
;

for_statement:
	for_begin SC_TK expression SC_TK for_update CP_TK statement
		{
		  if (TREE_CODE_CLASS (TREE_CODE ($3)) == 'c')
		    $3 = build_wfl_node ($3);
		  $$ = finish_for_loop (EXPR_WFL_LINECOL ($3), $3, $5, $7);
		}
|	for_begin SC_TK SC_TK for_update CP_TK statement
		{
		  $$ = finish_for_loop (0, NULL_TREE, $4, $6);
		  /* We have not condition, so we get rid of the EXIT_EXPR */
		  LOOP_EXPR_BODY_CONDITION_EXPR (LOOP_EXPR_BODY ($$), 0) =
		    empty_stmt_node;
		}
|	for_header type variable_declarator REL_CL_TK expression CP_TK statement
		{ /* TODO */ }
|	for_begin SC_TK error
		{yyerror ("Invalid control expression"); RECOVER;}
|	for_begin SC_TK expression SC_TK error
		{yyerror ("Invalid update expression"); RECOVER;}
|	for_begin SC_TK SC_TK error
		{yyerror ("Invalid update expression"); RECOVER;}
;

for_statement_nsi:
	for_begin SC_TK expression SC_TK for_update CP_TK statement_nsi
		{ $$ = finish_for_loop (EXPR_WFL_LINECOL ($3), $3, $5, $7);}
|	for_begin SC_TK SC_TK for_update CP_TK statement_nsi
		{
		  $$ = finish_for_loop (0, NULL_TREE, $4, $6);
		  /* We have not condition, so we get rid of the EXIT_EXPR */
		  LOOP_EXPR_BODY_CONDITION_EXPR (LOOP_EXPR_BODY ($$), 0) =
		    empty_stmt_node;
		}
|	for_header type variable_declarator REL_CL_TK expression CP_TK statement_nsi
		{ /* TODO */ }
;

for_header:
	FOR_TK OP_TK
		{
		  /* This scope defined for local variable that may be
                     defined within the scope of the for loop */
		  enter_block ();
		}
|	FOR_TK error
		{yyerror ("'(' expected"); DRECOVER(for_1);}
|	FOR_TK OP_TK error
		{yyerror ("Invalid init statement"); RECOVER;}
;

for_begin:
	for_header for_init
		{
		  /* We now declare the loop body. The loop is
                     declared as a for loop. */
		  tree body = build_loop_body (0, NULL_TREE, 0);
		  $$ =  build_new_loop (body);
		  FOR_LOOP_P ($$) = 1;
		  /* The loop is added to the current block the for
                     statement is defined within */
		  java_method_add_stmt (current_function_decl, $$);
		}
;
for_init:			/* Can be empty */
		{ $$ = empty_stmt_node; }
|	statement_expression_list
		{
		  /* Init statement recorded within the previously
                     defined block scope */
		  $$ = java_method_add_stmt (current_function_decl, $1);
		}
|	local_variable_declaration
		{
		  /* Local variable are recorded within the previously
		     defined block scope */
		  $$ = NULL_TREE;
		}
|	statement_expression_list error
		{yyerror ("';' expected"); DRECOVER(for_init_1);}
;

for_update:			/* Can be empty */
		{$$ = empty_stmt_node;}
|	statement_expression_list
		{ $$ = build_debugable_stmt (BUILD_LOCATION (), $1); }
;

statement_expression_list:
	statement_expression
		{ $$ = add_stmt_to_compound (NULL_TREE, NULL_TREE, $1); }
|	statement_expression_list C_TK statement_expression
		{ $$ = add_stmt_to_compound ($1, NULL_TREE, $3); }
|	statement_expression_list C_TK error
		{yyerror ("Missing term"); RECOVER;}
;

break_statement:
	BREAK_TK SC_TK
		{ $$ = build_bc_statement ($1.location, 1, NULL_TREE); }
|	BREAK_TK identifier SC_TK
		{ $$ = build_bc_statement ($1.location, 1, $2); }
|	BREAK_TK error
		{yyerror ("Missing term"); RECOVER;}
|	BREAK_TK identifier error
		{yyerror ("';' expected"); RECOVER;}
;

continue_statement:
	CONTINUE_TK SC_TK
		{ $$ = build_bc_statement ($1.location, 0, NULL_TREE); }
|       CONTINUE_TK identifier SC_TK
		{ $$ = build_bc_statement ($1.location, 0, $2); }
|	CONTINUE_TK error
		{yyerror ("Missing term"); RECOVER;}
|	CONTINUE_TK identifier error
		{yyerror ("';' expected"); RECOVER;}
;

return_statement:
	RETURN_TK SC_TK
		{ $$ = build_return ($1.location, NULL_TREE); }
|	RETURN_TK expression SC_TK
		{ $$ = build_return ($1.location, $2); }
|	RETURN_TK error
		{yyerror ("Missing term"); RECOVER;}
|	RETURN_TK expression error
		{yyerror ("';' expected"); RECOVER;}
;

throw_statement:
	THROW_TK expression SC_TK
		{
		  $$ = build1 (THROW_EXPR, NULL_TREE, $2);
		  EXPR_WFL_LINECOL ($$) = $1.location;
		}
|	THROW_TK error
		{yyerror ("Missing term"); RECOVER;}
|	THROW_TK expression error
		{yyerror ("';' expected"); RECOVER;}
;

assert_statement:
	ASSERT_TK expression REL_CL_TK expression SC_TK
		{
		  $$ = build_assertion ($1.location, $2, $4);
		}
|	ASSERT_TK expression SC_TK
		{
		  $$ = build_assertion ($1.location, $2, NULL_TREE);
		}
|	ASSERT_TK error
		{yyerror ("Missing term"); RECOVER;}
|	ASSERT_TK expression error
		{yyerror ("';' expected"); RECOVER;}
;

synchronized_statement:
	synchronized OP_TK expression CP_TK block
		{
		  $$ = build (SYNCHRONIZED_EXPR, NULL_TREE, $3, $5);
		  EXPR_WFL_LINECOL ($$) =
		    EXPR_WFL_LINECOL (MODIFIER_WFL (SYNCHRONIZED_TK));
		}
|	synchronized OP_TK expression CP_TK error
		{yyerror (" expected"); RECOVER;}
|	synchronized error
		{yyerror ("'(' expected"); RECOVER;}
|	synchronized OP_TK error CP_TK
		{yyerror ("Missing term"); RECOVER;}
|	synchronized OP_TK error
		{yyerror ("Missing term"); RECOVER;}
;

synchronized:
	modifiers
		{
		  check_modifiers (
             "Illegal modifier `%s'. Only `synchronized' was expected here",
				   $1, ACC_SYNCHRONIZED);
		  if ($1 != ACC_SYNCHRONIZED)
		    MODIFIER_WFL (SYNCHRONIZED_TK) =
		      build_wfl_node (NULL_TREE);
		}
;

try_statement:
	TRY_TK block catches
		{ $$ = build_try_statement ($1.location, $2, $3); }
|	TRY_TK block finally
		{ $$ = build_try_finally_statement ($1.location, $2, $3); }
|	TRY_TK block catches finally
		{ $$ = build_try_finally_statement
		    ($1.location, build_try_statement ($1.location,
						       $2, $3), $4);
		}
|	TRY_TK error
		{yyerror (" expected"); DRECOVER (try_statement);}
;

catches:
	catch_clause
|	catches catch_clause
		{
		  TREE_CHAIN ($2) = $1;
		  $$ = $2;
		}
;

catch_clause:
	catch_clause_parameter block
		{
		  java_method_add_stmt (current_function_decl, $2);
		  exit_block ();
		  $$ = $1;
		}
;

catch_clause_parameter:
	CATCH_TK OP_TK formal_parameter CP_TK
		{
		  /* We add a block to define a scope for
		     formal_parameter (CCBP). The formal parameter is
		     declared initialized by the appropriate function
		     call */
                  tree ccpb;
                  tree init;
                  if ($3)
                    {
                      ccpb = enter_block ();
                      init = build_assignment
                        (ASSIGN_TK, $2.location, TREE_PURPOSE ($3),
                         build (JAVA_EXC_OBJ_EXPR, ptr_type_node));
                      declare_local_variables (0, TREE_VALUE ($3),
                                               build_tree_list
					       (TREE_PURPOSE ($3), init));
                      $$ = build1 (CATCH_EXPR, NULL_TREE, ccpb);
                      EXPR_WFL_LINECOL ($$) = $1.location;
                    }
                  else
                    {
                      $$ = error_mark_node;
                    }
		}
|	CATCH_TK error
		{yyerror ("'(' expected"); RECOVER; $$ = NULL_TREE;}
|	CATCH_TK OP_TK error
		{
		  yyerror ("Missing term or ')' expected");
		  RECOVER; $$ = NULL_TREE;
		}
|	CATCH_TK OP_TK error CP_TK /* That's for () */
		{yyerror ("Missing term"); RECOVER; $$ = NULL_TREE;}
;

finally:
	FINALLY_TK block
		{ $$ = $2; }
|	FINALLY_TK error
		{yyerror (" expected"); RECOVER; }
;

/* 19.12 Production from 15: Expressions  */
primary:
	primary_no_new_array
|	array_creation_expression
;

primary_no_new_array:
	literal
|	THIS_TK
		{ $$ = build_this ($1.location); }
|	OP_TK expression CP_TK
		{$$ = $2;}
|	class_instance_creation_expression
|	field_access
|	method_invocation
|	array_access
|	type_literals
        /* Added, JDK1.1 inner classes. Documentation is wrong
           refering to a 'ClassName' (class_name) rule that doesn't
           exist. Used name: instead.  */
|	name DOT_TK THIS_TK
		{
		  tree wfl = build_wfl_node (this_identifier_node);
		  $$ = make_qualified_primary ($1, wfl, EXPR_WFL_LINECOL ($1));
		}
|	OP_TK expression error
		{yyerror ("')' expected"); RECOVER;}
|	name DOT_TK error
		{yyerror ("'class' or 'this' expected" ); RECOVER;}
|	primitive_type DOT_TK error
		{yyerror ("'class' expected" ); RECOVER;}
|	VOID_TK DOT_TK error
		{yyerror ("'class' expected" ); RECOVER;}
;

type_literals:
	name DOT_TK CLASS_TK
		{ $$ = build_incomplete_class_ref ($2.location, $1); }
|	array_type DOT_TK CLASS_TK
		{ $$ = build_incomplete_class_ref ($2.location, $1); }
|	primitive_type DOT_TK CLASS_TK
                { $$ = build_incomplete_class_ref ($2.location, $1); }
|	VOID_TK DOT_TK CLASS_TK
                {
                   $$ = build_incomplete_class_ref ($2.location,
                                                   void_type_node);
                }
;

class_instance_creation_expression:
	NEW_TK class_type OP_TK argument_list CP_TK
		{ $$ = build_new_invocation ($2, $4); }
|	NEW_TK class_type OP_TK CP_TK
		{ $$ = build_new_invocation ($2, NULL_TREE); }
|	anonymous_class_creation
        /* Added, JDK1.1 inner classes, modified to use name or
	   primary instead of primary solely which couldn't work in
	   all situations.  */
|	something_dot_new identifier OP_TK CP_TK
		{
		  tree ctor = build_new_invocation ($2, NULL_TREE);
		  $$ = make_qualified_primary ($1, ctor,
					       EXPR_WFL_LINECOL ($1));
		}
|	something_dot_new identifier OP_TK CP_TK class_body
|	something_dot_new identifier OP_TK argument_list CP_TK
		{
		  tree ctor = build_new_invocation ($2, $4);
		  $$ = make_qualified_primary ($1, ctor,
					       EXPR_WFL_LINECOL ($1));
		}
|	something_dot_new identifier OP_TK argument_list CP_TK class_body
|	NEW_TK error SC_TK
		{yyerror ("'(' expected"); DRECOVER(new_1);}
|	NEW_TK class_type error
		{yyerror ("'(' expected"); RECOVER;}
|	NEW_TK class_type OP_TK error
		{yyerror ("')' or term expected"); RECOVER;}
|	NEW_TK class_type OP_TK argument_list error
		{yyerror ("')' expected"); RECOVER;}
|	something_dot_new error
		{YYERROR_NOW; yyerror ("Identifier expected"); RECOVER;}
|	something_dot_new identifier error
		{yyerror ("'(' expected"); RECOVER;}
;

/* Created after JDK1.1 rules originally added to
   class_instance_creation_expression, but modified to use
   'class_type' instead of 'TypeName' (type_name) which is mentioned
   in the documentation but doesn't exist. */

anonymous_class_creation:
	NEW_TK class_type OP_TK argument_list CP_TK
		{ create_anonymous_class ($1.location, $2); }
        class_body
		{
		  tree id = build_wfl_node (DECL_NAME (GET_CPC ()));
		  EXPR_WFL_LINECOL (id) = EXPR_WFL_LINECOL ($2);

		  end_class_declaration (1);

		  /* Now we can craft the new expression */
		  $$ = build_new_invocation (id, $4);

		  /* Note that we can't possibly be here if
		     `class_type' is an interface (in which case the
		     anonymous class extends Object and implements
		     `class_type', hence its constructor can't have
		     arguments.) */

		  /* Otherwise, the innerclass must feature a
		     constructor matching `argument_list'. Anonymous
		     classes are a bit special: it's impossible to
		     define constructor for them, hence constructors
		     must be generated following the hints provided by
		     the `new' expression. Whether a super constructor
		     of that nature exists or not is to be verified
		     later on in verify_constructor_super.

		     It's during the expansion of a `new' statement
		     refering to an anonymous class that a ctor will
		     be generated for the anonymous class, with the
		     right arguments. */

		}
|	NEW_TK class_type OP_TK CP_TK
		{ create_anonymous_class ($1.location, $2); }
        class_body
		{
		  tree id = build_wfl_node (DECL_NAME (GET_CPC ()));
		  EXPR_WFL_LINECOL (id) = EXPR_WFL_LINECOL ($2);

		  end_class_declaration (1);

		  /* Now we can craft the new expression. The
                     statement doesn't need to be remember so that a
                     constructor can be generated, since its signature
                     is already known. */
		  $$ = build_new_invocation (id, NULL_TREE);
		}
;

something_dot_new:		/* Added, not part of the specs. */
	name DOT_TK NEW_TK
		{ $$ = $1; }
|	primary DOT_TK NEW_TK
		{ $$ = $1; }
;

argument_list:
	expression
		{
		  $$ = tree_cons (NULL_TREE, $1, NULL_TREE);
		  ctxp->formal_parameter_number = 1;
		}
|	argument_list C_TK expression
		{
		  ctxp->formal_parameter_number += 1;
		  $$ = tree_cons (NULL_TREE, $3, $1);
		}
|	argument_list C_TK error
		{yyerror ("Missing term"); RECOVER;}
;

array_creation_expression:
	NEW_TK primitive_type dim_exprs
		{ $$ = build_newarray_node ($2, $3, 0); }
|	NEW_TK class_or_interface_type dim_exprs
		{ $$ = build_newarray_node ($2, $3, 0); }
|	NEW_TK primitive_type dim_exprs dims
		{ $$ = build_newarray_node ($2, $3, pop_current_osb (ctxp));}
|	NEW_TK class_or_interface_type dim_exprs dims
		{ $$ = build_newarray_node ($2, $3, pop_current_osb (ctxp));}
        /* Added, JDK1.1 anonymous array. Initial documentation rule
           modified */
|	NEW_TK class_or_interface_type dims array_initializer
		{
		  char *sig;
		  int osb = pop_current_osb (ctxp);
		  while (osb--)
		    obstack_grow (&temporary_obstack, "[]", 2);
		  obstack_1grow (&temporary_obstack, '\0');
		  sig = obstack_finish (&temporary_obstack);
		  $$ = build (NEW_ANONYMOUS_ARRAY_EXPR, NULL_TREE,
			      $2, get_identifier (sig), $4);
		}
|	NEW_TK primitive_type dims array_initializer
		{
		  int osb = pop_current_osb (ctxp);
		  tree type = $2;
		  while (osb--)
		    type = build_java_array_type (type, -1);
		  $$ = build (NEW_ANONYMOUS_ARRAY_EXPR, NULL_TREE,
			      build_pointer_type (type), NULL_TREE, $4);
		}
|	NEW_TK error CSB_TK
		{yyerror ("'[' expected"); DRECOVER ("]");}
|	NEW_TK error OSB_TK
		{yyerror ("']' expected"); RECOVER;}
;

dim_exprs:
	dim_expr
		{ $$ = build_tree_list (NULL_TREE, $1); }
|	dim_exprs dim_expr
		{ $$ = tree_cons (NULL_TREE, $2, $$); }
;

dim_expr:
	OSB_TK expression CSB_TK
		{
		  if (JNUMERIC_TYPE_P (TREE_TYPE ($2)))
		    {
		      $2 = build_wfl_node ($2);
		      TREE_TYPE ($2) = NULL_TREE;
		    }
		  EXPR_WFL_LINECOL ($2) = $1.location;
		  $$ = $2;
		}
|	OSB_TK expression error
		{yyerror ("']' expected"); RECOVER;}
|	OSB_TK error
		{
		  yyerror ("Missing term");
		  yyerror ("']' expected");
		  RECOVER;
		}
;

dims:
	OSB_TK CSB_TK
		{
		  int allocate = 0;
		  /* If not initialized, allocate memory for the osb
                     numbers stack */
		  if (!ctxp->osb_limit)
		    {
		      allocate = ctxp->osb_limit = 32;
		      ctxp->osb_depth = -1;
		    }
		  /* If capacity overflown, reallocate a bigger chunk */
		  else if (ctxp->osb_depth+1 == ctxp->osb_limit)
		    allocate = ctxp->osb_limit << 1;

		  if (allocate)
		    {
		      allocate *= sizeof (int);
		      if (ctxp->osb_number)
			ctxp->osb_number = xrealloc (ctxp->osb_number,
						     allocate);
		      else
			ctxp->osb_number = xmalloc (allocate);
		    }
		  ctxp->osb_depth++;
		  CURRENT_OSB (ctxp) = 1;
		}
|	dims OSB_TK CSB_TK
		{ CURRENT_OSB (ctxp)++; }
|	dims OSB_TK error
		{ yyerror ("']' expected"); RECOVER;}
;

field_access:
	primary DOT_TK identifier
		{ $$ = make_qualified_primary ($1, $3, $2.location); }
		/*  FIXME - REWRITE TO:
		{ $$ = build_binop (COMPONENT_REF, $2.location, $1, $3); } */
|	SUPER_TK DOT_TK identifier
		{
		  tree super_wfl = build_wfl_node (super_identifier_node);
		  EXPR_WFL_LINECOL (super_wfl) = $1.location;
		  $$ = make_qualified_name (super_wfl, $3, $2.location);
		}
|	SUPER_TK error
		{yyerror ("Field expected"); DRECOVER (super_field_acces);}
;

method_invocation:
	name OP_TK CP_TK
		{ $$ = build_method_invocation ($1, NULL_TREE); }
|	name OP_TK argument_list CP_TK
		{ $$ = build_method_invocation ($1, $3); }
|	primary DOT_TK identifier OP_TK CP_TK
		{
		  if (TREE_CODE ($1) == THIS_EXPR)
		    $$ = build_this_super_qualified_invocation
		      (1, $3, NULL_TREE, 0, $2.location);
		  else
		    {
		      tree invok = build_method_invocation ($3, NULL_TREE);
		      $$ = make_qualified_primary ($1, invok, $2.location);
		    }
		}
|	primary DOT_TK identifier OP_TK argument_list CP_TK
		{
		  if (TREE_CODE ($1) == THIS_EXPR)
		    $$ = build_this_super_qualified_invocation
		      (1, $3, $5, 0, $2.location);
		  else
		    {
		      tree invok = build_method_invocation ($3, $5);
		      $$ = make_qualified_primary ($1, invok, $2.location);
		    }
		}
|	SUPER_TK DOT_TK identifier OP_TK CP_TK
		{
		  $$ = build_this_super_qualified_invocation
		    (0, $3, NULL_TREE, $1.location, $2.location);
		}
|	SUPER_TK DOT_TK identifier OP_TK argument_list CP_TK
		{
		  $$ = build_this_super_qualified_invocation
		    (0, $3, $5, $1.location, $2.location);
		}
        /* Screws up thing. I let it here until I'm convinced it can
           be removed. FIXME
|	primary DOT_TK error
		{yyerror ("'(' expected"); DRECOVER(bad);} */
|	SUPER_TK DOT_TK error CP_TK
		{ yyerror ("'(' expected"); DRECOVER (method_invocation); }
|	SUPER_TK DOT_TK error DOT_TK
		{ yyerror ("'(' expected"); DRECOVER (method_invocation); }
;

array_access:
	name OSB_TK expression CSB_TK
		{ $$ = build_array_ref ($2.location, $1, $3); }
|	primary_no_new_array OSB_TK expression CSB_TK
		{ $$ = build_array_ref ($2.location, $1, $3); }
|	name OSB_TK error
		{
		  yyerror ("Missing term and ']' expected");
		  DRECOVER(array_access);
		}
|	name OSB_TK expression error
		{
		  yyerror ("']' expected");
		  DRECOVER(array_access);
		}
|	primary_no_new_array OSB_TK error
		{
		  yyerror ("Missing term and ']' expected");
		  DRECOVER(array_access);
		}
|	primary_no_new_array OSB_TK expression error
		{
		  yyerror ("']' expected");
		  DRECOVER(array_access);
		}
;

postfix_expression:
	primary
|	name
|	post_increment_expression
|	post_decrement_expression
;

post_increment_expression:
	postfix_expression INCR_TK
		{ $$ = build_incdec ($2.token, $2.location, $1, 1); }
;

post_decrement_expression:
	postfix_expression DECR_TK
		{ $$ = build_incdec ($2.token, $2.location, $1, 1); }
;

trap_overflow_corner_case:
	pre_increment_expression
|	pre_decrement_expression
|	PLUS_TK unary_expression
		{$$ = build_unaryop ($1.token, $1.location, $2); }
|	unary_expression_not_plus_minus
|	PLUS_TK error
		{yyerror ("Missing term"); RECOVER}
;

unary_expression:
	trap_overflow_corner_case
		{
		  error_if_numeric_overflow ($1);
		  $$ = $1;
		}
|	MINUS_TK trap_overflow_corner_case
		{$$ = build_unaryop ($1.token, $1.location, $2); }
|	MINUS_TK error
		{yyerror ("Missing term"); RECOVER}
;

pre_increment_expression:
	INCR_TK unary_expression
		{$$ = build_incdec ($1.token, $1.location, $2, 0); }
|	INCR_TK error
		{yyerror ("Missing term"); RECOVER}
;

pre_decrement_expression:
	DECR_TK unary_expression
		{$$ = build_incdec ($1.token, $1.location, $2, 0); }
|	DECR_TK error
		{yyerror ("Missing term"); RECOVER}
;

unary_expression_not_plus_minus:
	postfix_expression
|	NOT_TK unary_expression
		{$$ = build_unaryop ($1.token, $1.location, $2); }
|	NEG_TK unary_expression
 		{$$ = build_unaryop ($1.token, $1.location, $2); }
|	cast_expression
|	NOT_TK error
		{yyerror ("Missing term"); RECOVER}
|	NEG_TK error
		{yyerror ("Missing term"); RECOVER}
;

cast_expression:		/* Error handling here is potentially weak */
	OP_TK primitive_type dims CP_TK unary_expression
		{
		  tree type = $2;
		  int osb = pop_current_osb (ctxp);
		  while (osb--)
		    type = build_java_array_type (type, -1);
		  $$ = build_cast ($1.location, type, $5);
		}
|	OP_TK primitive_type CP_TK unary_expression
		{ $$ = build_cast ($1.location, $2, $4); }
|	OP_TK expression CP_TK unary_expression_not_plus_minus
		{ $$ = build_cast ($1.location, $2, $4); }
|	OP_TK name dims CP_TK unary_expression_not_plus_minus
		{
		  const char *ptr;
		  int osb = pop_current_osb (ctxp);
		  obstack_grow (&temporary_obstack,
				IDENTIFIER_POINTER (EXPR_WFL_NODE ($2)),
				IDENTIFIER_LENGTH (EXPR_WFL_NODE ($2)));
		  while (osb--)
		    obstack_grow (&temporary_obstack, "[]", 2);
		  obstack_1grow (&temporary_obstack, '\0');
		  ptr = obstack_finish (&temporary_obstack);
		  EXPR_WFL_NODE ($2) = get_identifier (ptr);
		  $$ = build_cast ($1.location, $2, $5);
		}
|	OP_TK primitive_type OSB_TK error
		{yyerror ("']' expected, invalid type expression");}
|       OP_TK error
		{
	          YYNOT_TWICE yyerror ("Invalid type expression"); RECOVER;
		  RECOVER;
		}
|	OP_TK primitive_type dims CP_TK error
		{yyerror ("Missing term"); RECOVER;}
|	OP_TK primitive_type CP_TK error
		{yyerror ("Missing term"); RECOVER;}
|	OP_TK name dims CP_TK error
		{yyerror ("Missing term"); RECOVER;}
;

multiplicative_expression:
	unary_expression
|	multiplicative_expression MULT_TK unary_expression
		{
		  $$ = build_binop (BINOP_LOOKUP ($2.token),
				    $2.location, $1, $3);
		}
|	multiplicative_expression DIV_TK unary_expression
		{
		  $$ = build_binop (BINOP_LOOKUP ($2.token), $2.location,
				    $1, $3);
		}
|	multiplicative_expression REM_TK unary_expression
		{
		  $$ = build_binop (BINOP_LOOKUP ($2.token), $2.location,
				    $1, $3);
		}
|	multiplicative_expression MULT_TK error
		{yyerror ("Missing term"); RECOVER;}
|	multiplicative_expression DIV_TK error
		{yyerror ("Missing term"); RECOVER;}
|	multiplicative_expression REM_TK error
		{yyerror ("Missing term"); RECOVER;}
;

additive_expression:
	multiplicative_expression
|	additive_expression PLUS_TK multiplicative_expression
		{
		  $$ = build_binop (BINOP_LOOKUP ($2.token), $2.location,
				    $1, $3);
		}
|	additive_expression MINUS_TK multiplicative_expression
		{
		  $$ = build_binop (BINOP_LOOKUP ($2.token), $2.location,
				    $1, $3);
		}
|	additive_expression PLUS_TK error
		{yyerror ("Missing term"); RECOVER;}
|	additive_expression MINUS_TK error
		{yyerror ("Missing term"); RECOVER;}
;

shift_expression:
	additive_expression
|	shift_expression LS_TK additive_expression
		{
		  $$ = build_binop (BINOP_LOOKUP ($2.token), $2.location,
				    $1, $3);
		}
|	shift_expression SRS_TK additive_expression
		{
		  $$ = build_binop (BINOP_LOOKUP ($2.token), $2.location,
				    $1, $3);
		}
|	shift_expression ZRS_TK additive_expression
		{
		  $$ = build_binop (BINOP_LOOKUP ($2.token), $2.location,
				    $1, $3);
		}
|	shift_expression LS_TK error
		{yyerror ("Missing term"); RECOVER;}
|	shift_expression SRS_TK error
		{yyerror ("Missing term"); RECOVER;}
|	shift_expression ZRS_TK error
		{yyerror ("Missing term"); RECOVER;}
;

relational_expression:
	shift_expression
|	relational_expression LT_TK shift_expression
		{
		  $$ = build_binop (BINOP_LOOKUP ($2.token), $2.location,
				    $1, $3);
		}
|	relational_expression GT_TK shift_expression
		{
		  $$ = build_binop (BINOP_LOOKUP ($2.token), $2.location,
				    $1, $3);
		}
|	relational_expression LTE_TK shift_expression
		{
		  $$ = build_binop (BINOP_LOOKUP ($2.token), $2.location,
				    $1, $3);
		}
|	relational_expression GTE_TK shift_expression
		{
		  $$ = build_binop (BINOP_LOOKUP ($2.token), $2.location,
				    $1, $3);
		}
|	relational_expression INSTANCEOF_TK reference_type
		{ $$ = build_binop (INSTANCEOF_EXPR, $2.location, $1, $3); }
|	relational_expression LT_TK error
		{yyerror ("Missing term"); RECOVER;}
|	relational_expression GT_TK error
		{yyerror ("Missing term"); RECOVER;}
|	relational_expression LTE_TK error
		{yyerror ("Missing term"); RECOVER;}
|	relational_expression GTE_TK error
		{yyerror ("Missing term"); RECOVER;}
|	relational_expression INSTANCEOF_TK error
		{yyerror ("Invalid reference type"); RECOVER;}
;

equality_expression:
	relational_expression
|	equality_expression EQ_TK relational_expression
		{
		  $$ = build_binop (BINOP_LOOKUP ($2.token), $2.location,
				    $1, $3);
		}
|	equality_expression NEQ_TK relational_expression
		{
		  $$ = build_binop (BINOP_LOOKUP ($2.token), $2.location,
				    $1, $3);
		}
|	equality_expression EQ_TK error
		{yyerror ("Missing term"); RECOVER;}
|	equality_expression NEQ_TK error
		{yyerror ("Missing term"); RECOVER;}
;

and_expression:
	equality_expression
|	and_expression AND_TK equality_expression
		{
		  $$ = build_binop (BINOP_LOOKUP ($2.token), $2.location,
				    $1, $3);
		}
|	and_expression AND_TK error
		{yyerror ("Missing term"); RECOVER;}
;

exclusive_or_expression:
	and_expression
|	exclusive_or_expression XOR_TK and_expression
		{
		  $$ = build_binop (BINOP_LOOKUP ($2.token), $2.location,
				    $1, $3);
		}
|	exclusive_or_expression XOR_TK error
		{yyerror ("Missing term"); RECOVER;}
;

inclusive_or_expression:
	exclusive_or_expression
|	inclusive_or_expression OR_TK exclusive_or_expression
		{
		  $$ = build_binop (BINOP_LOOKUP ($2.token), $2.location,
				    $1, $3);
		}
|	inclusive_or_expression OR_TK error
		{yyerror ("Missing term"); RECOVER;}
;

conditional_and_expression:
	inclusive_or_expression
|	conditional_and_expression BOOL_AND_TK inclusive_or_expression
		{
		  $$ = build_binop (BINOP_LOOKUP ($2.token), $2.location,
				    $1, $3);
		}
|	conditional_and_expression BOOL_AND_TK error
		{yyerror ("Missing term"); RECOVER;}
;

conditional_or_expression:
	conditional_and_expression
|	conditional_or_expression BOOL_OR_TK conditional_and_expression
		{
		  $$ = build_binop (BINOP_LOOKUP ($2.token), $2.location,
				    $1, $3);
		}
|	conditional_or_expression BOOL_OR_TK error
		{yyerror ("Missing term"); RECOVER;}
;

conditional_expression:		/* Error handling here is weak */
	conditional_or_expression
|	conditional_or_expression REL_QM_TK expression REL_CL_TK conditional_expression
		{
		  $$ = build (CONDITIONAL_EXPR, NULL_TREE, $1, $3, $5);
		  EXPR_WFL_LINECOL ($$) = $2.location;
		}
|	conditional_or_expression REL_QM_TK REL_CL_TK error
		{
		  YYERROR_NOW;
		  yyerror ("Missing term");
		  DRECOVER (1);
		}
|	conditional_or_expression REL_QM_TK error
		{yyerror ("Missing term"); DRECOVER (2);}
|	conditional_or_expression REL_QM_TK expression REL_CL_TK error
		{yyerror ("Missing term"); DRECOVER (3);}
;

assignment_expression:
	conditional_expression
|	assignment
;

assignment:
	left_hand_side assignment_operator assignment_expression
		{ $$ = build_assignment ($2.token, $2.location, $1, $3); }
|	left_hand_side assignment_operator error
		{
		  YYNOT_TWICE yyerror ("Missing term");
		  DRECOVER (assign);
		}
;

left_hand_side:
	name
|	field_access
|	array_access
;

assignment_operator:
	assign_any
|	ASSIGN_TK
;

assign_any:
	PLUS_ASSIGN_TK
|	MINUS_ASSIGN_TK
|	MULT_ASSIGN_TK
|	DIV_ASSIGN_TK
|	REM_ASSIGN_TK
|	LS_ASSIGN_TK
|	SRS_ASSIGN_TK
|	ZRS_ASSIGN_TK
|	AND_ASSIGN_TK
|	XOR_ASSIGN_TK
|	OR_ASSIGN_TK
;


expression:
	assignment_expression
;

constant_expression:
	expression
;

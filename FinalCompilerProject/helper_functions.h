#pragma once

extern char* C_KEYWORD_ARRAY[];    //    size of 9 at the moment

unsigned int DEBUG;
unsigned int PAR_DEBUG;
unsigned int LEX_DEBUG;
int hel;    // To keep track of the highest severity of error, 0 = no errors, 1 = warning, 2 = error, 3 = fatal


/* Used in symbol_node struct, will hold the value of the given symbol if the symbol's kind is ID (a variable).*/
union data
{
    int d;
    double f;
};

/* Basic element of the symbol table, which is a hash table with linked-list collision resolution */
struct symbol_node
{
    char* symbol;
    union data val;
    int kind;
    struct symbol_node* next;
};

/* symbol tables are a dynamically allocated array of symbol_node structs */
typedef struct symbol_node* SYMBOL_TABLE;

/* Used to track existing scopes - effectively implemented as a stack of scope_node structs */
struct scope_node
{
    SYMBOL_TABLE symTab;
    struct scope_node* next;
    int is_new_scope;
};

/*
 * Used to track string literals found by the lexer, length will effectively be the starting memory address of
 * the string in VMQ's global memory space.  NOTE: THIS ASSUMES NO GLOBAL VARIABLES IN THE C++ FILE BEING COMPILED.
*/
struct strlit_node
{
    char* str;
    unsigned int loc;
    struct strlit_node* next;
};


typedef struct strlit_node* STRLIT_LIST;

struct strlit_node* appendToStrList(STRLIT_LIST* head, char* str, int eval_state);

// struct strlit_node* appendToStrStack(char* str);

/*
    Scope node will be used to keep track of the existing variables within the "live" scopes.
    Global scope will always exist, and as the parser enters new functions and blocks "deeper"
    scopes will come into existance.  A pointer will be kept that points to the global scope_node.
    Variables will be checked to see if they exist in global scope first and if not found then subsequent
    scopes will be checked.  If the variable is not found, then we have two basic options:

    1)    Declare it an error, in violation of C++ rules.

                            OR

    2)    Add it to the symbol table and work under the assumption that it is an integer intialized to 0.

    This will come into play later once we start needing to worry about variables.  We should get
    Hello, World! done first.
*/

/* Deletes current_scope struct, sets current_scope struct to correct scope post-pop */
void popScope();

/* Generates and initializes new scope struct, returns scope struct pointer of new scope */
struct scope_node* pushScope();

/* Generates hash key for given symbol */
unsigned int hash(const char* symbol);

/* Looks for symbol in tables, returns address of record that contains symbol (NULL, if not found) */
struct symbol_node* lookup(const char* symbol);

/* Adds symbol to current table, in the current scope.  Returns the address of the symbol_node. */
struct symbol_node* addSymbol(SYMBOL_TABLE symTab, char* symbol, int kind);

/* Generates a new scope_node struct and initilizes its symbol table. */
struct scope_node* initializeScope();

/* Generates a new, empty symbol table */
SYMBOL_TABLE generateSymbolTable(unsigned int table_size);

/* Populates passed symbol table with C keywords */
void populateSymbolTable(SYMBOL_TABLE symTab);

/* Prints value of passed symbol_record ptr, as well as info about the record pointed to. */
void printSymbolData(struct symbol_node* sym);

/* Returns the correct symbol_type of a string that is a C keyword for the project */
int getKind(char *str);

/* Prints the string equivalent of any valid symbol_type value passed to the function */
char* kindToString(int kind);

/*
 * Enum to pass to the yyerror function, making it easier to see
 * the level of severity of the error.
 */
typedef enum errorSeverity { warning = 1, error, fatal } errorLevel;

/*
 * Error function that can takes in a string description of the error that will be outputted to User.
 */
void yyerror(char *s, ...);

/*
 * Error function that takes in the enum indicating severity of the error and also
 * a string description of the error.
 */
void pError(errorLevel el, char *s, ...);

/*
 * AST node types.
 * There will be multiple types of ast nodes according to the specifications
 * in the book. These are rough drafts.
 */

//Base tree node
struct ast {
 int nodetype;
 struct ast *l;
 struct ast *r;
};

/*
    Tom:  Here's the plan for having this compiler push out a Hello World program.

    This would be much simpler if we couldn't nest output statements (e.g., cout << "str" << endl;
    is technically two output statements).  Due to this, we'll need to track how many things to
    output so we can make the correct calls in VMQ.  Best(ish) way to do this is a stack.

    The parser will end up parsing output statements precisely how they are read ...

    cout << "str" << (x + 5) << endl; will trigger the output_statement syntax rule actions from left to right order:

    COUT rule -> outstmt STREAMOUT STR_LIT rule -> outstmt STREAMOUT expr rule -> outstmt STREAMOUT ENDL rule

    This greatly simplifies the process from my previous understanding of how the parser deals with output statements.


    I've already taken care of calculating the VMQ addresses of string literals (see the appendToStrList()
    and strlit_node function and data structure for more info).  Essentially, after the lexer passes through
    the .cpp, all of the string literals (along with their VMQ global memory addresses) will be in a linked
    list, the head of which is available globally.  Each of the stringval AST's the parser creates will
    contain a pointer to the respective list element contained in the global string literal linked list.

    So, let's go over how the pre-cout node stack will work.  Each of the statements leading up to the
    cout node will be evaluated, the result of which (a struct containing the type of data and its VMQ mem loc)
    will be pushed onto the stack.  Once we reach the cout node, the stack will contain all of the data we need
    to make the appropriate VMQ output function calls.  Here's what a block of VMQ code looks like for a string
    output function call:

        p #4            ; Push a two-byte value (integer) onto the stack, in this case the str literal stored at loc 4.
        c 0 -11            ; calls str output function (-11 is the op-code for this, 0 just tosses the function output).
        ^ 2                ; Pop the two-bytes we pushed onto the stack (clean up).

    So once we hit the cout node and evaluate it, the evaluation function can perform the following actions:

    0) Check to see if the stack is empty, if no proceed to 1, if yes proceed to 7.
    1) Peek the top of the stack (remember, the stack contains the data type and VMQ address of the evaluated statements).
    2) Write the VMQ push statement using the data-type and VMQ address info from the struct on the top of the stack.
    3) Write the appropriate function (<c 0 -11> is for str output, <c 0 -9> is for int output - see documentation).
    4) Write the VMQ pop statement.
    5) Pop the top of the stack.
    6) Jump to 0.
    7) Free the cout AST node.

    That will take care of the output statements.

    There is a bit of trickiness in this, which is that the initial VMQ setup code (setting up global memory space and
    initializing the runtime environment) will need to be inserted above the existing function code that the evaluation
    function outputs.  I'm unfamiliar with C file I/O, so I don't know if there will be an easy way to insert into a file
    above pre-existing text.

    If there isn't an easy way to do the above, then alternatively we could just store all of the VMQ code lines in a linked
    list of strings, then write the initial VMQ setup code to a file, then dump the linked list of strings into the file after.

    Another method we could use is to just have the lexer perform a single scan over the file to grab all of the string
    literals, write the initial VMQ setup to the file, and then run the parser through the file as normal to get the rest
    of the VMQ code generated.

    I'll have to see what method would work best once I get there.

    In summary:  How to generate output statements.

    Example:    cout << "Hello, world!" << endl;

                    (nodetype = 345, str = 0xwhatever) -> (nodetype = 345, str = 0xwhatev3r) -> (output node)


    First node is evaluated (the node pertaining to endl), result pushed onto stack:    (str = "\n", loc = 0) <- stack top
    Second node is evaluated (node for "Hello, world!"), result pushed onto stack:        (str = "\n", loc = 0), (str = "Hello, world!", loc = 3) <- stack top

    Output node is evaluated.  The stack we were just populating is processed using the (0) - (7) steps above.

    If we opt to write directly to the file, then this part is simple.  If we're going to put it all in at the end, then we need to store
    generated VMQ lines somewhere until the file is completely parsed.

    That's it I suppose.  Hit me up on Skype if you have any questions.  I'm also usually in the CS lab on Monday/Wednesday/Friday from about 12 to 3.

*/

//string literal
struct stringval {
    int nodetype;    //    = 's' + 't' + 'r' = 115 + 116 + 114 = 345
    struct strlit_node* str; // points to struct that contains string and VMQ global memory location of string.
};

//int literal
struct intval {
 int nodetype;
 int number;
};

//float literal
struct floatval {
 int nodetype;
 float number;
};

//flow node
struct flow {
 int nodetype; /* Should accept type I(f), W(while) and F(or) */
 struct ast *cond; /* pointer to bool condition */
 struct ast *tl; /* if true, parse this tree */
 struct ast *el; /* else parse this tree */
};

//reference node(i.e. if you need to call a variable)
struct symref {
 int nodetype;    /* Potentially type N like the calculator */
 struct symbol_node *s;
};

//assignment node
struct symasgn {
 int nodetype;    /* type = */
 struct symbol_node *s;
 struct ast *v; /* value */
};

 /*
  * functions to build the AST
  */
struct ast *newast(int nodetype, struct ast *l, struct ast *r);
struct ast *newstr(struct strlit_node* strliteral);
struct ast *newint(int num);
struct ast *newfloat(float num);
struct ast *newflow(int nodetype, struct ast *cond, struct ast *tl, struct ast *tr);
struct ast *newref(struct symbol_node *s);
struct ast *newasgn(struct symbol_node *s, struct ast *v);
struct ast *newrel(int reltype, struct ast *l, struct ast *r);
  /*
   * Function to delete and free an AST
   */
void treefree(struct ast *);

// Pointer to head of stack containing VMQ statements (strings).
typedef struct strlit_node* VMQ_STACK;

struct eval_data
{
    int d;
    double f;
    char* str;
//    VMQ_STACK stack_head;
    STRLIT_LIST list_head;
};

// Function that will evaluate the AST, recursively.
void eval(struct ast *a);

void transferStack(VMQ_STACK dest, VMQ_STACK src);

void pushToStrStack(VMQ_STACK stk, char* str);

void popStrStack(VMQ_STACK stk);

// DEBUG print statement for AST's
void printAST(struct ast *a);

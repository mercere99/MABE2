Type System
Symbol Tables
Parser

LEVEL MAP:

Symbol            - []
Lexer             - []

SymbolTableBase   - [Symbol]

TypeInfo          - [Symbol,SymbolTableBase] Basic information for a user-defined type.
Symbol_Function   - [Symbol]
Symbol_Linked     - [Symbol]

Symbol_Scope      - [Symbol,Symbol_Function,Symbol_Linked,TypeInfo]

EmplodeType       - [Symbol_Scope,TypeInfo]

Symbol_Object     - [Symbol_Scope,EmplodeType]

AST               - [Symbol_Object,Symbol,SymbolTableBase]

Events            - [AST]
DataFile          - [EmplodeType]

SymbolTable       - [Events,Symbol_Scope]

Parser            - [AST,Lexer,SymbolTable]

Emplode           - [ALL]


TODO:

* Emplode as a whole should move from MABE to Empirical

* We need a consistent and functional ERROR SYSTEM.
Right now we either output direct to the command line OR create a Symbol_Error
with a meaningful error included, but it's then ignored and never printed.
What should happen is that an error message is raised and the appropriate interface
module handles it.

* Events need to be made more flexible.
Right now, all events are "timer" based, which means they are given a threshold for
when a particular signal variable reaches a certain value they start running and then
each time it makes it to the next threshold they run again until they pass the end
value.  Any of these variables can be disabled (so, for example, @start only runs
events once at the beginning), but each event type should be able to have its own
parameter types.

* Functions should be able to be defined inside of a config script.

* Arrays (possibly including generators that act like open-ended arrays).

* Literal arrays/generators.
5:10 should be 5,15,25,35,... while 6:1:9 should be 6,7,8,9
(More sophisticate schemes are also out there, and we could match them...)

* Built-in types for scopes, probably beginning with an underscore.
We now have _active, which if set to false will deactivate an object.  We have _desc 
added to handle custom descriptions for object (different from the class description),
but it's not hooked in.

* Need a stand-alone config interpreter along with a full test suite to make sure it's
working properly.

* Should be able to make whole new types (not just new instances) inside the interpreter.

* Should be able to adjust default values for existing types.  This will allow for config
files to be included in that just setup defaults, but don't actually build any new object.

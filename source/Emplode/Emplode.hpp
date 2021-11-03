/**
 *  @note This file is part of Emplode, currently within https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2021.
 *
 *  @file  Emplode.hpp
 *  @brief Manages all configuration with Emplode language.
 *  @note Status: BETA
 * 
 *  Example usage:
 *   Value a = 7;              // a is a variable with the value 7
 *   String b = "balloons";    // b is a variable equal to the literal string "balloons"
 *   Value c = a + 10;         // '+' will add values; c is a variable equal to 17.
 *   String d = "99 " + b;     // '+' will append strings; d is a variable equal to "99 balloons"
 *   // String e = "abc" + 12; // ERROR - cannot add strings and values!
 *   String  = "01" * a;       // e is now "01010101010101"
 *   Struct f = {              // f is a structure/scope/dictionary
 *     Value g = 1.7;          // Values are floating point.
 *     String h = "two";
 *     Struct i = {            // Structure-within-structures are allowed.
 *       Value j = 3;
 *     }
 *     String a = "shadow!";   // Variables can be redeclared in other scopes; shadows original.
 *     String j = "spooky!";   // A NEW variable since we are now out of Struct i.
 *     j = .a;                 // Change j to "shadow!"; initial . indicates current namespace.
 *     b = i.j;                // Namespaces can be stepped through with dots.
 *     c = ..a;                // A variable name beginning with a ".." indicates parent namespace.
 *   }                         // f has been initialized with six variables in its scope.
 *
 *   --- The functionality below does not yet work and may change when implemented ---
 *   f["g"] = 2.5;       // You can also access elements though indexing.
 *   f["new"] = 22;      // You can always add new fields to structures via indexing.
 *   // d["bad"] = 4;    // ERROR - You cannot add fields to non-structures.
 *   k = [ 1 , 2 , 3];   // k is a vector of values (vectors must have all types the same!)
 *   l = k[1];           // Vectors can be indexed into.
 *   m() = a * c;        // Functions have parens after the variable name; evaluated when called.
 *   n(o,p) = o + p;     // Functions may have arguments.
 *   q = 'q';            // Literal chars are translated immediately to their ascii value
 * 
 *   // use a : instead of a . to access built-in values.  Note a leading colon uses current scope.
 *   r = k:scope_size;         // = 3  (always a value)
 *   s = f:names;        // = ["a","b","c","g","h","i","j"] (vector of strings in alphabetical order)
 *   t = c:string;       // = "17"  (convert value to string)
 *   u = (t+"00"):value; // = 1700  (convert string to value; can use temporaries!)
 *   // ALSO- :is_string, :is_value, :is_struct, :is_array (return 0 or 1)
 *   //       :type (returns a string indicating type!)
 * 
 * 
 *  In practice, most settings will be pre-defined in typed scopes:
 *   MarkovBrain Sheep = {
 *     outputs = 10;
 *     node_weights = 0.75;
 *     recurrance = 5;
 *   }
 *   MarkovBrain Wolves = {
 *     outputs = 10;
 *     node_weights = 0.75;
 *     recurrance = 3;
 *   }
 *   modules = {
 *     Mutations = {
 *       copy_prob = 0.001;
 *       insert_prob = 0.05;
 *     }
 *   }
 */

#ifndef EMPLODE_HPP
#define EMPLODE_HPP

#include <fstream>
#include <iostream>
#include <string>
#include <utility>

#include "emp/base/assert.hpp"
#include "emp/base/map.hpp"
#include "emp/meta/TypeID.hpp"
#include "emp/tools/string_utils.hpp"

#include "AST.hpp"
#include "DataFile.hpp"
#include "EmplodeType.hpp"
#include "Symbol_Function.hpp"
#include "SymbolTable.hpp"
#include "Events.hpp"
#include "Lexer.hpp"
#include "TypeInfo.hpp"

namespace emplode {

  class ParseState {
  private:
    emp::TokenStream::Iterator pos;
    emp::Ptr<SymbolTable> symbol_table;
    emp::vector< emp::Ptr<Symbol_Scope> > scope_stack;
    emp::Ptr<Lexer> lexer;

  public:
    ParseState(emp::TokenStream::Iterator _pos, SymbolTable & _table,
               Symbol_Scope & _scope, Lexer & _lexer)
      : pos(_pos), symbol_table(&_table), lexer(&_lexer) { scope_stack.push_back(&_scope); }
    ParseState(ParseState &) = default;
    ~ParseState() { }

    ParseState & operator=(const ParseState &) = default;

    bool operator==(const ParseState & in) const { return pos == in.pos; }
    bool operator!=(const ParseState & in) const { return pos != in.pos; }
    bool operator< (const ParseState & in) const { return pos <  in.pos; }
    bool operator<=(const ParseState & in) const { return pos <= in.pos; }
    bool operator> (const ParseState & in) const { return pos >  in.pos; }
    bool operator>=(const ParseState & in) const { return pos >= in.pos; }

    ParseState & operator++() { ++pos; return *this; }
    ParseState operator++(int) { ParseState old(*this); ++pos; return old; }
    ParseState & operator--() { --pos; return *this; }
    ParseState operator--(int) { ParseState old(*this); --pos; return old; }

    bool IsValid() const { return pos.IsValid(); }
    bool AtEnd() const { return pos.AtEnd(); }

    size_t GetIndex() const { return pos.GetIndex(); }  ///< Return index in token stream.
    size_t GetTokenSize() const { return pos.IsValid() ? pos->lexeme.size() : 0; }
    Symbol_Scope & GetScope() {
      emp_assert(scope_stack.size() && scope_stack.back() != nullptr);
      return *scope_stack.back();
    }
    const Symbol_Scope & GetScope() const {
      emp_assert(scope_stack.size() && scope_stack.back() != nullptr);
      return *scope_stack.back();
    }
    const std::string & GetScopeName() const { return GetScope().GetName(); }

    std::string AsString() {
      return emp::to_string("[pos=", pos.GetIndex(),
                            ",lex='", AsLexeme(),
                            "',scope='", GetScope().GetName(),
                            "']");
    }

    bool IsID() const { return pos && lexer->IsID(*pos); }
    bool IsNumber() const { return pos && lexer->IsNumber(*pos); }
    bool IsChar() const { return pos && lexer->IsChar(*pos); }
    bool IsString() const { return pos && lexer->IsString(*pos); }
    bool IsDots() const { return pos && lexer->IsDots(*pos); }

    bool IsEvent() const { return symbol_table->HasEvent(AsLexeme()); }
    bool IsType() const { return symbol_table->HasType(AsLexeme()); }

    /// Convert the current state to a character; use \0 if cur token is not a symbol.
    char AsChar() const { return (pos && lexer->IsSymbol(*pos)) ? pos->lexeme[0] : 0; }

    /// Return the lexeme associate with the current state.
    const std::string & AsLexeme() const { return pos ? pos->lexeme : emp::empty_string(); }

    /// Return the lexeme associate with the current state AND advance the token stream.
    const std::string & UseLexeme() {
      const std::string & out = AsLexeme();
      pos++;
      return out;
    }

    /// Return whether the current token is the specified char; if so also advance token stream.
    bool UseIfChar(char test_char) {
      if (AsChar() != test_char) return false;
      ++pos;
      return true;
    }

    /// Report an error in parsing this file and exit.
    template <typename... Ts>
    void Error(Ts &&... args) const {
      std::string line_info = pos.AtEnd() ? "end of input" : emp::to_string("line ", pos->line_id);
      std::cout << "Error (" << line_info << " in '" << pos.GetTokenStream().GetName() << "'): "
                << emp::to_string(std::forward<Ts>(args)...) << "\nAborting." << std::endl;
      exit(1);
    }

    template <typename... Ts>
    void Require(bool test, Ts &&... args) const {
      if (!test) Error(std::forward<Ts>(args)...);
    }
    template <typename... Ts>
    void RequireID(Ts &&... args) const {
      if (!IsID()) Error(std::forward<Ts>(args)...);
    }
    template <typename... Ts>
    void RequireNumber(Ts &&... args) const {
      if (!IsNumber()) Error(std::forward<Ts>(args)...);
    }
    template <typename... Ts>
    void RequireString(Ts &&... args) const {
      if (!IsString()) Error(std::forward<Ts>(args)...);
    }
    template <typename... Ts>
    void RequireChar(char req_char, Ts &&... args) const {
      if (AsChar() != req_char) Error(std::forward<Ts>(args)...);
    }
    template <typename... Ts>
    void RequireLexeme(const std::string & lex, Ts &&... args) const {
      if (AsLexeme() != lex) Error(std::forward<Ts>(args)...);
    }

    template <typename... Ts>
    void UseRequiredChar(char req_char, Ts &&... args) {
      if (AsChar() != req_char) Error(std::forward<Ts>(args)...);
      ++pos;      
    }

    void PushScope(Symbol_Scope & _scope) { scope_stack.push_back(&_scope); }
    void PopScope() { scope_stack.pop_back(); }

    Symbol & LookupSymbol(const std::string & var_name, bool scan_scopes) {
      emp::Ptr<Symbol> out_symbol = GetScope().LookupSymbol(var_name, scan_scopes);
      // If we can't find this identifier, throw an error.
      if (out_symbol.IsNull()) {
        Error("'", var_name, "' does not exist as a parameter, variable, or type.",
              "  Current scope is '", GetScope().GetName(), "'");
      }
      return *out_symbol;
    }

    Symbol_StringVar & AddStringVar(const std::string & name, const std::string & desc) {
      return GetScope().AddStringVar(name, desc);
    }
    Symbol_DoubleVar & AddValueVar(const std::string & name, const std::string & desc) {
      return GetScope().AddValueVar(name, desc);
    }
    Symbol_Scope & AddScope(const std::string & name, const std::string & desc) {
      return GetScope().AddScope(name, desc);
    }
    Symbol_Object & AddObject(const std::string & type_name, const std::string & var_name) {
      return symbol_table->MakeObjSymbol(type_name, var_name, GetScope());
    }
  };


  class Emplode {
  public:
    using pos_t = emp::TokenStream::Iterator;

  protected:
    std::string filename;      ///< Source for for code to generate.
    Lexer lexer;               ///< Lexer to process input code.
    SymbolTable symbol_table;  ///< Management of identifiers.
    ASTNode_Block ast_root;    ///< Abstract syntax tree version of input file.
    bool debug = false;        ///< Should we print full debug information?
    std::unordered_map<std::string, size_t> precedence_map;  ///< Precedence levels for symbols.

    std::string ConcatLexemes(pos_t start_pos, pos_t end_pos) const {
      emp_assert(start_pos <= end_pos);
      emp_assert(start_pos.IsValid() && end_pos.IsValid());
      std::stringstream ss;
      while (start_pos < end_pos) {
        ss << start_pos->lexeme;
        if (start_pos != end_pos) ss << " ";     // spaces between tokens.
        if (start_pos->lexeme == ";") ss << " "; // extra space after semi-colons for now...
        ++start_pos;
      }
      return ss.str();
    }

    template <typename... Ts>
    void Debug(Ts... args) const {
      if (debug) std::cout << "DEBUG: " << emp::to_string(std::forward<Ts>(args)...) << std::endl;
    }


    /// Load a variable name from the provided scope.
    /// If create_ok is true, create any variables that we don't find.  Otherwise continue the
    /// search for them in successively outer (lower) scopes.
    [[nodiscard]] emp::Ptr<ASTNode_Leaf> ParseVar(ParseState & state,
                                                  bool create_ok=false,
                                                  bool scan_scopes=true);

    /// Load a value from the provided scope, which can come from a variable or a literal.
    [[nodiscard]] emp::Ptr<ASTNode> ParseValue(ParseState & state);

    /// Calculate the result of the provided operation on two computed entries.
    [[nodiscard]] emp::Ptr<ASTNode> ProcessOperation(const std::string & symbol,
                                                     emp::Ptr<ASTNode> value1,
                                                     emp::Ptr<ASTNode> value2);

    /// Calculate a full expression found in a token sequence, using the provided scope.
    [[nodiscard]] emp::Ptr<ASTNode> ParseExpression(ParseState & state, size_t prec_limit=1000);

    /// Parse the declaration of a variable and return the newly created Symbol
    Symbol & ParseDeclaration(ParseState & state);

    /// Parse an event description.
    emp::Ptr<ASTNode> ParseEvent(ParseState & state);

    /// Parse the next input in the specified Struct.  A statement can be a variable declaration,
    /// an expression, or an event.
    [[nodiscard]] emp::Ptr<ASTNode> ParseStatement(ParseState & state);

    /// Keep parsing statements until there aren't any more or we leave this scope. 
    [[nodiscard]] emp::Ptr<ASTNode_Block> ParseStatementList(ParseState & state) {
      Debug("Running ParseStatementList(", state.AsString(), ")");
      auto cur_block = emp::NewPtr<ASTNode_Block>(state.GetScope());
      while (state.IsValid() && state.AsChar() != '}') {
        // Parse each statement in the file.
        emp::Ptr<ASTNode> statement_node = ParseStatement(state);

        // If the current statement is real, add it to the current block.
        if (!statement_node.IsNull()) cur_block->AddChild( statement_node );
      }
      return cur_block;
    }

  public:
    Emplode(std::string in_filename="")
      : filename(in_filename)
      , symbol_table("Emplode")
      , ast_root(symbol_table.GetRootScope())
    {
      if (filename != "") Load(filename);

      // Setup operator precedence.
      size_t cur_prec = 0;
      precedence_map["("] = cur_prec++;
      precedence_map["**"] = cur_prec++;
      precedence_map["*"] = precedence_map["/"] = precedence_map["%"] = cur_prec++;
      precedence_map["+"] = precedence_map["-"] = cur_prec++;
      precedence_map["<"] = precedence_map["<="] = precedence_map[">"] = precedence_map[">="] = cur_prec++;
      precedence_map["=="] = precedence_map["!="] = cur_prec++;
      precedence_map["&&"] = cur_prec++;
      precedence_map["||"] = cur_prec++;
      precedence_map["="] = cur_prec++;

      // Setup default functions.

      // 'EXEC' dynamically executes the contents of a string.
      auto exec_fun = [this](const std::string & expression) { return Execute(expression); };
      AddFunction("EXEC", exec_fun, "Dynamically execute the string passed in.");

      // 'PRINT' is a simple debugging command to output the value of a variable.
      auto print_fun = [](const emp::vector<emp::Ptr<Symbol>> & args) {
          for (auto entry_ptr : args) std::cout << entry_ptr->AsString();
          return 0;
        };
      AddFunction("PRINT", print_fun, "Print out the provided variables.");

      // Default 1-input math functions
      AddFunction("ABS", [](double x){ return std::abs(x); }, "Absolute Value" );
      AddFunction("EXP", [](double x){ return emp::Pow(emp::E, x); }, "Exponentiation" );
      AddFunction("LOG2", [](double x){ return std::log(x); }, "Log base-2" );
      AddFunction("LOG10", [](double x){ return std::log10(x); }, "Log base-10" );

      AddFunction("SQRT", [](double x){ return std::sqrt(x); }, "Square Root" );
      AddFunction("CBRT", [](double x){ return std::cbrt(x); }, "Cube Root" );

      AddFunction("SIN", [](double x){ return std::sin(x); }, "Sine" );
      AddFunction("COS", [](double x){ return std::cos(x); }, "Cosine" );
      AddFunction("TAN", [](double x){ return std::tan(x); }, "Tangent" );
      AddFunction("ASIN", [](double x){ return std::asin(x); }, "Arc Sine" );
      AddFunction("ACOS", [](double x){ return std::acos(x); }, "Arc Cosine" );
      AddFunction("ATAN", [](double x){ return std::atan(x); }, "Arc Tangent" );
      AddFunction("SINH", [](double x){ return std::sinh(x); }, "Hyperbolic Sine" );
      AddFunction("COSH", [](double x){ return std::cosh(x); }, "Hyperbolic Cosine" );
      AddFunction("TANH", [](double x){ return std::tanh(x); }, "Hyperbolic Tangent" );
      AddFunction("ASINH", [](double x){ return std::asinh(x); }, "Hyperbolic Arc Sine" );
      AddFunction("ACOSH", [](double x){ return std::acosh(x); }, "Hyperbolic Arc Cosine" );
      AddFunction("ATANH", [](double x){ return std::atanh(x); }, "Hyperbolic Arc Tangent" );

      AddFunction("CEIL", [](double x){ return std::ceil(x); }, "Round UP" );
      AddFunction("FLOOR", [](double x){ return std::floor(x); }, "Round DOWN" );
      AddFunction("ROUND", [](double x){ return std::round(x); }, "Round to nearest" );

      AddFunction("ISINF", [](double x){ return std::isinf(x); }, "Test if Infinite" );
      AddFunction("ISNAN", [](double x){ return std::isnan(x); }, "Test if Not-a-number" );

      // Default 2-input math functions
      AddFunction("HYPOT", [](double x, double y){ return std::hypot(x,y); }, "Given sides, find hypotenuse" );
      AddFunction("LOG", [](double x, double y){ return emp::Pow(x,y); }, "Take log of arg1 with base arg2" );
      AddFunction("MIN", [](double x, double y){ return (x<y) ? x : y; }, "Return lesser value" );
      AddFunction("MAX", [](double x, double y){ return (x>y) ? x : y; }, "Return greater value" );
      AddFunction("POW", [](double x, double y){ return emp::Pow(x,y); }, "Take arg1 to the arg2 power" );

      // Default 3-input math functions
      auto math3_if = [](double x, double y, double z){ return (x!=0.0) ? y : z; };
      AddFunction("IF", math3_if, "If arg1 is true, return arg2, else arg3" );
      auto math3_clamp = [](double x, double y, double z){ return (x<y) ? y : (x>z) ? z : x; };
      AddFunction("CLAMP", math3_clamp, "Return arg1, forced into range [arg2,arg3]" );
      auto math3_to_scale = [](double x, double y, double z){ return (z-y)*x+y; };
      AddFunction("TO_SCALE", math3_to_scale, "Scale arg1 to arg2-arg3 as unit distance" );
      auto math3_from_scale = [](double x, double y, double z){ return (x-y) / (z-y); };
      AddFunction("FROM_SCALE", math3_from_scale, "Scale arg1 from arg2-arg3 as unit distance" );

      // Setup default DataFile type.
      auto df_init = [this](const std::string & name) {
        return emp::NewPtr<DataFile>(name, symbol_table.GetFileManager());
      };
      auto & df_type = AddType<DataFile>("DataFile", "Manage CSV-style date file output.", df_init, true);
      df_type.AddMemberFunction(
        "ADD_COLUMN",
        [exec_fun](DataFile & file, const std::string & title, const std::string & expression){
          return file.AddColumn(title, [exec_fun,expression](){
            std::string out_string = exec_fun(expression);
            if (!emp::is_number(out_string)) return emp::to_literal(out_string);
            return out_string;
          });
        },
        "Add a column to the associated DataFile.  Args: title, string to execute for result"
      );
    }

    // Prevent copy or move since we are using lambdas that capture 'this'
    Emplode(const Emplode &) = delete;
    Emplode(Emplode &&) = delete;
    Emplode & operator=(const Emplode &) = delete;
    Emplode & operator=(Emplode &&) = delete;

    /// Create a new type of event that can be used in the scripting language.
    Events & AddEventType(const std::string & name) { return symbol_table.AddEventType(name); }

    /// Add an instance of an event with an action that should be triggered.
    template <typename... Ts>
    void AddEvent(Ts &&... args) { symbol_table.AddEvent(std::forward<Ts>(args)...); }

    /// Indicate the an event trigger value has been updated; trigger associated events.
    void UpdateEventValue(const std::string & name, double new_value) {
      symbol_table.UpdateEventValue(name, new_value);
    }

    /// Trigger all events of a type (ignoring trigger values)
    void TriggerEvents(const std::string & name) { symbol_table.TriggerEvents(name); }


    template <typename... EXTRA_Ts, typename... ARG_Ts>
    TypeInfo & AddType(ARG_Ts &&... args) {
      return symbol_table.AddType<EXTRA_Ts...>( std::forward<ARG_Ts>(args)... );
    }


    /// To add a built-in function (at the root level) provide it with a name and description.
    /// As long as the function only requires types known to the config system, it should be
    /// converted properly.  For a variadic function, the provided function must take a
    /// vector of ASTNode pointers, but may return any known type.
    template <typename FUN_T>
    void AddFunction(const std::string & name, FUN_T fun, const std::string & desc) {
      symbol_table.AddFunction(name, fun, desc);
    }

    SymbolTable & GetSymbolTable() { return symbol_table; }
    const SymbolTable & GetSymbolTable() const { return symbol_table; }

    // Load a single, specified configuration file.
    void Load(const std::string & filename) {
      Debug("Running Load(", filename, ")");
      std::ifstream file(filename);           // Load the provided file.
      emp::TokenStream tokens = lexer.Tokenize(file, filename);          // Convert to more-usable tokens.
      file.close();                           // Close the file (now that it's converted)
      pos_t pos = tokens.begin();             // Start at the beginning of the file.

      // Parse and run the program, starting from the outer scope.
      ParseState state{pos, symbol_table, symbol_table.GetRootScope(), lexer};
      auto cur_block = ParseStatementList(state);
      cur_block->Process();

      // Store this AST onto the full set we're working with.
      ast_root.AddChild(cur_block);
    }

    // Sequentially load a series of configuration files.
    void Load(const emp::vector<std::string> & filenames) {
      for ( const std::string & fn : filenames) Load(fn);
    }

    // Load a single, specified configuration file.
    // @param statements List is statements to be parsed.
    // @param name Name of statement group (for error messages)
    void LoadStatements(const emp::vector<std::string> & statements, const std::string & name) {
      Debug("Running LoadStatements()");
      emp::TokenStream tokens = lexer.Tokenize(statements, name);    // Convert to tokens.
      pos_t pos = tokens.begin();

      // Parse and run the program, starting from the outer scope.
      ParseState state{pos, symbol_table, symbol_table.GetRootScope(), lexer};
      auto cur_block = ParseStatementList(state);
      cur_block->Process();

      // Store this AST onto the full set we're working with.
      ast_root.AddChild(cur_block);
    }

    // Load the provided statement and run it.
    std::string Execute(std::string_view statement, emp::Ptr<Symbol_Scope> scope=nullptr) {
      Debug("Running Execute()");
      if (!scope) scope = &symbol_table.GetRootScope();     // Default scope to root level.
      auto tokens = lexer.Tokenize(statement, "eval command"); // Convert to a TokenStream.
      tokens.push_back(lexer.ToToken(";"));                 // Ensure a semi-colon at end.
      pos_t pos = tokens.begin();                           // Start are beginning of stream.
      ParseState state{pos, symbol_table, symbol_table.GetRootScope(), lexer};
      auto cur_block = ParseStatement(state);               // Convert tokens to AST
      auto result_ptr = cur_block->Process();               // Process AST to get result symbol.
      std::string result = "";                              // Default result to an empty string.
      if (result_ptr) {
        result = result_ptr->AsString();                    // Convert result to output string.
        if (result_ptr->IsTemporary()) result_ptr.Delete(); // Delete the result symbol if done.
      }
      cur_block.Delete();                                   // Delete the AST.
      return result;                                        // Return the result string.
    }


    Emplode & Write(std::ostream & os=std::cout) {
      symbol_table.GetRootScope().WriteContents(os);
      os << '\n';
      symbol_table.PrintEvents(os);
      return *this;
    }

    Emplode & Write(const std::string & filename) {
      // If the filename is empty or "_", output to standard out.
      if (filename == "" || filename == "_") return Write();

      // Otherwise generate an output file.
      std::ofstream out_file(filename);
      return Write(out_file);
    }
  };

  //////////////////////////////////////////////////////////
  //  --==  Emplode member function Implementations!  ==--


  // Load a variable name from the provided scope.
  emp::Ptr<ASTNode_Leaf> Emplode::ParseVar(ParseState & state, bool create_ok, bool scan_scopes)
  {
    Debug("Running ParseVar(", state.AsString(), ",", create_ok, ",", scan_scopes, ")");

    // First, check for leading dots.
    if (state.IsDots()) {
      Debug("...found dots: ", state.AsLexeme());
      scan_scopes = false;             // One or more initial dots specify scope; don't scan!
      size_t num_dots = state.GetTokenSize();  // Extra dots shift scope.
      emp::Ptr<Symbol_Scope> cur_scope = &state.GetScope();
      while (num_dots-- > 1) {
        cur_scope = cur_scope->GetScope();
        if (cur_scope.IsNull()) state.Error("Too many dots; goes beyond global scope.");
      }
      ++state;

      // Recursively call in the found scope if needed; given leading dot, do not scan scopes.
      if (cur_scope.Raw() != &state.GetScope()) {
        state.PushScope(*cur_scope);
        auto result = ParseVar(state, create_ok, false);
        state.PopScope();
        return result;
      }
    }

    // Next, we must have a variable name.
    // @CAO: Or a : ?  E.g., technically "..:size" could give you the parent scope size.
    state.RequireID("Must provide a variable identifier!");
    std::string var_name = state.UseLexeme();

    // Lookup this variable.
    Debug("...looking up symbol '", var_name,
          "' starting at scope '", state.GetScopeName(),
          "'; scanning=", scan_scopes);
    Symbol & cur_symbol = state.LookupSymbol(var_name, scan_scopes);

    // If this variable just provided a scope, keep going.
    if (state.IsDots()) {
      state.PushScope(cur_symbol.AsScope());
      auto result = ParseVar(state, create_ok, false);
      state.PopScope();
      return result;
    }

    // Otherwise return the variable as a leaf!
    return emp::NewPtr<ASTNode_Leaf>(&cur_symbol);
  }

  // Load a value from the provided scope, which can come from a variable or a literal.
  emp::Ptr<ASTNode> Emplode::ParseValue(ParseState & state) {
    Debug("Running ParseValue(", state.AsString(), ")");

    // Anything that begins with an identifier or dots must represent a variable.  Refer!
    if (state.IsID() || state.IsDots()) return ParseVar(state, false, true);

    // A literal number should have a temporary created with its value.
    if (state.IsNumber()) {
      Debug("...value is a number: ", state.AsLexeme());
      double value = emp::from_string<double>(state.UseLexeme()); // Calculate value.
      return MakeTempLeaf(value);                                 // Return temporary Symbol.
    }

    // A literal char should be converted to its ASCII value.
    if (state.IsChar()) {
      Debug("...value is a char: ", state.AsLexeme());
      char lit_char = emp::from_literal_char(state.UseLexeme());  // Convert the literal char.
      return MakeTempLeaf((double) lit_char);                     // Return temporary Symbol.
    }

    // A literal string should be converted to a regular string and used.
    if (state.IsString()) {
      Debug("...value is a string: ", state.AsLexeme());
      std::string str = emp::from_literal_string(state.UseLexeme()); // Convert the literal string.
      return MakeTempLeaf(str);                                      // Return temporary Symbol.
    }

    // If we have an open parenthesis, process everything inside into a single value...
    if (state.UseIfChar('(')) {
      emp::Ptr<ASTNode> out_ast = ParseExpression(state);
      state.UseRequiredChar(')', "Expected a close parenthesis in expression.");
      return out_ast;
    }

    state.Error("Expected a value, found: ", state.AsLexeme());

    return nullptr;
  }

  // Process a single provided operation on two Symbol objects.
  emp::Ptr<ASTNode> Emplode::ProcessOperation(const std::string & symbol,
                                             emp::Ptr<ASTNode> in_node1,
                                             emp::Ptr<ASTNode> in_node2)
  {
    emp_assert(!in_node1.IsNull());
    emp_assert(!in_node2.IsNull());

    // If this operation is assignment, do so!
    if (symbol == "=") return emp::NewPtr<ASTNode_Assign>(in_node1, in_node2);
    
    // If the first argument is numeric, assume we are using a math operator.
    if (in_node1->IsNumeric()) {

      // Determine the output value and put it in a temporary node.
      emp::Ptr<ASTNode_Math2> out_val = emp::NewPtr<ASTNode_Math2>(symbol);

      if (symbol == "+") out_val->SetFun( [](double v1, double v2){ return v1 + v2; } );
      else if (symbol == "-") out_val->SetFun( [](double v1, double v2){ return v1 - v2; } );
      else if (symbol == "**") out_val->SetFun( [](double v1, double v2){ return emp::Pow(v1, v2); } );
      else if (symbol == "*") out_val->SetFun( [](double v1, double v2){ return v1 * v2; } );
      else if (symbol == "/") out_val->SetFun( [](double v1, double v2){ return v1 / v2; } );
      else if (symbol == "%") out_val->SetFun( [](double v1, double v2){ return emp::Mod(v1, v2); } );
      else if (symbol == "==") out_val->SetFun( [](double v1, double v2){ return v1 == v2; } );
      else if (symbol == "!=") out_val->SetFun( [](double v1, double v2){ return v1 != v2; } );
      else if (symbol == "<")  out_val->SetFun( [](double v1, double v2){ return v1 < v2; } );
      else if (symbol == "<=") out_val->SetFun( [](double v1, double v2){ return v1 <= v2; } );
      else if (symbol == ">")  out_val->SetFun( [](double v1, double v2){ return v1 > v2; } );
      else if (symbol == ">=") out_val->SetFun( [](double v1, double v2){ return v1 >= v2; } );

      // @CAO: Need to still handle these last two differently for short-circuiting.
      else if (symbol == "&&") out_val->SetFun( [](double v1, double v2){ return v1 && v2; } );
      else if (symbol == "||") out_val->SetFun( [](double v1, double v2){ return v1 || v2; } );

      out_val->AddChild(in_node1);
      out_val->AddChild(in_node2);

      return out_val;
    }

    // Otherwise assume that we are dealing with strings.
    if (symbol == "+") {
      auto out_val = emp::NewPtr<ASTNode_Op2<std::string,std::string,std::string>>(symbol);
      out_val->SetFun([](std::string val1, std::string val2){ return val1 + val2; });
      out_val->AddChild(in_node1);
      out_val->AddChild(in_node2);

      return out_val;
    }
    else if (symbol == "*") {
      auto fun = [](std::string val1, double val2) {
        std::string out_string;
        out_string.reserve(val1.size() * (size_t) val2);
        for (size_t i = 0; i < (size_t) val2; i++) out_string += val1;
        return out_string;
      };

      auto out_val = emp::NewPtr<ASTNode_Op2<std::string,std::string,double>>(symbol);
      out_val->SetFun(fun);
      out_val->AddChild(in_node1);
      out_val->AddChild(in_node2);

      return out_val;
    }
    else {
      auto out_val = emp::NewPtr<ASTNode_Op2<double,std::string,std::string>>(symbol);

      if (symbol == "==")      out_val->SetFun([](std::string v1, std::string v2){ return v1 == v2; });
      else if (symbol == "!=") out_val->SetFun([](std::string v1, std::string v2){ return v1 != v2; });
      else if (symbol == "<")  out_val->SetFun([](std::string v1, std::string v2){ return v1 < v2; });
      else if (symbol == "<=") out_val->SetFun([](std::string v1, std::string v2){ return v1 <= v2; });
      else if (symbol == ">")  out_val->SetFun([](std::string v1, std::string v2){ return v1 > v2; });
      else if (symbol == ">=") out_val->SetFun([](std::string v1, std::string v2){ return v1 >= v2; });

      out_val->AddChild(in_node1);
      out_val->AddChild(in_node2);

      return out_val;
    }

    return nullptr;
  }
                                      

  // Calculate an expression in the provided scope.
  emp::Ptr<ASTNode> Emplode::ParseExpression(ParseState & state, size_t prec_limit) {
    Debug("Running ParseExpression(", state.AsString(), ", limit=", prec_limit, ")");

    // @CAO Should test for unary operators at the beginning of an expression.

    /// Process a value (and possibly more!)
    emp::Ptr<ASTNode> cur_node = ParseValue(state);
    std::string op = state.AsLexeme();

    Debug("...back in ParseExpression; op=`", op, "`; state=", state.AsString());

    while ( emp::Has(precedence_map, op) && precedence_map[op] < prec_limit ) {
      ++state; // Move past the current operator
      // Do we have a function call?
      if (op == "(") {
        // Collect arguments.
        emp::vector< emp::Ptr<ASTNode> > args;
        while (state.AsChar() != ')') {
          emp::Ptr<ASTNode> next_arg = ParseExpression(state);
          args.push_back(next_arg);          // Save this argument.
          if (state.AsChar() != ',') break;  // If we don't have a comma, no more args!
          ++state;                           // Move on to the next argument.
        }
        state.UseRequiredChar(')', "Expected a ')' to end function call.");

        // cur_node should have evaluated itself to a function; a Call node will link that
        // function with its arguments, run it, and return the result.
        cur_node = emp::NewPtr<ASTNode_Call>(cur_node, args);
      }

      // Otherwise we must have a binary math operation.
      else {
        emp::Ptr<ASTNode> node2 = ParseExpression(state, precedence_map[op]);
        cur_node = ProcessOperation(op, cur_node, node2);
      }

      // Move the current value over to cur_node and check if we have a new operator...
      op = state.AsLexeme();
    }

    emp_assert(!cur_node.IsNull());
    return cur_node;
  }

  // Parse an the declaration of a variable.
  Symbol & Emplode::ParseDeclaration(ParseState & state) {
    std::string type_name = state.UseLexeme();
    state.RequireID("Type name '", type_name, "' must be followed by variable to declare.");
    std::string var_name = state.UseLexeme();

    if (type_name == "String") {
      return state.AddStringVar(var_name, "Local string variable.");
    }
    else if (type_name == "Value") {
      return state.AddValueVar(var_name, "Local value variable.");
    }
    else if (type_name == "Struct") {
      return state.AddScope(var_name, "Local struct");
    }

    // Otherwise we have an object of a custom type to add.
    Debug("Building object '", var_name, "' of type '", type_name, "'");
    return state.AddObject(type_name, var_name);
  }

  // Parse an event description.
  emp::Ptr<ASTNode> Emplode::ParseEvent(ParseState & state) {
    state.UseRequiredChar('@', "All event declarations must being with an '@'.");
    state.RequireID("Events must start by specifying event name.");
    const std::string & event_name = state.UseLexeme();
    state.UseRequiredChar('(', "Expected parentheses after '", event_name, "' for args.");

    emp::vector<emp::Ptr<ASTNode>> args;
    while (state.AsChar() != ')') {
      args.push_back( ParseExpression(state) );
      state.UseIfChar(',');                     // Skip comma if next (does allow trailing comma)
    }
    state.UseRequiredChar(')', "Event args must end in a ')'");

    emp::Ptr<ASTNode> action = ParseStatement(state);

    Debug("Building event '", event_name, "' with args ", args);

    auto setup_event = [this, event_name](emp::Ptr<ASTNode> action,
                                           const emp::vector<emp::Ptr<Symbol>> & args) {
      AddEvent(
        event_name, action,
        (args.size() > 0) ? args[0]->AsDouble() : 0.0,
        (args.size() > 1) ? args[1]->AsDouble() : 0.0,
        (args.size() > 2) ? args[2]->AsDouble() : -1.0
      );
    };

    return emp::NewPtr<ASTNode_Event>(event_name, action, args, setup_event);
  }

  // Process the next input in the specified Struct.
  emp::Ptr<ASTNode> Emplode::ParseStatement(ParseState & state) {
    Debug("Running ParseStatement(", state.AsString(), ")");

    // Allow a statement with an empty line.
    if (state.UseIfChar(';')) { return nullptr; }

    // Allow a statement to be a new scope.
    if (state.UseIfChar('{')) {
      // @CAO Need to add an anonymous scope (that gets written properly)
      emp::Ptr<ASTNode> out_node = ParseStatementList(state);
      state.UseRequiredChar('}', "Expected '}' to close scope.");
      return out_node;
    }

    // Allow event definitions if a statement begins with an '@'
    if (state.AsChar() == '@') return ParseEvent(state);

    // Allow this statement to be a declaration if it begins with a type.
    if (state.IsType()) {
      Symbol & new_symbol = ParseDeclaration(state);
  
      // If the next symbol is a ';' this is a declaration without an assignment.
      if (state.AsChar() == ';') {
        state++;           // Skip the semi-colon.
        return nullptr;  // We are done!
      }

      // If this symbol is a new scope, it can be populated now either directly (with in braces)
      // or indirectly (with and assignment)
      if (new_symbol.IsScope()) {
        if (state.UseIfChar('{')) {
          state.PushScope(new_symbol.AsScope());
          emp::Ptr<ASTNode> out_node = ParseStatementList(state);
          state.PopScope();
          state.UseRequiredChar('}', "Expected scope '", new_symbol.GetName(), "' to end with a '}'.");
          return out_node;
        }

        state.RequireChar('=', "Expected scope '", new_symbol.GetName(),
                     "' definition to start with a '{' or '='; found ''", state.AsLexeme(), "'.");
        
      }      

      // Otherwise rewind so that the new variable can be used to start an expression.
      --state;
    }


    // If we made it here, remainder should be an expression.
    emp::Ptr<ASTNode> out_node = ParseExpression(state);

    // Expressions must end in a semi-colon.
    state.UseRequiredChar(';', "Expected ';' at the end of a statement; found: ", state.AsLexeme());

    return out_node;
  }

}

#endif

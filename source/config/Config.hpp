/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2021.
 *
 *  @file  Config.hpp
 *  @brief Manages all configuration of MABE runs (full parser implementation here)
 *  @note Status: ALPHA
 * 
 *  Example usage:
 *   a = 7;              // a is a variable with the value 7
 *   b = "balloons";     // b is a variable equal to the literal string "balloons"
 *   c = a + 10;         // '+' will add values; c is a variable equal to 17.
 *   d = "99 " + b;      // '+' will append strings; d is a variable equal to "99 balloons"
 *   // e = "abc" + 123; // ERROR - cannot add strings and values!
 *   f = {               // f is a structure/scope/dictionary
 *     g = 1;
 *     h = "two";
 *     i = {
 *       j = 3;
 *     }
 *     a = "shadow!";    // A variable can be redeclared in other scopes, shadowing the original.
 *                       //  Note: the LHS assumes current scope; on RHS will search outer scopes.
 *     j = "spooky!";    // A NEW variable since we are out of the namespace of the other j.
 *     j = .a;           // Change j to "shadow"; an initial . indicates current namespace.
 *     b = i.j;          // Namespaces can be stepped through with dots.
 *     c = ..a;          // A variable name beginning with a ".." indicates parent namespace.
 *     c = @f.i.j;       // A variable name beginning with an @ must have its full path specified.
 *   }                   // f has been initialized with seven variables in its scope.
 *   f["new"] = 22;      // You can always add new fields to structures.
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
 *  In practice:
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

#ifndef MABE_CONFIG_H
#define MABE_CONFIG_H

#include <fstream>

#include "emp/base/assert.hpp"
#include "emp/base/map.hpp"
#include "emp/meta/TypeID.hpp"
#include "emp/tools/string_utils.hpp"

#include "ConfigAST.hpp"
#include "ConfigEvents.hpp"
#include "ConfigFunction.hpp"
#include "ConfigLexer.hpp"
#include "ConfigScope.hpp"
#include "ConfigType.hpp"

namespace mabe {

  class Config {
  public:
    struct TypeInfo {
      size_t type_id;
      std::string desc;
      std::function<ConfigType & (const std::string &)> init_fun;
    };

  protected:
    std::string filename;             ///< Source for for code to generate.
    ConfigLexer lexer;                ///< Lexer to process input code.
    emp::vector<emp::Token> tokens;   ///< Tokenized version of input file.
    ASTNode_Block ast_root;           ///< Abstract syntax tree version of input file.
    bool debug = false;               ///< Should we print full debug information?

    ConfigScope root_scope;           ///< All variables from the root level.

    /// A map of names to event groups.
    std::map<std::string, ConfigEvents> events_map;

    /// A map of all types available in the script.
    std::unordered_map<std::string, TypeInfo> type_map;

    /// A list of precedence levels for symbols.
    std::unordered_map<std::string, size_t> precedence_map;

    // -- Helper functions --
    bool HasToken(int pos) const { return (pos >= 0) && (pos < (int) tokens.size()); }
    bool IsID(int pos) const { return HasToken(pos) && lexer.IsID(tokens[pos]); }
    bool IsNumber(int pos) const { return HasToken(pos) && lexer.IsNumber(tokens[pos]); }
    bool IsChar(int pos) const { return HasToken(pos) && lexer.IsChar(tokens[pos]); }
    bool IsString(int pos) const { return HasToken(pos) && lexer.IsString(tokens[pos]); }
    bool IsDots(int pos) const { return HasToken(pos) && lexer.IsDots(tokens[pos]); }

    bool IsType(int pos) const { return HasToken(pos) && emp::Has(type_map, tokens[pos].lexeme); }

    char AsChar(int pos) const {
      return (HasToken(pos) && lexer.IsSymbol(tokens[pos])) ? tokens[pos].lexeme[0] : 0;
    }
    const std::string & AsLexeme(int pos) const {
      return HasToken(pos) ? tokens[pos].lexeme : emp::empty_string();
    }
    size_t GetSize(int pos) const { return HasToken(pos) ? tokens[pos].lexeme.size() : 0; }

    std::string ConcatLexemes(size_t start_pos, size_t end_pos) const {
      emp_assert(start_pos <= end_pos);
      emp_assert(end_pos <= tokens.size());
      std::stringstream ss;    
      for (size_t i = start_pos; i < end_pos; i++) {
        if (i > start_pos) ss << " ";  // No space with labels.
        ss << tokens[i].lexeme;
        if (tokens[i].lexeme == ";") ss << " "; // Extra space after semi-colons for now...
      }
      return ss.str();
    }

    template <typename... Ts>
    void Error(int pos, Ts... args) const {
      std::cout << "Error (line " << tokens[pos].line_id << "): " << emp::to_string(std::forward<Ts>(args)...) << "\nAborting." << std::endl;
      exit(1);
    }

    template <typename... Ts>
    void Debug(Ts... args) const {
      if (debug) std::cout << "DEBUG: " << emp::to_string(std::forward<Ts>(args)...) << std::endl;
    }

    template <typename... Ts>
    void Require(bool result, int pos, Ts... args) const {
      if (!result) { Error(pos, std::forward<Ts>(args)...); }
    }
    template <typename... Ts>
    void RequireID(int pos, Ts... args) const {
      if (!IsID(pos)) { Error(pos, std::forward<Ts>(args)...); }
    }
    template <typename... Ts>
    void RequireNumber(int pos, Ts... args) const {
      if (!IsNumber(pos)) { Error(pos, std::forward<Ts>(args)...); }
    }
    template <typename... Ts>
    void RequireString(int pos, Ts... args) const {
      if (!IsString(pos)) { Error(pos, std::forward<Ts>(args)...); }
    }
    template <typename... Ts>
    void RequireChar(char req_char, int pos, Ts... args) const {
      if (AsChar(pos) != req_char) { Error(pos, std::forward<Ts>(args)...); }
    }
    template <typename... Ts>
    void RequireLexeme(const std::string & req_str, int pos, Ts... args) const {
      if (AsLexeme(pos) != req_str) { Error(pos, std::forward<Ts>(args)...); }
    }

    /// Load a variable name from the provided scope.
    /// If create_ok is true, create any variables that we don't find.  Otherwise continue the
    /// search for them in successively outer (lower) scopes.
    [[nodiscard]] emp::Ptr<ASTNode_Leaf> ParseVar(size_t & pos,
                                                   ConfigScope & cur_scope,
                                                   bool create_ok=false,
                                                   bool scan_scopes=true);

    /// Load a value from the provided scope, which can come from a variable or a literal.
    [[nodiscard]] emp::Ptr<ASTNode> ParseValue(size_t & pos, ConfigScope & cur_scope);

    /// Calculate the result of the provided operation on two computed entries.
    [[nodiscard]] emp::Ptr<ASTNode> ProcessOperation(const std::string & symbol,
                                       emp::Ptr<ASTNode> value1,
                                       emp::Ptr<ASTNode> value2);

    /// Calculate a full expression found in a token sequence, using the provided scope.
    [[nodiscard]] emp::Ptr<ASTNode> ParseExpression(size_t & pos, ConfigScope & cur_scope, size_t prec_limit=1000);

    /// Parse the declaration of a variable and return the newly created ConfigEntry
    ConfigEntry & ParseDeclaration(size_t & pos, ConfigScope & scope);

    /// Parse an event description.
    emp::Ptr<ASTNode> ParseEvent(size_t & pos, ConfigScope & scope);

    /// Parse the next input in the specified Struct.  A statement can be a variable declaration,
    /// an expression, or an event.
    [[nodiscard]] emp::Ptr<ASTNode> ParseStatement(size_t & pos, ConfigScope & scope);

    /// Keep parsing statments until there aren't any more or we leave this scope. 
    [[nodiscard]] emp::Ptr<ASTNode_Block> ParseStatementList(size_t & pos, ConfigScope & scope) {
      Debug("Running ParseStatementList(", pos, ":('", AsLexeme(pos), "'),", scope.GetName(), ")");
      auto cur_block = emp::NewPtr<ASTNode_Block>();
      while (pos < tokens.size() && AsChar(pos) != '}') {
        // Parse each statement in the file.
        emp::Ptr<ASTNode> statement_node = ParseStatement(pos, scope);

        // If the current statement is real, add it to the current block.
        if (!statement_node.IsNull()) cur_block->AddChild( statement_node );
      }
      return cur_block;
    }

  public:
    Config(std::string in_filename="")
      : filename(in_filename)
      , root_scope("MABE", "Outer-most, global scope.", nullptr)
    {
      if (filename != "") Load(filename);

      // Initialize the type map.
      type_map["INVALID"] = TypeInfo{ (size_t) BaseType::INVALID, "Error, Invalid type!", nullptr };
      type_map["Void"] = TypeInfo{ (size_t) BaseType::VOID, "Non-type variable; no value", nullptr };
      type_map["Value"] = TypeInfo{ (size_t) BaseType::VALUE, "Numeric variable", nullptr };
      type_map["String"] = TypeInfo{ (size_t) BaseType::STRING, "String variable", nullptr };
      type_map["Struct"] = TypeInfo{ (size_t) BaseType::STRUCT, "User-made structure", nullptr };

      // Setup operator precedence.
      size_t cur_prec = 0;
      precedence_map["("] = cur_prec++;
      precedence_map["*"] = precedence_map["/"] = precedence_map["%"] = cur_prec++;
      precedence_map["+"] = precedence_map["-"] = cur_prec++;
      precedence_map["<"] = precedence_map["<="] = precedence_map[">"] = precedence_map[">="] = cur_prec++;
      precedence_map["=="] = precedence_map["!="] = cur_prec++;
      precedence_map["&&"] = cur_prec++;
      precedence_map["||"] = cur_prec++;
      precedence_map["="] = cur_prec++;
    }

    // Prevent copy or move since we are using lambdas that capture 'this'
    Config(const Config &) = delete;
    Config(Config &&) = delete;
    Config & operator=(const Config &) = delete;
    Config & operator=(Config &&) = delete;

    ~Config() { }

    /// Create a new type of event that can be used in the scripting language.
    ConfigEvents & AddEventType(const std::string & name) {
      emp_assert(!emp::Has(events_map, name));
      Debug ("Adding event type '", name, "'");
      return events_map[name];
    }

    /// Add an instance of an event with an action that should be triggered.
    void AddEvent(const std::string & name, emp::Ptr<ASTNode> action,
                  double first=0.0, double repeat=0.0, double max=-1.0) {
      emp_assert(emp::Has(events_map, name), name);
      Debug ("Adding event instance for '", name, "' (", first, ":", repeat, ":", max, ")");
      events_map[name].AddEvent(action, first, repeat, max);
    }

    /// Indicate the an event trigger value has been updated; trigger associated events.
    void UpdateEventValue(const std::string & name, double new_value) {
      emp_assert(emp::Has(events_map, name), name);
      Debug("Uppdating event value '", name, "' to ", new_value);
      events_map[name].UpdateValue(new_value);
    }

    /// Trigger all events of a type (ignoring trigger values)
    void TriggerEvents(const std::string & name) {
      emp_assert(emp::Has(events_map, name), name);
      events_map[name].TriggerAll();
    }

    /// Print all of the events to the provided stream.
    void PrintEvents(std::ostream & os) const {
      for (const auto & x : events_map) {
        x.second.Write(x.first, os);
      }
    }

    /// To add a type, provide the type name (that can be referred to in a script) and a function
    /// that should be called (with the variable name) when an instance of that type is created.
    /// The function must return a reference to the newly created instance.
    size_t AddType(const std::string & type_name, const std::string & desc,
                   std::function<ConfigType & (const std::string &)> init_fun)
    {
      emp_assert(!emp::Has(type_map, type_name));
      size_t type_id = type_map.size();
      type_map[type_name].type_id = type_id;
      type_map[type_name].desc = desc;
      type_map[type_name].init_fun = init_fun;
      return type_id;
    }

    /// Retrieve a uniqe type ID by providing the type name.
    size_t GetTypeID(const std::string & type_name) {
      emp_assert(emp::Has(type_map, type_name));
      return type_map[type_name].type_id;
    }

    /// To add a built-in function (at the root level) provide it with a name and description.
    /// As long as the function only requires types known to the config system, it should be
    /// converted properly.  For a variadic function, the provided std::function must take a
    /// vector of ASTNode pointers, but may return any known type.
    template <typename RETURN_T, typename... ARGS>
    void AddFunction(const std::string & name,
                     std::function<RETURN_T(ARGS...)> fun,
                     const std::string & desc) {
      root_scope.AddBuiltinFunction<RETURN_T, ARGS...>(name, fun, desc);
    }

    ConfigScope & GetRootScope() { return root_scope; }
    const ConfigScope & GetRootScope() const { return root_scope; }

    // Load a single, specified configuration file.
    void Load(const std::string & filename) {
      Debug("Running Load(", filename, ")");
      std::ifstream file(filename);           // Load the provided file.
      tokens = lexer.Tokenize(file);          // Convert to more-usable tokens.
      file.close();                           // Close the file (now that it's converted)
      size_t pos = 0;                         // Start at the beginning of the file.

      // Parse and run the program, starting from the outer scope.
      auto cur_block = ParseStatementList(pos, root_scope);
      cur_block->Process();

      // Store this AST onto the full set we're working with.
      ast_root.AddChild(cur_block);
    }

    // Sequentially load a series of configuration files.
    void Load(const emp::vector<std::string> & filenames) {
      for ( const std::string & fn : filenames) Load(fn);
    }

    // Load a single, specified configuration file.
    void LoadStatements(const emp::vector<std::string> & statements) {
      Debug("Running LoadStatements()");
      tokens = lexer.Tokenize(statements);    // Convert to more-usable tokens.
      size_t pos = 0;                         // Start at the beginning of the file.

      // Parse and run the program, starting from the outer scope.
      auto cur_block = ParseStatementList(pos, root_scope);
      cur_block->Process();

      // Store this AST onto the full set we're working with.
      ast_root.AddChild(cur_block);
    }


    Config & Write(std::ostream & os=std::cout) {
      root_scope.WriteContents(os);
      os << '\n';
      PrintEvents(os);
      return *this;
    }

    Config & Write(const std::string & filename) {
      // If the filename is empty or "_", output to standard out.
      if (filename == "" || filename == "_") return Write();

      // Otherwise generate an output file.
      std::ofstream ofile(filename);
      return Write(ofile);
    }
  };

  //////////////////////////////////////////////////////////
  //  --==  Config member function Implementations!  ==--


  // Load a variable name from the provided scope.
  emp::Ptr<ASTNode_Leaf> Config::ParseVar(size_t & pos,
                                           ConfigScope & cur_scope,
                                           bool create_ok, bool scan_scopes)
  {
    Debug("Running ParseVar(", pos, ":('", AsLexeme(pos), "'),", cur_scope.GetName(), ",", create_ok, ")");

    // First, check for leading dots.
    if (IsDots(pos)) {
      scan_scopes = false;             // One or more initial dots specify scope; don't scan!
      size_t num_dots = GetSize(pos);  // Extra dots shift scope.
      emp::Ptr<ConfigScope> scope_ptr = &cur_scope;
      while (num_dots-- > 1) {
        scope_ptr = scope_ptr->GetScope();
        if (scope_ptr.IsNull()) Error(pos, "Too many dots; goes beyond global scope.");
      }
      pos++;

      // Recursively call in the found scope if needed; given leading dot, do not scan scopes.
      if (scope_ptr.Raw() != &cur_scope) return ParseVar(pos, *scope_ptr, create_ok, false);
    }

    // Next, we must have a variable name.
    // @CAO: Or a : ?  E.g., technically "..:size" could give you the parent scope size.
    RequireID(pos, "Must provide a variable identifier!");
    std::string var_name = AsLexeme(pos++);

    // Lookup this variable.
    emp::Ptr<ConfigEntry> cur_entry = cur_scope.LookupEntry(var_name, scan_scopes);

    // If we can't find this variable, either build it or throw an error.
    if (cur_entry.IsNull()) {
      Error(pos, "'", var_name,
	    "' does not exist as a parameter, variable, or type.");
    }

    // If this variable just provided a scope, keep going.
    if (IsDots(pos)) return ParseVar(pos, cur_entry->AsScope(), create_ok, false);

    // Otherwise return the variable as a leaf!
    return emp::NewPtr<ASTNode_Leaf>(cur_entry);
  }

  emp::Ptr<ASTNode_Leaf> MakeTempDouble(double val) {
    auto out_ptr = emp::NewPtr<ConfigEntry_DoubleVar>("", val, "Temporary double", nullptr);
    out_ptr->SetTemporary();
    return emp::NewPtr<ASTNode_Leaf>(out_ptr);
  }

  emp::Ptr<ASTNode_Leaf> MakeTempString(const std::string & val) {
    auto out_ptr = emp::NewPtr<ConfigEntry_StringVar>("", val, "Temporary string", nullptr);
    out_ptr->SetTemporary();
    return emp::NewPtr<ASTNode_Leaf>(out_ptr);
  }

  // Load a value from the provided scope, which can come from a variable or a literal.
  emp::Ptr<ASTNode> Config::ParseValue(size_t & pos, ConfigScope & cur_scope) {
    Debug("Running ParseValue(", pos, ":('", AsLexeme(pos), "'),", cur_scope.GetName(), ")");

    // Anything that begins with an identifier or dots must represent a variable.  Refer!
    if (IsID(pos) || IsDots(pos)) return ParseVar(pos, cur_scope, false, true);

    // A literal number should have a temporary created with its value.
    if (IsNumber(pos)) {
      Debug("...value is a number: ", AsLexeme(pos));
      double value = emp::from_string<double>(AsLexeme(pos++)); // Calculate value.
      return MakeTempDouble(value);                             // Return temporary ConfigEntry.
    }

    // A literal char should be converted to its ASCII value.
    if (IsChar(pos)) {
      Debug("...value is a char: ", AsLexeme(pos));
      char lit_char = emp::from_literal_char(AsLexeme(pos++));  // Convert the literal char.
      return MakeTempDouble((double) lit_char);                 // Return temporary ConfigEntry.
    }

    // A literal string should be converted to a regular string and used.
    if (IsString(pos)) {
      Debug("...value is a string: ", AsLexeme(pos));
      std::string str = emp::from_literal_string(AsLexeme(pos++)); // Convert the literal string.
      return MakeTempString(str);                         // Return temporary ConfigEntry.
    }

    // If we have an open parenthesis, process everything inside into a single value...
    if (AsChar(pos) == '(') {
      pos++;
      emp::Ptr<ASTNode> out_ast = ParseExpression(pos, cur_scope);
      RequireChar(')', pos++, "Expected a close parenthesis in expression.");
      return out_ast;
    }

    Error(pos, "Expected a value, found: ", AsLexeme(pos));

    return nullptr;
  }

  // Process a single provided operation on two ConfigEntry objects.
  emp::Ptr<ASTNode> Config::ProcessOperation(const std::string & symbol,
                                                 emp::Ptr<ASTNode> in_node1,
                                                 emp::Ptr<ASTNode> in_node2)
  {
    emp_assert(!in_node1.IsNull());
    emp_assert(!in_node2.IsNull());

    // If this operation is assignment, do so!
    if (symbol == "=") return emp::NewPtr<ASTNode_Assign>(in_node1, in_node2);

    // If both values are numeric, act on the math operator.
// if (in_node1->IsNumeric() && in_node2->IsNumeric()) {
//   double val1 = in_node1->AsDouble();
//   double val2 = in_node2->AsDouble();
// }

    // Determine the output value and put it in a temporary node.
    std::function<double(double,double)> fun;
    if (symbol == "+") fun = [](double val1, double val2){ return val1 + val2; };
    else if (symbol == "-") fun = [](double val1, double val2){ return val1 - val2; };
    else if (symbol == "*") fun = [](double val1, double val2){ return val1 * val2; };
    else if (symbol == "/") fun = [](double val1, double val2){ return val1 / val2; };
    else if (symbol == "%") fun = [](double val1, double val2){ return ((size_t) val1) % ((size_t) val2); };
    else if (symbol == "==") fun = [](double val1, double val2){ return val1 == val2; };
    else if (symbol == "!=") fun = [](double val1, double val2){ return val1 != val2; };
    else if (symbol == "<")  fun = [](double val1, double val2){ return val1 < val2; };
    else if (symbol == "<=") fun = [](double val1, double val2){ return val1 <= val2; };
    else if (symbol == ">")  fun = [](double val1, double val2){ return val1 > val2; };
    else if (symbol == ">=") fun = [](double val1, double val2){ return val1 >= val2; };

    // @CAO: Need to still handle these last two differently for short-circuiting.
    else if (symbol == "&&") fun = [](double val1, double val2){ return val1 && val2; };
    else if (symbol == "||") fun = [](double val1, double val2){ return val1 || val2; };

    emp::Ptr<ASTNode_Math2> out_value = emp::NewPtr<ASTNode_Math2>(symbol);
    out_value->SetFun(fun);
    out_value->AddChild(in_node1);
    out_value->AddChild(in_node2);

    return out_value;
  }
                                      

  // Calculate an expression in the provided scope.
  emp::Ptr<ASTNode> Config::ParseExpression(size_t & pos, ConfigScope & scope, size_t prec_limit) {
    Debug("Running ParseExpression(", pos, ":('", AsLexeme(pos), "'),", scope.GetName(), ")");

    // @CAO Should test for unary operators at the beginning of an expression.

    /// Process a value (and possibly more!)
    emp::Ptr<ASTNode> cur_node = ParseValue(pos, scope);
    std::string symbol = AsLexeme(pos);
    while ( emp::Has(precedence_map, symbol) && precedence_map[symbol] < prec_limit ) {
      pos++;
      // Do we have a function call?
      if (symbol == "(") {
        // Collect arguments.
        emp::vector< emp::Ptr<ASTNode> > args;
        while (AsChar(pos) != ')') {
          emp::Ptr<ASTNode> next_arg = ParseExpression(pos, scope);
          args.push_back(next_arg);       // Save this argument.
          if (AsChar(pos) != ',') break;  // If we don't have a comma, no more args!
          pos++;                          // Move on to the next argument.
        }
        RequireChar(')', pos++, "Expected a ')' to end function call.");
        cur_node = emp::NewPtr<ASTNode_Call>(cur_node, args);
      }

      // Otherwise we must have a binary math operation.
      else {
        emp::Ptr<ASTNode> node2 = ParseExpression(pos, scope, precedence_map[symbol]);
        cur_node = ProcessOperation(symbol, cur_node, node2);
      }

      // Move the current value over to cur_node and check if we have a new symbol...
      symbol = AsLexeme(pos);
    }

    emp_assert(!cur_node.IsNull());
    return cur_node;
  }

  // Parse an the declaration of a variable.
  ConfigEntry & Config::ParseDeclaration(size_t & pos, ConfigScope & scope) {
    std::string type_name = AsLexeme(pos++);
    RequireID(pos, "Type name '", type_name, "' must be followed by variable to declare.");
    std::string var_name = AsLexeme(pos++);

    if (type_name == "String") {
      return scope.AddStringVar(var_name, "Local string variable.");
    }
    else if (type_name == "Value") {
      return scope.AddValueVar(var_name, "Local value variable.");
    }
    else if (type_name == "Struct") {
      return scope.AddScope(var_name, "Local struct");
    }

    // Otherwise we have a module to add; treat it as a struct.
    Debug("Building var '", var_name, "' of type '", type_name, "'");
    ConfigScope & new_scope = scope.AddScope(var_name, type_map[type_name].desc, type_name);
    ConfigType & new_obj = type_map[type_name].init_fun(var_name);
    new_obj.SetupScope(new_scope);
    new_obj.LinkVar(new_obj._active, "_active", "Should we activate this module? (0=off, 1=on)", true);
    new_obj.LinkVar(new_obj._desc,   "_desc",   "Special description for those object.", true);
    new_obj.SetupConfig();

    return new_scope;
  }

  // Parse an event description.
  emp::Ptr<ASTNode> Config::ParseEvent(size_t & pos, ConfigScope & scope) {
    RequireChar('@', pos++, "All event declarations must being with an '@'.");
    RequireID(pos, "Events must start by specifying event name.");
    const std::string & event_name = AsLexeme(pos++);
    RequireChar('(', pos++, "Expected parentheses after '", event_name, "' for args.");

    emp::vector<emp::Ptr<ASTNode>> args;
    while (AsChar(pos) != ')') {
      args.push_back( ParseExpression(pos, scope) );
      if (AsChar(pos) == ',') pos++;
    }
    RequireChar(')', pos++, "Event args must end in a ')'");

    emp::Ptr<ASTNode> action = ParseStatement(pos, scope);

    Debug("Building event '", event_name, "' with args ", args);

    auto setup_event = [this, event_name](emp::Ptr<ASTNode> action,
                                          const emp::vector<emp::Ptr<ConfigEntry>> & args) {
      AddEvent(event_name, action,
               (args.size() > 0) ? args[0]->AsDouble() : 0.0,
               (args.size() > 1) ? args[1]->AsDouble() : 0.0,
               (args.size() > 2) ? args[2]->AsDouble() : -1.0);
    };

    return emp::NewPtr<ASTNode_Event>(event_name, action, args, setup_event);
  }

  // Process the next input in the specified Struct.
  emp::Ptr<ASTNode> Config::ParseStatement(size_t & pos, ConfigScope & scope) {
    Debug("Running ParseStatement(", pos, ":('", AsLexeme(pos), "'),", scope.GetName(), ")");

    // Allow a statement with an empty line.
    if (AsChar(pos) == ';') { pos++; return nullptr; }

    // Allow a statement to be a new scope.
    if (AsChar(pos) == '{') {
      pos++;
      // @CAO Need to add an anonymous scope (that gets written properly)
      emp::Ptr<ASTNode> out_node = ParseStatementList(pos, scope);
      RequireChar('}', pos++, "Expected '}' to close scope.");
      return out_node;
    }

    // Allow event definitions if a statement begins with an '@'
    if (AsChar(pos) == '@') return ParseEvent(pos, scope);

    // Allow this statement to be a declaration if it begins with a type.
    if (IsType(pos)) {
      ConfigEntry & new_entry = ParseDeclaration(pos, scope);
  
      // If the next symbol is a ';' this is a declaration without an assignment.
      if (AsChar(pos) == ';') {
        pos++;           // Skip the semi-colon.
        return nullptr;  // We are done!
      }

      // If this entry is a new scope, it should be populated now.
      if (new_entry.IsScope()) {
        RequireChar('{', pos, "Expected scope '", new_entry.GetName(),
                    "' definition to start with a '{'; found ''", AsLexeme(pos), "'.");
        pos++;
        emp::Ptr<ASTNode> out_node = ParseStatementList(pos, new_entry.AsScope());
        RequireChar('}', pos++, "Expected scope '", new_entry.GetName(), "' to end with a '}'.");
        return out_node;
      }

      // Otherwise rewind so that variable can be used to start an expression.
      pos--;
    }


    // If we made it here, remainder should be an expression.
    emp::Ptr<ASTNode> out_node = ParseExpression(pos, scope);

    // Expressions must end in a semi-colon.
    RequireChar(';', pos++, "Expected ';' at the end of a statement.");

    return out_node;
  }

}
#endif

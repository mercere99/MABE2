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

#include "emp/base/assert.hpp"
#include "emp/base/map.hpp"
#include "emp/io/StreamManager.hpp"
#include "emp/meta/TypeID.hpp"
#include "emp/tools/string_utils.hpp"

#include "AST.hpp"
#include "DataFile.hpp"
#include "EmplodeType.hpp"
#include "Symbol_Function.hpp"
#include "Symbol_Scope.hpp"
#include "Events.hpp"
#include "Lexer.hpp"
#include "TypeInfo.hpp"

namespace emplode {

  class Emplode {
  public:
    using pos_t = emp::TokenStream::Iterator;

  protected:
    std::string filename;      ///< Source for for code to generate.
    Lexer lexer;               ///< Lexer to process input code.
    Symbol_Scope root_scope;   ///< All variables from the root level.
    ASTNode_Block ast_root;    ///< Abstract syntax tree version of input file.
    bool debug = false;        ///< Should we print full debug information?

    /// A map of names to event groups.
    std::map<std::string, Events> events_map;

    /// A map of all types available in the script.
    std::unordered_map<std::string, emp::Ptr<TypeInfo>> type_map;

    /// Management of built-in types.
    emp::StreamManager files;    ///< Track all file streams.

    /// A list of precedence levels for symbols.
    std::unordered_map<std::string, size_t> precedence_map;

    // -- Helper functions --
    bool IsID(pos_t pos) const { return pos.IsValid() && lexer.IsID(*pos); }
    bool IsNumber(pos_t pos) const { return pos.IsValid() && lexer.IsNumber(*pos); }
    bool IsChar(pos_t pos) const { return pos.IsValid() && lexer.IsChar(*pos); }
    bool IsString(pos_t pos) const { return pos.IsValid() && lexer.IsString(*pos); }
    bool IsDots(pos_t pos) const { return pos.IsValid() && lexer.IsDots(*pos); }

    bool IsType(pos_t pos) const { return pos.IsValid() && emp::Has(type_map, pos->lexeme); }

    char AsChar(pos_t pos) const {
      return (pos.IsValid() && lexer.IsSymbol(*pos)) ? pos->lexeme[0] : 0;
    }
    const std::string & AsLexeme(pos_t pos) const {
      return pos.IsValid() ? pos->lexeme : emp::empty_string();
    }
    size_t GetSize(pos_t pos) const { return pos.IsValid() ? pos->lexeme.size() : 0; }

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
    void Error(pos_t pos, Ts... args) const {
      std::string line_info = pos.AtEnd() ? "end of input" : emp::to_string("line ", pos->line_id);
      std::cout << "Error (" << line_info << " in '" << pos.GetTokenStream().GetName() << "'): "
                << emp::to_string(std::forward<Ts>(args)...) << "\nAborting." << std::endl;
      exit(1);
    }

    template <typename... Ts>
    void Debug(Ts... args) const {
      if (debug) std::cout << "DEBUG: " << emp::to_string(std::forward<Ts>(args)...) << std::endl;
    }

    template <typename... Ts>
    void Require(bool result, pos_t pos, Ts... args) const {
      if (!result) { Error(pos, std::forward<Ts>(args)...); }
    }
    template <typename... Ts>
    void RequireID(pos_t pos, Ts... args) const {
      if (!IsID(pos)) { Error(pos, std::forward<Ts>(args)...); }
    }
    template <typename... Ts>
    void RequireNumber(pos_t pos, Ts... args) const {
      if (!IsNumber(pos)) { Error(pos, std::forward<Ts>(args)...); }
    }
    template <typename... Ts>
    void RequireString(pos_t pos, Ts... args) const {
      if (!IsString(pos)) { Error(pos, std::forward<Ts>(args)...); }
    }
    template <typename... Ts>
    void RequireChar(char req_char, pos_t pos, Ts... args) const {
      if (AsChar(pos) != req_char) { Error(pos, std::forward<Ts>(args)...); }
    }
    template <typename... Ts>
    void RequireLexeme(const std::string & req_str, pos_t pos, Ts... args) const {
      if (AsLexeme(pos) != req_str) { Error(pos, std::forward<Ts>(args)...); }
    }

    /// Load a variable name from the provided scope.
    /// If create_ok is true, create any variables that we don't find.  Otherwise continue the
    /// search for them in successively outer (lower) scopes.
    [[nodiscard]] emp::Ptr<ASTNode_Leaf> ParseVar(pos_t & pos,
                                                  Symbol_Scope & cur_scope,
                                                  bool create_ok=false,
                                                  bool scan_scopes=true);

    /// Load a value from the provided scope, which can come from a variable or a literal.
    [[nodiscard]] emp::Ptr<ASTNode> ParseValue(pos_t & pos, Symbol_Scope & cur_scope);

    /// Calculate the result of the provided operation on two computed entries.
    [[nodiscard]] emp::Ptr<ASTNode> ProcessOperation(const std::string & symbol,
                                                     emp::Ptr<ASTNode> value1,
                                                     emp::Ptr<ASTNode> value2);

    /// Calculate a full expression found in a token sequence, using the provided scope.
    [[nodiscard]] emp::Ptr<ASTNode>
      ParseExpression(pos_t & pos, Symbol_Scope & cur_scope, size_t prec_limit=1000);

    /// Parse the declaration of a variable and return the newly created Symbol
    Symbol & ParseDeclaration(pos_t & pos, Symbol_Scope & scope);

    /// Parse an event description.
    emp::Ptr<ASTNode> ParseEvent(pos_t & pos, Symbol_Scope & scope);

    /// Parse the next input in the specified Struct.  A statement can be a variable declaration,
    /// an expression, or an event.
    [[nodiscard]] emp::Ptr<ASTNode> ParseStatement(pos_t & pos, Symbol_Scope & scope);

    /// Keep parsing statements until there aren't any more or we leave this scope. 
    [[nodiscard]] emp::Ptr<ASTNode_Block> ParseStatementList(pos_t & pos, Symbol_Scope & scope) {
      Debug("Running ParseStatementList(", pos.GetIndex(), ":('", AsLexeme(pos), "'),", scope.GetName(), ")");
      auto cur_block = emp::NewPtr<ASTNode_Block>(scope);
      while (pos.IsValid() && AsChar(pos) != '}') {
        // Parse each statement in the file.
        emp::Ptr<ASTNode> statement_node = ParseStatement(pos, scope);

        // If the current statement is real, add it to the current block.
        if (!statement_node.IsNull()) cur_block->AddChild( statement_node );
      }
      return cur_block;
    }

  public:
    Emplode(std::string in_filename="")
      : filename(in_filename)
      , root_scope("Emplode", "Outer-most, global scope.", nullptr)
      , ast_root(root_scope)
    {
      if (filename != "") Load(filename);

      // Initialize the type map.
      type_map["INVALID"] = emp::NewPtr<TypeInfo>( (size_t) BaseType::INVALID, "/*ERROR*/", "Error, Invalid type!" );
      type_map["Void"] = emp::NewPtr<TypeInfo>( (size_t) BaseType::VOID, "Void", "Non-type variable; no value" );
      type_map["Value"] = emp::NewPtr<TypeInfo>( (size_t) BaseType::VALUE, "Value", "Numeric variable" );
      type_map["String"] = emp::NewPtr<TypeInfo>( (size_t) BaseType::STRING, "String", "String variable" );
      type_map["Struct"] = emp::NewPtr<TypeInfo>( (size_t) BaseType::STRUCT, "Struct", "User-made structure" );

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
      files.SetOutputDefaultFile();  // Stream manager should default to files for output.
      auto df_init = [this](const std::string & name) { return emp::NewPtr<DataFile>(name, files); };
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

    ~Emplode() {
      // Clean up type information.
      for (auto [name, ptr] : type_map) ptr.Delete();
    }

    /// Create a new type of event that can be used in the scripting language.
    Events & AddEventType(const std::string & name) {
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
    template <typename FUN_T>
    TypeInfo & AddType(
      const std::string & type_name,
      const std::string & desc,
      FUN_T init_fun,
      emp::TypeID type_id,
      bool is_config_owned=false
    ) {
      emp_assert(!emp::Has(type_map, type_name), type_name, "Type already exists!");
      size_t index = type_map.size();
      auto info_ptr = emp::NewPtr<TypeInfo>( index, type_name, desc, init_fun, is_config_owned );
      info_ptr->LinkType(type_id);
      type_map[type_name] = info_ptr;
      return *type_map[type_name];
    }

    /// If the linked type can be provided as a template parameter, we can also double check that
    /// it is derived from EmplodeType (as it needs to be...)
    template <typename OBJECT_T, typename FUN_T>
    TypeInfo & AddType(
      const std::string & type_name,
      const std::string & desc,
      FUN_T init_fun,
      bool is_config_owned=false
    ) {
      static_assert(std::is_base_of<EmplodeType, OBJECT_T>(),
                    "Only EmplodeType objects can be used as a custom config type.");
      TypeInfo & info = AddType(type_name, desc, init_fun, emp::GetTypeID<OBJECT_T>(), is_config_owned);
      OBJECT_T::InitType(*this, info);
      return info;
    }

    /// If init_fun is not specified in add type, build our own and assume that we own the object.
    template <typename OBJECT_T>
    TypeInfo & AddType(const std::string & type_name, const std::string & desc) {
      return AddType<OBJECT_T>(type_name, desc,
                               [](const std::string & /*name*/){ return emp::NewPtr<OBJECT_T>(); },
                               true);
    }

    /// Also allow direct file management.
    emp::StreamManager & GetFileManager() { return files; }

    /// To add a built-in function (at the root level) provide it with a name and description.
    /// As long as the function only requires types known to the config system, it should be
    /// converted properly.  For a variadic function, the provided function must take a
    /// vector of ASTNode pointers, but may return any known type.
    template <typename FUN_T>
    void AddFunction(const std::string & name, FUN_T fun, const std::string & desc) {
      root_scope.AddBuiltinFunction(name, fun, desc);
    }

    Symbol_Scope & GetRootScope() { return root_scope; }
    const Symbol_Scope & GetRootScope() const { return root_scope; }

    // Load a single, specified configuration file.
    void Load(const std::string & filename) {
      Debug("Running Load(", filename, ")");
      std::ifstream file(filename);           // Load the provided file.
      emp::TokenStream tokens = lexer.Tokenize(file, filename);          // Convert to more-usable tokens.
      file.close();                           // Close the file (now that it's converted)
      pos_t pos = tokens.begin();             // Start at the beginning of the file.

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
    // @param statements List is statements to be parsed.
    // @param name Name of statement group (for error messages)
    void LoadStatements(const emp::vector<std::string> & statements, const std::string & name) {
      Debug("Running LoadStatements()");
      emp::TokenStream tokens = lexer.Tokenize(statements, name);    // Convert to tokens.
      pos_t pos = tokens.begin();

      // Parse and run the program, starting from the outer scope.
      auto cur_block = ParseStatementList(pos, root_scope);
      cur_block->Process();

      // Store this AST onto the full set we're working with.
      ast_root.AddChild(cur_block);
    }

    // Load the provided statement and run it.
    std::string Execute(std::string_view statement, emp::Ptr<Symbol_Scope> scope=nullptr) {
      Debug("Running Execute()");
      if (!scope) scope = &root_scope;                      // Default scope to root level.
      auto tokens = lexer.Tokenize(statement, "eval command"); // Convert to a TokenStream.
      tokens.push_back(lexer.ToToken(";"));                 // Ensure a semi-colon at end.
      pos_t pos = tokens.begin();                           // Start are beginning of stream.
      auto cur_block = ParseStatement(pos, root_scope);     // Convert tokens to AST
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
      root_scope.WriteContents(os);
      os << '\n';
      PrintEvents(os);
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
  emp::Ptr<ASTNode_Leaf> Emplode::ParseVar(pos_t & pos,
                                          Symbol_Scope & cur_scope,
                                          bool create_ok, bool scan_scopes)
  {
    Debug("Running ParseVar(", pos.GetIndex(), ":('", AsLexeme(pos), "'),", cur_scope.GetName(), ",", create_ok, ")");

    // First, check for leading dots.
    if (IsDots(pos)) {
      scan_scopes = false;             // One or more initial dots specify scope; don't scan!
      size_t num_dots = GetSize(pos);  // Extra dots shift scope.
      emp::Ptr<Symbol_Scope> scope_ptr = &cur_scope;
      while (num_dots-- > 1) {
        scope_ptr = scope_ptr->GetScope();
        if (scope_ptr.IsNull()) Error(pos, "Too many dots; goes beyond global scope.");
      }
      ++pos;

      // Recursively call in the found scope if needed; given leading dot, do not scan scopes.
      if (scope_ptr.Raw() != &cur_scope) return ParseVar(pos, *scope_ptr, create_ok, false);
    }

    // Next, we must have a variable name.
    // @CAO: Or a : ?  E.g., technically "..:size" could give you the parent scope size.
    RequireID(pos, "Must provide a variable identifier!");
    std::string var_name = AsLexeme(pos++);

    // Lookup this variable.
    emp::Ptr<Symbol> cur_symbol = cur_scope.LookupSymbol(var_name, scan_scopes);

    // If we can't find this variable, throw an error.
    if (cur_symbol.IsNull()) {
      Error(pos, "'", var_name, "' does not exist as a parameter, variable, or type.",
            "  Current scope is '", cur_scope.GetName(), "'");
    }

    // If this variable just provided a scope, keep going.
    if (IsDots(pos)) return ParseVar(pos, cur_symbol->AsScope(), create_ok, false);

    // Otherwise return the variable as a leaf!
    return emp::NewPtr<ASTNode_Leaf>(cur_symbol);
  }

  emp::Ptr<ASTNode_Leaf> MakeTempLeaf(double val) {
    auto out_ptr = emp::NewPtr<Symbol_DoubleVar>("", val, "Temporary double", nullptr);
    out_ptr->SetTemporary();
    return emp::NewPtr<ASTNode_Leaf>(out_ptr);
  }

  emp::Ptr<ASTNode_Leaf> MakeTempLeaf(const std::string & val) {
    auto out_ptr = emp::NewPtr<Symbol_StringVar>("", val, "Temporary string", nullptr);
    out_ptr->SetTemporary();
    return emp::NewPtr<ASTNode_Leaf>(out_ptr);
  }

  // Load a value from the provided scope, which can come from a variable or a literal.
  emp::Ptr<ASTNode> Emplode::ParseValue(pos_t & pos, Symbol_Scope & cur_scope) {
    Debug("Running ParseValue(", pos.GetIndex(), ":('", AsLexeme(pos), "'),", cur_scope.GetName(), ")");

    // Anything that begins with an identifier or dots must represent a variable.  Refer!
    if (IsID(pos) || IsDots(pos)) return ParseVar(pos, cur_scope, false, true);

    // A literal number should have a temporary created with its value.
    if (IsNumber(pos)) {
      Debug("...value is a number: ", AsLexeme(pos));
      double value = emp::from_string<double>(AsLexeme(pos++)); // Calculate value.
      return MakeTempLeaf(value);                             // Return temporary Symbol.
    }

    // A literal char should be converted to its ASCII value.
    if (IsChar(pos)) {
      Debug("...value is a char: ", AsLexeme(pos));
      char lit_char = emp::from_literal_char(AsLexeme(pos++));  // Convert the literal char.
      return MakeTempLeaf((double) lit_char);                 // Return temporary Symbol.
    }

    // A literal string should be converted to a regular string and used.
    if (IsString(pos)) {
      Debug("...value is a string: ", AsLexeme(pos));
      std::string str = emp::from_literal_string(AsLexeme(pos++)); // Convert the literal string.
      return MakeTempLeaf(str);                         // Return temporary Symbol.
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
  emp::Ptr<ASTNode> Emplode::ParseExpression(
    pos_t & pos,
    Symbol_Scope & scope,
    size_t prec_limit
  ) {
    Debug("Running ParseExpression(", pos.GetIndex(), ":('", AsLexeme(pos), "'),", scope.GetName(), ")");

    // @CAO Should test for unary operators at the beginning of an expression.

    /// Process a value (and possibly more!)
    emp::Ptr<ASTNode> cur_node = ParseValue(pos, scope);
    std::string symbol = AsLexeme(pos);
    while ( emp::Has(precedence_map, symbol) && precedence_map[symbol] < prec_limit ) {
      ++pos;
      // Do we have a function call?
      if (symbol == "(") {
        // Collect arguments.
        emp::vector< emp::Ptr<ASTNode> > args;
        while (AsChar(pos) != ')') {
          emp::Ptr<ASTNode> next_arg = ParseExpression(pos, scope);
          args.push_back(next_arg);       // Save this argument.
          if (AsChar(pos) != ',') break;  // If we don't have a comma, no more args!
          ++pos;                          // Move on to the next argument.
        }
        RequireChar(')', pos++, "Expected a ')' to end function call.");

        // cur_node should have evaluated itself to a function; a Call node will link that
        // function with its arguments, run it, and return the result.
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
  Symbol & Emplode::ParseDeclaration(pos_t & pos, Symbol_Scope & scope) {
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

    // Otherwise we have an object of a custom type to add.
    Debug("Building var '", var_name, "' of type '", type_name, "'");

    // Retrieve the information about the requested type.
    TypeInfo & type_info = *type_map[type_name];
    const std::string & type_desc = type_map[type_name]->GetDesc();
    const bool is_config_owned = type_info.GetOwned();

    // Use the TypeInfo associated with the provided type name to build an instance.
    emp::Ptr<EmplodeType> new_obj = type_info.MakeObj(var_name);

    // Setup a scope for this new type, linking the object to it.
    Symbol_Object & new_obj_symbol = scope.AddObject(var_name, type_desc, new_obj, is_config_owned);

    // Let the new object know about its scope.
    new_obj->Setup(new_obj_symbol, type_info);

    return new_obj_symbol;
  }

  // Parse an event description.
  emp::Ptr<ASTNode> Emplode::ParseEvent(pos_t & pos, Symbol_Scope & scope) {
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
                                          const emp::vector<emp::Ptr<Symbol>> & args) {
      AddEvent(event_name, action,
               (args.size() > 0) ? args[0]->AsDouble() : 0.0,
               (args.size() > 1) ? args[1]->AsDouble() : 0.0,
               (args.size() > 2) ? args[2]->AsDouble() : -1.0);
    };

    return emp::NewPtr<ASTNode_Event>(event_name, action, args, setup_event);
  }

  // Process the next input in the specified Struct.
  emp::Ptr<ASTNode> Emplode::ParseStatement(pos_t & pos, Symbol_Scope & scope) {
    Debug("Running ParseStatement(", pos.GetIndex(), ":('", AsLexeme(pos), "'),", scope.GetName(), ")");

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
      Symbol & new_symbol = ParseDeclaration(pos, scope);
  
      // If the next symbol is a ';' this is a declaration without an assignment.
      if (AsChar(pos) == ';') {
        pos++;           // Skip the semi-colon.
        return nullptr;  // We are done!
      }

      // If this symbol is a new scope, it should be populated now.
      if (new_symbol.IsScope()) {
        RequireChar('{', pos, "Expected scope '", new_symbol.GetName(),
                    "' definition to start with a '{'; found ''", AsLexeme(pos), "'.");
        pos++;
        emp::Ptr<ASTNode> out_node = ParseStatementList(pos, new_symbol.AsScope());
        RequireChar('}', pos++, "Expected scope '", new_symbol.GetName(), "' to end with a '}'.");
        return out_node;
      }

      // Otherwise rewind so that variable can be used to start an expression.
      --pos;
    }


    // If we made it here, remainder should be an expression.
    emp::Ptr<ASTNode> out_node = ParseExpression(pos, scope);

    // Expressions must end in a semi-colon.
    RequireChar(';', pos++, "Expected ';' at the end of a statement.");

    return out_node;
  }

}
#endif

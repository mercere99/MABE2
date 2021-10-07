/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2021.
 *
 *  @file  Config.hpp
 *  @brief Manages all configuration of MABE runs (full parser implementation here)
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

#ifndef MABE_CONFIG_H
#define MABE_CONFIG_H

#include <fstream>

#include "emp/base/assert.hpp"
#include "emp/base/map.hpp"
#include "emp/meta/TypeID.hpp"
#include "emp/tools/string_utils.hpp"

#include "ConfigAST.hpp"
#include "ConfigEvents.hpp"
#include "ConfigEntry_Function.hpp"
#include "ConfigLexer.hpp"
#include "ConfigEntry_Scope.hpp"
#include "ConfigType.hpp"

namespace mabe {

  class Config {
  public:
    // TypeInfo tracks a particular type to be used in the configuration langauge.
    struct TypeInfo {
      size_t index;
      std::string desc;
      emp::TypeID type_id;

      using init_fun_t = std::function<ConfigType & (const std::string &)>;
      init_fun_t init_fun;

      using entry_ptr_t = emp::Ptr<ConfigEntry>;
      using member_fun_t = std::function<entry_ptr_t(const emp::vector<entry_ptr_t> &)>;
      emp::map<std::string, member_fun_t> member_funs;

      // Constructor to allow a simple new configuration type
      TypeInfo(size_t in_id, const std::string & in_desc)
       : index(in_id), desc(in_desc) { }

      // Constructor to allow a new configuration type whose objects require initialization.
      TypeInfo(size_t in_id, const std::string & in_desc, init_fun_t in_init)
       : index(in_id), desc(in_desc), init_fun(in_init)
      {
      }

      // Link this TypeInfo object to a real C++ type.
      template <typename OBJECT_T>
      void LinkType() {
        static_assert(std::is_base_of<ConfigType, OBJECT_T>(),
                      "Only ConfigType objects can be used as a custom config type.");
        type_id = emp::GetTypeID<OBJECT_T>();
      }

      // Add a member function that can be called on objects of this type.
      template <typename RETURN_T, typename OBJECT_T, typename... PARAM_Ts>
      void AddMemberFunction(
        const std::string & name,
        std::function<RETURN_T(OBJECT_T &, PARAM_Ts...)> fun
      ) {
        // ----- Make sure function is legal -----
        // Is return type legal?
        static_assert(std::is_arithmetic<RETURN_T>() || std::is_same<RETURN_T, std::string>(),
                      "Config member functions must of a string or arithmetic return type");

        // Is the first parameter the correct type?
        emp_assert( type_id.IsType<OBJECT_T>(),
                    "First parameter must match config type of member function being created!",
                    type_id, emp::GetTypeID<OBJECT_T>() );

        // Are remaining parameters legal?
        constexpr bool params_ok =
         ((std::is_arithmetic<PARAM_Ts>() || std::is_same<PARAM_Ts, std::string>()) && ...);
        static_assert(params_ok, "Parameters 2+ in a member function must be string or arithmetic.");

        // ----- Transform this function into one that TypeInfo can make use of ----
        member_fun_t member_fun =
          [name,fun](ConfigType & obj, const emp::vector<entry_ptr_t> & args) {
            // Make sure we can convert the obj into the correct type.
            emp::Ptr<OBJECT_T> typed_ptr = dynamic_cast<OBJECT_T*>(&obj);

            // Make sure we have the correct number of arguments.
            if (args.size() != sizeof...(PARAM_Ts)) {
              std::cerr << "Error in call to function '" << name
                << "'; expected " << sizeof...(PARAM_Ts)
                << " arguments, but received " << args.size() << "."
                << std::endl;
            }
            //@CAO should collect file position information for the above error.

            // Call the provided function and return the result.
            int arg_id = 0;
            RETURN_T result = fun( *typed_ptr, args[arg_id++]->As<PARAM_Ts>()... );

            return result;
          };

        // Add this member function to the library we are building.
        member_funs[name] = member_fun;
      }
    };

    using pos_t = emp::TokenStream::Iterator;

  protected:
    std::string filename;         ///< Source for for code to generate.
    ConfigLexer lexer;            ///< Lexer to process input code.
    ConfigEntry_Scope root_scope; ///< All variables from the root level.
    ASTNode_Block ast_root;       ///< Abstract syntax tree version of input file.
    bool debug = false;           ///< Should we print full debug information?


    /// A map of names to event groups.
    std::map<std::string, ConfigEvents> events_map;

    /// A map of all types available in the script.
    std::unordered_map<std::string, emp::Ptr<TypeInfo>> type_map;

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
                                                  ConfigEntry_Scope & cur_scope,
                                                  bool create_ok=false,
                                                  bool scan_scopes=true);

    /// Load a value from the provided scope, which can come from a variable or a literal.
    [[nodiscard]] emp::Ptr<ASTNode> ParseValue(pos_t & pos, ConfigEntry_Scope & cur_scope);

    /// Calculate the result of the provided operation on two computed entries.
    [[nodiscard]] emp::Ptr<ASTNode> ProcessOperation(const std::string & symbol,
                                       emp::Ptr<ASTNode> value1,
                                       emp::Ptr<ASTNode> value2);

    /// Calculate a full expression found in a token sequence, using the provided scope.
    [[nodiscard]] emp::Ptr<ASTNode>
      ParseExpression(pos_t & pos, ConfigEntry_Scope & cur_scope, size_t prec_limit=1000);

    /// Parse the declaration of a variable and return the newly created ConfigEntry
    ConfigEntry & ParseDeclaration(pos_t & pos, ConfigEntry_Scope & scope);

    /// Parse an event description.
    emp::Ptr<ASTNode> ParseEvent(pos_t & pos, ConfigEntry_Scope & scope);

    /// Parse the next input in the specified Struct.  A statement can be a variable declaration,
    /// an expression, or an event.
    [[nodiscard]] emp::Ptr<ASTNode> ParseStatement(pos_t & pos, ConfigEntry_Scope & scope);

    /// Keep parsing statements until there aren't any more or we leave this scope. 
    [[nodiscard]] emp::Ptr<ASTNode_Block> ParseStatementList(pos_t & pos, ConfigEntry_Scope & scope) {
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
    Config(std::string in_filename="")
      : filename(in_filename)
      , root_scope("MABE", "Outer-most, global scope.", nullptr)
      , ast_root(root_scope)
    {
      if (filename != "") Load(filename);

      // Initialize the type map.
      type_map["INVALID"] = emp::NewPtr<TypeInfo>( (size_t) BaseType::INVALID, "Error, Invalid type!" );
      type_map["Void"] = emp::NewPtr<TypeInfo>( (size_t) BaseType::VOID, "Non-type variable; no value" );
      type_map["Value"] = emp::NewPtr<TypeInfo>( (size_t) BaseType::VALUE, "Numeric variable" );
      type_map["String"] = emp::NewPtr<TypeInfo>( (size_t) BaseType::STRING, "String variable" );
      type_map["Struct"] = emp::NewPtr<TypeInfo>( (size_t) BaseType::STRUCT, "User-made structure" );

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

      // 'EVAL' dynamically evaluates the contents of a string.
      std::function<std::string(const std::string &)> eval_fun =
        [this](const std::string & expression) { return Eval(expression); };
      AddFunction("EVAL", eval_fun, "Dynamically evaluate the string passed in.");

      // 'PRINT' is a simple debugging command to output the value of a variable.
      std::function<int(const emp::vector<emp::Ptr<ConfigEntry>> &)> print_fun =
        [](const emp::vector<emp::Ptr<ConfigEntry>> & args) {
          for (auto entry_ptr : args) std::cout << entry_ptr->AsString();
          return 0;
        };
      AddFunction("PRINT", print_fun, "Print out the provided variables.");

      // Default 1-input math functions
      std::function<double(double)> math1_fun = [](double x){ return std::abs(x); };
      AddFunction("ABS", math1_fun, "Absolute Value" );
      math1_fun = [](double x){ return emp::Pow(emp::E, x); };
      AddFunction("EXP", math1_fun, "Exponentiation" );
      math1_fun = [](double x){ return std::log(x); };
      AddFunction("LOG2", math1_fun, "Log base-2" );
      math1_fun = [](double x){ return std::log10(x); };
      AddFunction("LOG10", math1_fun, "Log base-10" );

      math1_fun = [](double x){ return std::sqrt(x); };
      AddFunction("SQRT", math1_fun, "Square Root" );
      math1_fun = [](double x){ return std::cbrt(x); };
      AddFunction("CBRT", math1_fun, "Cube Root" );

      math1_fun = [](double x){ return std::sin(x); };
      AddFunction("SIN", math1_fun, "Sine" );
      math1_fun = [](double x){ return std::cos(x); };
      AddFunction("COS", math1_fun, "Cosine" );
      math1_fun = [](double x){ return std::tan(x); };
      AddFunction("TAN", math1_fun, "Tangent" );
      math1_fun = [](double x){ return std::asin(x); };
      AddFunction("ASIN", math1_fun, "Arc Sine" );
      math1_fun = [](double x){ return std::acos(x); };
      AddFunction("ACOS", math1_fun, "Arc Cosine" );
      math1_fun = [](double x){ return std::atan(x); };
      AddFunction("ATAN", math1_fun, "Arc Tangent" );
      math1_fun = [](double x){ return std::sinh(x); };
      AddFunction("SINH", math1_fun, "Hyperbolic Sine" );
      math1_fun = [](double x){ return std::cosh(x); };
      AddFunction("COSH", math1_fun, "Hyperbolic Cosine" );
      math1_fun = [](double x){ return std::tanh(x); };
      AddFunction("TANH", math1_fun, "Hyperbolic Tangent" );
      math1_fun = [](double x){ return std::asinh(x); };
      AddFunction("ASINH", math1_fun, "Hyperbolic Arc Sine" );
      math1_fun = [](double x){ return std::acosh(x); };
      AddFunction("ACOSH", math1_fun, "Hyperbolic Arc Cosine" );
      math1_fun = [](double x){ return std::atanh(x); };
      AddFunction("ATANH", math1_fun, "Hyperbolic Arc Tangent" );

      math1_fun = [](double x){ return std::ceil(x); };
      AddFunction("CEIL", math1_fun, "Round UP" );
      math1_fun = [](double x){ return std::floor(x); };
      AddFunction("FLOOR", math1_fun, "Round DOWN" );
      math1_fun = [](double x){ return std::round(x); };
      AddFunction("ROUND", math1_fun, "Round to nearest" );

      math1_fun = [](double x){ return std::isinf(x); };
      AddFunction("ISINF", math1_fun, "Test if Infinite" );
      math1_fun = [](double x){ return std::isnan(x); };
      AddFunction("ISNAN", math1_fun, "Test if Not-a-number" );

      // Default 2-input math functions
      std::function<double(double,double)> math2_fun = [](double x, double y){ return std::hypot(x,y); };
      AddFunction("HYPOT", math2_fun, "Given sides, find hypotenuse" );
      math2_fun = [](double x, double y){ return emp::Pow(x,y); };
      AddFunction("LOG", math2_fun, "Take log of arg1 with base arg2" );
      math2_fun = [](double x, double y){ return (x<y) ? x : y; };
      AddFunction("MIN", math2_fun, "Return lesser value" );
      math2_fun = [](double x, double y){ return (x>y) ? x : y; };
      AddFunction("MAX", math2_fun, "Return greater value" );
      math2_fun = [](double x, double y){ return emp::Pow(x,y); };
      AddFunction("POW", math2_fun, "Take arg1 to the arg2 power" );

      // Default 3-input math functions
      std::function<double(double,double,double)> math3_fun =
        [](double x, double y, double z){ return (x!=0.0) ? y : z; };
      AddFunction("IF", math3_fun, "If arg1 is true, return arg2, else arg3" );
      math3_fun = [](double x, double y, double z){ return (x<y) ? y : (x>z) ? z : x; };
      AddFunction("CLAMP", math3_fun, "Return arg1, forced into range [arg2,arg3]" );
      math3_fun = [](double x, double y, double z){ return (z-y)*x+y; };
      AddFunction("TO_SCALE", math3_fun, "Scale arg1 to arg2-arg3 as unit distance" );
      math3_fun = [](double x, double y, double z){ return (x-y) / (z-y); };
      AddFunction("FROM_SCALE", math3_fun, "Scale arg1 from arg2-arg3 as unit distance" );
    }

    // Prevent copy or move since we are using lambdas that capture 'this'
    Config(const Config &) = delete;
    Config(Config &&) = delete;
    Config & operator=(const Config &) = delete;
    Config & operator=(Config &&) = delete;

    ~Config() {
      // Clean up type information.
      for (auto [name, ptr] : type_map) ptr.Delete();
    }

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
    size_t AddType(
      const std::string & type_name,
      const std::string & desc,
      std::function<ConfigType & (const std::string &)> init_fun
    ) {
      emp_assert(!emp::Has(type_map, type_name));
      size_t index = type_map.size();
      type_map[type_name] = emp::NewPtr<TypeInfo>( index, desc, init_fun );
      return index;
    }

    /// Retrieve a unique type ID by providing the type name.
    size_t GetIndex(const std::string & type_name) {
      emp_assert(emp::Has(type_map, type_name));
      return type_map[type_name]->index;
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

    ConfigEntry_Scope & GetRootScope() { return root_scope; }
    const ConfigEntry_Scope & GetRootScope() const { return root_scope; }

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
    std::string Eval(std::string_view statement, emp::Ptr<ConfigEntry_Scope> scope=nullptr) {
      Debug("Running Eval()");
      if (!scope) scope = &root_scope;                      // Default scope to root level.
      auto tokens = lexer.Tokenize(statement, "eval command"); // Convert to a TokenStream.
      tokens.push_back(lexer.ToToken(";"));                 // Ensure a semi-colon at end.
      pos_t pos = tokens.begin();                           // Start are beginning of stream.
      auto cur_block = ParseStatement(pos, root_scope);     // Convert tokens to AST
      auto result_ptr = cur_block->Process();               // Process AST to get result entry.
      std::string result = "";                              // Default result to an empty string.
      if (result_ptr) {
        result = result_ptr->AsString();                    // Convert result to output string.
        if (result_ptr->IsTemporary()) result_ptr.Delete(); // Delete the result entry if done.
      }
      cur_block.Delete();                                   // Delete the AST.
      return result;                                        // Return the result string.
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
      std::ofstream out_file(filename);
      return Write(out_file);
    }
  };

  //////////////////////////////////////////////////////////
  //  --==  Config member function Implementations!  ==--


  // Load a variable name from the provided scope.
  emp::Ptr<ASTNode_Leaf> Config::ParseVar(pos_t & pos,
                                          ConfigEntry_Scope & cur_scope,
                                          bool create_ok, bool scan_scopes)
  {
    Debug("Running ParseVar(", pos.GetIndex(), ":('", AsLexeme(pos), "'),", cur_scope.GetName(), ",", create_ok, ")");

    // First, check for leading dots.
    if (IsDots(pos)) {
      scan_scopes = false;             // One or more initial dots specify scope; don't scan!
      size_t num_dots = GetSize(pos);  // Extra dots shift scope.
      emp::Ptr<ConfigEntry_Scope> scope_ptr = &cur_scope;
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
    emp::Ptr<ConfigEntry> cur_entry = cur_scope.LookupEntry(var_name, scan_scopes);

    // If we can't find this variable, either build it or throw an error.
    if (cur_entry.IsNull()) {
      Error(pos, "'", var_name, "' does not exist as a parameter, variable, or type.");
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
  emp::Ptr<ASTNode> Config::ParseValue(pos_t & pos, ConfigEntry_Scope & cur_scope) {
    Debug("Running ParseValue(", pos.GetIndex(), ":('", AsLexeme(pos), "'),", cur_scope.GetName(), ")");

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
    
    // If the first argument is numeric, assume we are using a math operator.
    if (in_node1->IsNumeric()) {

      // Determine the output value and put it in a temporary node.
      std::function<double(double,double)> fun;
      if (symbol == "+") fun = [](double val1, double val2){ return val1 + val2; };
      else if (symbol == "-") fun = [](double val1, double val2){ return val1 - val2; };
      else if (symbol == "**") fun = [](double val1, double val2){ return emp::Pow(val1, val2); };
      else if (symbol == "*") fun = [](double val1, double val2){ return val1 * val2; };
      else if (symbol == "/") fun = [](double val1, double val2){ return val1 / val2; };
      else if (symbol == "%") fun = [](double val1, double val2){ return emp::Mod(val1, val2); };
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

    // Otherwise assume that we are dealing with strings.
    if (symbol == "+") {
      std::function<std::string(std::string,std::string)> fun;
      fun = [](std::string val1, std::string val2){ return val1 + val2; };

      auto out_value = emp::NewPtr<ASTNode_Op2<std::string,std::string,std::string>>(symbol);
      out_value->SetFun(fun);
      out_value->AddChild(in_node1);
      out_value->AddChild(in_node2);

      return out_value;
    }
    else if (symbol == "*") {
      std::function<std::string(std::string,double)> fun;
      fun = [](std::string val1, double val2) {
        std::string out_string;
        out_string.reserve(val1.size() * (size_t) val2);
        for (size_t i = 0; i < (size_t) val2; i++) out_string += val1;
        return out_string;
      };

      auto out_value = emp::NewPtr<ASTNode_Op2<std::string,std::string,double>>(symbol);
      out_value->SetFun(fun);
      out_value->AddChild(in_node1);
      out_value->AddChild(in_node2);

      return out_value;
    }
    else {
      std::function<double(std::string,std::string)> fun;
      if (symbol == "==") fun = [](std::string val1, std::string val2){ return val1 == val2; };
      else if (symbol == "!=") fun = [](std::string val1, std::string val2){ return val1 != val2; };
      else if (symbol == "<")  fun = [](std::string val1, std::string val2){ return val1 < val2; };
      else if (symbol == "<=") fun = [](std::string val1, std::string val2){ return val1 <= val2; };
      else if (symbol == ">")  fun = [](std::string val1, std::string val2){ return val1 > val2; };
      else if (symbol == ">=") fun = [](std::string val1, std::string val2){ return val1 >= val2; };

      auto out_value = emp::NewPtr<ASTNode_Op2<double,std::string,std::string>>(symbol);
      out_value->SetFun(fun);
      out_value->AddChild(in_node1);
      out_value->AddChild(in_node2);

      return out_value;
    }

    return nullptr;
  }
                                      

  // Calculate an expression in the provided scope.
  emp::Ptr<ASTNode> Config::ParseExpression(
    pos_t & pos,
    ConfigEntry_Scope & scope,
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
  ConfigEntry & Config::ParseDeclaration(pos_t & pos, ConfigEntry_Scope & scope) {
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
    ConfigEntry_Scope & new_scope = scope.AddScope(var_name, type_map[type_name]->desc, type_name);
    ConfigType & new_obj = type_map[type_name]->init_fun(var_name);
    new_obj.SetupScope(new_scope);
    new_obj.LinkVar(new_obj._active, "_active", "Should we activate this module? (0=off, 1=on)", true);
    new_obj.LinkVar(new_obj._desc,   "_desc",   "Special description for those object.", true);
    new_obj.SetupConfig();

    return new_scope;
  }

  // Parse an event description.
  emp::Ptr<ASTNode> Config::ParseEvent(pos_t & pos, ConfigEntry_Scope & scope) {
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
  emp::Ptr<ASTNode> Config::ParseStatement(pos_t & pos, ConfigEntry_Scope & scope) {
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

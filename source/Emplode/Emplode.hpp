/**
 *  @note This file is part of Emplode, currently within https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2022.
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
#include "emp/base/notify.hpp"
#include "emp/data/Datum.hpp"
#include "emp/meta/TypeID.hpp"
#include "emp/tools/string_utils.hpp"

#include "AST.hpp"
#include "DataFile.hpp"
#include "EmplodeType.hpp"
#include "EventManager.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
#include "Symbol_Function.hpp"
#include "SymbolTable.hpp"
#include "TypeInfo.hpp"

namespace emplode {

  class Emplode {
  public:
    using pos_t = emp::TokenStream::Iterator;

  protected:
    std::string filename;      ///< Source for for code to generate.
    SymbolTable symbol_table;  ///< Management of identifiers.
    Lexer lexer;               ///< Lexer to process input code.
    Parser parser;             ///< Parser to transform token stream into an abstract syntax tree.
    ASTNode_Block ast_root;    ///< Abstract syntax tree version of input file.

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

  public:
    Emplode(std::string in_filename="")
      : filename(in_filename)
      , symbol_table("Emplode")
      , ast_root(symbol_table.GetRootScope())
    {
      if (filename != "") Load(filename);

      // Setup default functions.

      // 'EXEC' dynamically executes the contents of a string.
      auto exec_fun = [this](const std::string & expression) { return Execute(expression); };
      AddFunction("EXEC", exec_fun, "Dynamically execute the string passed in.");

      // 'PRINT' is a simple debugging command to output the value of a variable.
      auto print_fun = [](const emp::vector<emp::Ptr<Symbol>> & args) {
          for (auto entry_ptr : args) entry_ptr->Print(std::cout);
          std::cout << std::endl;
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
      AddFunction("IF", [](double x, double y, double z){ return (x!=0.0) ? y : z; },
                  "If arg1 is true, return arg2, else arg3" );
      AddFunction("CLAMP", [](double x, double y, double z){ return (x<y) ? y : (x>z) ? z : x; },
                  "Return arg1, forced into range [arg2,arg3]" );
      AddFunction("TO_SCALE", [](double x, double y, double z){ return (z-y)*x+y; },
                  "Scale arg1 to arg2-arg3 as unit distance" );
      AddFunction("FROM_SCALE", [](double x, double y, double z){ return (x-y) / (z-y); },
                  "Scale arg1 from arg2-arg3 as unit distance" );

      // Setup default DataFile type.
      auto df_init = [this](const std::string & name) {
        return emp::NewPtr<DataFile>(name, symbol_table.GetFileManager());
      };
      auto df_copy = symbol_table.DefaultCopyFun<DataFile>();
      auto & df_type = AddType<DataFile>("DataFile", "Manage CSV-style date file output.",
                                         df_init, df_copy, true);
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
      df_type.AddMemberFunction(
        "ADD_SETUP",
        [exec_fun](DataFile & file, std::string cmd){
          return file.AddSetup( [exec_fun,cmd](){ exec_fun(cmd); });
        },
        "Add a command to be run each time before columns are output."
      );
    }

    // Prevent copy or move since we are using lambdas that capture 'this'
    Emplode(const Emplode &) = delete;
    Emplode(Emplode &&) = delete;
    Emplode & operator=(const Emplode &) = delete;
    Emplode & operator=(Emplode &&) = delete;

    /// Create a new type of event that can be used in the scripting language.
    bool AddSignal(const std::string & name) { return symbol_table.AddSignal(name); }

    /// Trigger all actions linked to a signal.
    template <typename... ARG_Ts>
    void Trigger(const std::string & name, ARG_Ts... args) {
      symbol_table.Trigger(name, std::forward<ARG_Ts>(args)...);
    }

    template <typename... EXTRA_Ts, typename... ARG_Ts>
    TypeInfo & AddType(ARG_Ts &&... args) {
      return symbol_table.AddType<EXTRA_Ts...>( std::forward<ARG_Ts>(args)... );
    }

    TypeInfo & GetType(const std::string & type_name) {
      return symbol_table.GetType(type_name);
    }

    const TypeInfo & GetType(const std::string & type_name) const {
      return symbol_table.GetType(type_name);
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
      std::ifstream file(filename);           // Load the provided file.
      emp::TokenStream tokens = lexer.Tokenize(file, filename);          // Convert to more-usable tokens.
      file.close();                           // Close the file (now that it's converted)
      pos_t pos = tokens.begin();             // Start at the beginning of the file.

      // Parse and run the program, starting from the outer scope.
      ParseState state{pos, symbol_table, symbol_table.GetRootScope(), lexer};
      auto cur_block = parser.ParseStatementList(state);
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
      emp::TokenStream tokens = lexer.Tokenize(statements, name);    // Convert to tokens.
      pos_t pos = tokens.begin();

      // Parse and run the program, starting from the outer scope.
      ParseState state{pos, symbol_table, symbol_table.GetRootScope(), lexer};
      auto cur_block = parser.ParseStatementList(state);
      cur_block->Process();

      // Store this AST onto the full set we're working with.
      ast_root.AddChild(cur_block);
    }

    // Load the provided statement, run it, and return the resulting value.
    emp::Datum Execute(std::string_view statement, emp::Ptr<Symbol_Scope> scope=nullptr) {
      if (!scope) scope = &symbol_table.GetRootScope();        // Default scope to root level.
      auto tokens = lexer.Tokenize(statement, "eval command"); // Convert to a TokenStream.
      tokens.push_back(lexer.ToToken(";"));                    // Ensure a semi-colon at end.
      pos_t pos = tokens.begin();                              // Start are beginning of stream.
      ParseState state{pos, symbol_table, symbol_table.GetRootScope(), lexer};
      auto cur_expr = parser.ParseStatement(state);            // Convert tokens to AST

      // Now place the expression in a temporary block.
      auto cur_block = emp::NewPtr<ASTNode_Block>(symbol_table.GetRootScope(), 0);
      cur_block->SetSymbolTable(state.GetSymbolTable());
      cur_block->AddChild(cur_expr);

      // Process just the expressions so that we can get a result from it.
      auto result_ptr = cur_expr->Process();                // Process AST to get result symbol.
      emp::Datum result;
      if (result_ptr) {
        if (result_ptr->IsNumeric()) result = result_ptr->AsDouble(); // Result is numeric output.
        else result = result_ptr->AsString();                         // Result is string output.
        if (result_ptr->IsTemporary()) result_ptr.Delete();           // Delete temp result symbol.
      }
      cur_block.Delete();                                   // Delete the temporary AST.
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

}

#endif

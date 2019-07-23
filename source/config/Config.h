/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  Config.h
 *  @brief Manages all configuration of MABE runs.
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
 *   r = k:size;         // = 3  (always a value)
 *   s = f:names;        // = ["a","b","c","g","h","i","j"] (vector of strings in alphabetical order)
 *   t = c:string;       // = "17"  (convert value to string)
 *   u = (t+"00"):value; // = 1700  (convert string to value; can use temporaries!)
 *   // ALSO- :is_string, :is_value, :is_struct, :is_array (return 0 or 1)
 *   //       :type (returns a string indicating type!)
 * 
 * 
 *  In practice:
 *   organism_types = {
 *     Sheep = {
 *       class = MarkovBrain;
 *       outputs = 10;
 *       node_weights = 0.75;
 *       recurrance = 5;
 *     }
 *     Wolves = {
 *       class = MarkovBrain;
 *       outputs = 10;
 *       node_weights = 0.75;
 *       recurrance = 3;
 *     }
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

#include "base/assert.h"
#include "meta/TypeID.h"
#include "tools/string_utils.h"

#include "ConfigEntry.h"
#include "ConfigLexer.h"
#include "ConfigLink.h"

namespace mabe {

  class Config {
  protected:
    std::string filename;             ///< Source for for code to generate.
    ConfigLexer lexer;                ///< Lexer to process input code.
    emp::vector<emp::Token> tokens;   ///< Tokenized version of input file.
    bool debug = false;               ///< Should we print full debug information?

   ConfigStruct root_struct;          ///< All variables from the root level.

    // -- Helper functions --
    bool HasToken(int pos) const { return (pos >= 0) && (pos < (int) tokens.size()); }
    bool IsID(int pos) const { return HasToken(pos) && lexer.IsID(tokens[pos]); }
    bool IsNumber(int pos) const { return HasToken(pos) && lexer.IsNumber(tokens[pos]); }
    bool IsChar(int pos) const { return HasToken(pos) && lexer.IsChar(tokens[pos]); }
    bool IsString(int pos) const { return HasToken(pos) && lexer.IsString(tokens[pos]); }
    bool IsDots(int pos) const { return HasToken(pos) && lexer.IsDots(tokens[pos]); }

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
      std::cout << "Error (token " << pos << "): " << emp::to_string(std::forward<Ts>(args)...) << "\nAborting." << std::endl;
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
    emp::Ptr<ConfigEntry> ProcessVar(size_t & pos,
                                     emp::Ptr<ConfigStruct> cur_scope,
                                     bool create_ok=false)
    {
      bool scan_scopes = !create_ok; // By default, we either create a variable OR scan for it.

      // First, check for leading dots.
      if (IsDots(pos)) {
        scan_scopes = false;             // One or more initial dots specify scope; don't scan!
        size_t num_dots = GetSize(pos);  // Extra dots shift scope.
        while (num_dots > 1) {
          cur_scope = cur_scope->GetScope();
          if (cur_scope.IsNull()) Error(pos, "Too many dots; goes beyond global scope.");
        }
        pos++;
      }

      // Next, we must have a variable name.
      // @CAO: Or a : ?  E.g., technically "..:size" could give you the parent scope size.
      RequireID(pos, "Must provide a variable identifier!");
      std::string var_name = AsLexeme(pos++);

      // Lookup this variable.
      emp::Ptr<ConfigEntry> cur_entry = cur_scope->LookupEntry(var_name, scan_scopes);

      // If we can't find this variable, either build it or throw an error.
      if (cur_entry.IsNull()) {
        if (!create_ok) Error(pos, "Variable identifier '", var_name, "' not found.");
        cur_entry = &cur_scope->AddPlaceholder(var_name);
        return cur_entry;
      }

      // If this variable just provided a scope, keep going.
      if (IsDots(pos)) return ProcessVar(pos, cur_scope, create_ok);

      // No proper entry was found; return null as an error.
      return nullptr;
    }

    // Load a value from the provided scope, which can come from a variable or a literal.
    // NOTE: May create temporary ConfigEntries that need to be cleaned up by receiver!
    emp::Ptr<ConfigEntry> ProcessValue(size_t & pos, emp::Ptr<ConfigStruct> cur_scope) {
      // Anything that begins with an identifier or dots must represent a variable.  Refer!
      if (IsID(pos) || IsDots(pos)) return ProcessVar(pos, cur_scope, false);

      // A literal number should have a temporary created with its value.
      if (IsNumber(pos)) {
        auto tmp_ptr = emp::NewPtr<ConfigValue>("","",nullptr);  // Create a temporary ConfigEntry
        tmp_ptr->Set(emp::from_string<double>(AsLexeme(pos)));   // Set entry to the value of token.
        return tmp_ptr;                                          // And return!
      }

      // A literal char should be converted to its ASCII value.
      if (IsChar(pos)) {
        auto tmp_ptr = emp::NewPtr<ConfigValue>("","",nullptr);  // Create a temporary ConfigEntry
        char lit_char = emp::from_literal_char(AsLexeme(pos));   // Convert the literal char.
        tmp_ptr->Set((double) lit_char);                         // Set entry to the value of token.
        return tmp_ptr;                                          // And return!
      }

      // A literal string should be converted to a regular string and used.
      if (IsString(pos)) {
        auto tmp_ptr = emp::NewPtr<ConfigString>("","",nullptr); // Create a temporary ConfigEntry
        tmp_ptr->Set(emp::from_literal_string(AsLexeme(pos)));   // Set entry to the value of token.
        return tmp_ptr;                                          // And return!
      }

      // No value found!
      return nullptr;
    }

    // Process the next input in the specified Struct.
    void ProcessStatement(size_t & pos, ConfigStruct & scope) {
      size_t start_pos = pos; // Track the starting position for semantic errors.

      // Basic structure: VAR = VALUE ;
      emp::Ptr<ConfigEntry> lhs = ProcessVar(pos, &scope, true);
      RequireChar('=', pos, "Expected '=' after variable '", lhs->GetName(),  "' for assignment.");
      emp::Ptr<ConfigEntry> rhs = ProcessValue(pos, &scope);
      RequireChar(';', pos, "Expected ';' at the end of a statement.");

      // If the LHS is a placeholder, fix it!
      if (lhs->IsPlaceholder()) {
        // If the RHS is a temporary, we can use it rather than reallocate; otherwise clone it.
        emp::Ptr<ConfigEntry> new_entry;
        if (rhs->IsTemporary()) {
          new_entry = rhs;
          rhs = nullptr;
        }
        else {
          new_entry = rhs->Clone();
        }

        // Reset the new entry to have the correct name, etc.
        const std::string & name = lhs->GetName();
        new_entry->SetName(name).SetDesc(lhs->GetDesc()).SetDefault(lhs->GetDefaultVal());

        // Replace the placeholder with the new entry in the proper scope.
        lhs->GetScope()->Replace(name, new_entry);
        lhs.Delete();
      }

      // Otherwise if variable already exists, make sure types align.
      else {
        if (lhs->GetType() != rhs->GetType()) {
          Error(start_pos, "Type mis-match in assignment to ", lhs->GetName());
        }
      }

      // If we didn't just move over the RHS...
      if (!rhs.IsNull()) {
        // Act on the assignment!
        lhs->CopyValue(*rhs);

        // If the RHS is a temporary, delete it!
        if (rhs->IsTemporary()) rhs.Delete();
      }


    }

  public:

  };

}
#endif

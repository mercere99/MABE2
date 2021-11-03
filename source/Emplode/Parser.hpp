/**
 *  @note This file is part of Emplode, currently within https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021.
 *
 *  @file  Parser.hpp
 *  @brief Manages parsing of Emplode language input streams.
 *  @note Status: BETA
 * 
 */

#ifndef EMPLODE_PARSER_HPP
#define EMPLODE_PARSER_HPP

#include <string>
#include <utility>

#include "emp/base/Ptr.hpp"
#include "emp/base/vector.hpp"
#include "emp/tools/string_utils.hpp"

#include "Lexer.hpp"
#include "Symbol_Scope.hpp"
#include "SymbolTable.hpp"

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


}
#endif

/**
 *  @note This file is part of Emplode, currently within https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021-2022.
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

#include "AST.hpp"
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
    ParseState(const ParseState &) = default;
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
    int GetLine() const { return (int) pos->line_id; }
    size_t GetTokenSize() const { return pos.IsValid() ? pos->lexeme.size() : 0; }
    SymbolTable & GetSymbolTable() { return *symbol_table; }
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

    bool IsKeyword() const { return pos && lexer->IsKeyword(*pos); }    
    bool IsID() const { return pos && lexer->IsID(*pos); }
    bool IsNumber() const { return pos && lexer->IsNumber(*pos); }
    bool IsString() const { return pos && lexer->IsString(*pos); }
    bool IsDots() const { return pos && lexer->IsDots(*pos); }

    bool IsSignal() const { return symbol_table->HasSignal(AsLexeme()); }
    bool IsType() const { return symbol_table->HasType(AsLexeme()); }

    /// Convert the current state to a character; use \0 if cur token is not a symbol.
    char AsChar() const { return (pos && lexer->IsSymbol(*pos)) ? pos->lexeme[0] : 0; }

    /// Return the token associate with the current state.
    emp::Token AsToken() const { return *pos; }

    /// Return the lexeme associate with the current state.
    const std::string & AsLexeme() const { return pos ? pos->lexeme : emp::empty_string(); }

    /// Return the lexeme associate with the current state AND advance the token stream.
    const std::string & UseLexeme() {
      const std::string & out = AsLexeme();
      pos++;
      return out;
    }

    /// Return whether the current token is the specified lexeme; if so also advance token stream.
    bool UseIfLexeme(const std::string & test_str) {
      if (AsLexeme() != test_str) return false;
      ++pos;
      return true;
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
      
      emp::notify::Error("(", line_info, " in '", pos.GetTokenStream().GetName(), "'): ",
                         emp::to_string(std::forward<Ts>(args)...), "\nAborting.");
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

    Symbol_Var & AddLocalVar(const std::string & name, const std::string & desc) {
      return GetScope().AddLocalVar(name, desc);
    }
    Symbol_Scope & AddScope(const std::string & name, const std::string & desc) {
      return GetScope().AddScope(name, desc);
    }
    Symbol_Object & AddObject(const std::string & type_name, const std::string & var_name) {
      return symbol_table->MakeObjSymbol(type_name, var_name, GetScope());
    }

    /// Add an instance of an event with an action that should be triggered.
    template <typename... Ts>
    void AddAction(Ts &&... args) { symbol_table->AddAction(std::forward<Ts>(args)...); }
  };



  class Parser {
  private:
    std::unordered_map<std::string, size_t> precedence_map;  ///< Precedence levels for symbols.
    bool debug = false;                                      ///< Print full debug information?

    template <typename... Ts>
    void Debug(Ts... args) const {
      if (debug) std::cout << "DEBUG: " << emp::to_string(std::forward<Ts>(args)...) << std::endl;
    }

  public:
    Parser() {
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
    }
    ~Parser() {}

    /// Load a variable name from the provided scope.
    /// If create_ok is true, create any variables that we don't find.  Otherwise continue the
    /// search for them in successively outer (lower) scopes.
    [[nodiscard]] emp::Ptr<ASTNode_Leaf> ParseVar(ParseState & state,
                                                  bool create_ok=false,
                                                  bool scan_scopes=true);

    /// Load a value from the provided scope, which can come from a variable or a literal.
    [[nodiscard]] emp::Ptr<ASTNode> ParseValue(ParseState & state);

    /// Calculate the result of the provided operation on two computed entries.
    [[nodiscard]] emp::Ptr<ASTNode> ProcessOperation(const emp::Token & op_token,
                                                     emp::Ptr<ASTNode> value1,
                                                     emp::Ptr<ASTNode> value2);

    /// Calculate a full expression found in a token sequence, using the provided scope.
    /// @param state The current start of the parser and input stream
    /// @param decl_ok Can this expression begin with a declaration of a variable?
    /// @param prec_limit What is the highest precedence that expression should process?
    [[nodiscard]] emp::Ptr<ASTNode> ParseExpression(ParseState & state,
                                                    bool decl_ok=false,
                                                    size_t prec_limit=1000);

    /// Parse the declaration of a variable and return the newly created Symbol
    Symbol & ParseDeclaration(ParseState & state);

    /// Parse an event description.
    emp::Ptr<ASTNode> ParseEvent(ParseState & state);

    /// Parse a specialty keyword statement (such as IF, WHILE, etc)
    emp::Ptr<ASTNode> ParseKeywordStatement(ParseState & state);

    /// Parse the next input in the specified Struct.  A statement can be a variable declaration,
    /// an expression, or an event.
    [[nodiscard]] emp::Ptr<ASTNode> ParseStatement(ParseState & state);

    /// Keep parsing statements until there aren't any more or we leave this scope. 
    [[nodiscard]] emp::Ptr<ASTNode_Block> ParseStatementList(ParseState & state) {
      Debug("Running ParseStatementList(", state.AsString(), ")");
      auto cur_block = emp::NewPtr<ASTNode_Block>(state.GetScope(), state.GetLine());
      cur_block->SetSymbolTable(state.GetSymbolTable());
      while (state.IsValid() && state.AsChar() != '}') {
        // Parse each statement in the file.
        emp::Ptr<ASTNode> statement_node = ParseStatement(state);

        // If the current statement is real, add it to the current block.
        if (!statement_node.IsNull()) cur_block->AddChild( statement_node );
      }
      return cur_block;
    }
  };

  // Load a variable name from the provided scope.
  emp::Ptr<ASTNode_Leaf> Parser::ParseVar(ParseState & state, bool create_ok, bool scan_scopes)
  {
    int start_line = state.GetLine();
    Debug("Running ParseVar(", state.AsString(), ",", create_ok, ",", scan_scopes,
          ") at line ", start_line);

    // Check for leading dots to require this scope (one dot) or indicate a lower-level scope.
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
    return emp::NewPtr<ASTNode_Leaf>(&cur_symbol, start_line);
  }

  // Load a value from the provided scope, which can come from a variable or a literal.
  emp::Ptr<ASTNode> Parser::ParseValue(ParseState & state) {
    Debug("Running ParseValue(", state.AsString(), ")");

    // First check for a unary negation at the start of the value.
    if (state.UseIfChar('-')) {
      auto out_val = emp::NewPtr<ASTNode_Math1>("unary negation", state.GetLine());
      out_val->SetFun( [](double val){ return -val; } );
      out_val->AddChild(ParseValue(state));
      return out_val;
    }

    // Anything that begins with an identifier or dots must represent a variable.  Refer!
    if (state.IsID() || state.IsDots()) return ParseVar(state, false, true);

    // A literal number should have a temporary created with its value.
    if (state.IsNumber()) {
      Debug("...value is a number: ", state.AsLexeme());
      double value = emp::from_string<double>(state.UseLexeme()); // Calculate value.
      return MakeTempLeaf(value);                                 // Return temporary Symbol.
    }

    // A literal string should be converted to a regular string and used.
    if (state.IsString()) {
      Debug("...value is a string: ", state.AsLexeme());
      std::string str = emp::from_literal_string(state.UseLexeme(), "\"'`"); // Convert literal string.
      return MakeTempLeaf(str);                                              // Return temporary Symbol.
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
  emp::Ptr<ASTNode> Parser::ProcessOperation(const emp::Token & op_token,
                                             emp::Ptr<ASTNode> in_node1,
                                             emp::Ptr<ASTNode> in_node2)
  {
    const std::string symbol = op_token.lexeme;
    emp_assert(!in_node1.IsNull());
    emp_assert(!in_node2.IsNull());

    // If this operation is assignment, do so!
    if (symbol == "=") return emp::NewPtr<ASTNode_Assign>(in_node1, in_node2, op_token.line_id);
    
    // If the first argument is numeric, assume we are using a math operator.
    if (in_node1->IsNumeric()) {

      // Determine the output value and put it in a temporary node.
      emp::Ptr<ASTNode_Math2> out_val = emp::NewPtr<ASTNode_Math2>(symbol, op_token.line_id);

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
      auto out_val =
        emp::NewPtr<ASTNode_Op2<std::string,std::string,std::string>>(symbol, op_token.line_id);
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

      auto out_val = emp::NewPtr<ASTNode_Op2<std::string,std::string,double>>(symbol, op_token.line_id);
      out_val->SetFun(fun);
      out_val->AddChild(in_node1);
      out_val->AddChild(in_node2);

      return out_val;
    }
    else {
      auto out_val = emp::NewPtr<ASTNode_Op2<double,std::string,std::string>>(symbol, op_token.line_id);

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
                                      

  /// Calculate a full expression found in a token sequence, using the provided scope.
  /// @param state The current start of the parser and input stream
  /// @param decl_ok Can this expression begin with a declaration of a variable?
  /// @param prec_limit What is the highest precedence that expression should process?

  emp::Ptr<ASTNode> Parser::ParseExpression(ParseState & state, bool decl_ok, size_t prec_limit) {
    Debug("Running ParseExpression(", state.AsString(), ", decl_ok=", decl_ok, ", limit=", prec_limit, ")");

    // Allow this statement to be a declaration if it begins with a type.
    if (decl_ok && state.IsType()) {
      Symbol & new_symbol = ParseDeclaration(state);
  
      // If this symbol is a new scope, it can be populated now either directly (in braces)
      // or indirectly (with an assignment)
      if (new_symbol.IsScope()) {
        if (state.UseIfChar('{')) {
          state.PushScope(new_symbol.AsScope());
          emp::Ptr<ASTNode> out_node = ParseStatementList(state);
          state.PopScope();
          state.UseRequiredChar('}', "Expected scope '", new_symbol.GetName(), "' to end with a '}'.");
          return out_node;
        }
      }      

      // Otherwise rewind so that the new variable can be used to start an expression.
      --state;
    }

    /// Process a value (and possibly more!)
    emp::Ptr<ASTNode> cur_node = ParseValue(state);
    emp::Token op_token = state.AsToken();
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
        cur_node = emp::NewPtr<ASTNode_Call>(cur_node, args, op_token.line_id);
      }

      // Otherwise we must have a binary math operation.
      else {
        emp::Ptr<ASTNode> node2 = ParseExpression(state, false, precedence_map[op]);
        cur_node = ProcessOperation(op_token, cur_node, node2);
      }

      // Move the current value over to cur_node and check if we have a new operator...
      op = state.AsLexeme();
      op_token = state.AsToken();
    }

    emp_assert(!cur_node.IsNull());
    return cur_node;
  }

  // Parse an the declaration of a variable.
  Symbol & Parser::ParseDeclaration(ParseState & state) {
    std::string type_name = state.UseLexeme();
    state.RequireID("Type name '", type_name, "' must be followed by variable to declare.");
    std::string var_name = state.UseLexeme();

    if (type_name == "Var") return state.AddLocalVar(var_name, "Local variable.");
    else if (type_name == "Struct") return state.AddScope(var_name, "Local struct");

    // Otherwise we have an object of a custom type to add.
    Debug("Building object '", var_name, "' of type '", type_name, "'");
    return state.AddObject(type_name, var_name);
  }

  // Parse an event description.
  emp::Ptr<ASTNode> Parser::ParseEvent(ParseState & state) {
    emp::Token start_token = state.AsToken();
    state.UseRequiredChar('@', "All event declarations must being with an '@'.");
    state.RequireID("Events must start by specifying signal name.");
    const std::string & trigger_name = state.UseLexeme();
    state.UseRequiredChar('(', "Expected parentheses after '", trigger_name, "' for args.");

    emp::vector<emp::Ptr<ASTNode>> args;
    while (state.AsChar() != ')') {
      args.push_back( ParseExpression(state, true) );
      state.UseIfChar(',');                     // Skip comma if next (does allow trailing comma)
    }
    state.UseRequiredChar(')', "Event args must end in a ')'");

    auto action_block = emp::NewPtr<ASTNode_Block>(state.GetScope(), state.GetLine());
    action_block->SetSymbolTable(state.GetSymbolTable());
    emp::Ptr<ASTNode> action_node = ParseStatement(state);

    // If the action statement is real, add it to the action block.
    if (!action_node.IsNull()) action_block->AddChild( action_node );

    Debug("Building event '", trigger_name, "' with args ", args);

    state.AddAction(trigger_name, args, action_block, start_token.line_id);

    return nullptr;
  }

  /// Parse a specialty keyword statement (such as IF, WHILE, etc)
  emp::Ptr<ASTNode> Parser::ParseKeywordStatement(ParseState & state) {
    size_t keyword_line = state.GetLine();

    if (state.UseIfLexeme("IF")) {
      state.UseRequiredChar('(', "Expected '(' to begin IF test condition.");
      emp::Ptr<ASTNode> test_node = ParseExpression(state);
      state.UseRequiredChar(')', "Expected ')' to end IF test condition.");
      emp::Ptr<ASTNode> true_node = ParseStatement(state);
      emp::Ptr<ASTNode> else_node = nullptr;
      if (state.UseIfLexeme("ELSE")) else_node = ParseStatement(state);
      return emp::NewPtr<ASTNode_If>(test_node, true_node, else_node, keyword_line);
    }


    // If we made it this far, we have an error.  Identify and deal with it!

    if (state.UseIfLexeme("ELSE")) state.Error("'ELSE' must be preceded by an 'IF' statement.");
    else state.Error("Keyword '", state.AsLexeme(), "' not yet implemented.");

    return nullptr;
  }

  // Process the next input in the specified Struct.
  emp::Ptr<ASTNode> Parser::ParseStatement(ParseState & state) {
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

    // Allow select commands that are only possible at the full statement level (not expressions)
    if (state.IsKeyword()) return ParseKeywordStatement(state);

    // If we made it here, remainder should be an expression; it may begin with a declaration.
    emp::Ptr<ASTNode> out_node = ParseExpression(state, true);

    // Expressions must end in a semi-colon.
    state.UseRequiredChar(';', "Expected ';' at the end of a statement; found: ", state.AsLexeme());

    return out_node;
  }  
}
#endif

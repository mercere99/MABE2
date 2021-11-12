/**
 *  @note This file is part of Emplode, currently within https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2021.
 *
 *  @file  Symbol.hpp
 *  @brief Manages a single configuration entry (e.g., variables + base for scopes and functions).
 *  @note Status: BETA
 * 
 *  The symbol table for the configuration language is managed as a collection of
 *  configuration entries.  These include specializations for Symbol_Linked (for linked
 *  variables), Symbol_Function and Symbol_Scope, all defined in their own files
 *  and derived from Symbol.
 * 
 *  Development Notes:
 *  - Currently we are not using Format; this would be useful if we want to type-check inputs more
 *    carefully.
 *  - When a Symbol is used for a temporary value, it doesn't actually need name or desc;
 *    we can probably remove these pretty easily to save on memory if needed.
 */

#ifndef EMPLODE_SYMBOL_HPP
#define EMPLODE_SYMBOL_HPP

#include <type_traits>

#include "emp/base/assert.hpp"
#include "emp/base/Ptr.hpp"
#include "emp/base/vector.hpp"
#include "emp/math/Range.hpp"
#include "emp/meta/meta.hpp"
#include "emp/meta/TypeID.hpp"
#include "emp/tools/string_utils.hpp"
#include "emp/tools/value_utils.hpp"

namespace emplode {

  class EmplodeType;
  class Symbol_Function;
  class Symbol_Object;
  class Symbol_Scope;
  class TypeInfo;

  class Symbol {
  protected:
    std::string name;             ///< Unique name for symbol; empty name implies temporary.
    std::string desc;             ///< Description to put in comments for this symbol.
    emp::Ptr<Symbol_Scope> scope; ///< Which scope was this variable defined in?

    bool is_temporary = false;    ///< Is this Symbol temporary and should be deleted?
    bool is_builtin = false;      ///< Built-in entries should not be written to config files.
  
    enum class Format { NONE=0, SCOPE,
                        BOOL, INT, UNSIGNED, DOUBLE,                                    // Values
                        STRING, FILENAME, PATH, URL, ALPHABETIC, ALPHANUMERIC, NUMERIC  // Strings
                      };
    Format format = Format::NONE;

    // If we know the constraints on this parameter we can perform better error checking.
    emp::Range<double> range;  ///< Min and max values allowed for this config entry (if numerical).
    bool integer_only=false;   ///< Should we only allow integer values?

    using symbol_ptr_t = emp::Ptr<Symbol>;

    // Helper functions.

    /// Write out the provided description at the comment_offset.  The start_pos is where the
    /// text currently is.   For multi-line comments, make sure to indent properly.
    void WriteDesc(std::ostream & os, size_t comment_offset, size_t start_pos) const {
      // If there is no description, provide a newline and stop.
      if (desc.size() == 0) {
        std::cout << '\n';
        return;
      }

      // Break the description at the newlines.
      emp::vector<std::string> lines = emp::slice(desc);

      for (const auto & line : lines) {
        // Find the current line to print.
        while (start_pos++ < comment_offset) os << " ";
        os << "// " << line << '\n';
        start_pos = 0;
      }
    }

  public:
    Symbol(const std::string & _name,
                const std::string & _desc,
                emp::Ptr<Symbol_Scope> _scope)
      : name(_name), desc(_desc), scope(_scope) { }
    Symbol(const Symbol &) = default;
    virtual ~Symbol() { }

    const std::string & GetName() const noexcept { return name; }
    const std::string & GetDesc() const noexcept { return desc; }
    emp::Ptr<Symbol_Scope> GetScope() { return scope; }
    bool IsTemporary() const noexcept { return is_temporary; }
    bool IsBuiltin() const noexcept { return is_builtin; }
    Format GetFormat() const noexcept { return format; }

    virtual std::string GetTypename() const { return "Unknown"; }

    virtual bool IsNumeric() const { return false; }   ///< Is symbol any kind of number?
    virtual bool IsBool() const { return false; }      ///< Is symbol a Boolean value?
    virtual bool IsDouble() const { return false; }    ///< Is symbol a floting point value?
    virtual bool IsInt() const { return false; }       ///< Is symbol a integer value?
    virtual bool IsString() const { return false; }    ///< Is symbol a string?

    virtual bool IsError() const { return false; }     ///< Does symbol flag an error?
    virtual bool IsFunction() const { return false; }  ///< Is symbol a function?
    virtual bool IsObject() const { return false; }    ///< Is symbol associated with C++ object?
    virtual bool IsScope() const { return false; }     ///< Is symbol a full scope?

    virtual bool IsLocal() const { return false; }     ///< Was symbol defined in config file?

    virtual bool HasNumericReturn() const { return false; } ///< Is symbol a function that returns a number?
    virtual bool HasStringReturn() const { return false; }  ///< Is symbol a function that returns a string?

    Symbol & SetName(const std::string & in) { name = in; return *this; }
    Symbol & SetDesc(const std::string & in) { desc = in; return *this; }
    Symbol & SetTemporary(bool in=true) { is_temporary = in; return *this; }
    Symbol & SetBuiltin(bool in=true) { is_builtin = in; return *this; }

    virtual double AsDouble() const { emp_assert(false); return 0.0; }
    virtual std::string AsString() const { emp_assert(false); return ""; }

    virtual Symbol & SetValue(double in) { (void) in; emp_assert(false, in); return *this; }
    virtual Symbol & SetString(const std::string & in) { (void) in; emp_assert(false, in); return *this; }

    virtual emp::Ptr<Symbol_Function> AsFunctionPtr() { return nullptr; }
    virtual emp::Ptr<const Symbol_Function> AsFunctionPtr() const { return nullptr; }
    virtual emp::Ptr<Symbol_Object> AsObjectPtr() { return nullptr; }
    virtual emp::Ptr<const Symbol_Object> AsObjectPtr() const { return nullptr; }
    virtual emp::Ptr<Symbol_Scope> AsScopePtr() { return nullptr; }
    virtual emp::Ptr<const Symbol_Scope> AsScopePtr() const { return nullptr; }

    Symbol_Function & AsFunction() { emp_assert(AsFunctionPtr()); return *(AsFunctionPtr()); }
    const Symbol_Function & AsFunction() const { emp_assert(AsFunctionPtr()); return *(AsFunctionPtr()); }
    Symbol_Object & AsObject() { emp_assert(AsObjectPtr()); return *(AsObjectPtr()); }
    const Symbol_Object & AsObject() const { emp_assert(AsObjectPtr()); return *(AsObjectPtr()); }
    Symbol_Scope & AsScope() { emp_assert(AsScopePtr()); return *(AsScopePtr()); }
    const Symbol_Scope & AsScope() const { emp_assert(AsScopePtr()); return *(AsScopePtr()); }

    virtual emp::Ptr<EmplodeType> GetObjectPtr() { return nullptr; }
    virtual emp::Ptr<const EmplodeType> GetObjectPtr() const { return nullptr; }
    virtual emp::Ptr<const TypeInfo> GetTypeInfoPtr() const { return nullptr; }

    /// A generic As() function that will call the appropriate converter.
    template <typename T>
    decltype(auto) As() {
      // If a const type is requested, non-const can be converted, so work with that.
      using decay_T = std::decay_t<T>;
      constexpr bool is_nonconst_ref = !std::is_const_v<T> && std::is_reference_v<T>;

      // If we have a numeric or string request, run the appropriate conversion.
      if constexpr (std::is_arithmetic<decay_T>() && !is_nonconst_ref) {
        return static_cast<T>(AsDouble());
      }
      else if constexpr (std::is_same<T, std::string>() ||
                         std::is_same<T, const std::string &>()) {
        return AsString();
      }

      // If we want either a pointer or reference to the current object, return it.
      else if constexpr (std::is_same<decay_T, emp::Ptr<Symbol>>()) { return this; }
      else if constexpr (std::is_same<decay_T, Symbol>()) { return *this; }

      // If we want a dervied Symbol type, convert and return it.
      else if constexpr (std::is_base_of<Symbol, decay_T>()) {
        emp::Ptr<decay_T> out_ptr = dynamic_cast<decay_T*>(this);
        emp_assert(out_ptr); // @CAO: Should provide a user error.
        return *out_ptr;
      }

      // If we want a user-defined type, it must be derived from EmplodeType.
      else if constexpr (std::is_base_of<EmplodeType, decay_T>()) {
        emp::Ptr<EmplodeType> obj_ptr = GetObjectPtr();
        emp_assert(obj_ptr);   // @CAO: Should provide a user error.
        emp::Ptr<decay_T> out_ptr = obj_ptr.DynamicCast<decay_T>();
        emp_assert(out_ptr);   // @CAO: Should provide a user error.
        return *out_ptr;
      }

      // Oh no! We don't know this type...
      else {
        static_assert(emp::dependent_false<T>(), "Invalid conversion for Symbol::As()");
        emp_error(emp::GetTypeID<T>());  // Print more info when above line is commented out.
        auto out = emp::NewPtr<std::remove_reference_t<decay_T>>();
        return (T) *out;
      }
    }

    Symbol & SetMin(double min) { range.SetLower(min); return *this; }
    Symbol & SetMax(double max) { range.SetUpper(max); return *this; }

    // Try to copy another config symbol into this one; return true if successful.
    virtual bool CopyValue(const Symbol & ) { return false; }

    /// If this symbol is a scope, we should be able to lookup other entries inside it.
    virtual symbol_ptr_t LookupSymbol(const std::string & in_name, bool /* scan_scopes */=true) {
      return (in_name == "") ? this : nullptr;
    }
    virtual emp::Ptr<const Symbol>
    LookupSymbol(const std::string & in_name, bool /* scan_scopes */=true) const {
      return (in_name == "") ? this : nullptr;
    }
    virtual bool Has(const std::string & in_name) const { return (bool) LookupSymbol(in_name); }

    /// If this symbol is a function, we should be able to call it.
    virtual symbol_ptr_t Call(const emp::vector<symbol_ptr_t> & args);

    // --- Implicit conversion operators ---
    operator double() const { return AsDouble(); }
    operator int() const { return static_cast<int>(AsDouble()); }
    operator size_t() const { return static_cast<size_t>(AsDouble()); }
    operator std::string() const { return AsString(); }
    operator emp::Ptr<Symbol>() { return this; }
    operator EmplodeType&() { return *GetObjectPtr(); }

    /// Allocate a duplicate of this class.
    virtual symbol_ptr_t Clone() const = 0;

    virtual const Symbol & Write(std::ostream & os=std::cout, const std::string & prefix="",
                                      size_t comment_offset=32) const
    {
      // If this is a built-in symbol, don't print it.
      if (IsBuiltin()) return *this;

      // Setup this symbol.
      std::string cur_line = prefix;
      if (IsLocal()) cur_line += emp::to_string(GetTypename(), " ", name, " = ");
      else cur_line += emp::to_string(name, " = ");

      // Print the current value of this variable; if it's a string make sure to turn it to a literal.
      cur_line += IsString() ? emp::to_literal(AsString()) : AsString();
      cur_line += ";";
      os << cur_line;

      // Write out the description for this line.
      WriteDesc(os, comment_offset, cur_line.size());

      return *this;
    }

    // Generate a string with information about this symbol.
    std::string DebugString() const {
      std::string out = emp::to_string(
        "Symbol '", GetName(),
        "' type=", GetTypename(),
        " scope=", scope ? scope.Cast<Symbol>()->GetName() : "[none]");

      if (IsTemporary()) out += " TEMPORARY";
      if (IsBuiltin()) out += " BUILTIN";
      if (IsError()) out += " ERROR";
      if (IsNumeric()) out += " Numeric";
      if (IsString()) out += " String";
      if (IsFunction()) out += " Function";
      if (IsObject()) out += " Object";
      if (IsScope()) out += " Scope";
      if (IsLocal()) out += " Local";
      if (IsFunction()) out += " Function";
      if (HasNumericReturn()) out += " (numeric return)";
      if (HasStringReturn()) out += " (string return)";

      return out;
    }

  };

  /// A generic version of a symbol for an internally maintained variable.
  template <typename T>
  class Symbol_Var : public Symbol {
  private:
    T value = 0;
  public:
    static_assert(std::is_arithmetic<T>(), "Symbol_Var must use std::string or arithmetic values.");

    using this_t = Symbol_Var<T>;

    template <typename... ARGS>
    Symbol_Var(const std::string & in_name,
                    T default_val,
                    const std::string & in_desc="",
                    emp::Ptr<Symbol_Scope> in_scope=nullptr)
      : Symbol(in_name, in_desc, in_scope), value(default_val) { ; }
    Symbol_Var(const Symbol_Var<T> &) = default;

    std::string GetTypename() const override {
      if constexpr (std::is_scalar_v<T>) return "Value";
      else return "Unknown";
    }

    symbol_ptr_t Clone() const override { return emp::NewPtr<this_t>(*this); }

    double AsDouble() const override { return (double) value; }
    std::string AsString() const override { return emp::to_string(value); }
    Symbol & SetValue(double in) override { value = (T) in; return *this; }
    Symbol & SetString(const std::string & in) override {
      value = emp::from_string<T>(in);
      return *this;
    }

    bool IsNumeric() const override { return std::is_scalar_v<T>; }
    bool IsBool() const override { return std::is_same<bool, T>(); }
    bool IsInt() const override { return std::is_same<int, T>(); }
    bool IsDouble() const override { return std::is_same<double, T>(); }

    bool IsLocal() const override { return true; }

    bool CopyValue(const Symbol & in) override { SetValue(in.AsDouble()); return true; }
  };
  using Symbol_DoubleVar = Symbol_Var<double>;

  /// Symbol as a temporary variable of type STRING.
  template<>
  class Symbol_Var<std::string> : public Symbol {
  private:
    std::string value;
  public:
    using this_t = Symbol_Var<std::string>;

    template <typename... ARGS>
    Symbol_Var(const std::string & in_name, const std::string & in_val, ARGS &&... args)
      : Symbol(in_name, std::forward<ARGS>(args)...), value(in_val) { ; }
    Symbol_Var(const Symbol_Var<std::string> &) = default;

    std::string GetTypename() const override { return "String"; }

    symbol_ptr_t Clone() const override { return emp::NewPtr<this_t>(*this); }

    double AsDouble() const override { return emp::from_string<double>(value); }
    std::string AsString() const override { return value; }
    Symbol & SetValue(double in) override { value = emp::to_string(in); return *this; }
    Symbol & SetString(const std::string & in) override { value = in; return *this; }

    bool IsString() const override { return true; }
    bool IsLocal() const override { return true; }

    bool CopyValue(const Symbol & in) override { value = in.AsString(); return true; }
  };
  using Symbol_StringVar = Symbol_Var<std::string>;

  /// A Symbol to transmit an error due to invalid parsing.
  /// The description provides the error and the IsError() flag is set to true.
  class Symbol_Error : public Symbol {
  private:
    using this_t = Symbol_Error;
  public:
    template <typename... ARGS>
    Symbol_Error(ARGS &&... args)
      : Symbol("__Error", emp::to_string(args...), nullptr) { is_temporary = true; }

    std::string GetTypename() const override { return "[[Error]]"; }

    bool IsError() const override { return true; }

    symbol_ptr_t Clone() const override { return emp::NewPtr<this_t>(*this); }
  };


  ////////////////////////////////////////////////////
  //  Function definitions...

  emp::Ptr<Symbol> Symbol::Call( const emp::vector<symbol_ptr_t> & /* args */ ) {
    return emp::NewPtr<Symbol_Error>("Cannot call a function on non-function '", name, "'.");
  }

}

#endif

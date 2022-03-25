/**
 *  @note This file is part of Emplode, currently within https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2022.
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
#include "emp/base/error.hpp"
#include "emp/base/Ptr.hpp"
#include "emp/base/vector.hpp"
#include "emp/data/Datum.hpp"
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

    virtual std::string GetTypename() const = 0;       ///< Derived classes must provide type info.

    virtual bool IsNumeric() const { return false; }   ///< Is symbol any kind of number?
    virtual bool IsString() const { return false; }    ///< Is symbol a string?
    virtual bool HasValue() const { return false; }    ///< Is a unique value associated with this symbol?

    virtual bool IsError() const { return false; }     ///< Does symbol flag an error?
    virtual bool IsFunction() const { return false; }  ///< Is symbol a function?
    virtual bool IsObject() const { return false; }    ///< Is symbol associated with C++ object?
    virtual bool IsScope() const { return false; }     ///< Is symbol a full scope?
    virtual bool IsContinue() const { return false; }  ///< Is symbol a "continue" signal?
    virtual bool IsBreak() const { return false; }     ///< Is symbol a "break" signal?

    virtual bool IsLocal() const { return false; }     ///< Was symbol defined in config file?

    virtual bool HasNumericReturn() const { return false; } ///< Is symbol a function that returns a number?
    virtual bool HasStringReturn() const { return false; }  ///< Is symbol a function that returns a string?

    Symbol & SetName(const std::string & in) { name = in; return *this; }
    Symbol & SetDesc(const std::string & in) { desc = in; return *this; }
    Symbol & SetTemporary(bool in=true) { is_temporary = in; return *this; }
    Symbol & SetBuiltin(bool in=true) { is_builtin = in; return *this; }

    virtual double AsDouble() const { return std::nan("NaN"); }
    virtual std::string AsString() const { return "[[__INVALID SYMBOL CONVERSION__]]"; }
    virtual void Print(std::ostream & os) const { os << AsString(); }

    virtual Symbol & SetValue(double in) { (void) in; emp_assert(false, in); return *this; }
    virtual Symbol & SetString(const std::string & in) { (void) in; emp_assert(false, in); return *this; }
    Symbol & operator=(double in) { return SetValue(in); }
    Symbol & operator=(const std::string & in) { return SetString(in); }

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
    virtual emp::TypeID GetObjectType() const { return emp::GetTypeID<void>(); }
    virtual bool HasObjectType(emp::TypeID in_type) const { return in_type == GetObjectType(); }
    template <typename T> bool HasObjectType() const { return HasObjectType(emp::GetTypeID<T>()); }

    /// Helper struct to determine the return type for As.
    template <typename T> struct AsRT_impl { using type = T; };            // Default: T requested
    template <typename T> struct AsRT_impl<const T &> { using type = T; }; // const ref -> value

    /// A generic As() function that will call the appropriate converter.
    template <typename T>
    auto As() -> typename AsRT_impl<T>::type {
      // If a const type is requested, non-const can be converted, so work with that.
      using decay_T = std::decay_t<T>;
      // constexpr bool is_const = std::is_const_v<T>;
      constexpr bool is_ref = std::is_reference_v<T>;
      // constexpr bool is_substitutable = is_const || !is_ref;
      constexpr bool is_substitutable = !is_ref;

      // If we have a numeric or string request, run the appropriate conversion.
      if constexpr (std::is_arithmetic<decay_T>() && is_substitutable) {
        return static_cast<T>(AsDouble());
      }
      else if constexpr (std::is_same<T, std::string>() ||
                         std::is_same<T, const std::string &>()) {
        return AsString();
      }

      // If we want either a pointer or reference to a Symbol object, return it.
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
        // If have an object, see what we can convert it to.
        if (obj_ptr){
          // First, check if this is already the correct type.
          emp::Ptr<decay_T> typed_obj_ptr = obj_ptr.DynamicCast<decay_T>();

          // If not, check if we can substitute it for another type.
          if (!typed_obj_ptr) {
            // If not (and we can use an r-value) build a temporary value!
            if constexpr (is_substitutable) {
              return decay_T::template MakeRValueFrom<decay_T>(*obj_ptr);
            }

            // If both options fail, report an error.
            emp_error("Cannot convert symbol to target object type.  Symbol: ",
                      DebugString(), "  Target: ", emp::GetTypeID<T>());
          }

          return *typed_obj_ptr;
        }

        // We must be trying to return an object from something other than another object.
        if constexpr (is_substitutable) {
          if (IsNumeric()) return decay_T::template MakeRValueFrom<decay_T>(AsDouble());
          return decay_T::template MakeRValueFrom<decay_T>(AsString());
        }

        emp_error("Cannot convert symbol to target object type.", DebugString(), emp::GetTypeID<T>());
        auto out = emp::NewPtr<std::remove_reference_t<decay_T>>();
        return (T) *out;
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

  /// A symbol for an internally maintained variable.
  class Symbol_Var : public Symbol {
  private:
    emp::Datum value;

    using scope_ptr_t = emp::Ptr<Symbol_Scope>;
  public:
    Symbol_Var(const std::string & _n, double _v,              const std::string & _d="", scope_ptr_t _s=nullptr)
      : Symbol(_n, _d, _s), value(_v) {}
    Symbol_Var(const std::string & _n, const std::string & _v, const std::string & _d="", scope_ptr_t _s=nullptr)
      : Symbol(_n, _d, _s), value(_v) {}
    Symbol_Var(const std::string & _n, const emp::Datum & _v,  const std::string & _d="", scope_ptr_t _s=nullptr)
      : Symbol(_n, _d, _s), value(_v) {}
    Symbol_Var(const std::string & _n, const Symbol_Var & _v,  const std::string & _d="", scope_ptr_t _s=nullptr)
      : Symbol(_n, _d, _s), value(_v.value) {}

    Symbol_Var(const Symbol_Var &) = default;
    Symbol_Var(double _val)              : Symbol("__Auto__", "", nullptr), value(_val) {}
    Symbol_Var(const std::string & _val) : Symbol("__Auto__", "", nullptr), value(_val) {}
    Symbol_Var(const emp::Datum & _val)  : Symbol("__Auto__", "", nullptr), value(_val) {}

    std::string GetTypename() const override { return "Var"; }

    symbol_ptr_t Clone() const override { return emp::NewPtr<Symbol_Var>(*this); }

    double AsDouble() const override { return value.AsDouble(); }
    std::string AsString() const override { return value.AsString(); }
    void Print(std::ostream & os) const override {
      if (value.IsDouble()) os << value.NativeDouble();
      else os << value.NativeString();
    }
    Symbol & SetValue(double in) override { value = in; return *this; }
    Symbol & SetString(const std::string & in) override { value = in; return *this; }

    bool HasValue() const override { return true; }

    bool IsNumeric() const override { return value.IsDouble(); }
    bool IsString() const override { return value.IsString(); }
    bool IsLocal() const override { return true; }

    bool CopyValue(const Symbol & in) override {
      if (in.IsNumeric()) SetValue(in.AsDouble());
      else SetString(in.AsString());
      return true;
    }
  };


  class Symbol_Special : public Symbol {
  public:
    enum Type { CONTINUE, BREAK, UNKNOWN };

  private:
    using this_t = Symbol_Special;
    Type type;

    static std::string ToString(Type in_type) {
      switch (in_type) {
        case CONTINUE: return "CONTINUE";
        case BREAK: return "BREAK";
        default: return "UNKNOWN";
      }
    }
    std::string ToString() const { return ToString(type); }

  public:
    Symbol_Special(Type in_type) : Symbol("__Special", ToString(in_type), nullptr), type(in_type) {}
    std::string GetTypename() const override { return emp::to_string("[[Special::", ToString(), "]]"); }

    symbol_ptr_t Clone() const override { return emp::NewPtr<this_t>(*this); }

    bool IsContinue() const override { return type == CONTINUE; }  ///< Is symbol a "continue" signal?
    bool IsBreak() const override { return type == BREAK; }  ///< Is symbol a "break" signal?
  };

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

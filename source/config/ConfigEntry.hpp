/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2021.
 *
 *  @file  ConfigEntry.hpp
 *  @brief Manages a single configuration entry (e.g., variables + base for scopes and functions).
 *  @note Status: ALPHA
 * 
 * 
 *  Development Notes:
 *  - Currently we are not using Format; this would be useful if we want to type-check inputs more
 *    carefully.
 *  - When a ConfigEntry is used for a temporary value, it doesn't acutally need name or desc;
 *    we can probably remove these pretty easily to save on memory if needed.
 */

#ifndef MABE_CONFIG_ENTRY_H
#define MABE_CONFIG_ENTRY_H

#include <type_traits>

#include "emp/base/assert.hpp"
#include "emp/base/Ptr.hpp"
#include "emp/base/vector.hpp"
#include "emp/math/Range.hpp"
#include "emp/meta/TypeID.hpp"
#include "emp/tools/string_utils.hpp"
#include "emp/tools/value_utils.hpp"

namespace mabe {

  class ConfigScope;

  class ConfigEntry {
  protected:
    std::string name;             ///< Unique name for this entry; empty name implied temporary.
    std::string desc;             ///< Description to put in comments for this entry.
    emp::Ptr<ConfigScope> scope;  ///< Which scope was this variable defined in?

    bool is_temporary = false;    ///< Is this ConfigEntry temporary and should be deleted?
    bool is_builtin = false;      ///< Built-in entries should not be written to config files.
  
    enum class Format { NONE=0, SCOPE,
                        BOOL, INT, UNSIGNED, DOUBLE,                                    // Values
                        STRING, FILENAME, PATH, URL, ALPHABETIC, ALPHANUMERIC, NUMERIC  // Strings
                      };
    Format format = Format::NONE;

    // If we know the constraints on this parameter we can perform better error checking.
    emp::Range<double> range;  ///< Min and max values allowed for this config entry (if numerical).
    bool integer_only=false;   ///< Should we only allow integer values?

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
    ConfigEntry(const std::string & _name,
                const std::string & _desc,
                emp::Ptr<ConfigScope> _scope)
      : name(_name), desc(_desc), scope(_scope) { }
    ConfigEntry(const ConfigEntry &) = default;
    virtual ~ConfigEntry() { }

    const std::string & GetName() const noexcept { return name; }
    const std::string & GetDesc() const noexcept { return desc; }
    emp::Ptr<ConfigScope> GetScope() { return scope; }
    bool IsTemporary() const noexcept { return is_temporary; }
    bool IsBuiltIn() const noexcept { return is_builtin; }
    Format GetFormat() const noexcept { return format; }

    virtual std::string GetTypename() const { return "Unknown"; }

    virtual bool IsNumeric() const { return false; }   ///< Is entry any kind of number?
    virtual bool IsBool() const { return false; }      ///< Is entry a Boolean value?
    virtual bool IsInt() const { return false; }       ///< Is entry a integer value?
    virtual bool IsDouble() const { return false; }    ///< Is entry a floting point value?
    virtual bool IsString() const { return false; }    ///< Is entry a string?

    virtual bool IsLocal() const { return false; }     ///< Was entry defined in config file?
    virtual bool IsFunction() const { return false; }  ///< Is entry a function?
    virtual bool IsScope() const { return false; }     ///< Is entry a full scope?
    virtual bool IsError() const { return false; }     ///< Does entry flag an error?

    virtual bool HasNumericReturn() const { return false; } ///< Is entry a function that returns a number?
    virtual bool HasStringReturn() const { return false; }  ///< Is entry a function that returns a string?

    ConfigEntry & SetName(const std::string & in) { name = in; return *this; }
    ConfigEntry & SetDesc(const std::string & in) { desc = in; return *this; }
    ConfigEntry & SetTemporary(bool in=true) { is_temporary = in; return *this; }
    ConfigEntry & SetBuiltIn(bool in=true) { is_builtin = in; return *this; }

    virtual double AsDouble() const { emp_assert(false); return 0.0; }
    virtual std::string AsString() const { emp_assert(false); return ""; }
    template <typename T>
    T As() const {
      if constexpr (std::is_same<T,double>()) return AsDouble();
      else return AsString();
    }

    virtual ConfigEntry & SetValue(double in) { (void) in; emp_assert(false, in); return *this; }
    virtual ConfigEntry & SetString(const std::string & in) { (void) in; emp_assert(false, in); return *this; }

    virtual emp::Ptr<ConfigScope> AsScopePtr() { return nullptr; }
    ConfigScope & AsScope() {
      emp_assert(AsScopePtr());
      return *(AsScopePtr());
    }

    /// A generic As() function that will call the appropriate converter.
    template <typename T>
    T As() {
      using base_T = std::remove_const_t<T>;
      if constexpr (std::is_same<base_T, emp::Ptr<ConfigEntry>>()) { return this; }
      else if constexpr (std::is_same<base_T, ConfigEntry &>()) { return *this; }
      else if constexpr (std::is_same<base_T, std::string>()) { return AsString(); }
      else if constexpr (std::is_same<base_T, ConfigScope&>()) { return AsScope(); }
      else if constexpr (std::is_arithmetic<base_T>()) { return (T) AsDouble(); }
      else {
        // Oh oh... we don't know this type...
        emp_error("Trying to convert a ConfigEntry to an unknown type: ",
                  emp::GetTypeID<T>().GetName());
        return base_T();
      }
    }

    ConfigEntry & SetMin(double min) { range.SetLower(min); return *this; }
    ConfigEntry & SetMax(double max) { range.SetLower(max); return *this; }

    // Try to copy another config entry into this one; return true if successful.
    virtual bool CopyValue(const ConfigEntry & ) { return false; }

    /// If this entry is a scope, we should be able to lookup other entries inside it.
    virtual emp::Ptr<ConfigEntry>
    LookupEntry(const std::string & in_name, bool /* scan_scopes */=true) {
      return (in_name == "") ? this : nullptr;
    }
    virtual emp::Ptr<const ConfigEntry>
    LookupEntry(const std::string & in_name, bool /* scan_scopes */=true) const {
      return (in_name == "") ? this : nullptr;
    }
    virtual bool Has(const std::string & in_name) const { return (bool) LookupEntry(in_name); }

    /// If this entry is a function, we should be able to call it.
    virtual emp::Ptr<ConfigEntry> Call( emp::vector<emp::Ptr<ConfigEntry>> args );

    /// Allocate a duplicate of this class.
    virtual emp::Ptr<ConfigEntry> Clone() const = 0;

    virtual const ConfigEntry & Write(std::ostream & os=std::cout, const std::string & prefix="",
                                      size_t comment_offset=32) const
    {
      // If this is a built-in entry, don't print it.
      if (IsBuiltIn()) return *this;

      // Setup this entry.
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
  };

  /// ConfigEntry can be linked directly to a real variable.
  template <typename T>
  class ConfigEntry_Linked : public ConfigEntry {
  private:
    T & var;
  public:
    using this_t = ConfigEntry_Linked<T>;

    template <typename... ARGS>
    ConfigEntry_Linked(const std::string & in_name, T & in_var, ARGS &&... args)
      : ConfigEntry(in_name, std::forward<ARGS>(args)...), var(in_var) { ; }
    ConfigEntry_Linked(const this_t &) = default;

    std::string GetTypename() const override {
      if constexpr (std::is_scalar_v<T>) return "Value";
      else return "Unknown";
    }

    emp::Ptr<ConfigEntry> Clone() const override { return emp::NewPtr<this_t>(*this); }

    double AsDouble() const override { return (double) var; }
    std::string AsString() const override { return emp::to_string(var); }
    ConfigEntry & SetValue(double in) override { var = (T) in; return *this; }
    ConfigEntry & SetString(const std::string & in) override {
      var = emp::from_string<T>(in);
      return *this;
    }

    bool IsNumeric() const override { return std::is_scalar_v<T>; }
    bool IsBool() const override { return std::is_same<bool, T>(); }
    bool IsInt() const override { return std::is_same<int, T>(); }
    bool IsDouble() const override { return std::is_same<double, T>(); }

    bool CopyValue(const ConfigEntry & in) override { var = in.AsDouble(); return true; }
  };

  /// Specialization for ConfigEntry linked to a string variable.
  template <>
  class ConfigEntry_Linked<std::string> : public ConfigEntry {
  private:
    std::string & var;
  public:
    using this_t = ConfigEntry_Linked<std::string>;

    template <typename... ARGS>
    ConfigEntry_Linked(const std::string & in_name, std::string & in_var, ARGS &&... args)
      : ConfigEntry(in_name, std::forward<ARGS>(args)...), var(in_var) { ; }
    ConfigEntry_Linked(const this_t &) = default;

    std::string GetTypename() const override { return "String"; }

    emp::Ptr<ConfigEntry> Clone() const override { return emp::NewPtr<this_t>(*this); }

    double AsDouble() const override { return emp::from_string<double>(var); }
    std::string AsString() const override { return var; }
    ConfigEntry & SetValue(double in) override { var = emp::to_string(in); return *this; }
    ConfigEntry & SetString(const std::string & in) override { var = in; return *this; }

    bool IsString() const override { return true; }

    bool CopyValue(const ConfigEntry & in) override { var = in.AsString(); return true; }
  };

  /// ConfigEntry can be linked to a pair of (Get and Set) functions.
  template <typename T>
  class ConfigEntry_Functions : public ConfigEntry {
  private:
    std::function<T()> get_fun;
    std::function<void(const T &)> set_fun;
  public:
    using this_t = ConfigEntry_Functions<T>;

    template <typename... ARGS>
    ConfigEntry_Functions(const std::string & in_name,
                       std::function<T()> in_get,
                       std::function<void(const T &)> in_set,
                       ARGS &&... args)
      : ConfigEntry(in_name, std::forward<ARGS>(args)...)
      , get_fun(in_get)
      , set_fun(in_set)
    { ; }
    ConfigEntry_Functions(const this_t &) = default;

    std::string GetTypename() const override { return "[[Function]]"; }

    emp::Ptr<ConfigEntry> Clone() const override { return emp::NewPtr<this_t>(*this); }

    double AsDouble() const override { return emp::ToDouble( get_fun() ); }
    std::string AsString() const override { return emp::to_string( get_fun() ); }
    ConfigEntry & SetValue(double in) override { set_fun(emp::FromDouble<T>(in)); return *this; }
    ConfigEntry & SetString(const std::string & in) override {
      set_fun( emp::from_string<T>(in) );
      return *this;
    }

    bool IsNumeric() const override { return std::is_scalar_v<T>; }
    bool IsBool() const override { return std::is_same<bool, T>(); }
    bool IsInt() const override { return std::is_same<int, T>(); }
    bool IsDouble() const override { return std::is_same<double, T>(); }
    bool IsString() const override { return std::is_same<std::string, T>(); }

    bool CopyValue(const ConfigEntry & in) override { SetString( in.AsString() ); return true; }
  };


  /// A generic version of a config entry for a maintained variable.
  template <typename T>
  class ConfigEntry_Var : public ConfigEntry {
  private:
    T value = 0;
  public:
    using this_t = ConfigEntry_Var<T>;

    template <typename... ARGS>
    ConfigEntry_Var(const std::string & in_name, T default_val,
                    const std::string & in_desc="", emp::Ptr<ConfigScope> in_scope=nullptr)
      : ConfigEntry(in_name, in_desc, in_scope), value(default_val) { ; }
    ConfigEntry_Var(const ConfigEntry_Var<T> &) = default;

    std::string GetTypename() const override {
      if constexpr (std::is_scalar_v<T>) return "Value";
      else return "Unknown";
    }

    emp::Ptr<ConfigEntry> Clone() const override { return emp::NewPtr<this_t>(*this); }

    double AsDouble() const override { return (double) value; }
    std::string AsString() const override { return emp::to_string(value); }
    ConfigEntry & SetValue(double in) override { value = (T) in; return *this; }
    ConfigEntry & SetString(const std::string & in) override {
      value = emp::from_string<T>(in);
      return *this;
    }

    bool IsNumeric() const override { return std::is_scalar_v<T>; }
    bool IsBool() const override { return std::is_same<bool, T>(); }
    bool IsInt() const override { return std::is_same<int, T>(); }
    bool IsDouble() const override { return std::is_same<double, T>(); }

    bool IsLocal() const override { return true; }

    bool CopyValue(const ConfigEntry & in) override { SetValue(in.AsDouble()); return true; }
  };
  using ConfigEntry_DoubleVar = ConfigEntry_Var<double>;

  /// ConfigEntry as a temporary variable of type STRING.
  template<>
  class ConfigEntry_Var<std::string> : public ConfigEntry {
  private:
    std::string value;
  public:
    using this_t = ConfigEntry_Var<std::string>;

    template <typename... ARGS>
    ConfigEntry_Var(const std::string & in_name, const std::string & in_val, ARGS &&... args)
      : ConfigEntry(in_name, std::forward<ARGS>(args)...), value(in_val) { ; }
    ConfigEntry_Var(const ConfigEntry_Var<std::string> &) = default;

    std::string GetTypename() const override { return "String"; }

    emp::Ptr<ConfigEntry> Clone() const override { return emp::NewPtr<this_t>(*this); }

    double AsDouble() const override { return emp::from_string<double>(value); }
    std::string AsString() const override { return value; }
    ConfigEntry & SetValue(double in) override { value = emp::to_string(in); return *this; }
    ConfigEntry & SetString(const std::string & in) override { value = in; return *this; }

    bool IsString() const override { return true; }
    bool IsLocal() const override { return true; }

    bool CopyValue(const ConfigEntry & in) override { value = in.AsString(); return true; }
  };
  using ConfigEntry_StringVar = ConfigEntry_Var<std::string>;

  /// A ConfigEntry to transmit an error.  The description provides the error and the IsError() flag
  /// is set to true.
  class ConfigEntry_Error : public ConfigEntry {
  private:
    using this_t = ConfigEntry_Error;
  public:
    template <typename... ARGS>
    ConfigEntry_Error(ARGS &&... args)
      : ConfigEntry("__Error", emp::to_string(args...), nullptr) { is_temporary = true; }

    std::string GetTypename() const override { return "[[Error]]"; }

    bool IsError() const override { return true; }

    emp::Ptr<ConfigEntry> Clone() const override { return emp::NewPtr<this_t>(*this); }
  };


  ////////////////////////////////////////////////////
  //  Function definitions...

  emp::Ptr<ConfigEntry> ConfigEntry::Call( emp::vector<emp::Ptr<ConfigEntry>> /* args */ ) {
    return emp::NewPtr<ConfigEntry_Error>("Cannot call a function on non-function '", name, "'.");
  }

}

#endif

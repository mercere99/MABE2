/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2021.
 *
 *  @file  ConfigEntry.hpp
 *  @brief Manages a single configuration entry (e.g., variables + base for scopes and functions).
 *  @note Status: BETA
 * 
 *  The symbol table for the configuration language is managed as a collection of
 *  configuration entries.  These include specializations for ConfigEntry_Linked (for linked
 *  variables), ConfigEntry_Function and ConfigEntry_Scope, all defined in their own files
 *  and derived from ConfigEntry.
 * 
 *  Development Notes:
 *  - Currently we are not using Format; this would be useful if we want to type-check inputs more
 *    carefully.
 *  - When a ConfigEntry is used for a temporary value, it doesn't actually need name or desc;
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

  class ConfigEntry_Scope;

  class ConfigEntry {
  protected:
    std::string name;                  ///< Unique name for entry; empty name implies temporary.
    std::string desc;                  ///< Description to put in comments for this entry.
    emp::Ptr<ConfigEntry_Scope> scope; ///< Which scope was this variable defined in?

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
                emp::Ptr<ConfigEntry_Scope> _scope)
      : name(_name), desc(_desc), scope(_scope) { }
    ConfigEntry(const ConfigEntry &) = default;
    virtual ~ConfigEntry() { }

    const std::string & GetName() const noexcept { return name; }
    const std::string & GetDesc() const noexcept { return desc; }
    emp::Ptr<ConfigEntry_Scope> GetScope() { return scope; }
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

    virtual emp::Ptr<ConfigEntry_Scope> AsScopePtr() { return nullptr; }
    ConfigEntry_Scope & AsScope() {
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
      else if constexpr (std::is_same<base_T, ConfigEntry_Scope&>()) { return AsScope(); }
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

  /// A generic version of a config entry for an internally maintained variable.
  template <typename T>
  class ConfigEntry_Var : public ConfigEntry {
  private:
    T value = 0;
  public:
    using this_t = ConfigEntry_Var<T>;

    template <typename... ARGS>
    ConfigEntry_Var(const std::string & in_name,
                    T default_val,
                    const std::string & in_desc="",
                    emp::Ptr<ConfigEntry_Scope> in_scope=nullptr)
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

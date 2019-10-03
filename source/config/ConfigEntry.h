/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  ConfigEntry.h
 *  @brief Manages a single configuration entry.
 *  @note Status: ALPHA
 * 
 * 
 *  Development Notes:
 *  - When a ConfigEntry is used for a temporary value, it doesn't acutally need name, desc
 *    or default_val; we can probably remove these pretty easily to save on memory if needed.
 */

#ifndef MABE_CONFIG_ENTRY_H
#define MABE_CONFIG_ENTRY_H

#include <type_traits>

#include "base/assert.h"
#include "base/Ptr.h"
#include "base/vector.h"
#include "meta/TypeID.h"
#include "tools/Range.h"
#include "tools/string_utils.h"
#include "tools/value_utils.h"

namespace mabe {

  class ConfigScope;

  class ConfigEntry {
  protected:
    std::string name;             ///< Unique name for this entry; empty name implied temporary.
    std::string desc;             ///< Description to put in comments for this entry.
    std::string default_val;      ///< String representing value to use in generated config file.
    emp::Ptr<ConfigScope> scope;  ///< Which scope was this variable defined in?

    bool is_temporary = false;    ///< Is this ConfigEntry temporary and should be deleted?
  
    enum class Format { NONE=0, SCOPE,
                        BOOL, INT, UNSIGNED, DOUBLE,                                    // Values
                        STRING, FILENAME, PATH, URL, ALPHABETIC, ALPHANUMERIC, NUMERIC  // Strings
                      };
    Format format = Format::NONE;

    // If we know the constraints on this parameter we can perform better error checking.
    emp::Range<double> range;  ///< Min and max values allowed for this config entry (if numerical).
    bool integer_only=false;   ///< Should we only allow integer values?

  public:
    ConfigEntry(const std::string & _name,
                const std::string & _desc,
                emp::Ptr<ConfigScope> _scope)
      : name(_name), desc(_desc), scope(_scope) { }
    ConfigEntry(const ConfigEntry &) = default;
    virtual ~ConfigEntry() { }

    const std::string & GetName() const noexcept { return name; }
    const std::string & GetDesc() const noexcept { return desc; }
    const std::string & GetDefaultVal() const noexcept { return default_val; }
    emp::Ptr<ConfigScope> GetScope() { return scope; }
    bool IsTemporary() const noexcept { return is_temporary; }
    Format GetFormat() const noexcept { return format; }

    virtual bool IsNumeric() const { return false; }
    virtual bool IsBool() const { return false; }
    virtual bool IsInt() const { return false; }
    virtual bool IsDouble() const { return false; }
    virtual bool IsString() const { return false; }

    virtual bool IsScope() const { return false; }
    virtual bool IsError() const { return false; }

    /// Set the default string for this entry.
    ConfigEntry & SetName(const std::string & in) { name = in; return *this; }
    ConfigEntry & SetDesc(const std::string & in) { desc = in; return *this; }
    ConfigEntry & SetDefault(const std::string & in) { default_val = in; return *this; }
    ConfigEntry & SetTemporary(bool in=true) { is_temporary = in; return *this; }

    virtual double AsDouble() const { emp_assert(false); return 0.0; }
    virtual std::string AsString() const { emp_assert(false); return ""; }
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
      if constexpr (std::is_same<T, double>) { return AsDouble(); }
      else if constexpr (std::is_same<T, std::string>) { return AsString(); }
      else if constexpr (std::is_same<T, ConfigScope&>) { return AsScope(); }
      else if constexpr (std::is_same<T, int>) { return (int) AsDouble(); }
      else if constexpr (std::is_same<T, bool>) { return (bool) AsDouble(); }
      else {
        // Oh oh... we don't know this type...
        static_assert(false, "Trying to convert a ConfigEntry to an unknown type!");
        return T();
      }
    }

    ConfigEntry & SetMin(double min) { range.SetLower(min); return *this; }
    ConfigEntry & SetMax(double max) { range.SetLower(max); return *this; }

    // Try to copy another config entry into this one; return true if successfule.
    virtual bool CopyValue(const ConfigEntry & in) { return false; }

    /// Shift the current value to be the new default value.
    virtual void UpdateDefault() { default_val = ""; }

    virtual emp::Ptr<ConfigEntry> LookupEntry(std::string in_name, bool scan_scopes=true) {
      return (in_name == "") ? this : nullptr;
    }
    virtual emp::Ptr<const ConfigEntry> LookupEntry(std::string in_name, bool scan_scopes=true) const {
      return (in_name == "") ? this : nullptr;
    }
    virtual bool Has(std::string in_name) const { return (bool) LookupEntry(in_name); }

    /// Allocate a duplicate of this class.
    virtual emp::Ptr<ConfigEntry> Clone() const = 0;

    virtual ConfigEntry & Write(std::ostream & os=std::cout, const std::string & prefix="",
                                size_t comment_offset=40) {
      // Print this entry.
      os << prefix << name << " = ";

      // Keep track of how many characters we've printed.
      size_t char_count = prefix.size() + name.size() + 3;

      // If a default value has been provided, print it.  Otherwise print the current value.
      // if (default_val.size()) {
      //   os << default_val;
      //   char_count += default_val.size();
      // }
      // else {
      //   os << AsString();
      //   char_count += AsString().size();
      // }

      // Print the current value of this variable; if it's a string make sure to turn it to a literal.
      std::string out_str = IsString() ? emp::to_literal(AsString()) : AsString();
      os << out_str;
      char_count += out_str.size();

      // End each line with a semi-colon.
      os << ";";
      char_count++;

      // Print a comment if we have one.
      if (desc.size()) {
        while (char_count++ < comment_offset) os << " ";
        os << "// " << desc;
      }
      os << std::endl;

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

  /// Specializatin for ConfigEntry linked to a string variable.
  template <> class ConfigEntry_Linked<std::string> : public ConfigEntry {
  private:
    std::string & var;
  public:
    using this_t = ConfigEntry_Linked<std::string>;

    template <typename... ARGS>
    ConfigEntry_Linked(const std::string & in_name, std::string & in_var, ARGS &&... args)
      : ConfigEntry(in_name, std::forward<ARGS>(args)...), var(in_var) { ; }
    ConfigEntry_Linked(const this_t &) = default;

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


  /// ConfigEntry as a temporary variable of type DOUBLE.
  class ConfigEntry_DoubleVar : public ConfigEntry {
  private:
    double value = 0.0;
  public:
    using this_t = ConfigEntry_DoubleVar;

    template <typename... ARGS>
    ConfigEntry_DoubleVar(const std::string & in_name, double default_val, ARGS &&... args)
      : ConfigEntry(in_name, std::forward<ARGS>(args)...), value(default_val) { ; }
    ConfigEntry_DoubleVar(const ConfigEntry_DoubleVar &) = default;

    emp::Ptr<ConfigEntry> Clone() const override { return emp::NewPtr<this_t>(*this); }

    double AsDouble() const override { return value; }
    std::string AsString() const override { return emp::to_string(value); }
    ConfigEntry & SetValue(double in) override { value = in; return *this; }
    ConfigEntry & SetString(const std::string & in) override {
      value = emp::from_string<double>(in);
      return *this;
    }

    bool IsNumeric() const override { return true; }
    bool IsDouble() const override { return true; }

    bool CopyValue(const ConfigEntry & in) override { value = in.AsDouble(); return true; }
  };

  /// ConfigEntry as a temporary variable of type STRING.
  class ConfigEntry_StringVar : public ConfigEntry {
  private:
    std::string value;
  public:
    using this_t = ConfigEntry_StringVar;

    template <typename... ARGS>
    ConfigEntry_StringVar(const std::string & in_name, const std::string & in_val, ARGS &&... args)
      : ConfigEntry(in_name, std::forward<ARGS>(args)...), value(in_val) { ; }
    ConfigEntry_StringVar(const ConfigEntry_StringVar &) = default;

    emp::Ptr<ConfigEntry> Clone() const override { return emp::NewPtr<this_t>(*this); }

    double AsDouble() const override { return emp::from_string<double>(value); }
    std::string AsString() const override { return value; }
    ConfigEntry & SetValue(double in) override { value = emp::to_string(in); return *this; }
    ConfigEntry & SetString(const std::string & in) override { value = in; return *this; }

    bool IsString() const override { return true; }

    bool CopyValue(const ConfigEntry & in) override { value = in.AsString(); return true; }
  };


  /// A ConfigEntry to transmit an error.  The description provides the error and the IsError() flag
  /// is set to true.
  class ConfigEntry_Error : public ConfigEntry {
  public:
    ConfigEntry_Error(const std::string & msg)
      : ConfigEntry("Error", msg, nullptr) { ; }

    bool IsError() const override { return true; }
  };
}

#endif

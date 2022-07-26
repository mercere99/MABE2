/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021.
 *
 *  @file  ErrorManager.hpp
 *  @brief Handles errors during a run.
 *
 *  An ErrorManager tracks errors as they occur and ensures that they are properly reported when
 *  appropriate.
 */

#ifndef MABE_ERROR_MANAGER_HPP
#define MABE_ERROR_MANAGER_HPP

#include <string>

#include "emp/base/assert.hpp"
#include "emp/base/error.hpp"
#include "emp/base/Ptr.hpp"
#include "emp/base/vector.hpp"
#include "emp/tools/string_utils.hpp"

namespace mabe {

  class ErrorManager {
  private:
    emp::vector<std::string> errors;                           ///< Log of errors
    emp::vector<std::string> warnings;                         ///< Log of warnings
    std::function<void(const std::string &)> error_callback;   ///< Function to call on errors
    std::function<void(const std::string &)> warning_callback; ///< Function to call on warnings
    bool active = false;                                       ///< Print immediately or hold?
    size_t next_error = 0;                                     ///< ID of next error to notify
    size_t next_warning = 0;                                   ///< ID of next warning to notify

  public:
    ErrorManager(std::function<void(const std::string &)> _error_cb,
                 std::function<void(const std::string &)> _warning_cb)
    : error_callback(_error_cb)
    , warning_callback(_warning_cb)
    { }

    const emp::vector<std::string> & GetErrors() const { return errors; }
    const emp::vector<std::string> & GetWarnings() const { return warnings; }
    size_t GetNumErrors() const { return errors.size(); }
    size_t GetNumWarnings() const { return warnings.size(); }
    bool IsActive() const { return active; }

    void SetErrorCallback(std::function<void(const std::string &)> in_cb) {
      error_callback = in_cb;
    }

    void SetWarningCallback(std::function<void(const std::string &)> in_cb) {
      warning_callback = in_cb;
    }

    /// Notify about a new error; details can be any types convertable to strings.
    template <typename... Ts>
    void AddError(Ts &&... args) {
      // If we are in debug mode, trigger the error immediately.
      #ifndef NDEBUG
        emp_error(args...);
      #endif
      // Otherwise store it to deal with it when requested.
      errors.push_back( emp::to_string( std::forward<Ts>(args)... ));

      // If active, deal with it immediately.
      if (active) {
        error_callback(errors.back());
        next_error = errors.size();
      }
    }

    /// Notify about a new warning; details can be any types convertable to strings.
    template <typename... Ts>
    void AddWarning(Ts &&... args) {
      // If we are in debug mode, trigger the warning immediately.
      //emp_warning(args...);

      // Otherwise store it to deal with it when requested.
      warnings.push_back( emp::to_string( std::forward<Ts>(args)... ));

      // If active, deal with it immediately.
      if (active) {
        warning_callback(warnings.back());
        next_warning = warnings.size();
      }
    }

    /// Output held errors and warnings, but do not change active status.
    void Flush() {
      while (next_error   < errors.size())   error_callback(errors[next_error++]);
      while (next_warning < warnings.size()) warning_callback(warnings[next_warning++]);
    }

    /// Unlock the ErrorManager and output all held errors.
    void Activate() {
      active = true;
      Flush();
    }

    /// Lock the ErrorManager; it will still collect errors but not pass them on.
    void Deactivate() {
      active = false;
    }

    /// Remove all errors (don't change active status)
    void Clear() {
      errors.resize(0);
      next_error = 0;
      warnings.resize(0);
      next_warning = 0;
    }

  };

}

#endif

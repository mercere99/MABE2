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
    emp::vector<std::string> errors;                   ///< Log of any errors that have occured.
    std::function<void(const std::string &)> trigger;  ///< Function to call on errors.
    bool active = false;                               ///< Print immediately or hold messages?
    size_t next = 0;                                   ///< ID of next message to notify about.

  public:
    ErrorManager(std::function<void(const std::string &)> in_trigger) : trigger(in_trigger) { }

    const emp::vector<std::string> & GetErrors() const { return errors; }
    size_t GetNumErrors() const { return errors.size(); }
    bool IsActive() const { return active; }

    void SetTrigger(std::function<void(const std::string &)> in_trigger) { trigger = in_trigger; }

     // -- Error Handling --
    template <typename... Ts>
    void AddError(Ts &&... args) {
      // If we are in debug mode, trigger the error immediately.
      emp_error(args...);

      // Otherwise store it to deal with it when requested.
      errors.push_back( emp::to_string( std::forward<Ts>(args)... ));

      // If active, deal with it immediately.
      if (active) {
        trigger(errors.back());
        next = errors.size();
      }
    }

    // Output held errors, but do not change active status.
    void Flush() {
      while (next < errors.size()) {
        trigger(errors[next]);
        next++;
      }
    }

    // Unlock the ErrorManager and output all held errors.
    void Activate() {
      active = true;
      Flush();
    }

    // Lock the ErrorManager; it will still collect errors but not pass them on.
    void Deactivate() {
      active = false;
    }

    // Remove all errors.
    void Clear() {
      errors.resize(0);
      next = 0;
    }

  };

}

#endif
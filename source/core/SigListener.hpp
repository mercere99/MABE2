/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021.
 *
 *  @file  SigListener.hpp
 *  @brief Tool to trigger a specified member function on other classes when triggered.
 *
 *  A SigListener tracks which Modules respond to a specific signal.  They maintain pointers
 *  to modules and call them when requested.  The base class manages common functionality.
 */

#ifndef MABE_SIGNAL_LISTENER_H
#define MABE_SIGNAL_LISTENER_H

#include "emp/base/array.hpp"
#include "emp/base/Ptr.hpp"
#include "emp/base/vector.hpp"

#include "OrgIterator.hpp"

namespace mabe {

  template <typename MODULE_T>
  struct SigListenerBase : public emp::vector<emp::Ptr<MODULE_T>> {
    using mod_ptr_t = emp::Ptr<MODULE_T>;
    using id_t = typename MODULE_T::SignalID;

    std::string name;          ///< Name of this signal type.
    id_t id;   ///< ID of this signal
    mod_ptr_t cur_mod;         ///< Which module is currently running?

    SigListenerBase(const std::string & _name="",
                    id_t _id=MODULE_T::SIG_UNKNOWN)
      : name(_name), id(_id) {;}
    SigListenerBase(const SigListenerBase &) = default;
    SigListenerBase(SigListenerBase &&) = default;
    SigListenerBase & operator=(const SigListenerBase &) = default;
    SigListenerBase & operator=(SigListenerBase &&) = default;
  };

  /// Each set of modules to be called when a specific signal is triggered should be identified
  /// in a SigListener object that has full type information.
  template <typename MODULE_T, typename RETURN, typename... ARGS>
  struct SigListener : public SigListenerBase<MODULE_T> {
    using base_t = SigListenerBase<MODULE_T>;
    using mod_ptr_t = emp::Ptr<MODULE_T>;

    /// Define ModMemFun as the proper signal call type.
    typedef RETURN (MODULE_T::*ModMemFun)(ARGS...);

    /// Store the member-function call that this SigListener should handle.
    ModMemFun fun;

    /// A SigListener constructor takes both the member function that its supposed to call
    /// and a master list of module vectors that it should put itself it.
    SigListener(const std::string & _name,
                typename MODULE_T::SignalID _id,
                ModMemFun _fun,
                emp::array< emp::Ptr<base_t>, (size_t) MODULE_T::NUM_SIGNALS> & signal_ptrs)
        : base_t(_name, _id), fun(_fun)
    {
      signal_ptrs[base_t::id] = this;
    }

    template <typename... ARGS2>
    void Trigger(ARGS2 &&... args) {
      for (mod_ptr_t mod_ptr : *this) {
        base_t::cur_mod = mod_ptr;
        emp_assert(!mod_ptr.IsNull());
        (mod_ptr.Raw()->*fun)( std::forward<ARGS2>(args)... );
      }
      base_t::cur_mod = nullptr;
    }

    template <typename... ARGS2>
    OrgPosition FindPosition(ARGS2 &&... args) {
      OrgPosition result;
      for (mod_ptr_t mod_ptr : *this) {
        result = (mod_ptr.Raw()->*fun)(std::forward<ARGS2>(args)...);
        if (result.IsValid()) break;
      }
      return result;
    }
  };

}

#endif

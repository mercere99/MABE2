/**
 *  @note This file is part of Emplode, currently within https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2021.
 *
 *  @file  EventManager.hpp
 *  @brief Manages events for configurations.
 *  @note Status: BETA
 * 
 *  Manages different sets of events that can be triggered.
 * 
 *  An EVENT is a set of actions to be executed when an associated signal is triggered.
 *
 *  An ACTION is an AST tree to be executed, possibly with parameters.
 * 
 *  A SIGNAL has a string (identifier) and is associated with a set of zero or more
 *  actions to take when triggered.
 * 
 *  A event TRIGGER occurs to signify an event in a run (such as a new update or a
 *  collision); it specifies the signal that it is triggering and a set of associated
 *  data (to provide args to the actions)
 * 
 */

#ifndef EMPLODE_EVENT_MANAGER_HPP
#define EMPLODE_EVENT_MANAGER_HPP

#include <string>

#include "emp/base/map.hpp"
#include "emp/base/Ptr.hpp"

#include "AST.hpp"

namespace emplode {

  class EventManager {
  private:
    using symbol_ptr_t = emp::Ptr<Symbol>;
    using symbol_vec_t = emp::vector<symbol_ptr_t>;
    using node_ptr_t = emp::Ptr<ASTNode>;
    using node_vec_t = emp::vector< node_ptr_t >;
    struct Event;

    std::unordered_map<std::string, emp::Ptr<Event>> event_map;
    SymbolTableBase & symbol_table;

    struct Action {
      std::string signal_name;
      node_vec_t params;
      node_ptr_t action;
      size_t def_line;

      Action(const std::string & _signal, node_vec_t _params, node_ptr_t _action, size_t _line)
      : signal_name(_signal), params(_params), action(_action), def_line(_line) { }
      ~Action() {
        for (auto x : params) x.Delete();
        action.Delete();
      }

      void Trigger(const symbol_vec_t & args) {
        if (args.size() < params.size()) {
          std::cerr << "ERROR: Trigger for signal '" << signal_name
                    << "' (defined on " << def_line << ") called with " << args.size()
                    << " arguments, but " << params.size() << " parameters need values."
                    << std::endl;
          exit(1);
        }

        // Setup all of the parameters.
        for (size_t param_id = 0; param_id < params.size(); ++param_id) {
          symbol_ptr_t param_sym = params[param_id]->Process();

          if (param_sym->IsTemporary()) {
            std::cerr << "ERROR (line " << def_line << "): parameter " << param_id
                      << " is invalid; not a proper lvalue." << std::endl;
            exit(1);
          }

          bool success = param_sym->CopyValue(*args[param_id]);
          if (!success) {
            std::cerr << "ERROR: setting action parameter '"
                      << param_sym->GetName() << "' failed" << std::endl;
            exit(1);
          }
        }

        // Once all of the parameter values are in place, run the action!
        symbol_ptr_t result = action->Process();
        if (result && result->IsTemporary()) result.Delete();
      }

      void Write(std::ostream & os) const {
        os << "@" << signal_name << "(";
        // @CAO: Write out parameters...      
        os << ") ";
        action->Write(os);
        os << ";\n";
      }
    };

    struct Event {
      std::string signal_name;
      size_t num_params;
      emp::vector<emp::Ptr<Action>> actions;

      Event(const std::string & _name, size_t _params)
        : signal_name(_name), num_params(_params) { }
      ~Event() { for (auto ptr : actions) ptr.Delete(); }

      void Trigger(symbol_vec_t args) {
        for (emp::Ptr<Action> action : actions) {
          action->Trigger(args);
        }
      }

      void Write(std::ostream & os) const {
        for (emp::Ptr<Action> action : actions) {
          action->Write(os);
        }
      }
      
    };

  public:
    EventManager(SymbolTableBase & _s_table) : symbol_table(_s_table) { ; }
    ~EventManager() {
      // Must delete all events in the queue.
      for (auto [name, ptr] : event_map) {
        ptr.Delete();
      }
    }

    bool HasSignal(const std::string & signal_name) const {
      return emp::Has(event_map, signal_name);
    }

    bool AddSignal(const std::string & signal_name, size_t num_params) {
      // @CAO Needs to become a user-level error?
      emp_assert(!emp::Has(event_map, signal_name), "Signal reused!", signal_name);

      event_map[signal_name] = emp::NewPtr<Event>(signal_name, num_params);

      return true;
    }

    /// Add a new event action
    bool AddAction(
      const std::string & signal_name,  ///< Name of signal to trigger using
      node_vec_t params,                ///< Parameters to set before taking action
      node_ptr_t action,                ///< Abstract syntax tree to run when triggered
      size_t def_line                   ///< What file line was this defined on?
    ) {
      // @CAO Needs to become a user-level error?
      emp_assert(emp::Has(event_map, signal_name), "Unknown signal used!", signal_name);

      auto action_ptr = emp::NewPtr<Action>(signal_name, params, action, def_line);
      event_map[signal_name]->actions.push_back(action_ptr);

      return true;
    }

    template <typename... ARG_TS>
    bool Trigger(const std::string & signal_name, ARG_TS... args) {
      // @CAO Make into user-level error.
      emp_assert(emp::Has(event_map, signal_name), "Unknown signal being triggered!", signal_name);

      const std::string location = emp::to_string("trigger of ", signal_name);
      symbol_vec_t symbol_args = { symbol_table.ValueToSymbol(args, location)... };
      event_map[signal_name]->Trigger(symbol_args);

      // Now that all of the actions have been run, clean up the symbol_args.
      for (auto symbol_ptr : symbol_args) {
        if (symbol_ptr->IsTemporary()) symbol_ptr.Delete();
      }

      return true;
    }

    /// Print all of the events being tracked here.
    void Write(std::ostream & os) const {
      for (auto [name, ptr] : event_map) {
        ptr->Write(os);
      }
    }
  };


}

#endif

/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  Organism.h
 *  @brief A base class for all organisms in MABE.
 *  @note Status: ALPHA
 *
 *  All organism types in MABE must override the mabe::Organism class and provide a valid
 *  OrganismManager (see OrganismManager.h for more information).
 *
 *  ...
 *
 *  MABE environment modules may also require organisms that can interact with the environment.
 *  Interactions occur with ACTIONS that the organisms may take and EVENTS that the organisms
 *  may respond to.  In many cases an action will also trigger an event as a response (for
 *  example an active sensor action.)
 *
 *  To build an Organism that handles actions and events, you must override the member functions
 *  AddAction(), AddEvent(), and TriggerEvent().  Note that each of these are associated with a
 *  data payload type; the Organism_data_Ts type pack below lists all legal data types.  While
 *  an organism is not required to deal with all of them (or any of them!), it may not be usable
 *  with some module types if they require specific types of data communication.  Note that any
 *  incompatabilities should be identified on startup and explained to the user.
 *
 *  bool AddAction(const std::string & action_name,
 *                 std::function<void(Organism &, data_type)> trigger_function,
 *                 int response_event_id)
 *  This function is called by a module to provide an organism with callable functions that will
 *  allow it to take specified actions (e.g., move, uptake resources, actively sense, etc.)
 *   'action_name' provides a unique name for this action that an organism can usually ignore.
 *   'trigger_function' is the functor that should be called for an organsim to take this action.
 *     Note that the organism must provide a reference to itself, followed by any required data.
 *   'response_event_id' is a unique id to indicate which event provides a response to this action.
 *     (if response_event_id is -1, no direct response should be expected)
 *  The return value should be 'true' if this action was successfully incorporated into the
 *  organism, or 'false' if it failed.  A 'false' on a required action will trigger a warning or
 *  error during run initialization.
 *  
 *  bool AddEvent(const std::string & event_name,
 *                int event_id,
 *                data_type)
 *  This function is called by a module at the beginning of a run to indicate a type of event
 *  that may occur.  It provides a unique name for the event, a unique event_id (which will match
 *  the relevant ID in a call to AddAction if this event is a response), and a dummy object of
 *  the proper data type (to facilitate function overloading without using templates.)
 *  The return value should be 'true' if this event was successfully incorporated into the
 *  organism, or 'false' if it failed.  A 'false' on a required event will trigger a warning or
 *  error during run initialization.
 *  
 *  void TrggerEvent(int event_id,
 *                   data_type data)
 *  This function is called during a run by a module on an organism.  The module indicated the
 *  ID of the event that has just occurred, along with the data associated with this specific
 *  event instance.  No return data is necessary.
 *  
 */

#ifndef MABE_ORGANISM_H
#define MABE_ORGANISM_H

#include "base/assert.h"
#include "data/VarMap.h"
#include "meta/TypeID.h"
#include "tools/BitVector.h"
#include "tools/string_utils.h"

#include "OrganismManagerBase.h"

namespace mabe {

    /// This TypePack contains the set of data types that can be passed between Organism and any
    /// Modules that define the external world.  Always use the most appropriate type BUT prefer
    /// types higher on the list when two types are otherwise equivilent.

    using Organism_data_Ts = emp::TypePack<
      double,                               // 64-bit floating-point value
      const emp::vector<double> &,          // Consecutive series of floating-point values
      const std::map<size_t,double> &,      // Non-consecutive floating-point values (sparse memory)
      const emp::BitVector &,               // Series of bit values
      const std::string &,                  // String value
      const emp::vector<std::string> &,     // Series of string values
      const std::map<std::string,double> &, // Set of strings associated with floating-point values

      // If all else fails, a chunk of memory can be passed using a combination of pointer and size.
      // This type is not fully modular and should be used only as a last resort.
      const std::pair< emp::Ptr<unsigned char>, size_t >
    >;

  // An OrganismBase class adds functionality for dealing with a specific data type.
  // This templated class allows us to maintain the simple list of Organism_data_Ts above.
  template <typename T> struct OrganismBase : public OrganismBase<typename T::pop> {
    virtual ~OrganismBase() { ; }

    // Use functions from base classes AND overload them with the current data type.
    using OrganismBase<typename T::pop>::AddAction;
    using OrganismBase<typename T::pop>::TriggerEvent;
    using action_fun_t = std::function<void(Organism &, T)>;
    virtual bool AddAction(const std::string &, action_fun_t, int) { return false; }
    virtual bool AddEvent(const std::string & event_name, int event_id, T) { return false; }
    virtual void TriggerEvent(int, T) { ; }
  };

  template <> struct OrganismBase<emp::TypePack<>> {
    virtual ~OrganismBase() { ; }

    // Define functions with NO data parameters
    using action_fun_t = std::function<void(Organism &)>;
    virtual bool AddAction(const std::string &, action_fun_t, int) { return false; }
    virtual bool AddEvent(const std::string & event_name, int event_id) { return false; }
    virtual void TriggerEvent(int) { ; }
  };

  class Organism : public OrganismBase<Organism_data_Ts> {
  protected:
    emp::VarMap var_map;            ///< Dynamic variables assigned to organism
    OrganismManagerBase & manager;  ///< Manager for the specific organism type

    // Helper functions.
    ConfigScope & GetScope() { return manager.GetScope(); }

  protected:
    // Forward all variable linkage to the organism's manager.
    template <typename... Ts>
    auto & LinkVar(Ts &&... args) { return manager.LinkVar(args...); }

    template <typename VAR_T, typename DEFAULT_T>
    auto & LinkFuns(std::function<VAR_T()> get_fun,
                    std::function<void(const VAR_T &)> set_fun,
                    const std::string & name,
                    const std::string & desc,
                    DEFAULT_T default_val) {
      return manager.LinkFuns<VAR_T, DEFAULT_T>(get_fun, set_fun, name, desc, default_val);
    }

    // template <typename... Ts>
    // auto & LinkPop(Ts &&... args) { return manager.LinkPop(args...); }

  public:
    Organism(OrganismManagerBase & _man) : manager(_man) { ; }
    virtual ~Organism() { ; }

    OrganismManagerBase & GetManager() { return manager; }
    const OrganismManagerBase & GetManager() const { return manager; }

    bool HasVar(const std::string & name) const { return var_map.Has(name); }
    template <typename T> T & GetVar(const std::string & name) { return var_map.Get<T>(name); }
    template <typename T> const T & GetVar(const std::string & name) const { return var_map.Get<T>(name); }

    template <typename T>
    void SetVar(const std::string & name, const T & value) {
      var_map.Set(name, value);
    }

    /// Test if this organism represents an empy cell.
    virtual bool IsEmpty() const noexcept { return false; }

    // --- Functions for overriding ---

    /// We MUST be able to make a copy of organisms for MABE to function.  If this function
    /// is not overridden, try to the equivilent function in the organism manager.
    virtual emp::Ptr<Organism> Clone() const { return manager.CloneOrganism(*this); }

    /// If we are going to print organisms (to screen or file) we need to be able to convert
    /// them to strings.  If this function is not overridden, try to the equivilent function
    /// in the organism manager.
    virtual std::string ToString() { return manager.ToString(*this); }

    /// For evolution to function, we need to be able to mutate offspring.
    virtual size_t Mutate(emp::Random & random) { return manager.Mutate(*this, random); }

    /// Completely randomize a new organism (typically for initialization)
    virtual void Randomize(emp::Random & random) { manager.Randomize(*this, random); }

    /// Generate an output and place it in the VarMap under the provided name (default = "result").
    /// Arguments are the output name int he VarMap and the output ID.
    virtual void GenerateOutput(const std::string & ="result", size_t=0) { ; }

    /// Request output type (multiple types are possible); default to unknown.
    /// Argument is the output ID.
    virtual emp::TypeID GetOutputType(size_t=0) { return emp::TypeID(); }


    /// --- Extra functions for when this is used a a prototype organism ---
    
    /// Setup organism-specific configuration options.
    virtual void SetupConfig() { ; }

  };

}
#endif

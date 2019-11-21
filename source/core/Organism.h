/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  Organism.h
 *  @brief A base class for all organisms in MABE.
 *  @note Status: ALPHA
 */

#ifndef MABE_ORGANISM_H
#define MABE_ORGANISM_H

#include "base/assert.h"
#include "data/VarMap.h"
#include "meta/TypeID.h"
#include "tools/string_utils.h"

#include "OrganismManagerBase.h"

namespace mabe {

  class Organism {
  protected:
    emp::VarMap var_map;         ///< Dynamic variables assigned to organism
    OrganismManagerBase & manager;   ///< Manager for the specific organism type

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

    /// Actions are specific functions that can be triggered by organisms.  AddAction() provides
    /// access to the passed-in function and returns a bool indicating whether the function is
    /// usable.
    using name_t = const std::string &;
    virtual bool AddAction(name_t, std::function<void()>) { return false; }
    virtual bool AddAction(name_t, std::function<void(double)>) { return false; }
    virtual bool AddAction(name_t, std::function<void(const emp::vector<double> &)>) { return false; }
    virtual bool AddAction(name_t, std::function<void(const std::map<size_t,double> &)>) { return false; }
    virtual bool AddAction(name_t, std::function<void(const std::string &)>) { return false; }
    virtual bool AddAction(name_t, std::function<void(const emp::vector<std::string> &)>) { return false; }
    virtual bool AddAction(name_t, std::function<void(const std::map<size_t,std::string> &)>) { return false; }
    virtual bool AddAction(name_t, std::function<void(const std::map<std::string,double> &)>) { return false; }
    virtual bool AddAction(name_t, std::function<void(const emp::BitVector &)>) { return false; }


    /// --- Extra functions for when this is used a a prototype organism ---
    
    /// Setup organism-specific configuration options.
    virtual void SetupConfig() { ; }

  };

}
#endif

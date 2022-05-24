/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021.
 *
 *  @file  MABEBase.hpp
 *  @brief Signal and population management for the master controller object in a MABE run.
 */

#ifndef MABE_MABE_BASE_H
#define MABE_MABE_BASE_H

#include <string>

#include "emp/base/array.hpp"
#include "emp/base/notify.hpp"
#include "emp/base/Ptr.hpp"
#include "emp/base/vector.hpp"

#include "ModuleBase.hpp"
#include "Population.hpp"
#include "SigListener.hpp"

namespace mabe {

  /// MABEBase sets up all population-manipulation functionality to ensure consistant handling
  /// (nowhere else can alter a Population object); all manipulation calls ultimately come through
  /// the limited functions defined here.  MABEBase is a friend class of Population, with all
  /// operations to manipulate organisms is a population listed as private.

  class MABEBase {
  protected:
    bool exit_now=false;     ///< Do we need to immediately clean up and exit the run?
    emp::Random random;      ///< Master random number generator
    size_t update = 0;       ///< How many times has Update() been called?
    bool verbose = false;    ///< Should we output extra information during setup?

    /// Maintain a master array of pointers to all SigListeners.
    using sig_base_t = SigListenerBase<ModuleBase>;
    emp::array< emp::Ptr<sig_base_t>, (size_t) ModuleBase::NUM_SIGNALS > sig_ptrs;

    /// Maintain a collection of all modules used in this run.
    using mod_ptr_t = emp::Ptr<ModuleBase>;
    emp::vector<mod_ptr_t> modules;  

    // --- Track which modules need to have each signal type called on them. ---
    // BeforeUpdate(size_t update_ending)
    SigListener<ModuleBase,void,size_t> before_update_sig;
    // OnUpdate(size_t new_update)
    SigListener<ModuleBase,void,size_t> on_update_sig;
    // BeforeRepro(OrgPosition parent_pos) 
    SigListener<ModuleBase,void,OrgPosition> before_repro_sig;
    // OnOffspringReady(Organism & offspring, OrgPosition parent_pos, Population & target_pop)
    SigListener<ModuleBase,void,Organism &, OrgPosition, Population &> on_offspring_ready_sig;
    // OnInjectReady(Organism & inject_org, Population & target_pop)
    SigListener<ModuleBase,void,Organism &, Population &> on_inject_ready_sig;
    // BeforePlacement(Organism & org, OrgPosition target_pos, OrgPosition parent_pos)
    SigListener<ModuleBase,void,Organism &, OrgPosition, OrgPosition> before_placement_sig;
    // OnPlacement(OrgPosition placement_pos)
    SigListener<ModuleBase,void,OrgPosition> on_placement_sig;
    // BeforeMutate(Organism & org)
    SigListener<ModuleBase,void,Organism &> before_mutate_sig; // TO IMPLEMENT
    // OnMutate(Organism & org)
    SigListener<ModuleBase,void,Organism &> on_mutate_sig; // TO IMPLEMENT
    // BeforeDeath(OrgPosition remove_pos)
    SigListener<ModuleBase,void,OrgPosition> before_death_sig;
    // BeforeSwap(OrgPosition pos1, OrgPosition pos2)
    SigListener<ModuleBase,void,OrgPosition,OrgPosition> before_swap_sig;
    // OnSwap(OrgPosition pos1, OrgPosition pos2)
    SigListener<ModuleBase,void,OrgPosition,OrgPosition> on_swap_sig;
    // BeforePopResize(Population & pop, size_t new_size)
    SigListener<ModuleBase,void,Population &,size_t> before_pop_resize_sig;
    // OnPopResize(Population & pop, size_t old_size)
    SigListener<ModuleBase,void,Population &,size_t> on_pop_resize_sig;
    // BeforeExit()
    SigListener<ModuleBase,void> before_exit_sig;
    // OnHelp()
    SigListener<ModuleBase,void> on_help_sig;

    /// If a module fails to use a signal, we never check it again UNLESS we are explicitly
    /// told to rescan the signals (perhaps because new functionality was enabled.)
    bool rescan_signals = true;

    // Protected constructor so that base class cannot be instantiated except from derived class.
    MABEBase()
    : before_update_sig("before_update", ModuleBase::SIG_BeforeUpdate, &ModuleBase::BeforeUpdate, sig_ptrs)
    , on_update_sig("on_update", ModuleBase::SIG_OnUpdate, &ModuleBase::OnUpdate, sig_ptrs)
    , before_repro_sig("before_repro", ModuleBase::SIG_BeforeRepro, &ModuleBase::BeforeRepro, sig_ptrs)
    , on_offspring_ready_sig("on_offspring_ready", ModuleBase::SIG_OnOffspringReady, &ModuleBase::OnOffspringReady, sig_ptrs)
    , on_inject_ready_sig("on_inject_ready", ModuleBase::SIG_OnInjectReady, &ModuleBase::OnInjectReady, sig_ptrs)
    , before_placement_sig("before_placement", ModuleBase::SIG_BeforePlacement, &ModuleBase::BeforePlacement, sig_ptrs)
    , on_placement_sig("on_placement", ModuleBase::SIG_OnPlacement, &ModuleBase::OnPlacement, sig_ptrs)
    , before_mutate_sig("before_mutate", ModuleBase::SIG_BeforeMutate, &ModuleBase::BeforeMutate, sig_ptrs)
    , on_mutate_sig("on_mutate", ModuleBase::SIG_OnMutate, &ModuleBase::OnMutate, sig_ptrs)
    , before_death_sig("before_death", ModuleBase::SIG_BeforeDeath, &ModuleBase::BeforeDeath, sig_ptrs)
    , before_swap_sig("before_swap", ModuleBase::SIG_BeforeSwap, &ModuleBase::BeforeSwap, sig_ptrs)
    , on_swap_sig("on_swap", ModuleBase::SIG_OnSwap, &ModuleBase::OnSwap, sig_ptrs)
    , before_pop_resize_sig("before_pop_resize", ModuleBase::SIG_BeforePopResize, &ModuleBase::BeforePopResize, sig_ptrs)
    , on_pop_resize_sig("on_pop_resize", ModuleBase::SIG_OnPopResize, &ModuleBase::OnPopResize, sig_ptrs)
    , before_exit_sig("before_exit", ModuleBase::SIG_BeforeExit, &ModuleBase::BeforeExit, sig_ptrs)
    , on_help_sig("on_help", ModuleBase::SIG_OnHelp, &ModuleBase::OnHelp, sig_ptrs)
    { ;  }

  public:
    virtual ~MABEBase() { }

    void SetupBase() {
      emp::notify::Unpause();
    }

    // --- Basic accessors ---
    emp::Random & GetRandom() { return random; }
    size_t GetUpdate() const noexcept { return update; }
    bool GetVerbose() const { return verbose; }

    /// Trigger exit from run.
    void RequestExit() { exit_now = true; }

    /// Setup signals to be rescanned; call this if any signal is updated in a module.
    void RescanSignals() { rescan_signals = true; }

    /// All insertions of organisms into a population should come through AddOrgAt
    /// @param[in] org_ptr points to the organism being added (which will now be owned by the population).
    /// @param[in] pos is the position to perform the insertion.
    /// @param[in] ppos is the parent position (required if it exists; not used with inject).
    void AddOrgAt(emp::Ptr<Organism> org_ptr, OrgPosition pos, OrgPosition ppos=OrgPosition()) {
      emp_assert(org_ptr);                               // Must have a non-null organism to insert.
      before_placement_sig.Trigger(*org_ptr, pos, ppos); // Notify listerners org is about to be placed.
      ClearOrgAt(pos);                                   // Clear any organism already in this position.
      pos.PopPtr()->SetOrg(pos.Pos(), org_ptr);          // Put the new organism in place.
      on_placement_sig.Trigger(pos);                     // Notify listeners org has been placed.
    }

    /// All permanent deletion of organisms from a population should come through here.
    /// If the relevant position is already empty, nothing happens.
    /// After the position is cleared, caller must replace (possibly with an empty org) or resize away.
    /// @param[in] pos is the position to perform the deletion.
    void ClearOrgAt(OrgPosition pos) {
      emp_assert(pos.IsValid());
      if (pos.IsEmpty()) return;                    // Already empty? Nothing to remove!

      before_death_sig.Trigger(pos);                // Send signal of current organism dying.
      pos.Pop().ExtractOrg(pos.Pos()).Delete();     // Delete current organism.
    }

    /// All movement of organisms from one population position to another should come through here.
    void SwapOrgs(OrgPosition pos1, OrgPosition pos2) {
      emp_assert(pos1.IsValid());
      emp_assert(pos2.IsValid());
      before_swap_sig.Trigger(pos1, pos2);
      emp::Ptr<Organism> org1 = pos1.PopPtr()->ExtractOrg(pos1.Pos());
      emp::Ptr<Organism> org2 = pos2.PopPtr()->ExtractOrg(pos2.Pos());
      if (!org1->IsEmpty()) pos2.PopPtr()->SetOrg(pos2.Pos(), org1);
      if (!org2->IsEmpty()) pos1.PopPtr()->SetOrg(pos1.Pos(), org2);
      on_swap_sig.Trigger(pos1, pos2);
    }

    /// Change the size of a population.  If shrinking, clear orgs at removed positions;
    /// if growing, new positions will have empty organisms.
    void ResizePop(Population & pop, size_t new_size) {
      // Clean up any organisms that may be getting deleted.
      const size_t old_size = pop.GetSize();                // Track the starting size.
      if (old_size == new_size) return;                     // If size isn't changing, we're done!

      before_pop_resize_sig.Trigger(pop, new_size);         // Signal that resize about to happen.

      for (size_t pos = new_size; pos < old_size; pos++) {  // Clear all orgs out of range.
        ClearOrgAt( OrgPosition(pop, pos) );
      }

      pop.Resize(new_size);                                 // Do the actual resize.

      on_pop_resize_sig.Trigger(pop, old_size);             // Signal that resize has happened.
    }

    /// Add a single, empty position onto the end of a population.
    PopIterator PushEmpty(Population & pop) {
      before_pop_resize_sig.Trigger(pop, pop.GetSize()+1);
      PopIterator it = pop.PushEmpty();
      on_pop_resize_sig.Trigger(pop, pop.GetSize()-1);
      return it;
    }

    // Interface function for MABEScript
    virtual size_t GetRandomSeed() const = 0;
    virtual void SetRandomSeed(size_t in_seed) = 0;
    virtual Population & AddPopulation(const std::string & name, size_t pop_size=0) = 0;
    virtual void CopyPop(const Population & from_pop, Population & to_pop) = 0;
    virtual void MoveOrgs(Population & from_pop, Population & to_pop, bool reset_to) = 0;
  };
}

#endif

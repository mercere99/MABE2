/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2024
 *
 *  @file  TrackAncestor.hpp
 *  @brief MABE module to track info about the ancestor of each organism.
 */

#ifndef MABE_TRACK_ANCESTOR_HPP
#define MABE_TRACK_ANCESTOR_HPP

#include "../core/MABE.hpp"
#include "../core/Module.hpp"

namespace mabe {

  /// Track information about an organism's ancestor.
  class TrackAncestor : public Module {
  private:
    OwnedTrait<double> inject_time{this, "inject_time", "Update this lineage was injected"};
    OwnedTrait<size_t> clade_id{this, "clade_id", "Unique ID for the clade from this ancestor"};

    size_t next_clade = 0; // What value should the next clade ID have?

  public:
    TrackAncestor(mabe::MABE & control,
                const emp::String & name="TrackAncestor",
                const emp::String & desc="Module to choose the top fitness organisms for replication.")
      : Module(control, name, desc)
    { SetAnalyzeMod(true); } 
    ~TrackAncestor() { }

    // Setup member functions associated with this class.
    static void InitType(emplode::TypeInfo &) {
    }

    void SetupConfig() override {
    }

    void SetupModule() override {
    }

    void OnInjectReady(Organism & org, Population &) override {
      inject_time(org) = static_cast<double>(control.GetUpdate());
      clade_id(org) = next_clade++;
    }
  };

  MABE_REGISTER_MODULE(TrackAncestor, "Track info about the original ancestor of each organism.");
}

#endif

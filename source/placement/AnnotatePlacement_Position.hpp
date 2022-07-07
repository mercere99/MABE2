/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021-2022.
 *
 *  @file  AnnotatePlacement_Position.hpp
 *  @brief Stores organism's position as a trait on birth/inject
 */

#ifndef MABE_ANNOTATE_PLACEMENT_H
#define MABE_ANNOTATE_PLACEMENT_H

#include "../core/MABE.hpp"
#include "../core/Module.hpp"

namespace mabe {

  /// \brief Stores organism's position as a trait on birth/inject
  class AnnotatePlacement_Position : public Module {
  private:
    Collection target_collect;         ///< Collection of populations to manage
    std::string pos_trait = "org_pos"; ///< Name of trait storing organism's position

  public:
    AnnotatePlacement_Position(mabe::MABE & control,
                    const std::string & name="AnnotatePlacement_Position",
                    const std::string & desc="Store org's position as trait on placement.")
      : Module(control, name, desc)
    {
      SetPlacementMod(true);
    }
    ~AnnotatePlacement_Position() { }

    /// Set up variables for configuration file
    void SetupConfig() override {
      LinkCollection(target_collect, "target", "Population(s) to annotate.");
      LinkVar(pos_trait, "pos_trait", "Name of trait that will hold organism's position");
    }

    /// Initialize position trait
    void SetupModule() override {
      AddSharedTrait<OrgPosition>(pos_trait, "Organism's position in the population", {});
    }

    /// When an organism is placed (via birth or inject), store its position as a trait
    void OnPlacement(OrgPosition pos) override {
      if(target_collect.HasPopulation(*pos.PopPtr())){
        Organism& org = pos.PopPtr()->At(pos.Pos());
        org.SetTrait<OrgPosition>(pos_trait, pos);
      }
    }

  };

  MABE_REGISTER_MODULE(AnnotatePlacement_Position, "Store org's position as trait on placement.");
}

#endif

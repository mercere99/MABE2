/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2021.
 *
 *  @file  AnnotatePlacement.hpp
 *  @brief Places org position in org upon birth/inject
 */

#ifndef MABE_ANNOTATE_PLACEMENT_H
#define MABE_ANNOTATE_PLACEMENT_H

#include "../core/MABE.hpp"
#include "../core/Module.hpp"

namespace mabe {

  class AnnotatePlacement : public Module {
  private:
    Collection target_collect;
    std::string pos_trait = "org_pos";

  public:
    AnnotatePlacement(mabe::MABE & control,
                    const std::string & name="AnnotatePlacement",
                    const std::string & desc="Store org's position as trait on placement.")
      : Module(control, name, desc), target_collect(control.GetPopulation(0))
    {
      SetPlacementMod(true);
    }
    ~AnnotatePlacement() { }

    void SetupConfig() override {
      LinkCollection(target_collect, "target", "Population(s) to manage.");
      LinkVar(pos_trait, "pos_trait", "Name of trait that will hold organism's position");
    }

    void SetupModule() override {
      AddOwnedTrait<OrgPosition>(pos_trait, "Organism's position in the population", {});
    }

    void OnPlacement(OrgPosition pos) override {
      Organism& org = pos.PopPtr()->At(pos.Pos());
      org.SetTrait<OrgPosition>(pos_trait, pos);
    }

  };

  MABE_REGISTER_MODULE(AnnotatePlacement, "Store org's position as trait on placement.");
}

#endif


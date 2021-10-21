#ifndef MABE_ANALYZE_SYSTEMATICS_MODULE_H
#define MABE_ANALYZE_SYSTEMATICS_MODULE_H

#include "../core/MABE.hpp"
#include "../core/Module.hpp"
#include "../core/TraitSet.hpp"
#include "emp/Evolve/Systematics.hpp"
#include "emp/data/DataFile.hpp"


namespace mabe {

class AnalyzeSystematics : public Module {
private:

    // Systematics manager
    bool store_outside;
    bool store_ancestors;
    std::string taxon_info;
    emp::Systematics <Organism, std::string> sys;

    // Output
    int snapshot_start;
    int snapshot_frequency;
    int snapshot_end;
    std::string snapshot_file_root_name;
    int data_start;
    int data_frequency;
    int data_end;
    std::string data_file_name;
    emp::DataFile data; 

public:
    AnalyzeSystematics(mabe::MABE & control,
               const std::string & name="AnalyzeSystematics",
               const std::string & desc="Module to track the population's phylogeny.",
               bool _storeout = 0, bool _storeanc = 1, const std::string & _taxon_info = "taxon_info")
      : Module(control, name, desc), 
      store_outside(_storeout), 
      store_ancestors(_storeanc),
      taxon_info(_taxon_info),
      sys([this](Organism& org){org.GenerateOutput(); return org.GetTraitAsString(org.GetTraitID(taxon_info));}, true, _storeanc, _storeout, true),
      snapshot_start(-1),
      snapshot_frequency(1),
      snapshot_end(-1),
      snapshot_file_root_name("phylogeny"),
      data_start(-1),
      data_frequency(-1),
      data_end(-1),
      data_file_name("phylogenetic_data.csv"),
      data("")
    {
      SetAnalyzeMod(true);    ///< Mark this module as an analyze module.
    }
    ~AnalyzeSystematics() { }

    void SetupConfig() override {
      LinkVar(store_outside, "store_outside", "Store all taxa that ever existed.(1 = TRUE)" );
      LinkVar(store_ancestors, "store_ancestors", "Store all ancestors of extant taxa.(1 = TRUE)" );
      LinkVar(taxon_info, "taxon_info", "Which trait should we identify unique taxa based on");
      LinkVar(data_file_name, "data_file_name", "Filename for systematics data file.");
      LinkVar(snapshot_file_root_name, "snapshot_file_root_name", "Filename for snapshot files (will have update number and .csv appended to end)");
      LinkRange(snapshot_start, snapshot_frequency, snapshot_end, "snapshot_updates", "Which updates should we output a snapshot of the phylogeny?");
      LinkRange(data_start, data_frequency, data_end, "data_updates", "Which updates should we output a data from the phylogeny?");

    }

    void SetupModule() override {
      // Setup the traits.
      // TODO: Ideally it would be great if we didn't have to list all possible allowed types here
      AddRequiredTrait<std::string, emp::BitVector, int, double, emp::vector<int>>(taxon_info);
     
      // Setup the data file
      data = emp::DataFile(data_file_name);
      sys.AddPhylogeneticDiversityDataNode();
      sys.AddPairwiseDistanceDataNode();
      sys.AddEvolutionaryDistinctivenessDataNode();
      std::function<size_t ()> updatefun = [this](){return control.GetUpdate();};
      data.AddFun(updatefun,"Generation", "The current generation");
      data.AddCurrent(*sys.GetDataNode("phylogenetic_diversity"), "phylogenetic_diversity","The current phylogenetic diversity.", true, true);
      data.PrintHeaderKeys();
      data.SetTimingRange(data_start, data_frequency, data_end);    

      // Setup the snapshot file
      std::function<std::string(const emp::Taxon<std::string> &)> snapshot_fun = [](const emp::Taxon<std::string> & taxon){return taxon.GetInfo();};
      sys.AddSnapshotFun(snapshot_fun, "taxon_info", "The string representation of the information that is used to delineate what counts as a different taxon.");
    }
      
    void OnUpdate(size_t update) override {
      sys.Update();

      if (update >= snapshot_start && update <= snapshot_end && (update - snapshot_start) % snapshot_frequency == 0) {
        sys.Snapshot(snapshot_file_root_name + "_" + emp::to_string(update) + ".csv");
      }
      data.Update(update);
      //if (update >= data_start && update <= data_end && (update - data_start) % data_frequency == 0) {
        //sys.data("phylogeny_" + emp::to_string(update) + ".csv");
    }

    void BeforeDeath(OrgPosition pos) override {
      sys.RemoveOrg({pos.Pos(), (size_t)pos.PopID()});
    }

    void BeforePlacement(Organism& org, OrgPosition pos, OrgPosition ppos) override {
      if (ppos.IsValid()) {
        sys.AddOrg(org, {pos.Pos(), (size_t)pos.PopID()}, {ppos.Pos(), (size_t)ppos.PopID()});
      } else {
        // We're injecting so no parent
        // Double-check that this is happening because pop is null,
        // not because parent position is illegal
        // emp_assert(ppos.PopPtr().IsNull() && "Illegal parent position");
        sys.AddOrg(org, {pos.Pos(), (size_t)pos.PopID()}, nullptr);
      }
    }

    void OnSwap(OrgPosition pos1, OrgPosition pos2) override {
      sys.SwapPositions({pos1.Pos(), (size_t)pos1.PopID()}, {pos2.Pos(), (size_t)pos2.PopID()});
    }
};

    MABE_REGISTER_MODULE(AnalyzeSystematics, "Module to track the population's phylogeny.");
}
#endif

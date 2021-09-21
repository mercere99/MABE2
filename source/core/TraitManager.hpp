/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021.
 *
 *  @file  TraitManager.hpp
 *  @brief Handling of multiple traits and how they relate to users.
 *
 *  A TraitManager facilitates the creation and destruction of TraitInfo object, which are
 *  stored in DataMaps and maintain access information about classes (modules) that use those
 *  traits.
 */

#ifndef MABE_TRAIT_MANAGER_HPP
#define MABE_TRAIT_MANAGER_HPP

#include <string>
#include <unordered_map>

#include "emp/base/Ptr.hpp"

#include "TraitInfo.hpp"

namespace mabe {

  /// TraitManager handles a collection of traits used by modules.
  /// @param MOD_T The type of modules using the traits.

  template <typename MOD_T>
  class TraitManager {
  private:
    /// Information about organism traits.  TraitInfo specifies which modules are allowed to
    /// (or expected to) access each trait, as well as how that trait should be initialized,
    /// archived, and summarized.
    std::unordered_map<std::string, emp::Ptr<TraitInfo>> trait_map;

    /// Configuration should happen BEFORE traits are created, so this calls starts locked.
    bool locked = true;

    /// ErrorManager passed in from main program.
    mabe::ErrorManager & error_man;

    /// Count the total number of errors encountered.
    int error_count = 0;

  public:
    TraitManager(mabe::ErrorManager & in_error_man) : error_man(in_error_man) { }
    ~TraitManager() {
      for (auto [name,trait_ptr] : trait_map) {
        trait_ptr.Delete();  // Delete all trait info.
      }
    }

    size_t GetSize() const { return trait_map.size(); }

    bool GetLocked() const { return locked; }
    void Lock() { locked = true; }
    void Unlock() { locked = false; }

    template <typename... Ts>
    void AddError(Ts &&... args) {
      error_man.AddError(std::forward<Ts>(args)...);
    }

    /// Register all of the traits in the the provided DataMap.
    void RegisterAll(emp::DataMap & data_map) {
      for (auto [name,trait_ptr] : trait_map) {
        trait_ptr->Register(data_map);
      }
    }

    /**
     *  Add a new organism trait.
     *  @param T The preferred type for this trait.
     *  @param ALT_Ts Alternative types that can be allowed for non-owning of this trait.
     *  @param mod_ptr Pointer to the module that uses this trait.
     *  @param access The accesses method the module is requesting for this trait.
     *  @param desc A brief description of this trait.
     *  @param default_value The value to use for this trait when it is not otherwise set.
     */
    template <typename T, typename... ALT_Ts>
    TraitInfo & AddTrait(emp::Ptr<MOD_T> mod_ptr,
                         TraitInfo::Access access,
                         const std::string & trait_name,
                         const std::string & desc,
                         const T & default_val)
    {
      const std::string & mod_name = mod_ptr->GetName();

      // Traits must be added in the SetupModule() function for the given modules;
      // afterward the trait manager is locked and additional new traits are not allowed.
      if (locked) {
        AddError("Module '", mod_name, "' adding trait '", trait_name,
                 "' before config files have loaded; should be done in SetupModule().");
      }

      // Traits cannot be added without access information.
      if (access == TraitInfo::UNKNOWN) {
        AddError("Module ", mod_name, " trying to add trait named '", trait_name,
                 "' with UNKNOWN access type.");
      }

      // Determine the type options this module can handle.
      emp::vector<emp::TypeID> alt_types = emp::GetTypeIDs<T, ALT_Ts...>();
      emp::Sort(alt_types);
 
      // If the trait does not already exist, build it as a new trait.
      emp::Ptr<TraitInfo> cur_trait = nullptr;
      if (emp::Has(trait_map, trait_name) == false) {
        cur_trait = emp::NewPtr<TypedTraitInfo<T>>(trait_name, default_val);
        cur_trait->SetAltTypes(alt_types);
        cur_trait->SetDesc(desc);
        trait_map[trait_name] = cur_trait;
      }
      
      // Otherwise make sure that it is consistent with previous modules.
      else {
        cur_trait = trait_map[trait_name];

        // Make sure that the SAME module isn't defining a trait twice.
        if (cur_trait->HasAccess(mod_ptr)) {
          AddError("Module ", mod_name, " is creating multiple traits named '",
                   trait_name, "'.");
        }

        // Figure out if the alternative types are compatable.
        emp::vector<emp::TypeID> prev_alt_types = cur_trait->GetAltTypes();
        emp::vector<emp::TypeID> intersect_types = emp::FindIntersect(alt_types, prev_alt_types);

        // Make sure the type setup for this trait is compatable with the current module.
        if ( !emp::Has(alt_types, cur_trait->GetType()) ) {
          // Previous type does not match current options; can we switch over to type T?
          if (cur_trait->IsAllowedType<T>()) {
            cur_trait.Delete();
            cur_trait = emp::NewPtr<TypedTraitInfo<T>>(trait_name, default_val);
            cur_trait->SetDesc(desc);
            trait_map[trait_name] = cur_trait;
          }

          // @CAO Technically, we can shift to any of the intersect types.

          // Otherwise we have incompatable types...
          else {
            AddError("Module ", mod_name, " is trying to use trait '",
                    trait_name, "' of type ", emp::GetTypeID<T>(),
                    "; Previously defined in module(s) ",
                    emp::to_english_list(cur_trait->GetModuleNames()),
                    " as type ", cur_trait->GetType());
          }
        }

        // Update the alternate types
        cur_trait->SetAltTypes(intersect_types);
      }

      // Add this module's access to the trait.
      bool is_manager = mod_ptr->IsManageMod();
      cur_trait->AddAccess(mod_name, mod_ptr, access, is_manager);

      return *cur_trait;
    }

    /////////////////////////////////////////////////
    //  --- Trait verification functions ---

    /// Make sure that there are no illegal states in this trait setup.
    bool VerifyValid(const std::string & trait_name, emp::Ptr<TraitInfo> trait_ptr) {
      // NO traits should be of UNKNOWN access.
      if (trait_ptr->GetUnknownCount()) {
        error_man.AddError("Unknown access mode for trait '", trait_name,
                            "' in module(s) ", emp::to_english_list(trait_ptr->GetUnknownNames()),
                            " (internal error!)");
        return false;
      }

      return true;
    }

    /// Verify that modules are handling private access of a trait correctly.
    bool VerifyPrivacy(const std::string & trait_name, emp::Ptr<TraitInfo> trait_ptr) {
      // Only one module can be involved for PRIVATE access.
      if (trait_ptr->GetPrivateCount() > 1) {
        std::stringstream error_msg;
        error_msg << "Multiple modules declaring trait '" << trait_name
                  << "' as private: " << emp::to_english_list(trait_ptr->GetPrivateNames()) << ".\n"
                  << "[Suggestion: if traits are supposed to be distinct, prepend names with a\n"
                  << " module-specific prefix.  Otherwise modules need to be edited to not have\n"
                  << " trait private.]";
        error_man.AddError(error_msg.str());
        return false;
      }

      if (trait_ptr->GetPrivateCount() && trait_ptr->GetModuleCount() > 1) {
        error_man.AddError("Trait '", trait_name, "' is private in module '",
                trait_ptr->GetPrivateNames()[0],
                "'; should not be used by other modules.\n",
                "[Suggestion: if traits are supposed to be distinct, prepend private name with a\n",
                " module-specific prefix.  Otherwise module needs to be edited to not have\n",
                " trait private.]");
        return false;
      }

      return true;
    }

    /// Verify that modules are allowing only a single owner of a trait.
    bool VerifyOwnership(const std::string & trait_name, emp::Ptr<TraitInfo> trait_ptr) {
      // A trait that is OWNED or GENERATED cannot have other modules writing to it.
      const size_t claim_count = trait_ptr->GetOwnedCount() + trait_ptr->GetGeneratedCount();

      if (claim_count > 1) {
        auto mod_names = emp::Concat(trait_ptr->GetOwnedNames(), trait_ptr->GetGeneratedNames());
        std::stringstream error_msg;
        error_msg << "Multiple modules declaring ownership of trait '" << trait_name << "': "
                  << emp::to_english_list(mod_names) << ".\n"
                  << "[Suggestion: if traits are supposed to be distinct, prepend names with a\n"
                  << " module-specific prefix.  Otherwise modules should be edited to change trait\n"
                  << " to be SHARED (and all can modify) or have all but one shift to REQUIRED.]";
        error_man.AddError(error_msg.str());
        return false;
      }

      if (claim_count && trait_ptr->IsShared()) {
        auto mod_names = emp::Concat(trait_ptr->GetOwnedNames(), trait_ptr->GetGeneratedNames());
        error_man.AddError("Trait '", trait_name,
          "' is fully OWNED by module '", mod_names[0],
          "'; it cannot be SHARED (written to) by other modules:",
          emp::to_english_list(trait_ptr->GetSharedNames()),
          "[Suggestion: if traits are supposed to be distinct, prepend private name with a\n",
          " module-specific prefix.  Otherwise module needs to be edited to make trait\n",
          " SHARED or have all but one shift to REQUIRED.]");
        return false;
      }

      return true;
    }

    /// Verify that modules use traits the ways other modules require.
    bool VerifyRequirements(const std::string & trait_name, emp::Ptr<TraitInfo> trait_ptr) {
      // A REQUIRED trait must have another module write to it (i.e. OWNED, GENERATED or SHARED).
      if (trait_ptr->IsRequired() &&
              !trait_ptr->IsOwned() && !trait_ptr->IsShared() && !trait_ptr->IsGenerated()) {
        error_man.AddError("Trait '", trait_name, "' marked REQUIRED by module(s) ",
                emp::to_english_list(trait_ptr->GetRequiredNames()),
                "'; must be written to by other modules.\n",
                "[Suggestion: set another module to write to this trait (where it is either\n",
                " SHARED or OWNED).]");
        return false;
      }

      // A GENERATED trait requires another module to read (REQUIRE) it.
      else if (trait_ptr->IsGenerated() && !trait_ptr->IsRequired()) {
        error_man.AddError("Trait '", trait_name, "' marked GENERATED by module(s) ",
                emp::to_english_list(trait_ptr->GetGeneratedNames()),
                "'; must be read by other modules.");
        return false;
      }

      return true;
    }

    /// Make sure modules are accessing traits correctly and consistently.
    bool Verify(bool verbose) {
      if (verbose) {
        std::cout << "Analyzing configuration of " << trait_map.size() << " traits.\n";
      }

      // Loop through all of the traits to ensure there are no conflicts.
      for (auto [trait_name, trait_ptr] : trait_map) {
        if (verbose) {
          std::cout << "...scanning '" << trait_name << "' with "
                    << trait_ptr->GetModuleCount() << " modules:"
                    << " private=" << trait_ptr->GetPrivateCount()
                    << " owned=" << trait_ptr->GetOwnedCount()
                    << " generated=" << trait_ptr->GetGeneratedCount()
                    << " shared=" << trait_ptr->GetSharedCount()
                    << " required=" << trait_ptr->GetRequiredCount()
                    << std::endl;
        }

        if (!VerifyValid(trait_name, trait_ptr) ||
            !VerifyPrivacy(trait_name, trait_ptr) ||
            !VerifyOwnership(trait_name, trait_ptr) ||
            !VerifyRequirements(trait_name, trait_ptr)) {
          error_count++;
        }        
      }

      return error_count;
    }

  };
}

#endif
/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021-2024.
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

#include <unordered_map>

#include "emp/base/Ptr.hpp"
#include "emp/meta/type_traits.hpp"
#include "emp/base/notify.hpp"
#include "emp/tools/String.hpp"

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
    std::unordered_map<emp::String, emp::Ptr<TraitInfo>> trait_map;

    /// Configuration should happen BEFORE traits are created, so manager starts locked.
    bool locked = true;

    /// Count the total number of errors encountered.
    int error_count = 0;

  public:
    TraitManager() { }
    ~TraitManager() {
      for (auto [name,trait_ptr] : trait_map) {
        trait_ptr.Delete();  // Delete all trait info.
      }
    }

    size_t GetSize() const { return trait_map.size(); }

    bool GetLocked() const { return locked; }
    void Lock() { locked = true; }
    void Unlock() { locked = false; }

    /// Register all of the traits in the the provided DataMap.
    void RegisterAll(emp::DataMap & data_map) {
      for (auto [name,trait_ptr] : trait_map) {
        trait_ptr->Register(data_map);
      }
    }

    void ResetAll(emp::DataMap & data_map){
      for (auto [name,trait_ptr] : trait_map) trait_ptr->ResetToDefault(data_map);
    }

    /**
     *  Add a new organism trait.
     *  @param T The preferred type for this trait.
     *  @param ALT_Ts Alternative types that can be allowed for non-owning of this trait.
     *  @param mod_ptr Pointer to the module that uses this trait.
     *  @param access The accesses method the module is requesting for this trait.
     *  @param trait_name String with the unique name for this trait.
     *  @param desc A brief description of this trait.
     *  @param default_value The value to use for this trait when it is not otherwise set.
     *  @param count A count of how many values are associated with this trait.
     */
    template <typename T, typename... ALT_Ts>
    TraitInfo & AddTrait(emp::Ptr<MOD_T> mod_ptr,
                         TraitInfo::Access access,
                         const emp::String & trait_name,
                         const emp::String & desc,
                         const T & default_val,
                         size_t count)
    {
      const emp::String & mod_name = mod_ptr->GetName();

      // Traits must be added in the SetupModule() function for the given modules;
      // afterward the trait manager is locked and additional new traits are not allowed.
      emp_assert(!locked, "Module adding trait before config files are loaded; add in SetupModule().",
                 mod_name, trait_name);

      // Traits cannot be added without access information.
      emp_assert(access != TraitInfo::UNKNOWN,
                 "Module trying to add trait with UNKNOWN access type.",
                 mod_name, trait_name);

      // Determine the type options this module can handle.
      emp::vector<emp::TypeID> alt_types = emp::GetTypeIDs<T, ALT_Ts...>();
      emp::Sort(alt_types);
 
      // If the trait does not already exist, build it as a new trait.
      emp::Ptr<TraitInfo> cur_trait = nullptr;
      if (emp::Has(trait_map, trait_name) == false) {
        cur_trait = emp::NewPtr<TypedTraitInfo<T>>(trait_name, default_val, count);
        cur_trait->SetAltTypes(alt_types);
        cur_trait->SetDesc(desc);
        trait_map[trait_name] = cur_trait;
      }
      
      // If it was previously defined as "Any Type", just use the new types.
      else if (trait_map[trait_name]->IsAnyType()) {
        cur_trait = trait_map[trait_name];
        // Build a new version of the trait information with the actual type options.
        emp::Ptr<TraitInfo> new_trait = emp::NewPtr<TypedTraitInfo<T>>(trait_name, default_val, count);
        cur_trait->SetAltTypes(alt_types);  // Include the alternate types.
        new_trait->SetDesc(desc);           // Update the description.
        new_trait->AddAccess(*cur_trait);   // Move over previous access attempts.
        trait_map[trait_name] = new_trait;  // Activate the new trait information.
        cur_trait.Delete();                 // Delete the old trait information.
        cur_trait = new_trait;
      }

      // Otherwise make sure that it is consistent with previous modules.
      else {
        cur_trait = trait_map[trait_name];

        // Make sure that the SAME module isn't defining a trait twice.
        if (cur_trait->HasAccess(mod_ptr)) {
          emp::notify::Error("Module ", mod_name, " is creating multiple traits named '",
                   trait_name, "'.");
        }

        // Figure out if the alternative types are compatable.
        emp::vector<emp::TypeID> prev_alt_types = cur_trait->GetAltTypes();
        emp::vector<emp::TypeID> intersect_types = emp::FindIntersect(alt_types, prev_alt_types);

        // Make sure the value count is consistent with previous modules.
        if ( cur_trait->GetValueCount() != count ) {
          if (count == TraitInfo::ANY_COUNT) count = cur_trait->GetValueCount();
          else if (cur_trait->GetValueCount() == TraitInfo::ANY_COUNT) cur_trait->SetValueCount(count);
          else {
            emp::notify::Error("Module ", mod_name, " is trying to use trait '",
                                trait_name, " with value count ", count,
                                ", but previously defined in module(s) ",
                                emp::MakeEnglishList(cur_trait->GetModuleNames()),
                                " with value count ", cur_trait->GetValueCount());
          }
        }

        // Make sure the type setup for this trait is compatable with the current module.
        if ( !emp::Has(alt_types, cur_trait->GetType()) ) {
          // Previous type does not match current options; can we switch over to type T?
          if (cur_trait->IsAllowedType<T>()) {
            // Build a new version of the trait information with the updated type.
            emp::Ptr<TraitInfo> new_trait = emp::NewPtr<TypedTraitInfo<T>>(trait_name, default_val, count);
            new_trait->SetDesc(desc);          // Update the description.
            new_trait->AddAccess(*cur_trait);  // Move over previous access attempts.
            trait_map[trait_name] = new_trait; // Activate the new trait information.
            cur_trait.Delete();                // Delete the old trait information.
            cur_trait = new_trait;
          }

          // @CAO Technically, we can shift to any of the intersect types.

          // Otherwise we have incompatible types...
          else {
            emp::notify::Error("Module ", mod_name, " is trying to use trait '",
                               trait_name, "' of type ", emp::GetTypeID<T>(),
                               "; Previously defined in module(s) ",
                               emp::MakeEnglishList(cur_trait->GetModuleNames()),
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

    /**
     *  Add a new organism trait that will only be viewed as a string.
     *  @param mod_ptr Pointer to the module that uses this trait.
     *  @param trait_name String with the unique name for this trait.
     */
    TraitInfo & AddTraitAsString(emp::Ptr<MOD_T> mod_ptr, const emp::String & trait_name)
    {
      const emp::String & mod_name = mod_ptr->GetName();

      // Traits must be added in the SetupModule() function for the given modules;
      // afterward the trait manager is locked and additional new traits are not allowed.
      if (locked) {
        emp::notify::Error("Module '", mod_name, "' adding trait '", trait_name,
                           "' before config files have loaded; should be done in SetupModule().");
      }

      // If the trait does not already exist, build it as a new trait.
      emp::Ptr<TraitInfo> cur_trait = nullptr;
      if (emp::Has(trait_map, trait_name) == false) {
        trait_map[trait_name] = emp::NewPtr<TraitInfoAsString>(trait_name);
      }
      
      // Otherwise make sure that it is consistent with previous modules.
      else {
        cur_trait = trait_map[trait_name];

        // Make sure that the SAME module isn't defining a trait twice.
        if (cur_trait->HasAccess(mod_ptr)) {
          emp::notify::Error("Module ", mod_name, " is creating multiple traits named '",
                   trait_name, "'.");
        }
      }

      // Add this module's access to the trait.
      bool is_manager = mod_ptr->IsManageMod();
      cur_trait->AddAccess(mod_name, mod_ptr, TraitInfo::Access::REQUIRED, is_manager);

      return *cur_trait;
    }


    /////////////////////////////////////////////////
    //  --- Trait verification functions ---

    /// Make sure that all traits have valid settings..
    bool VerifyValid(const emp::String & trait_name, emp::Ptr<TraitInfo> trait_ptr) {
      // NO traits should be of UNKNOWN access.
      if (trait_ptr->GetUnknownCount()) {
        emp::notify::Error("Unknown access mode for trait '", trait_name,
                            "' in module(s) ", emp::MakeEnglishList(trait_ptr->GetUnknownNames()),
                            " (internal error!)");
        return false;
      }

      if (trait_ptr->GetValueCount() == TraitInfo::ANY_COUNT) {
        emp::notify::Error("No count specified for '", trait_name,
                            "' in module(s) ", emp::MakeEnglishList(trait_ptr->GetModuleNames()),
                            " (internal error!)");
        return false;
      }

      return true;
    }

    /// Verify that modules are handling private access of a trait correctly.
    bool VerifyPrivacy(const emp::String & trait_name, emp::Ptr<TraitInfo> trait_ptr) {
      // Only one module can be involved for PRIVATE access.
      if (trait_ptr->GetPrivateCount() > 1) {
        std::stringstream error_msg;
        error_msg << "Multiple modules declaring trait '" << trait_name
                  << "' as private: " << emp::MakeEnglishList(trait_ptr->GetPrivateNames()) << ".\n"
                  << "[Suggestion: if traits are supposed to be distinct, prepend names with a\n"
                  << " module-specific prefix.  Otherwise modules need to be edited to not have\n"
                  << " trait private.]";
        emp::notify::Error(error_msg.str());
        return false;
      }

      if (trait_ptr->GetPrivateCount() && trait_ptr->GetModuleCount() > 1) {
        emp::notify::Error("Trait '", trait_name, "' is private in module '",
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
    bool VerifyOwnership(const emp::String & trait_name, emp::Ptr<TraitInfo> trait_ptr) {
      // A trait that is OWNED or GENERATED cannot have other modules writing to it.
      const size_t claim_count = trait_ptr->GetOwnedCount() + trait_ptr->GetGeneratedCount();

      if (claim_count > 1) {
        auto mod_names = emp::Concat(trait_ptr->GetOwnedNames(), trait_ptr->GetGeneratedNames());
        std::stringstream error_msg;
        error_msg << "Multiple modules declaring ownership of trait '" << trait_name << "': "
                  << emp::MakeEnglishList(mod_names) << ".\n"
                  << "[Suggestion: if traits are supposed to be distinct, prepend names with a\n"
                  << " module-specific prefix.  Otherwise modules should be edited to change trait\n"
                  << " to be SHARED (and all can modify) or have all but one shift to REQUIRED.]";
        emp::notify::Error(error_msg.str());
        return false;
      }

      if (claim_count && trait_ptr->IsShared()) {
        auto mod_names = emp::Concat(trait_ptr->GetOwnedNames(), trait_ptr->GetGeneratedNames());
        emp::notify::Error("Trait '", trait_name,
          "' is fully OWNED by module '", mod_names[0],
          "'; it cannot be SHARED (written to) by other modules:",
          emp::MakeEnglishList(trait_ptr->GetSharedNames()),
          "[Suggestion: if traits are supposed to be distinct, prepend private name with a\n",
          " module-specific prefix.  Otherwise module needs to be edited to make trait\n",
          " SHARED or have all but one shift to REQUIRED.]");
        return false;
      }

      return true;
    }

    /// Verify that modules use traits the ways other modules require.
    bool VerifyRequirements(const emp::String & trait_name, emp::Ptr<TraitInfo> trait_ptr) {
      // A REQUIRED trait must have another module write to it (i.e. OWNED, GENERATED or SHARED).
      if (trait_ptr->IsRequired() &&
              !trait_ptr->IsOwned() && !trait_ptr->IsShared() && !trait_ptr->IsGenerated()) {
        emp::notify::Error("Trait '", trait_name, "' marked REQUIRED by module(s) '",
                emp::MakeEnglishList(trait_ptr->GetRequiredNames()),
                "'; must be written to by other modules.\n",
                "[Suggestion: set another module to write to this trait (where it is either\n",
                " SHARED or OWNED).]");
        return false;
      }

      // A GENERATED trait requires another module to read (REQUIRE) it.
      else if (trait_ptr->IsGenerated() && !trait_ptr->IsRequired()) {
        emp::notify::Error("Trait '", trait_name, "' marked GENERATED by module(s) ",
                emp::MakeEnglishList(trait_ptr->GetGeneratedNames()),
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

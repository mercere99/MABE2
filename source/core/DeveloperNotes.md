This directory contains all of the core functionality for MABE.  This code should never be
changed for individual experients.

# Layout

The core components of MABE are below.  The first four files are tools with no internal dependancies.  Each of the remaining files depend on all of those above it.

data_collect.hpp    - Tools to extract data from elements in a container. 
SigListener.hpp     - Tool to trigger a specified member function on other classes when triggered.
TraitInfo.hpp       - Specifications for module/trait interactions (on organisms, populations, etc.)
TraitSet.hpp        - Collections of traits, all with the same type (or a vector of that type)

ModuleBase.hpp      - Core functionality for interfacing with all module types.
Organism.hpp        - Information about a single agent; ModuleBase is interface for OrganismManager.
OrgIterator.hpp     - Tools for identifying organism locations and stepping through sets of them.
Population.hpp      - Collection of Organisms (some of which could be EmptyOrganisms)
Collection.hpp      - A more flexible collection of organisms or whole populations for manipulation.
MABE.hpp            - Main contoller object; manipulates Populations and Organisms
Module.hpp          - Modify main MABE contoller functions.
OrganismManager.hpp - Specialty Module type to manage organisms with shared configuration.
EmptyOrganism.hpp   - Specialty Organism type to represent empty cells.


# Component Development

When Building new components for MABE, new organism types should be derived from the Organism class (with an optional specialized OrganismManager) and new modules should be derived from Module.

## Organisms

An organism can be devleoped as a class unto itself OR it can make use of a specialized OrganismManager that stores shared inforation.  If an organism class doesn't have a specialized manager, it must override a few specialty "prototype" functions to specify the configuration parameters that it uses.

## Modules


# Core MABE Development

## Adding new Module Signals

From a design perspective, we should keep the number of module signals to a minimum.  If you
decide that an additional signal is required, carefully follow the checklist below to add it.
If any step is missed, the results can end up being hard to debug.

ModuleBase.h:
* Add a description of the new signal in the comments at the top of the file.
* Add a SIG_* id for the signal in the SignalID enum
* Declare a virtual member function to catch the signal in the SIGNALS section

Module.h:
* Overide the virtual member function for the signal (base method to catch the function)

MABE.h:
* Setup a new SigListener in the member variables section of MABEBase
* Initialize the new SigListener in the initializer section of the MABEBase constructor.
* Hook in a signal trigger in the appropriate place(s)


# Next steps in Core MABE Development

* MABE::FindInjectPosition should use modules explicitly associated with populations, not scan all modules.  This can be setup by modules declaring themselves as handling population structure needing to indicate WHICH populations (perhaps in SetupModule()?)

* Problem to resolve: trait access will often be limited to have only one module WRITE a trait.  These seem important for organism managers, but that limitation should only be for THAT organism type.  If, for example, we have a bits trait, ANY organism manager should be able to write to that trait.  A simple solution would be to have access apply to ALL organism managers when any one needs to set it.

* Problem to resolve: We want configuration to be able to provide equations (e.g., for a fitness function), not just always putting in a final value.  Perhaps setup with functions?  Or strings that get interpreted within the module?  Or a specialty module that interprets strings and writes the result to a different trait?

* Problem to resolve: Different types of organisms will have different traits that they need to deal with.  For example, "Wolves" vs "Sheep".  Do unused values just stay at a default?  Or can we have organism categories that each have their own trait layouts?

* Setup Organisms to be composed of brains, genomes, and adaptors (to be assembled during configuration).  Right now organisms must be built as a whole class, which is much less flexible.
  
* Update Config system to allow more generic categories.

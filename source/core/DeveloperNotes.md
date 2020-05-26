This directory contains all of the core functionality for MABE.  None of this code should be
changed for individual experients.

# Layout

The eight core components of MABE are below.  Each component depends on all of those above it.

TraitInfo.h       - Specifications for module/trait interactions (on organisms, populations, etc.)
ModuleBase.h      - Core functionality for interfacing with all module types.
Organism.h        - Information about a single agent; ModuleBase is interface for OrganismManager
Population.h      - Collection of Organisms (some of which could be EmptyOrganisms)
MABE.h            - Main contoller object; manipulates Populations and Organisms
Module.h          - Modify main MABE contoller functions.
OrganismManager.h - Specialty Module type to manage organisms with shared configuration.
EmptyOrganism.h   - Specialty Organism type to represent empty cells.


# Component Development

When Building new components for MABE, new organism types should be derived from the Organism class (with an optional specialized OrganismManager) and new modules should be derived from Module.

## Organisms

An organism can be devleoped as a class unto itself OR it can make use of a specialized OrganismManager that stores shared inforation.  If an organism class doesn't have a specialized manager, it must override a few specialty "prototype" functions to specify the configuration parameters that is uses.

## Modules


# Core MABE Development

## Adding new Module Signals

From a design perspective, we should keep the number of module signals to a minimum.  If you
decide that an additional signal is required, carefully follow the checklist below to add it.
If any step is missed, the results can end up being hard to debug.

* Add a description of the new signal in the comments at the top of ModuleBase.h
* Add a SIG_* id for the signal in the SignalID enum in ModuleBase
* Declare a virtual member function to catch the signal in the SIGNALS section of ModuleBase
* Overide the virtual member function for the signal in Module.h (base method to catch the function)
* Setup a new SigListener in the member variables section of MABEBase (in MABE.h)
* Initialize the new SigListener in the initializer section of the MABEBase constructor.
* Hook in a signal trigger in the appropriate places, most likely in MABE.h
  


# Important next steps in Core MABE Development

* Split SetupTraits() out of SetupModule() for user clarity.

* Get rid of SetMinPop(); this should be figured out automatically.

* MABE::FindInjectPosition should use modules explicitly associated with populations, not scan all modules.  This can be setup by modules declaring themselves as handling population structure needing to indicate WHICH populations (perhaps in SetupModule()?)

* Cleanup comments in SigListener definitions in MABEBase (MABE.h); right now they can easily be disassociated from the actual signal definition on the line below.  If, instead, we put each input on one line, we could provide desriptions right next to them.  (probably something similar for signal in Module.h and remove comment from the top of ModuleBase.h, putting in a pointer to Module.h instead.

* Setup Organisms to be composed of brains, genomes, and adaptors (to be assembled during configuration).  Right now organisms must be built as a whole class, which is much less flexible.
  
* Update Config system to allow more generic categories.

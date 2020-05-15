This director contains all of the core functionality for MABE.  None of this code should be
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
EmptyOrganism.h   - Specialty organism type to represent empty cells.

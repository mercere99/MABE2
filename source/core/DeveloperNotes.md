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

## Modules


# TODO: Core Development

* Setup Organisms to be composed of brains, genomes, and adaptors (to be assembled during configuration).
  Right now organisms must be built as a whole class, which is much less flexible.
  
* Update Config system to allow more generic categories.

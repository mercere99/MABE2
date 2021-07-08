=================
Trait Information
=================

Overview
--------

A TraitInfo object contains basic information a about a single trait being tracked in an
organism.  In addition to the name, type, and description of the trait, it also describes:
the ``TARGET`` method, the ``ACCESS`` method, the ``INIT`` method, the ``ARCHIVE`` method, and the ``SUMMARY`` method.

TARGET method
-------------

The ``TARGET`` indicates what type of object the trait should be applied to.

* ``ORGANISM``   - Every organism in MABE must have this trait.
* ``POPULATION`` - Collections of organisms must have this trait.
* ``MODULE``     - Every module attached to MABE must have this trait.
* ``MANAGER``    - Every OrganismManager must have this trait.

ACCESS method
-------------

The ``ACCESS`` method to be used for a trait by each module.  A trait can be

* ``PRIVATE``    - Only this module can modify the trait; no others should even read it.
* ``OWNED``      - Only this module can modify the trait, but other modules can read it.
* ``GENERATED``  - Only this module can modify the trait, but other modules MUST read it.
* ``SHARED``     - This module will read and write this trait, but others are allowed to as well.
* ``REQUIRED``   - This module will read the trait; another module must write to it.
* ``OPTIONAL``   - This module can read the trait; must first check if it exists.

INIT method
-----------

The ``INIT`` method describes how a trait should be initialized in a new offspring. 
(note that injected organisms always get the DEFAULT value.)

* ``DEFAULT``    - Always initialize this trait to its default value.
* ``FIRST``      - Initialize trait to the first parent's value (only parent for asexual runs)
* ``AVERAGE``    - Initialize trait to the average value of all parents.
* ``MINIMUM``    - Initialize trait to the minimum value of all parents.
* ``MAXIMUM``    - Initialize trait to the maximum value of all parents.

ARCHIVE method
--------------

The ``ARCHIVE`` method determines how many older values should be saved with each organism.

* ``NONE``       - Only the most recent value should be tracked, no archived values.
* ``AT_BIRTH``   - Store value of this trait was born with in "birth_(name)".
* ``LAST_REPRO`` - Store value of trait at the last reproduction in "last_(name)".
* ``ALL_REPROS`` - Store all value of trait at each reproduction event in "archive_(name)".
* ``ALL_VALUES`` - Store every value change of trait at any time in "sequence_(name)".


SUMMARY method
--------------

The ``SUMMARY`` method determines how a trait should be summarized over a collection of organisms.

* ``IGNORE``      - Don't include this trait in phyla records.
* ``AVERAGE``     -  Average of current value of all organisms (or final value at death).
* ``SUMMARY``     -  Basic summary (min, max, count, ave) of current/final values.
* ``FULL``        -  Store ALL current/final values for organisms.
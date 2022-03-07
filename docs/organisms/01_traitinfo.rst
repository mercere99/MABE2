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

* ``ORGANISM`` Every organism in MABE must have this trait.
* ``POPULATION`` Collections of organisms must have this trait.
* ``MODULE`` Every module attached to MABE must have this trait.
* ``MANAGER`` Every OrganismManager must have this trait.


ACCESS method
-------------

The ``ACCESS`` method to be used for a trait by each module.  A trait can be

* ``PRIVATE`` Only this module can modify the trait; no others should even read it.
* ``OWNED`` Only this module can modify the trait, but other modules can read it.
* ``GENERATED`` Only this module can modify the trait, but other modules MUST read it.
* ``SHARED`` This module will read and write this trait, but others are allowed to as well.
* ``REQUIRED`` This module will read the trait; another module must write to it.
* ``OPTIONAL`` This module can read the trait; must first check if it exists.

INIT method
-----------

The ``INIT`` method describes how a trait should be initialized in a new offspring. 
(note that injected organisms always get the DEFAULT value.)

* ``DEFAULT`` Always initialize this trait to its default value.
* ``FIRST`` Initialize trait to the first parent's value (only parent for asexual runs)
* ``AVERAGE`` Initialize trait to the average value of all parents.
* ``MINIMUM`` Initialize trait to the minimum value of all parents.
* ``MAXIMUM`` Initialize trait to the maximum value of all parents.

The default ``INIT`` method is ``DEFAULT``.

ARCHIVE method
--------------

The ``ARCHIVE`` method determines how many older values should be saved with each organism.

* ``NONE`` Only the most recent value should be tracked, no archived values.
* ``AT_BIRTH`` Store value of this trait was born with in "birth_(name)".
* ``LAST_REPRO`` Store value of trait at the last reproduction in "last_(name)".
* ``ALL_REPROS`` Store all value of trait at each reproduction event in "archive_(name)".
* ``ALL_VALUES`` Store every value change of trait at any time in "sequence_(name)".

The default ``ARCHIVE`` method is ``NONE``.


SUMMARY method
--------------

The ``SUMMARY`` method determines how a trait should be summarized over a collection of organisms.

* ``IGNORE`` Don't include this trait in phyla records.
* ``AVERAGE`` Average of current value of all organisms (or final value at death).
* ``SUMMARY`` Basic summary (min, max, count, ave) of current/final values.
* ``FULL`` Store ALL current/final values for organisms.

The default ``SUMMARY`` method is ``NONE``.

Helper Functions
----------------

The ``TraitInfo`` class comes with a number of public helper functions.

Getter Functions
****************

.. code-block:: cpp

  const std::string & GetName() const

.. code-block:: cpp

  const std::string & GetDesc() const

.. code-block:: cpp

  emp::TypeID GetType() const

.. code-block:: cpp

  Init GetInit() const

.. code-block:: cpp

  Archive GetArchive()

.. code-block:: cpp

  Summary GetSummary()

.. code-block:: cpp

  const emp::vector<emp::TypeID> & GetAltTypes() const

..

    What other types are allowed?

.. code-block:: cpp

  bool HasDefault() const

..

    Was a default value set for this trait?

.. code-block:: cpp

  const T & GetDefault() const

..


Setter Functions
****************

.. code-block:: cpp

  TraitInfo & SetName(const std::string & in_name)

.. code-block:: cpp

  TraitInfo & SetDesc(const std::string & in_desc)

.. code-block:: cpp

  TraitInfo & AddAccess(const std::string & in_name, mod_ptr_t in_mod, Access access)

..

    Add a module that can access this trait.

.. code-block:: cpp 

  virtual void Register(emp::DataMap & dm)

..

    Register this trait in the provided DataMap.

Parent Functions
****************

.. code-block:: cpp

  bool GetResetParent() const 

..
    Is the parent ALSO reset on birth?

.. code-block:: cpp

  TraitInfo & SetInheritParent()

..

    Set the current value of this trait to be automatically inherited by offspring.


To set the average, minimum, or maximum across parents for this trait to be automatically inherited by offspring use the 
corresponding setter function. For example, to set the average use the function: 

.. code-block:: cpp

  TraitInfo & SetInheritAverage()

..

.. code-block:: cpp

  TraitInfo & SetParentReset()

..

    Set the parent to ALSO reset to the same value as the offspring on divide.

To set the  the previous value or ALL previous values of this trait to be stored on birth or reset
use the corresponding setter function. For example, to set only the pervious value use the function:

.. code-block:: cpp

  TraitInfo & SetArchiveLast()

Type Functions
**************

.. code-block:: cpp

  void SetAltTypes(const emp::vector<emp::TypeID> & in_alt_types)

.. code-block:: cpp

  template <typename T> bool IsType() const

.. code-block:: cpp

  bool IsAllowedType(emp::TypeID test_type) const

.. code-block:: cpp

  template <typename T> bool IsAllowedType() const

Module/Access Functions
***********************

.. code-block:: cpp

  Access GetAccess(mod_ptr_t mod_ptr) const

..

    Determine what kind of access a module has.

.. code-block:: cpp

  bool HasAccess(mod_ptr_t mod_ptr) const

..

    Determine if a module has any knd of access to this trait.

.. code-block:: cpp

  size_t GetModuleCount() const

..

    How many modules can access this trait?

.. code-block:: cpp

  size_t GetAccessCount(Access access) const

..

    How many modules can access this trait using a specified access mode?


To determine the specific access mode of a trait use the boolean function for that access mode. For example, to 
determine if a trait has ``Private`` access use the function:

.. code-block:: cpp

  bool IsPrivate() const

..

To determine the number of traits with a specified access mode use the corresponding getter function. For 
example, to determine the number of traits with ``Private`` access use the function:

.. code-block:: cpp

  size_t GetPrivateCount() const

.. code-block:: cpp

  emp::vector<std::string> GetModuleNames() const

.. code-block:: cpp

  emp::vector<std::string> GetModuleNames(Access test_access)

..

    What specific modules have a given access mode?

To determine the specific modules with a specified mode use the corresponding getter function. 
For example, to determine the specific modules with ``Private`` access is use the function:

.. code-block:: cpp

  emp::vector<std::string> GetPrivateNames() const

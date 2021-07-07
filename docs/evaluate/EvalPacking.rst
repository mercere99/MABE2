==========
Packing
==========

Overview
--------

The EvalPacking module evaluates organisms based off their ability to correctly "pack" bricks of bits. A brick is sequence of exactly n ones. 
It is packed on both sides  by  at least k zeros. The fitness is awarded to the organism based on how many bricks are correctly packed, no partial credit is given. 
No packing is needed in front of the first brick.

Traits
------

For a description of each type of trait, see traits documentation that doesn't exist yet.


+----------------+-------------------+--------------------------------------------+
| Type of Trait  | Trait Name        | Trait Description                          |
+================+===================+============================================+
|  Private       |    N/A            |                                            |
+----------------+-------------------+--------------------------------------------+
|  Owned         | ``fitness_trait`` |  Stores the score (number of correctly     |
|                |                   |  packed bricks) of each organism.          |
+----------------+-------------------+--------------------------------------------+
|  Generated     |    N/A            |                                            |
+----------------+-------------------+--------------------------------------------+
|  Shared        |    N/A            |                                            |
+----------------+-------------------+--------------------------------------------+
|  Required      | ``bits_trait``    |  Stores the bit sequence to be evaluated.  |
+----------------+-------------------+--------------------------------------------+
|  Optional      |    N/A            |                                            |
+----------------+-------------------+--------------------------------------------+
  
Parameters
----------
* ``brick_size`` the number of ones necessary to make a full brick.
* ``packing_size`` the minimum number of zeros necessary to pack each brick.
Packing may be shared between two bricks. 
I/O
---

Input
*****

None.

Output
******

A single score based on the number of correctly packed bricks.

Example
-------

Compiling and Running
*********************

To run MABE with the Packing Evaluator, navigate to the build directory and use the command 
``make`` followed by the command ``./MABE -f settings/Packing.mabe``.
Parameters may be set in the ``Packing.mabe`` file.

Organisms
*********
+--------------+----------------+------------------+---------------+
| Bits         | ``brick_size`` | ``packing_size`` | Fitness Score |
+==============+================+==================+===============+
| 111011101110 | 3              | 1                | 3             |
+--------------+----------------+------------------+---------------+
| 111001110111 | 3              | 1                | 2             |
+--------------+----------------+------------------+---------------+
| 001111001111 | 4              | 2                | 1             |
+--------------+----------------+------------------+---------------+
| 111111111111 | 2              | 0                | 6             |
+--------------+----------------+------------------+---------------+
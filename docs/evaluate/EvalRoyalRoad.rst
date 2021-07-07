==========
RoyalRoad
==========

Overview
--------

The EvalRoyalRoad module evaluates organisms based on the RoyalRoad method. 
The fitness is awarded to the organism based on how many consecutive bricks (a sequence of n ones) are present with a penalty given for incomplete bricks at the end of the road.

Traits
------

For a description of each type of trait, see traits documentation that doesn't exist yet.


+----------------+-------------------+--------------------------------------------+
| Type of Trait  | Trait Name        | Trait Description                          |
+================+===================+============================================+
|  Private       |    N/A            |                                            |
+----------------+-------------------+--------------------------------------------+
|  Owned         | ``fitness_trait`` |  Stores the score (length of road minus    |
|                |                   |  penalty) of each organism.                |
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

I/O
---

Input
*****

None.

Output
******

A single score based on the length of the road minus the penalty.

Example
-------

To run MABE with the RoyalRoad Evaluator, navigate to the build directory and use the command 
``make`` followed by the command ``./MABE -f settings/RoyalRoad.mabe``.
Parameters may be set in the ``RoyalRoad.mabe`` file.
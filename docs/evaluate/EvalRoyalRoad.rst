==========
RoyalRoad
==========

Overview
--------

The EvalRoyalRoad module evaluates organisms based on the RoyalRoad method. 
The fitness is awarded to the organism based on how many consecutive bricks (a sequence of n ones) are present at the beginning of the organism. 
A penalty is given for incomplete bricks at the end of the road.
See Example for more details.
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
* ``size_t brick_size`` the number of ones necessary to make a full brick.

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

Compling and Running
********************

To run MABE with the RoyalRoad Evaluator, navigate to the build directory and use the command 
``make`` followed by the command ``./MABE -f settings/RoyalRoad.mabe``.
Parameters may be set in the ``RoyalRoad.mabe`` file. To see complete output navigate to the ``output.csv`` file.

Organisms
*********

+--------------+----------------+------------------+---------------+
| Bits         | ``brick_size`` | ``extra_bit_cost | Fitness Score |
+==============+================+==================+===============+
| 111111111    | 3              | 0.5              | 9             |
+--------------+----------------+------------------+---------------+
| 111111000111 | 3              | 0.5              | 6             |
+--------------+----------------+------------------+---------------+
| 111111000111 | 4              | 0.5              | 2             |
+--------------+----------------+------------------+---------------+

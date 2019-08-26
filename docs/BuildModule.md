#How to build a MABE module

## Basic Structure of a module

## Building your constructor

## The SetupConfig() and SetupModule() functions

## Functions to trigger on events

## Functions to respond to requests

## Extras needed for official modules

The rules above all assume that you are trying to build a MABE module that will
be functional for your own needs and for you to share with others.  If you are
trying to build a module to be shipped with the core MABE distribution, there
are a few other things that you will need to do.

1. The heading at the top of your file should be done in doxygen style, with a
   a release under the MIT Software license and an @brief description of the
   module.

1. The include guard should begin with MABE_

1. The module should be placed in the "mabe" namespace.

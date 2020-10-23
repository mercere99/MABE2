/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020.
 *
 *  @file  data_collect.h
 *  @brief Functions to collect data from containers.
 *
 *  A collection of mechanisms to agregate data from arbitrary objects in arbitrary containers.
 * 
 *  Each build function must know the data type it is working with (DATA_T), the type of container
 *  it should expect (CONTAIN_T), and be provided a function that will take a container element and
 *  return the appropriate value of type DATA_T.
 */

#ifndef EMP_DATA_COLLECT_H
#define EMP_DATA_COLLECT_H

namespace emp {

  template <typename DATA_T, typename CONTAIN_T, typename FUN_T>
  auto BuildCollectFun_Count(FUN_T get_fun) {
    return [get_fun](const CONTAIN_T & container) {
      std::unordered_set<DATA_T> vals;
      for (const auto & entry : container) {
        vals.insert( get_fun(entry) );
      }
      return emp::to_string(vals.size());
    };
  };


};

#endif
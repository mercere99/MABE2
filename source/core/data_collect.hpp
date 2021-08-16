/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020.
 *
 *  @file  data_collect.hpp
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

#include <functional>
#include <string>

#include "emp/tools/string_utils.hpp"

namespace emp {

  // Count up the number of distinct values.
  template <typename DATA_T, typename CONTAIN_T, typename FUN_T>
  auto BuildCollectFun_Index(FUN_T get_fun, const size_t index) {
    return [get_fun,index](const CONTAIN_T & container) {
      if (container.size() <= index) return "Nan"s;
      return emp::to_string( get_fun( container.At(index) ) );
    };
  }


  // Count up the number of distinct values.
  template <typename DATA_T, typename CONTAIN_T, typename FUN_T>
  auto BuildCollectFun_Unique(FUN_T get_fun) {
    return [get_fun](const CONTAIN_T & container) {
      std::unordered_set<DATA_T> vals;
      for (const auto & entry : container) {
        vals.insert( get_fun(entry) );
      }
      return emp::to_string(vals.size());
    };
  }


  template <typename DATA_T, typename CONTAIN_T, typename FUN_T>
  auto BuildCollectFun_Mode(FUN_T get_fun) {
    return [get_fun](const CONTAIN_T & container) {
      std::map<DATA_T, size_t> vals;
      for (const auto & entry : container) {
        vals[ get_fun(entry) ]++;
      }
      DATA_T mode_val;
      size_t mode_count = 0;

      for (auto [cur_val, cur_count] : vals) {
        if (cur_count > mode_count) {
          mode_count = cur_count;
          mode_val = cur_val;
        }
      }
      return emp::to_string(mode_val);
    };
  }

  template <typename DATA_T, typename CONTAIN_T, typename FUN_T>
  auto BuildCollectFun_Min(FUN_T get_fun) {
    return [get_fun](const CONTAIN_T & container) {
      DATA_T min{};
      if constexpr (std::is_arithmetic_v<DATA_T>) {
        min = std::numeric_limits<DATA_T>::max();
      }
      else if constexpr (std::is_same_v<std::string, DATA_T>) {
        min = std::string('~',22);   // '~' is ascii char 126 (last printable one.)
      }
      for (const auto & entry : container) {
        const DATA_T cur_val = get_fun(entry);
        if (cur_val < min) min = cur_val;
      }
      return emp::to_string(min);
    };
  }

  template <typename DATA_T, typename CONTAIN_T, typename FUN_T>
  auto BuildCollectFun_Max(FUN_T get_fun) {
    return [get_fun](const CONTAIN_T & container) {
      DATA_T max{};
      if constexpr (std::is_arithmetic_v<DATA_T>) {
        max = std::numeric_limits<DATA_T>::lowest();
      }
      for (const auto & entry : container) {
        const DATA_T cur_val = get_fun(entry);
        if (cur_val > max) max = cur_val;
      }
      return emp::to_string(max);
    };
  }

  template <typename DATA_T, typename CONTAIN_T, typename FUN_T>
  auto BuildCollectFun_Mean(FUN_T get_fun) {
    return [get_fun](const CONTAIN_T & container) {
      if constexpr (std::is_arithmetic_v<DATA_T>) {
        double total = 0.0;
        size_t count = 0;
        for (const auto & entry : container) {
          total += (double) get_fun(entry);
          count++;
        }
        return emp::to_string( total / count );
      }
      return std::string{"nan"};
    };
  }

  //template <typename DATA_T, typename CONTAIN_T, typename FUN_T>
  //auto BuildCollectFun_Median(FUN_T get_fun) {
  //  return [get_fun](const CONTAIN_T & container) {
  //    emp::vector<DATA_T> values(container.size());
  //    size_t count = 0;
  //    for (const auto & entry : container) {
  //      values[count++] = get_fun(entry);
  //    }
  //    emp::Sort(values);
  //    return emp::to_string( values[count/2] );
  //  };
  //}

  template <typename DATA_T, typename CONTAIN_T, typename FUN_T>
  auto BuildCollectFun_Variance(FUN_T get_fun) {
    return [get_fun](const CONTAIN_T & container) {
      if constexpr (std::is_arithmetic_v<DATA_T>) {
        double total = 0.0;
        const double N = (double) container.size();
        for (const auto & entry : container) {
          total += (double) get_fun(entry);
        }
        double mean = total / N;
        double var_total = 0.0;
        for (const auto & entry : container) {
          double cur_val = mean - (double) get_fun(entry);
          var_total += cur_val * cur_val;
        }

        return emp::to_string( var_total / (N-1) );
      }
      return std::string{"nan"};
    };
  }

  template <typename DATA_T, typename CONTAIN_T, typename FUN_T>
  auto BuildCollectFun_StandardDeviation(FUN_T get_fun) {
    return [get_fun](const CONTAIN_T & container) {
      if constexpr (std::is_arithmetic_v<DATA_T>) {
        double total = 0.0;
        const double N = (double) container.size();
        for (const auto & entry : container) {
          total += (double) get_fun(entry);
        }
        double mean = total / N;
        double var_total = 0.0;
        for (const auto & entry : container) {
          double cur_val = mean - (double) get_fun(entry);
          var_total += cur_val * cur_val;
        }

        return emp::to_string( sqrt(var_total / (N-1)) );
      }
      return std::string{"nan"};
    };
  }

  template <typename DATA_T, typename CONTAIN_T, typename FUN_T>
  auto BuildCollectFun_Sum(FUN_T get_fun) {
    return [get_fun](const CONTAIN_T & container) {
      if constexpr (std::is_arithmetic_v<DATA_T>) {
        double total = 0.0;
        for (const auto & entry : container) {
          total += (double) get_fun(entry);
        }
        return emp::to_string( total );
      }
      return std::string{"nan"};
    };
  }

  template <typename DATA_T, typename CONTAIN_T, typename FUN_T>
  auto BuildCollectFun_Entropy(FUN_T get_fun) {
    return [get_fun](const CONTAIN_T & container) {
      std::map<DATA_T, size_t> vals;
      for (const auto & entry : container) {
        vals[ get_fun(entry) ]++;
      }
      const size_t N = container.size();
      double entropy = 0.0;
      for (auto [entry, count] : vals) {
        double p = ((double) count) / (double) N;
        entropy -= p * log2(p);
      }
      return emp::to_string(entropy);
    };
  }

  template <typename DATA_T, typename CONTAIN_T, typename FUN_T>
  std::function<std::string(const CONTAIN_T &)>
  BuildCollectFun(std::string type, FUN_T get_fun) {
    // ### DEFAULT
    // If no trait function is specified, assume that we should use the first index.
    if (type == "") type = "0";

    // Return the index if a simple number was provided.
    if (emp::is_digits(type)) {
      size_t index = emp::from_string<size_t>(type);
      return emp::BuildCollectFun_Index<DATA_T, CONTAIN_T>(get_fun, index);
    }

    // Return the number of distinct values found in this trait.
    else if (type == "unique" || type == "richness") {
      return emp::BuildCollectFun_Unique<DATA_T, CONTAIN_T>(get_fun);
    }

    // Return the most common value found for this trait.
    else if (type == "mode" || type == "dom" || type == "dominant") {
      return emp::BuildCollectFun_Mode<DATA_T, CONTAIN_T>(get_fun);
    }

    // Return the lowest trait value.
    else if (type == "min") {
      return emp::BuildCollectFun_Min<DATA_T, CONTAIN_T>(get_fun);
    }

    // Return the highest trait value.
    else if (type == "max") {
      return emp::BuildCollectFun_Max<DATA_T, CONTAIN_T>(get_fun);
    }

    // Return the average trait value.
    else if (type == "ave" || type == "mean") {
      return emp::BuildCollectFun_Mean<DATA_T, CONTAIN_T>(get_fun);
    }

    // Return the middle-most trait value.
    //else if (type == "median") {
    //  return emp::BuildCollectFun_Median<DATA_T, CONTAIN_T>(get_fun);
    //}

    // Return the standard deviation of all trait values.
    else if (type == "variance") {
      return emp::BuildCollectFun_Variance<DATA_T, CONTAIN_T>(get_fun);
    }

    // Return the standard deviation of all trait values.
    else if (type == "stddev") {
      return emp::BuildCollectFun_StandardDeviation<DATA_T, CONTAIN_T>(get_fun);
    }

    // Return the total of all trait values.
    else if (type == "sum" || type=="total") {
      return emp::BuildCollectFun_Sum<DATA_T, CONTAIN_T>(get_fun);
    }

    // Return the entropy of values for this trait.
    else if (type == "entropy") {
      return emp::BuildCollectFun_Entropy<DATA_T, CONTAIN_T>(get_fun);
    }

    return std::function<std::string(const CONTAIN_T &)>();
  }

};

#endif

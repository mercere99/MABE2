/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020.
 *
 *  @file  data_collect.hpp
 *  @brief Functions to collect data from containers.
 *
 *  A collection of mechanisms to aggregate data from arbitrary objects in arbitrary containers.
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
  namespace DataCollect {

    // Return the value at a specified index.
    template <typename CONTAIN_T, typename FUN_T>
    std::string Index(const CONTAIN_T & container, FUN_T get_fun, const size_t index) {
      if (container.size() <= index) return "Nan"s;
      return emp::to_string( get_fun( container.At(index) ) );
    }


    // Count up the number of distinct values.
    template <typename DATA_T, typename CONTAIN_T, typename FUN_T>
    auto Unique(const CONTAIN_T & container, FUN_T get_fun) {
      std::unordered_set<DATA_T> vals;
      for (const auto & entry : container) {
        vals.insert( get_fun(entry) );
      }
      return emp::to_string(vals.size());
    }


    template <typename DATA_T, typename CONTAIN_T, typename FUN_T>
    auto Mode(const CONTAIN_T & container, FUN_T get_fun) {
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
    }

    template <typename DATA_T, typename CONTAIN_T, typename FUN_T>
    auto Min(const CONTAIN_T & container, FUN_T get_fun) {
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
    }

    template <typename DATA_T, typename CONTAIN_T, typename FUN_T>
    auto Max(const CONTAIN_T & container, FUN_T get_fun) {
      DATA_T max{};
      if constexpr (std::is_arithmetic_v<DATA_T>) {
        max = std::numeric_limits<DATA_T>::lowest();
      }
      for (const auto & entry : container) {
        const DATA_T cur_val = get_fun(entry);
        if (cur_val > max) max = cur_val;
      }
      return emp::to_string(max);
    }

    template <typename DATA_T, typename CONTAIN_T, typename FUN_T>
    auto MinID(const CONTAIN_T & container, FUN_T get_fun) {
      DATA_T min{};
      if constexpr (std::is_arithmetic_v<DATA_T>) {
        min = std::numeric_limits<DATA_T>::max();
      }
      else if constexpr (std::is_same_v<std::string, DATA_T>) {
        min = std::string('~',22);   // '~' is ascii char 126 (last printable one.)
      }
      size_t id = 0;
      size_t min_id = 0;
      for (const auto & entry : container) {
        const DATA_T cur_val = get_fun(entry);
        if (cur_val < min) { min = cur_val; min_id = id; }
        ++id;
      }
      return emp::to_string(min_id);
    }

    template <typename DATA_T, typename CONTAIN_T, typename FUN_T>
    auto MaxID(const CONTAIN_T & container, FUN_T get_fun) {
      DATA_T max{};
      if constexpr (std::is_arithmetic_v<DATA_T>) {
        max = std::numeric_limits<DATA_T>::lowest();
      }
      size_t id = 0;
      size_t max_id = 0;
      for (const auto & entry : container) {
        const DATA_T cur_val = get_fun(entry);
        if (cur_val > max) { max = cur_val; max_id = id; }
        ++id;
      }
      return emp::to_string(max_id);
    }

    template <typename DATA_T, typename CONTAIN_T, typename FUN_T>
    auto Mean(const CONTAIN_T & container, FUN_T get_fun) {
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
    }

    template <typename DATA_T, typename CONTAIN_T, typename FUN_T>
    auto Median(const CONTAIN_T & container, FUN_T get_fun) {
      emp::vector<DATA_T> values(container.size());
      size_t count = 0;
      for (const auto & entry : container) {
        values[count++] = get_fun(entry);
      }
      emp::Sort(values);
      return emp::to_string( values[count/2] );
    }

    template <typename DATA_T, typename CONTAIN_T, typename FUN_T>
    auto Variance(const CONTAIN_T & container, FUN_T get_fun) {
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
    }

    template <typename DATA_T, typename CONTAIN_T, typename FUN_T>
    auto StandardDeviation(const CONTAIN_T & container, FUN_T get_fun) {
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
    }

    template <typename DATA_T, typename CONTAIN_T, typename FUN_T>
    auto Sum(const CONTAIN_T & container, FUN_T get_fun) {
      if constexpr (std::is_arithmetic_v<DATA_T>) {
        double total = 0.0;
        for (const auto & entry : container) {
          total += (double) get_fun(entry);
        }
        return emp::to_string( total );
      }
      return std::string{"nan"};
    }

    template <typename DATA_T, typename CONTAIN_T, typename FUN_T>
    auto Entropy(const CONTAIN_T & container, FUN_T get_fun) {
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
    }
  } // End namespace DataCollect

  template <typename DATA_T, typename CONTAIN_T, typename FUN_T>
  std::function<std::string(const CONTAIN_T &)>
  BuildCollectFun(std::string action, FUN_T get_fun) {
    // ### DEFAULT
    // If no trait function is specified, assume that we should use the first index.
    if (action == "") action = "0";

    // Return the index if a simple number was provided.
    if (emp::is_digits(action)) {
      size_t index = emp::from_string<size_t>(action);
      return [get_fun,index](const CONTAIN_T & container) {
        return DataCollect::Index<CONTAIN_T>(container, get_fun, index);
      };
    }

    // Return the number of distinct values found in this trait.
    else if (action == "unique" || action == "richness") {
      return [get_fun](const CONTAIN_T & container) {
        return DataCollect::Unique<DATA_T, CONTAIN_T>(container, get_fun);
      };
    }

    // Return the most common value found for this trait.
    else if (action == "mode" || action == "dom" || action == "dominant") {
      return [get_fun](const CONTAIN_T & container) {
        return DataCollect::Mode<DATA_T, CONTAIN_T>(container, get_fun);
      };
    }

    // Return the lowest trait value.
    else if (action == "min") {
      return [get_fun](const CONTAIN_T & container) {
        return DataCollect::Min<DATA_T, CONTAIN_T>(container, get_fun);
      };
    }

    // Return the highest trait value.
    else if (action == "max") {
      return [get_fun](const CONTAIN_T & container) {
        return DataCollect::Max<DATA_T, CONTAIN_T>(container, get_fun);
      };
    }

    // Return the lowest trait value.
    else if (action == "min_id") {
      return [get_fun](const CONTAIN_T & container) {
        return DataCollect::MinID<DATA_T, CONTAIN_T>(container, get_fun);
      };
    }

    // Return the highest trait value.
    else if (action == "max_id") {
      return [get_fun](const CONTAIN_T & container) {
        return DataCollect::MaxID<DATA_T, CONTAIN_T>(container, get_fun);
      };
    }

    // Return the average trait value.
    else if (action == "ave" || action == "mean") {
      return [get_fun](const CONTAIN_T & container) {
        return DataCollect::Mean<DATA_T, CONTAIN_T>(container, get_fun);
      };
    }

    // Return the middle-most trait value.
    else if (action == "median") {
      return [get_fun](const CONTAIN_T & container) {
        return DataCollect::Median<DATA_T, CONTAIN_T>(container, get_fun);
      };
    }

    // Return the standard deviation of all trait values.
    else if (action == "variance") {
      return [get_fun](const CONTAIN_T & container) {
        return DataCollect::Variance<DATA_T, CONTAIN_T>(container, get_fun);
      };
    }

    // Return the standard deviation of all trait values.
    else if (action == "stddev") {
      return [get_fun](const CONTAIN_T & container) {
        return DataCollect::StandardDeviation<DATA_T, CONTAIN_T>(container, get_fun);
      };
    }

    // Return the total of all trait values.
    else if (action == "sum" || action == "total") {
      return [get_fun](const CONTAIN_T & container) {
        return DataCollect::Sum<DATA_T, CONTAIN_T>(container, get_fun);
      };
    }

    // Return the entropy of values for this trait.
    else if (action == "entropy") {
      return [get_fun](const CONTAIN_T & container) {
        return DataCollect::Entropy<DATA_T, CONTAIN_T>(container, get_fun);
      };
    }

    return std::function<std::string(const CONTAIN_T &)>();
  }

}

#endif

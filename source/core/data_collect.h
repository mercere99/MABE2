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
        max = std::numeric_limits<DATA_T>::min();
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

  template <typename DATA_T, typename CONTAIN_T, typename FUN_T>
  auto BuildCollectFun_Median(FUN_T get_fun) {
    return [get_fun](const CONTAIN_T & container) {
      emp::vector<DATA_T> values(container.size());
      size_t count = 0;
      for (const auto & entry : container) {
        values[count++] = get_fun(entry);
      }
      emp::Sort(values);
      return emp::to_string( values[count/2] );
    };
  }

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



};

#endif
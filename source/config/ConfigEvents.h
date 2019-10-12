/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  ConfigEvents.h
 *  @brief Manages events for configurations.
 *  @note Status: ALPHA
 */

#ifndef MABE_CONFIG_EVENTS_H
#define MABE_CONFIG_EVENTS_H

#include <set>
#include <string>

#include "base/Ptr.h"

#include "ConfigAST.h"

namespace mabe {

  struct TimedEvent {
    size_t id = 0;                 ///< A unique ID for this event.
    emp::Ptr<ASTNode> ast_action;  ///< Parse tree to exectute when triggered.
    double next = 0.0;             ///< When should we start triggering this event.
    double repeat = 0.0;           ///< How often should it repeat (0.0 for no repeat)
    double max = -1.0;             ///< Maximum value that this value can reach (neg for no max)
    bool multi_ok = true;          ///< If multiple repeats are past, should all be triggered?

    bool operator==(const TimedEvent & in) { return id == in.id; }
    bool operator!=(const TimedEvent & in) { return id != in.id; }
    bool operator<(const TimedEvent & in) {
      return (next == in.next) ? (id < in.id) : (next < in.next);
    }
  };

  class EventQueue {
  private:
    std::set<TimedEvent> queue;
    double last_value = 0.0;
    size_t next_id = 1;
  public:
    void AddEvent(emp::Ptr<ASTNode> ast_action,
                  double first=0.0, double repeat=0.0, double max=-1.0, bool multi_ok=true) {
      queue.emplace({next_id++, ast_action, first, repeat, max, multi_ok});
    }
  };


}

#endif

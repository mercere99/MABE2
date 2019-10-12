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

#include <string>

#include "base/map.h"
#include "base/Ptr.h"

#include "ConfigAST.h"

namespace mabe {

  class ConfigEvents {
  private:

    struct TimedEvent {
      size_t id = 0;                 ///< A unique ID for this event.
      emp::Ptr<ASTNode> ast_action;  ///< Parse tree to exectute when triggered.
      double next = 0.0;             ///< When should we start triggering this event.
      double repeat = 0.0;           ///< How often should it repeat (0.0 for no repeat)
      double max = -1.0;             ///< Maximum value that this value can reach (neg for no max)
      bool active = true;            ///< Is this event still active?

      TimedEvent(size_t _id, emp::Ptr<ASTNode> _node,
                double _next, double _repeat, double _max)
        : id(_id), ast_action(_node), next(_next), repeat(_repeat), max(_max), active(next <= max)
      { ; }
      ~TimedEvent() { ast_action.Delete(); }

      bool Trigger() {
        ast_action->Process();
        next += repeat;

        // Return "active" if we ARE repeating and the next time is stiil within range.
        return (repeat != 0.0 && next <= max);
      }
    };

    emp::multimap<double, emp::Ptr<TimedEvent>> queue;
    double cur_value = 0.0;
    size_t next_id = 1;

    // -- Helper functions. --
    void AddEvent(emp::Ptr<TimedEvent> in_event) {
      queue.insert({in_event->next, in_event});
    }

    emp::Ptr<TimedEvent> PopEvent() {
      emp::Ptr<TimedEvent> out_event = queue.begin()->second;
      queue.erase(queue.begin());
      return out_event;
    }

  public:
    ConfigEvents() { ; }
    ~ConfigEvents() {
      // Must delete all AST nodes in the queue.
      for (auto [time, event_ptr] : queue) {
        event_ptr.Delete();
      }
    }

    bool AddEvent(emp::Ptr<ASTNode> action, double first=0.0, double repeat=0.0, double max=-1.0) {
      emp_assert(first >= 0.0, first);
      emp_assert(repeat >= 0.0, repeat);

      // Skip all events before the current time.
      if (first < cur_value) {
        if (repeat == 0.0) return false;      // If no repeat, we simply missed this one.
        double offset = cur_value - first;    // Figure out how far we need to advance this event.
        double steps = ceil(offset / repeat); // How many steps through repeat will this be?
        first += repeat * steps;              // Fast-forward!
      }

      // If we are already after max time, this event cannot be triggered.
      if (first > max) return false;

      AddEvent( emp::NewPtr<TimedEvent>(next_id++, action, first, repeat, max) );

      return true;
    }

    void UpdateValue(size_t in_value) {
      while (queue.size() && queue.begin()->first <= in_value) {
        emp::Ptr<TimedEvent> cur_event = PopEvent();
        bool do_repeat = cur_event->Trigger();
        if (do_repeat) AddEvent(cur_event);
      }
      cur_value = in_value;
    }
  };


}

#endif

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

  struct TimedEvent {
    size_t id = 0;                 ///< A unique ID for this event.
    emp::Ptr<ASTNode> ast_action;  ///< Parse tree to exectute when triggered.
    double next = 0.0;             ///< When should we start triggering this event.
    double repeat = 0.0;           ///< How often should it repeat (0.0 for no repeat)
    double max = -1.0;             ///< Maximum value that this value can reach (neg for no max)
    bool active = true;            ///< Is thie event still active?

    TimedEvent(size_t _id, emp::Ptr<ASTNode> _node,
               double _next, double _repeat, double _max)
      : id(_id), ast_action(_node), next(_next), repeat(_repeat), max(_max), active(next <= max)
    { ; }
    TimedEvent(const TimedEvent &) = default;
    TimedEvent(TimedEvent &&) = default;
    ~TimedEvent() { ; }

    TimedEvent & operator=(const TimedEvent &) = default;
    TimedEvent & operator=(TimedEvent &&) = default;

    double GetNext() const { return next; }
    double GetRepeat() const { return repeat; }
    double GetMax() const { return max; }

    bool IsActive() const { return active; }

    bool Trigger() {
      if (active) {
        ast_action->Process();
        next += repeat;
        if (repeat == 0.0 || next > max) active = false;
      }
      return active;
    }

    void DeleteAST() { ast_action.Delete(); ast_action = nullptr; }

    bool operator==(const TimedEvent & in) const { return id == in.id; }
    bool operator!=(const TimedEvent & in) const { return id != in.id; }
    bool operator<(const TimedEvent & in) const {
      return (next == in.next) ? (id < in.id) : (next < in.next);
    }
  };

  class ConfigEvents {
  private:
    emp::multimap<double, emp::Ptr<TimedEvent>> queue;
    double last_value = 0.0;
    size_t next_id = 1;
  public:
    ConfigEvents() { ; }
    ~ConfigEvents() {
      // Must delete all AST nodes in the queue.
      for (auto [time, event_ptr] : queue) {
        event_ptr->DeleteAST();
        event_ptr.Delete();
      }
    }

    void AddEvent(emp::Ptr<TimedEvent> in_event) {
      queue.insert({in_event->GetNext(), in_event});
    }

    void AddEvent(emp::Ptr<ASTNode> action, double first=0.0, double repeat=0.0, double max=-1.0) {
      AddEvent( emp::NewPtr<TimedEvent>(next_id++, action, first, repeat, max) );
    }

    void UpdateValue(size_t in_value) {
      while (queue.size() && queue.begin()->first <= in_value) {
        auto it = queue.begin();
        emp::Ptr<TimedEvent> cur_event = it->second;
        queue.erase(it);
        cur_event->Trigger();
        AddEvent(cur_event);
      }
      last_value = in_value;
    }
  };


}

#endif

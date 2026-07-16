#include "hsm.hpp"
#include "Display.h"
#include "config.h"

#include "esp_log.h"
#include "freertos/task.h"
#include <cstring>

static const char *STATE_MACHINE_TAG{"HSM"};

// Must match your Display geometry (used to center the leaf name).

// How long each low-level function stays on screen.
static const int STATE_MACHINE_STEP_MS{800};

// ---------------------------------------------------------------------------
//  Hierarchy table
//  One row per real state, indexed by the State enum. Every state knows its
//  parent. Superstates additionally declare which child is entered first
//  (`initial`); leaf states declare where to go when they finish (`next`).
// ---------------------------------------------------------------------------
struct StateNode {
  const char *name; // shown on the LCD for leaves; log-only for superstates
  State parent;     // State::NONE for top-level states
  State initial;    // superstate: first child to enter; State::NONE for leaves
  State next;       // leaf: state to transition to when done; State::NONE super
  bool isLeaf;
};

struct NodeTable {
  StateNode data[static_cast<size_t>(State::COUNT)];

  constexpr const StateNode &operator[](State s) const {
    return data[static_cast<size_t>(s)];
  }
};

static constexpr NodeTable NODES = {{
    /*  { name,           Parent State,              Initial State,        Next
       State,               isLeaf }, */
    {"Idle", State::NONE, State::NONE, State::SCANNING, true},
    {"Scanning", State::NONE, State::MOVE, State::NONE, false},
    {"Move", State::SCANNING, State::NONE, State::SCAN, true},
    {"Scan", State::SCANNING, State::NONE, State::COMPARE, true},
    {"Compare", State::SCANNING, State::NONE, State::SIGNAL_PROCESSING, true},
    {"Jamming", State::NONE, State::JAM, State::NONE, false},
    {"Jam", State::JAMMING, State::NONE, State::IDLE, true},
    {"Sig Proc", State::NONE, State::FILTER_NOISE, State::NONE, false},
    {"Filter Noise", State::SIGNAL_PROCESSING, State::NONE, State::DEMODULATE,
     true},
    {"Demodulate", State::SIGNAL_PROCESSING, State::NONE, State::DECODE, true},
    {"Decode", State::SIGNAL_PROCESSING, State::NONE, State::JAMMING, true},
}};

/* Runtime */
static Display *display_{nullptr};
static State current_{State::IDLE};   // always points at a *leaf*
static State lastSuper_{State::NONE}; // last top-level box we were in

// Walk a (possibly super) state down to the leaf that should actually run.
static State resolveLeaf(State s) {
  while (s != State::NONE && !NODES[s].isLeaf) {
    s = NODES[s].initial;
  }
  return s;
}

// Walk up to the top-level ancestor (the "box" in the diagram).
static State topSuper(State s) {
  while (s != State::NONE && NODES[s].parent != State::NONE) {
    s = NODES[s].parent;
  }
  return s;
}

// Print a leaf name centered on the top row. Only leaves call this.
static void showLeaf(const char *name) {
  display_->clear();
  int len = static_cast<int>(strlen(name));
  int col = (LCD_COLS - len) / 2;
  if (col < 0)
    col = 0;
  display_->setCursor(col, 0);
  display_->write(name);
}

// Conditional transitions. By default every leaf follows its table `next`,
// but states that depend on sensor input can branch here instead. This is
// where the hierarchy pays off: return a *superstate* and it auto-resolves
// to that superstate's initial leaf.
static State decide(State finishedLeaf, State defaultNext) {
  switch (finishedLeaf) {
  case State::COMPARE:
    // TODO: read your real detector here, e.g.
    //   if (signalDetected) return State::SIGNAL_PROCESSING;
    //   if (threatDetected) return State::JAMMING;
    //   return State::SCANNING;   // nothing found -> keep scanning (Move)
    return defaultNext;

  default:
    return defaultNext;
  }
}

void hsm_init(Display &display) {
  display_ = &display;
  current_ = State::IDLE;
  lastSuper_ = State::NONE;
}

void hsm_run() {
  const State finished = current_;
  const StateNode &node = NODES[finished];

  // Announce superstate changes on the serial log (never on the LCD).
  State super = topSuper(finished);
  if (super != lastSuper_) {
    ESP_LOGI(STATE_MACHINE_TAG, ">> entering superstate: %s",
             NODES[super].name);
    lastSuper_ = super;
  }

  // Only the low-level leaf functions print to the screen.
  showLeaf(node.name);
  ESP_LOGI(STATE_MACHINE_TAG, "   run leaf: %s", node.name);

  vTaskDelay(pdMS_TO_TICKS(STATE_MACHINE_STEP_MS));

  // Advance: pick the next state, then resolve any superstate to its leaf.
  current_ = resolveLeaf(decide(finished, node.next));
}

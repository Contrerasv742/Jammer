#pragma once
#include <cstdint>

// Forward declaration so the header stays lightweight.
class Display;

// ---------------------------------------------------------------------------
//  Hierarchical State Machine
//
//  Top-level "superstates" (Idle, Scanning, Jamming, Signal Processing) group
//  the low-level leaf functions. Only leaf states ever draw to the LCD --
//  superstate changes are logged to the serial console instead. This mirrors
//  the boxes in your draw.io diagram: Scanning -> {Move, Scan, Compare}, etc.
// ---------------------------------------------------------------------------
enum class State : uint8_t {
    // -- top-level state (leaf: no children) --
    IDLE = 0,

    // -- Scanning superstate + its leaves --
    SCANNING,          // superstate
    MOVE,
    SCAN,
    COMPARE,

    // -- Jamming superstate + its leaf --
    JAMMING,           // superstate
    JAM,

    // -- Signal Processing superstate + its leaves --
    SIGNAL_PROCESSING, // superstate
    FILTER_NOISE,
    DEMODULATE,
    DECODE,

    COUNT,             // number of real states (keep last of the reals)
    NONE = 0xFF        // sentinel for "no parent / no substate"
};

// Bind the state machine to your Display instance.
void sm_init(Display& display);

// Run exactly one leaf: draw it, wait, then advance to the next state.
// Call this repeatedly from your main loop.
void sm_run();

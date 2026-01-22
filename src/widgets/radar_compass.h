// Radar Compass Widget
// Provides a 2D radar/compass display with distance rings, cardinal directions,
// FOV wedge, and celestial indicators (sun/moon)
//
// This module renders a top-down compass view that rotates based on platform
// azimuth. The FOV wedge always points up (showing where the camera is aimed),
// while cardinal directions (N, E, S, W) rotate around the compass.

#ifndef WIDGETS_RADAR_COMPASS_H
#define WIDGETS_RADAR_COMPASS_H

// ════════════════════════════════════════════════════════════
// WIDGET INCLUDES
// ════════════════════════════════════════════════════════════
#include "config/osd_config.h"
#include "core/osd_context.h"

#include <stdbool.h>

// Forward declare state type (implementation uses osd_state.h accessors)
typedef struct _ser_JonGUIState osd_state_t;

// ════════════════════════════════════════════════════════════
// RADAR COMPASS WIDGET API
// ════════════════════════════════════════════════════════════

// Initialize radar compass widget
//
// Loads fonts for labels and SVGs for celestial indicators.
//
// Parameters:
//   ctx:    OSD context
//   config: Radar compass configuration from JSON
//   celestial_config: Celestial indicators configuration from JSON
//
// Notes:
//   - Loads fonts from resources/fonts/
//   - Loads celestial SVGs from resources/radar_indicators/
//
// Example:
//   radar_compass_config_t config = {
//     .enabled = true,
//     .position_x = 810,
//     .position_y = 730,
//     .size = 300,
//     .num_rings = 3,
//     .ring_distances = {1.0, 5.0, 20.0},
//     ...
//   };
//   radar_compass_init(ctx, &config, &celestial_config);
bool radar_compass_init(osd_context_t *ctx,
                        const radar_compass_config_t *config,
                        const celestial_indicators_config_t *celestial_config);

// Render radar compass widget
//
// Renders the compass at the configured screen position with rotation
// based on platform azimuth from the proto message. FOV wedge angle
// comes from camera FOV data in the proto message.
//
// Parameters:
//   ctx:      OSD context (contains framebuffer and radar compass state)
//   pb_state: Telemetry state containing compass and camera data
//
// Returns:
//   true if compass was rendered, false if disabled
//
// Rendering Process:
//   1. Extract platform azimuth from state
//   2. Extract camera FOV from state (day or thermal based on variant)
//   3. Draw concentric distance rings
//   4. Draw ring distance labels (if enabled)
//   5. Draw rotated cardinal direction labels (N, E, S, W)
//   6. Draw FOV wedge pointing up
//   7. Calculate and draw celestial indicators (sun, moon)
//
// Notes:
//   - Compass rotates so north moves; FOV wedge always points up
//   - Celestial bodies positioned by azimuth on compass edge
//   - Celestial body size/opacity varies by altitude
bool radar_compass_render(osd_context_t *ctx, osd_state_t *pb_state);

// Cleanup radar compass resources
//
// Frees allocated fonts and SVG resources.
//
// Parameters:
//   ctx: OSD context
void radar_compass_cleanup(osd_context_t *ctx);

#endif // WIDGETS_RADAR_COMPASS_H

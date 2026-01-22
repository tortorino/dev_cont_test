// OSD Context Structure
// Core data structure passed to all widgets for rendering
//
// This header is the ONLY header widgets need to access the OSD context.
// It deliberately excludes WASM-specific details to keep widget code clean.

#ifndef CORE_OSD_CONTEXT_H
#define CORE_OSD_CONTEXT_H

#include "../config/osd_config.h"
#include "../resources/font.h"
#include "../resources/svg.h"
#include "framebuffer.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// ════════════════════════════════════════════════════════════
// OSD CONTEXT
// ════════════════════════════════════════════════════════════
//
// The osd_context_t structure contains everything a widget needs:
//   - Framebuffer to render into
//   - Configuration (colors, positions, sizes)
//   - Pre-loaded resources (fonts, SVGs)
//   - Render state (frame count)
//
// Widgets should NOT modify context fields directly except through
// the provided helper functions.

typedef struct
{
  // ──────────────────────────────────────────────────────────
  // FRAMEBUFFER (render target)
  // ──────────────────────────────────────────────────────────
  uint32_t *framebuffer;
  uint32_t width;
  uint32_t height;

  // ──────────────────────────────────────────────────────────
  // CONFIGURATION (loaded from JSON at init)
  // ──────────────────────────────────────────────────────────
  osd_config_t config;

  // ──────────────────────────────────────────────────────────
  // RESOURCES (pre-loaded at init)
  // ──────────────────────────────────────────────────────────
  // Per-widget fonts (each widget can have its own font)
  font_resource_t font_timestamp;
  font_resource_t font_speed_indicators;
  font_resource_t font_variant_info;

  svg_resource_t cross_svg;  // Crosshair SVG icon
  svg_resource_t circle_svg; // Circle SVG icon

  // ──────────────────────────────────────────────────────────
  // INTERNAL STATE (managed by framework - widgets read-only)
  // ──────────────────────────────────────────────────────────

  // Proto buffer (internal - use osd_state.h accessors instead)
  uint8_t proto_buffer[4096];
  size_t proto_size;
  bool proto_valid;

  // Radar compass state
  bool radar_compass_enabled;
  int radar_compass_x;
  int radar_compass_y;
  int radar_compass_size;

  // Distance rings
  int radar_compass_num_rings;
  float radar_compass_ring_distances[RADAR_COMPASS_MAX_RINGS];
  uint32_t radar_compass_ring_color;
  float radar_compass_ring_thickness;
  bool radar_compass_show_ring_labels;
  int radar_compass_ring_label_font_size;
  font_resource_t font_radar_compass_ring_labels;

  // Cardinal directions
  uint32_t radar_compass_cardinal_color;
  int radar_compass_cardinal_font_size;
  font_resource_t font_radar_compass_cardinals;

  // FOV wedge
  uint32_t radar_compass_fov_fill_color;
  uint32_t radar_compass_fov_outline_color;
  float radar_compass_fov_outline_thickness;

  // Celestial indicators (sun and moon on radar compass)
  bool celestial_enabled;
  bool celestial_show_sun;
  bool celestial_show_moon;
  float celestial_indicator_scale;
  float celestial_visibility_threshold;
  svg_resource_t celestial_sun_svg;
  svg_resource_t celestial_moon_svg;

  // Rendering state
  bool needs_render;
  uint32_t frame_count;
} osd_context_t;

// ════════════════════════════════════════════════════════════
// CONTEXT HELPER FUNCTIONS
// ════════════════════════════════════════════════════════════

// Convert OSD context to framebuffer view (for rendering primitives)
//
// Usage:
//   framebuffer_t fb = osd_ctx_get_framebuffer(ctx);
//   draw_line(&fb, x0, y0, x1, y1, color, thickness);
static inline framebuffer_t
osd_ctx_get_framebuffer(osd_context_t *ctx)
{
  framebuffer_t fb;
  framebuffer_init(&fb, ctx->framebuffer, ctx->width, ctx->height);
  return fb;
}

// Get screen center coordinates
static inline void
osd_ctx_get_center(const osd_context_t *ctx, int *cx, int *cy)
{
  *cx = (int)ctx->width / 2;
  *cy = (int)ctx->height / 2;
}

#endif // CORE_OSD_CONTEXT_H

// Radar Compass Widget Implementation
// 2D top-down compass with distance rings, cardinal directions,
// FOV wedge, and celestial indicators

#include "radar_compass.h"

#include "core/framebuffer.h"
#include "osd_state.h"
#include "rendering/blending.h"
#include "rendering/primitives.h"
#include "rendering/text.h"
#include "resources/font.h"
#include "resources/svg.h"
#include "utils/celestial_position.h"
#include "utils/logging.h"
#include "utils/math_decl.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ════════════════════════════════════════════════════════════
// INTERNAL HELPERS
// ════════════════════════════════════════════════════════════

// Convert degrees to radians (float version)
static inline float
deg_to_radf(float degrees)
{
  return degrees * (float)(M_PI / 180.0);
}

// ════════════════════════════════════════════════════════════
// INITIALIZATION
// ════════════════════════════════════════════════════════════

bool
radar_compass_init(osd_context_t *ctx,
                   const radar_compass_config_t *config,
                   const celestial_indicators_config_t *celestial_config)
{
  if (!ctx || !config)
    {
      LOG_ERROR("radar_compass_init: NULL context or config");
      return false;
    }

  // Store basic config
  ctx->radar_compass_enabled = config->enabled;
  ctx->radar_compass_x       = config->position_x;
  ctx->radar_compass_y       = config->position_y;
  ctx->radar_compass_size    = config->size;

  // Store ring config
  ctx->radar_compass_num_rings = config->num_rings;
  if (ctx->radar_compass_num_rings > RADAR_COMPASS_MAX_RINGS)
    ctx->radar_compass_num_rings = RADAR_COMPASS_MAX_RINGS;

  for (int i = 0; i < ctx->radar_compass_num_rings; i++)
    {
      ctx->radar_compass_ring_distances[i] = config->ring_distances[i];
    }

  ctx->radar_compass_ring_color           = config->ring_color;
  ctx->radar_compass_ring_thickness       = config->ring_thickness;
  ctx->radar_compass_show_ring_labels     = config->show_ring_labels;
  ctx->radar_compass_ring_label_font_size = config->ring_label_font_size;

  // Store cardinal config
  ctx->radar_compass_cardinal_color     = config->cardinal_color;
  ctx->radar_compass_cardinal_font_size = config->cardinal_font_size;

  // Store FOV wedge config
  ctx->radar_compass_fov_fill_color        = config->fov_fill_color;
  ctx->radar_compass_fov_outline_color     = config->fov_outline_color;
  ctx->radar_compass_fov_outline_thickness = config->fov_outline_thickness;

  // Load ring label font
  if (config->ring_label_font_path[0] != '\0')
    {
      if (!font_load(&ctx->font_radar_compass_ring_labels,
                     config->ring_label_font_path))
        {
          LOG_WARN("radar_compass_init: Failed to load ring label font: %s",
                   config->ring_label_font_path);
        }
    }

  // Load cardinal font
  if (config->cardinal_font_path[0] != '\0')
    {
      if (!font_load(&ctx->font_radar_compass_cardinals,
                     config->cardinal_font_path))
        {
          LOG_WARN("radar_compass_init: Failed to load cardinal font: %s",
                   config->cardinal_font_path);
        }
    }

  // Store celestial config
  if (celestial_config)
    {
      ctx->celestial_enabled         = celestial_config->enabled;
      ctx->celestial_show_sun        = celestial_config->show_sun;
      ctx->celestial_show_moon       = celestial_config->show_moon;
      ctx->celestial_indicator_scale = celestial_config->indicator_scale;
      ctx->celestial_visibility_threshold
        = celestial_config->visibility_threshold;

      // Load celestial SVGs
      if (celestial_config->sun_svg_path[0] != '\0')
        {
          if (!svg_load(&ctx->celestial_sun_svg,
                        celestial_config->sun_svg_path))
            {
              LOG_WARN("radar_compass_init: Failed to load sun SVG: %s",
                       celestial_config->sun_svg_path);
            }
        }

      if (celestial_config->moon_svg_path[0] != '\0')
        {
          if (!svg_load(&ctx->celestial_moon_svg,
                        celestial_config->moon_svg_path))
            {
              LOG_WARN("radar_compass_init: Failed to load moon SVG: %s",
                       celestial_config->moon_svg_path);
            }
        }

      // Initialize celestial calculation engine
      celestial_init();
    }
  else
    {
      ctx->celestial_enabled = false;
    }

  LOG_INFO("radar_compass_init: Initialized at (%d, %d) size=%d rings=%d",
           ctx->radar_compass_x, ctx->radar_compass_y, ctx->radar_compass_size,
           ctx->radar_compass_num_rings);

  return true;
}

// ════════════════════════════════════════════════════════════
// RENDERING
// ════════════════════════════════════════════════════════════

// Perspective scale factor: cos(45°) ≈ 0.707
// This creates the effect of viewing the compass from 45 degrees above
#define PERSPECTIVE_SCALE 0.5f

// Draw distance rings (elliptical for perspective view)
static void
draw_rings(framebuffer_t *fb,
           osd_context_t *ctx,
           int cx,
           int cy,
           float radius_x,
           float radius_y)
{
  if (ctx->radar_compass_num_rings <= 0)
    return;

  // Find the maximum distance (outermost ring)
  float max_distance = ctx->radar_compass_ring_distances[0];
  for (int i = 1; i < ctx->radar_compass_num_rings; i++)
    {
      if (ctx->radar_compass_ring_distances[i] > max_distance)
        max_distance = ctx->radar_compass_ring_distances[i];
    }

  // Draw each ring as an ellipse
  for (int i = 0; i < ctx->radar_compass_num_rings; i++)
    {
      float distance = ctx->radar_compass_ring_distances[i];
      float scale    = distance / max_distance;
      float ring_rx  = radius_x * scale;
      float ring_ry  = radius_y * scale;

      draw_ellipse_outline(fb, cx, cy, ring_rx, ring_ry,
                           ctx->radar_compass_ring_color,
                           ctx->radar_compass_ring_thickness);
    }
}

// Draw ring distance labels (positioned on ellipse edge)
static void
draw_ring_labels(framebuffer_t *fb,
                 osd_context_t *ctx,
                 int cx,
                 int cy,
                 float radius_x,
                 float radius_y,
                 float rotation_deg)
{
  if (!ctx->radar_compass_show_ring_labels)
    return;
  if (ctx->radar_compass_num_rings <= 0)
    return;
  if (!ctx->font_radar_compass_ring_labels.data)
    return;

  // Find max distance for scaling
  float max_distance = ctx->radar_compass_ring_distances[0];
  for (int i = 1; i < ctx->radar_compass_num_rings; i++)
    {
      if (ctx->radar_compass_ring_distances[i] > max_distance)
        max_distance = ctx->radar_compass_ring_distances[i];
    }

  // Draw labels at each ring, positioned to the right (90° from up)
  for (int i = 0; i < ctx->radar_compass_num_rings; i++)
    {
      float distance = ctx->radar_compass_ring_distances[i];
      float scale    = distance / max_distance;
      float ring_rx  = radius_x * scale;
      float ring_ry  = radius_y * scale;

      // Position label to the right of ring (90° in compass coords)
      float label_angle_deg = 90.0f; // Right side, fixed position
      float label_angle_rad = deg_to_radf(label_angle_deg);

      // Position on ellipse edge
      int label_x = cx + (int)(ring_rx * sinf(label_angle_rad));
      int label_y = cy - (int)(ring_ry * cosf(label_angle_rad));

      // Format label
      char label[16];
      if (distance >= 1.0f)
        snprintf(label, sizeof(label), "%.0fkm", distance);
      else
        snprintf(label, sizeof(label), "%.0fm", distance * 1000.0f);

      // Measure text for centering
      int text_width
        = text_measure_width(&ctx->font_radar_compass_ring_labels, label,
                             ctx->radar_compass_ring_label_font_size);

      // Draw label (offset slightly from ring)
      text_render_with_outline(fb, &ctx->font_radar_compass_ring_labels, label,
                               label_x - text_width / 2 + 5,
                               label_y
                                 - ctx->radar_compass_ring_label_font_size / 2,
                               ctx->radar_compass_ring_color,
                               0xFF000000, // Black outline
                               ctx->radar_compass_ring_label_font_size, 1);
    }
}

// Draw cardinal direction labels (N, E, S, W) positioned on ellipse
static void
draw_cardinals(framebuffer_t *fb,
               osd_context_t *ctx,
               int cx,
               int cy,
               float radius_x,
               float radius_y,
               float rotation_deg)
{
  if (!ctx->font_radar_compass_cardinals.data)
    return;

  // Cardinal directions: N=0°, E=90°, S=180°, W=270° (compass convention)
  static const char *cardinals[]       = { "N", "E", "S", "W" };
  static const float cardinal_angles[] = { 0.0f, 90.0f, 180.0f, 270.0f };

  // Label offset from edge (inward, 85% of radii)
  float offset_x = radius_x * 0.85f;
  float offset_y = radius_y * 0.85f;

  for (int i = 0; i < 4; i++)
    {
      // Apply rotation to cardinal angle
      float angle_deg = cardinal_angles[i] + rotation_deg;
      float angle_rad = deg_to_radf(angle_deg);

      // Calculate position on ellipse (compass convention: 0=up, clockwise)
      int label_x = cx + (int)(offset_x * sinf(angle_rad));
      int label_y = cy - (int)(offset_y * cosf(angle_rad));

      // Measure text for centering
      int text_width
        = text_measure_width(&ctx->font_radar_compass_cardinals, cardinals[i],
                             ctx->radar_compass_cardinal_font_size);

      // Center the text
      int text_x = label_x - text_width / 2;
      int text_y = label_y - ctx->radar_compass_cardinal_font_size / 2;

      // Draw with outline for visibility
      text_render_with_outline(fb, &ctx->font_radar_compass_cardinals,
                               cardinals[i], text_x, text_y,
                               ctx->radar_compass_cardinal_color,
                               0xFF000000, // Black outline
                               ctx->radar_compass_cardinal_font_size, 1);
    }
}

// Draw FOV wedge (elliptical for perspective view)
static void
draw_fov_wedge(framebuffer_t *fb,
               osd_context_t *ctx,
               int cx,
               int cy,
               float radius_x,
               float radius_y,
               float fov_angle_deg)
{
  if (fov_angle_deg <= 0.0f)
    return;

  // FOV wedge always points up (north in screen coords)
  // Start angle and end angle centered on up direction (0° in our compass
  // convention)
  float half_fov    = fov_angle_deg / 2.0f;
  float start_angle = -half_fov; // Left edge of wedge
  float end_angle   = half_fov;  // Right edge of wedge

  // Draw filled elliptical wedge (semi-transparent)
  draw_ellipse_wedge_filled(fb, cx, cy, radius_x, radius_y, start_angle,
                            end_angle, ctx->radar_compass_fov_fill_color);

  // Draw elliptical outline
  draw_ellipse_wedge_outline(fb, cx, cy, radius_x, radius_y, start_angle,
                             end_angle, ctx->radar_compass_fov_outline_color,
                             ctx->radar_compass_fov_outline_thickness);
}

// Draw celestial indicator (sun or moon) positioned on ellipse edge
static void
draw_celestial_indicator(framebuffer_t *fb,
                         osd_context_t *ctx,
                         svg_resource_t *svg,
                         int cx,
                         int cy,
                         float radius_x,
                         float radius_y,
                         float rotation_deg,
                         double body_azimuth,
                         double body_altitude)
{
  if (!svg->image)
    return;

  // Calculate position on ellipse edge
  // Body azimuth is absolute (0=north), so we apply rotation to position it
  // relative to the rotated compass
  float relative_azimuth_deg = (float)body_azimuth + rotation_deg;
  float angle_rad            = deg_to_radf(relative_azimuth_deg);

  // Position on edge of ellipse (90% of radii)
  int pos_x = cx + (int)(radius_x * 0.9f * sinf(angle_rad));
  int pos_y = cy - (int)(radius_y * 0.9f * cosf(angle_rad));

  // Calculate size and alpha based on altitude
  // Higher altitude = larger and brighter
  // Below horizon = smaller and fainter
  float scale;
  float alpha;

  if (body_altitude > 0)
    {
      // Above horizon: scale from 1.0 at horizon to 1.5 at zenith
      scale = 1.0f + (float)(body_altitude / 90.0) * 0.5f;
      alpha = 1.0f;
    }
  else
    {
      // Below horizon: scale from 0.7 at horizon to 0.4 at nadir
      // Alpha from 0.5 at horizon to 0.2 at nadir
      scale = 0.7f + (float)(body_altitude / 90.0) * 0.3f;
      alpha = 0.5f + (float)(body_altitude / 90.0) * 0.3f;
      if (alpha < 0.2f)
        alpha = 0.2f;
    }

  // Apply configured scale
  scale *= ctx->celestial_indicator_scale;

  // Base indicator size (proportional to compass size)
  int base_size      = ctx->radar_compass_size / 8;
  int indicator_size = (int)(base_size * scale);
  if (indicator_size < 8)
    indicator_size = 8;

  // Draw centered on position
  int render_x = pos_x - indicator_size / 2;
  int render_y = pos_y - indicator_size / 2;

  svg_render_with_alpha(fb, svg, render_x, render_y, indicator_size,
                        indicator_size, alpha);
}

// Draw celestial indicators (sun and moon) positioned on ellipse
static void
draw_celestial_indicators(framebuffer_t *fb,
                          osd_context_t *ctx,
                          osd_state_t *state,
                          int cx,
                          int cy,
                          float radius_x,
                          float radius_y,
                          float rotation_deg)
{
  if (!ctx->celestial_enabled)
    return;

  // Get GPS position for celestial calculations
  osd_gps_position_t gps;
  if (!osd_state_get_gps(state, &gps) || !gps.valid)
    return;

  // Get timestamp
  int64_t timestamp = gps.timestamp;
  if (timestamp == 0)
    timestamp = osd_state_get_timestamp(state);
  if (timestamp == 0)
    return;

  // Create observer location
  observer_location_t observer = { .latitude  = gps.latitude,
                                   .longitude = gps.longitude,
                                   .altitude  = gps.altitude };

  // Calculate celestial positions
  celestial_positions_t positions = celestial_calculate(timestamp, observer);

  // Draw sun indicator
  if (ctx->celestial_show_sun && positions.sun.valid)
    {
      if (positions.sun.altitude >= ctx->celestial_visibility_threshold)
        {
          draw_celestial_indicator(
            fb, ctx, &ctx->celestial_sun_svg, cx, cy, radius_x, radius_y,
            rotation_deg, positions.sun.azimuth, positions.sun.altitude);
        }
    }

  // Draw moon indicator
  if (ctx->celestial_show_moon && positions.moon.valid)
    {
      if (positions.moon.altitude >= ctx->celestial_visibility_threshold)
        {
          draw_celestial_indicator(
            fb, ctx, &ctx->celestial_moon_svg, cx, cy, radius_x, radius_y,
            rotation_deg, positions.moon.azimuth, positions.moon.altitude);
        }
    }
}

bool
radar_compass_render(osd_context_t *ctx, osd_state_t *pb_state)
{
  if (!ctx || !ctx->radar_compass_enabled)
    return false;

  // Get framebuffer
  framebuffer_t fb = osd_ctx_get_framebuffer(ctx);

  // Calculate compass geometry with perspective projection
  // The compass is viewed from 45 degrees above, creating an ellipse
  float base_radius = (float)ctx->radar_compass_size / 2.0f;
  float radius_x    = base_radius; // Horizontal radius unchanged
  float radius_y    = base_radius * PERSPECTIVE_SCALE; // Vertical compressed

  // Center point (adjusted for ellipse)
  int cx = ctx->radar_compass_x + ctx->radar_compass_size / 2;
  int cy = ctx->radar_compass_y
           + (int)(ctx->radar_compass_size * PERSPECTIVE_SCALE / 2.0f);

  // Get platform azimuth for rotation
  double platform_azimuth = 0.0;
  osd_state_get_orientation(pb_state, &platform_azimuth, NULL, NULL);

  // Rotation: negative azimuth so north moves opposite to platform heading
  // When platform points east (90°), north should appear to the left (-90°)
  float rotation_deg = -(float)platform_azimuth;

  // Get camera FOV for wedge
  double fov_angle = 0.0;
#ifdef OSD_STREAM_DAY
  fov_angle = osd_state_get_camera_fov_day(pb_state);
#elif OSD_STREAM_THERMAL
  fov_angle = osd_state_get_camera_fov_heat(pb_state);
#endif

  // Default FOV if not available
  if (fov_angle <= 0.0)
    fov_angle = 45.0;

  // Draw in order: rings, labels, FOV wedge, cardinals, celestial
  // (back to front for proper layering)

  // 1. Draw distance rings (elliptical)
  draw_rings(&fb, ctx, cx, cy, radius_x, radius_y);

  // 2. Draw ring labels
  draw_ring_labels(&fb, ctx, cx, cy, radius_x, radius_y, rotation_deg);

  // 3. Draw FOV wedge (semi-transparent, elliptical)
  draw_fov_wedge(&fb, ctx, cx, cy, radius_x, radius_y, (float)fov_angle);

  // 4. Draw cardinal directions
  draw_cardinals(&fb, ctx, cx, cy, radius_x, radius_y, rotation_deg);

  // 5. Draw celestial indicators (on top)
  draw_celestial_indicators(&fb, ctx, pb_state, cx, cy, radius_x, radius_y,
                            rotation_deg);

  return true;
}

// ════════════════════════════════════════════════════════════
// CLEANUP
// ════════════════════════════════════════════════════════════

void
radar_compass_cleanup(osd_context_t *ctx)
{
  if (!ctx)
    return;

  // Free fonts
  font_free(&ctx->font_radar_compass_ring_labels);
  font_free(&ctx->font_radar_compass_cardinals);

  // Free celestial SVGs
  svg_free(&ctx->celestial_sun_svg);
  svg_free(&ctx->celestial_moon_svg);

  // Cleanup celestial engine
  celestial_cleanup();

  ctx->radar_compass_enabled = false;

  LOG_INFO("radar_compass_cleanup: Resources freed");
}

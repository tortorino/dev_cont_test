#ifndef OSD_CONFIG_H
#define OSD_CONFIG_H

#include <stdbool.h>
#include <stdint.h>

// ════════════════════════════════════════════════════════════
// CONFIGURATION STRUCTURES
// ════════════════════════════════════════════════════════════
//
// This file contains all OSD configuration structures that are
// loaded from JSON configuration files at runtime. These structures
// define the appearance and behavior of all OSD widgets.
//
// ════════════════════════════════════════════════════════════

// Crosshair orientation
typedef enum
{
  CROSSHAIR_ORIENTATION_VERTICAL, // Standard + shape
  CROSSHAIR_ORIENTATION_DIAGONAL  // X shape (45 degrees)
} crosshair_orientation_t;

// Individual crosshair element configuration
typedef struct
{
  bool enabled;
  uint32_t color;  // ARGB format (0xAARRGGBB)
  float thickness; // Pixels
} crosshair_element_t;

// Crosshair configuration (loaded from config.xml)
typedef struct
{
  // Overall enable
  bool enabled;

  // Orientation
  crosshair_orientation_t orientation;

  // Center dot
  crosshair_element_t center_dot;
  float center_dot_radius;

  // Cross arms (4 lines)
  crosshair_element_t cross;
  float cross_length; // Length of each arm
  float cross_gap;    // Gap from center

  // Circle
  crosshair_element_t circle;
  float circle_radius;

  // SVG sources (optional - if empty, use primitive rendering)
  char cross_svg_path[256];
  char circle_svg_path[256];
} crosshair_config_t;

// Timestamp configuration
typedef struct
{
  bool enabled;
  uint32_t color;
  int font_size;
  int pos_x;
  int pos_y;
  char font_path[256];
} timestamp_config_t;

// Speed indicators configuration
typedef struct
{
  bool enabled;
  uint32_t color;
  int font_size;
  float threshold;           // Min normalized speed (0.0-1.0) to show indicator
  float max_speed_azimuth;   // Max speed in degrees/s for azimuth
  float max_speed_elevation; // Max speed in degrees/s for elevation
  char font_path[256];
} speed_config_t;

// Variant info widget configuration
typedef struct
{
  bool enabled;
  int pos_x;
  int pos_y;
  uint32_t color;
  int font_size;
  char font_path[256];
} variant_info_config_t;

// Maximum number of distance rings for radar compass
#define RADAR_COMPASS_MAX_RINGS 5

// Radar compass configuration
// A 2D top-down compass display with distance rings, cardinal directions,
// FOV wedge, and celestial indicators (sun/moon)
typedef struct
{
  bool enabled;
  int position_x;
  int position_y;
  int size; // Diameter in pixels

  // Distance rings (configurable)
  int num_rings;                                 // Number of rings (1-5)
  float ring_distances[RADAR_COMPASS_MAX_RINGS]; // Distance in km for each ring
  uint32_t ring_color;
  float ring_thickness;
  bool show_ring_labels;
  int ring_label_font_size;
  char ring_label_font_path[256];

  // Cardinal directions (N, E, S, W)
  uint32_t cardinal_color;
  int cardinal_font_size;
  char cardinal_font_path[256];

  // FOV wedge (angle comes from protobuf state, not config)
  uint32_t fov_fill_color;    // Semi-transparent fill
  uint32_t fov_outline_color; // Edge color
  float fov_outline_thickness;
} radar_compass_config_t;

// Celestial indicators configuration (sun and moon on radar compass)
// Indicators are positioned by azimuth on the compass edge,
// with size/opacity varying by altitude (higher = larger/brighter)
typedef struct
{
  bool enabled;               // Master enable for all celestial indicators
  bool show_sun;              // Show sun indicator
  bool show_moon;             // Show moon indicator
  float indicator_scale;      // Base scale factor (1.0 = 100% of default size)
  float visibility_threshold; // Min altitude (degrees) to show indicator (e.g.,
                              // -5.0)
  char sun_svg_path[256];     // Sun indicator SVG
  char moon_svg_path[256];    // Moon indicator SVG
} celestial_indicators_config_t;

// Full OSD configuration
typedef struct
{
  crosshair_config_t crosshair;
  timestamp_config_t timestamp;
  speed_config_t speed_indicators;
  variant_info_config_t variant_info;
  radar_compass_config_t radar_compass;
  celestial_indicators_config_t celestial_indicators;
} osd_config_t;

#endif // OSD_CONFIG_H

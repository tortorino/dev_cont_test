// Primitive Drawing Functions
// Low-level geometric rendering primitives for OSD graphics
//
// All functions operate on a framebuffer and use alpha blending for smooth
// compositing. Coordinates are in pixels, with (0,0) at top-left.

#ifndef RENDERING_PRIMITIVES_H
#define RENDERING_PRIMITIVES_H

#include "../core/framebuffer.h"

#include <stdint.h>

// ════════════════════════════════════════════════════════════
// POINT DRAWING
// ════════════════════════════════════════════════════════════

// Draw single pixel with alpha blending
//
// Performs bounds checking and alpha blending automatically.
// If (x, y) is out of bounds, does nothing.
//
// Usage:
//   draw_pixel(&fb, 100, 100, 0xFF0000FF);  // Red pixel
void draw_pixel(framebuffer_t *fb, int x, int y, uint32_t color);

// ════════════════════════════════════════════════════════════
// LINE DRAWING
// ════════════════════════════════════════════════════════════

// Draw line from (x0, y0) to (x1, y1) with thickness
//
// Uses Bresenham's line algorithm with thickness support.
// Thick lines are drawn as squares perpendicular to the line.
//
// Parameters:
//   fb: Framebuffer to draw on
//   x0, y0: Start point
//   x1, y1: End point
//   color: RGBA color (0xAABBGGRR format)
//   thickness: Line width in pixels (can be fractional)
//
// Usage:
//   draw_line(&fb, 0, 0, 100, 100, 0xFF0000FF, 2.0f);  // Red diagonal line
void draw_line(framebuffer_t *fb,
               int x0,
               int y0,
               int x1,
               int y1,
               uint32_t color,
               float thickness);

// ════════════════════════════════════════════════════════════
// CIRCLE DRAWING
// ════════════════════════════════════════════════════════════

// Draw filled circle centered at (cx, cy) with given radius
//
// Uses simple distance check: draws all pixels within radius.
//
// Parameters:
//   fb: Framebuffer to draw on
//   cx, cy: Center point
//   radius: Circle radius in pixels (can be fractional)
//   color: RGBA color (0xAABBGGRR format)
//
// Usage:
//   draw_filled_circle(&fb, 100, 100, 10.0f, 0xFF0000FF);  // Red circle
void draw_filled_circle(
  framebuffer_t *fb, int cx, int cy, float radius, uint32_t color);

// Draw circle outline (hollow circle) with thickness
//
// Uses midpoint circle algorithm with thickness.
// Draws all pixels between inner and outer radius.
//
// Parameters:
//   fb: Framebuffer to draw on
//   cx, cy: Center point
//   radius: Circle radius in pixels (center of stroke)
//   color: RGBA color (0xAABBGGRR format)
//   thickness: Stroke width in pixels (can be fractional)
//
// Usage:
//   draw_circle_outline(&fb, 100, 100, 10.0f, 0xFF0000FF, 2.0f);
void draw_circle_outline(framebuffer_t *fb,
                         int cx,
                         int cy,
                         float radius,
                         uint32_t color,
                         float thickness);

// ════════════════════════════════════════════════════════════
// RECTANGLE DRAWING
// ════════════════════════════════════════════════════════════

// Draw filled rectangle with top-left at (x, y)
//
// Parameters:
//   fb: Framebuffer to draw on
//   x, y: Top-left corner
//   w, h: Width and height in pixels
//   color: RGBA color (0xAABBGGRR format)
//
// Usage:
//   draw_rect_filled(&fb, 10, 10, 100, 50, 0xFF0000FF);  // Red rectangle
void
draw_rect_filled(framebuffer_t *fb, int x, int y, int w, int h, uint32_t color);

// Draw rectangle outline with thickness
//
// Parameters:
//   fb: Framebuffer to draw on
//   x, y: Top-left corner
//   w, h: Width and height in pixels
//   color: RGBA color (0xAABBGGRR format)
//   thickness: Stroke width in pixels (can be fractional)
//
// Usage:
//   draw_rect_outline(&fb, 10, 10, 100, 50, 0xFF0000FF, 2.0f);
void draw_rect_outline(framebuffer_t *fb,
                       int x,
                       int y,
                       int w,
                       int h,
                       uint32_t color,
                       float thickness);

// ════════════════════════════════════════════════════════════
// ELLIPSE DRAWING
// ════════════════════════════════════════════════════════════

// Draw ellipse outline with separate X and Y radii
//
// Parameters:
//   fb: Framebuffer to draw on
//   cx, cy: Center point
//   radius_x: Horizontal radius in pixels
//   radius_y: Vertical radius in pixels
//   color: RGBA color (0xAABBGGRR format)
//   thickness: Stroke width in pixels
//
// Usage:
//   draw_ellipse_outline(&fb, 100, 100, 50.0f, 35.0f, 0xFF00FF00, 2.0f);
void draw_ellipse_outline(framebuffer_t *fb,
                          int cx,
                          int cy,
                          float radius_x,
                          float radius_y,
                          uint32_t color,
                          float thickness);

// ════════════════════════════════════════════════════════════
// ARC AND WEDGE DRAWING
// ════════════════════════════════════════════════════════════

// Draw arc (partial circle outline) from start_angle to end_angle
//
// Angles are in degrees, measured clockwise from the positive Y-axis (up).
// 0° = up, 90° = right, 180° = down, 270° = left.
// The arc is drawn as line segments for smooth appearance.
//
// Parameters:
//   fb: Framebuffer to draw on
//   cx, cy: Center point of the arc
//   radius: Arc radius in pixels
//   start_angle_deg: Starting angle in degrees
//   end_angle_deg: Ending angle in degrees
//   color: RGBA color (0xAABBGGRR format)
//   thickness: Arc stroke width in pixels
//   segments: Number of line segments (higher = smoother, 0 = auto)
//
// Usage:
//   draw_arc(&fb, 100, 100, 50.0f, 0.0f, 90.0f, 0xFF00FF00, 2.0f, 0);
void draw_arc(framebuffer_t *fb,
              int cx,
              int cy,
              float radius,
              float start_angle_deg,
              float end_angle_deg,
              uint32_t color,
              float thickness,
              int segments);

// Draw filled wedge (pie slice) from center to edge
//
// Creates a filled sector from the center point outward.
// Angles are in degrees, measured clockwise from up (0° = up).
//
// Parameters:
//   fb: Framebuffer to draw on
//   cx, cy: Center point (tip of wedge)
//   radius: Wedge radius in pixels
//   start_angle_deg: Starting angle in degrees
//   end_angle_deg: Ending angle in degrees
//   color: RGBA color (0xAABBGGRR format)
//
// Usage:
//   draw_wedge_filled(&fb, 100, 100, 50.0f, -45.0f, 45.0f, 0x8000FF00);
void draw_wedge_filled(framebuffer_t *fb,
                       int cx,
                       int cy,
                       float radius,
                       float start_angle_deg,
                       float end_angle_deg,
                       uint32_t color);

// Draw wedge outline (arc + two radial lines)
//
// Draws the outline of a pie slice: two lines from center to edge,
// connected by an arc.
//
// Parameters:
//   fb: Framebuffer to draw on
//   cx, cy: Center point (tip of wedge)
//   radius: Wedge radius in pixels
//   start_angle_deg: Starting angle in degrees
//   end_angle_deg: Ending angle in degrees
//   color: RGBA color (0xAABBGGRR format)
//   thickness: Stroke width in pixels
//
// Usage:
//   draw_wedge_outline(&fb, 100, 100, 50.0f, -45.0f, 45.0f, 0xFF00FF00, 2.0f);
void draw_wedge_outline(framebuffer_t *fb,
                        int cx,
                        int cy,
                        float radius,
                        float start_angle_deg,
                        float end_angle_deg,
                        uint32_t color,
                        float thickness);

// ════════════════════════════════════════════════════════════
// ELLIPTICAL ARC AND WEDGE DRAWING (for perspective views)
// ════════════════════════════════════════════════════════════

// Draw elliptical arc with separate X and Y radii
//
// Parameters:
//   fb: Framebuffer to draw on
//   cx, cy: Center point
//   radius_x, radius_y: Horizontal and vertical radii
//   start_angle_deg, end_angle_deg: Arc angles in degrees (0° = up, clockwise)
//   color: RGBA color
//   thickness: Stroke width in pixels
//   segments: Number of line segments (0 = auto)
void draw_ellipse_arc(framebuffer_t *fb,
                      int cx,
                      int cy,
                      float radius_x,
                      float radius_y,
                      float start_angle_deg,
                      float end_angle_deg,
                      uint32_t color,
                      float thickness,
                      int segments);

// Draw filled elliptical wedge (pie slice with elliptical edge)
void draw_ellipse_wedge_filled(framebuffer_t *fb,
                               int cx,
                               int cy,
                               float radius_x,
                               float radius_y,
                               float start_angle_deg,
                               float end_angle_deg,
                               uint32_t color);

// Draw elliptical wedge outline
void draw_ellipse_wedge_outline(framebuffer_t *fb,
                                int cx,
                                int cy,
                                float radius_x,
                                float radius_y,
                                float start_angle_deg,
                                float end_angle_deg,
                                uint32_t color,
                                float thickness);

#endif // RENDERING_PRIMITIVES_H

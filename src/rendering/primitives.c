#include "primitives.h"

#include "utils/math_decl.h"

#include <math.h>
#include <stdlib.h>

// ════════════════════════════════════════════════════════════
// POINT DRAWING IMPLEMENTATION
// ════════════════════════════════════════════════════════════

void
draw_pixel(framebuffer_t *fb, int x, int y, uint32_t color)
{
  // Use framebuffer's bounds-checked blend function
  framebuffer_blend_pixel(fb, x, y, color);
}

// ════════════════════════════════════════════════════════════
// LINE DRAWING IMPLEMENTATION
// ════════════════════════════════════════════════════════════

void
draw_line(framebuffer_t *fb,
          int x0,
          int y0,
          int x1,
          int y1,
          uint32_t color,
          float thickness)
{
  // Bresenham's line algorithm with thickness
  int dx  = abs(x1 - x0);
  int dy  = abs(y1 - y0);
  int sx  = x0 < x1 ? 1 : -1;
  int sy  = y0 < y1 ? 1 : -1;
  int err = dx - dy;

  int half_thick = (int)(thickness / 2.0f);

  while (1)
    {
      // Draw thick point (square stamp perpendicular to line)
      for (int ty = -half_thick; ty <= half_thick; ty++)
        {
          for (int tx = -half_thick; tx <= half_thick; tx++)
            {
              draw_pixel(fb, x0 + tx, y0 + ty, color);
            }
        }

      // Check if we've reached the end point
      if (x0 == x1 && y0 == y1)
        {
          break;
        }

      // Bresenham's step
      int e2 = 2 * err;
      if (e2 > -dy)
        {
          err -= dy;
          x0 += sx;
        }
      if (e2 < dx)
        {
          err += dx;
          y0 += sy;
        }
    }
}

// ════════════════════════════════════════════════════════════
// CIRCLE DRAWING IMPLEMENTATION
// ════════════════════════════════════════════════════════════

void
draw_filled_circle(
  framebuffer_t *fb, int cx, int cy, float radius, uint32_t color)
{
  // Simple distance-based filling
  int r = (int)radius;

  for (int y = -r; y <= r; y++)
    {
      for (int x = -r; x <= r; x++)
        {
          // Check if point is within circle
          if (x * x + y * y <= r * r)
            {
              draw_pixel(fb, cx + x, cy + y, color);
            }
        }
    }
}

void
draw_circle_outline(framebuffer_t *fb,
                    int cx,
                    int cy,
                    float radius,
                    uint32_t color,
                    float thickness)
{
  // Midpoint circle algorithm with thickness
  // Draw all pixels between inner and outer radius
  int r_outer = (int)(radius + thickness / 2.0f);
  int r_inner = (int)(radius - thickness / 2.0f);

  if (r_inner < 0)
    {
      r_inner = 0;
    }

  for (int y = -r_outer; y <= r_outer; y++)
    {
      for (int x = -r_outer; x <= r_outer; x++)
        {
          int dist_sq = x * x + y * y;

          // Check if point is in the annular region (donut)
          if (dist_sq >= r_inner * r_inner && dist_sq <= r_outer * r_outer)
            {
              draw_pixel(fb, cx + x, cy + y, color);
            }
        }
    }
}

// ════════════════════════════════════════════════════════════
// RECTANGLE DRAWING IMPLEMENTATION
// ════════════════════════════════════════════════════════════

void
draw_rect_filled(framebuffer_t *fb, int x, int y, int w, int h, uint32_t color)
{
  // Draw all pixels in rectangle
  for (int py = y; py < y + h; py++)
    {
      for (int px = x; px < x + w; px++)
        {
          draw_pixel(fb, px, py, color);
        }
    }
}

void
draw_rect_outline(framebuffer_t *fb,
                  int x,
                  int y,
                  int w,
                  int h,
                  uint32_t color,
                  float thickness)
{
  // Draw four lines to form rectangle outline
  int t = (int)thickness;

  // Top edge
  draw_rect_filled(fb, x, y, w, t, color);

  // Bottom edge
  draw_rect_filled(fb, x, y + h - t, w, t, color);

  // Left edge
  draw_rect_filled(fb, x, y + t, t, h - 2 * t, color);

  // Right edge
  draw_rect_filled(fb, x + w - t, y + t, t, h - 2 * t, color);
}

// ════════════════════════════════════════════════════════════
// ARC AND WEDGE DRAWING IMPLEMENTATION
// ════════════════════════════════════════════════════════════

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Convert degrees to radians (float version)
static inline float
deg_to_radf(float degrees)
{
  return degrees * (float)(M_PI / 180.0);
}

// Convert angle from "clockwise from up" to standard math convention
// Input: 0° = up, 90° = right, 180° = down, 270° = left (clockwise)
// Output: Standard math radians (0 = right, counter-clockwise)
static inline float
compass_to_math_rad(float compass_deg)
{
  // Compass: 0=up, 90=right, 180=down, 270=left (clockwise)
  // Math: 0=right, 90=up, 180=left, 270=down (counter-clockwise)
  // Convert: math_angle = 90 - compass_angle
  return deg_to_radf(90.0f - compass_deg);
}

void
draw_arc(framebuffer_t *fb,
         int cx,
         int cy,
         float radius,
         float start_angle_deg,
         float end_angle_deg,
         uint32_t color,
         float thickness,
         int segments)
{
  // Auto-calculate segments if not specified
  if (segments <= 0)
    {
      // Use ~1 segment per 3 degrees for smooth arcs
      float angle_span = fabsf(end_angle_deg - start_angle_deg);
      segments         = (int)(angle_span / 3.0f);
      if (segments < 4)
        segments = 4;
      if (segments > 120)
        segments = 120;
    }

  float angle_step = (end_angle_deg - start_angle_deg) / (float)segments;

  for (int i = 0; i < segments; i++)
    {
      float angle1 = start_angle_deg + (float)i * angle_step;
      float angle2 = start_angle_deg + (float)(i + 1) * angle_step;

      // Convert to math convention
      float rad1 = compass_to_math_rad(angle1);
      float rad2 = compass_to_math_rad(angle2);

      // Calculate endpoints
      int x1 = cx + (int)(radius * cosf(rad1));
      int y1 = cy - (int)(radius * sinf(rad1));
      int x2 = cx + (int)(radius * cosf(rad2));
      int y2 = cy - (int)(radius * sinf(rad2));

      draw_line(fb, x1, y1, x2, y2, color, thickness);
    }
}

void
draw_wedge_filled(framebuffer_t *fb,
                  int cx,
                  int cy,
                  float radius,
                  float start_angle_deg,
                  float end_angle_deg,
                  uint32_t color)
{
  // Ensure start < end for consistent iteration
  if (start_angle_deg > end_angle_deg)
    {
      float tmp       = start_angle_deg;
      start_angle_deg = end_angle_deg;
      end_angle_deg   = tmp;
    }

  // Convert angles to math convention
  float start_rad = compass_to_math_rad(start_angle_deg);
  float end_rad   = compass_to_math_rad(end_angle_deg);

  // Normalize to [0, 2*PI) range
  while (start_rad < 0)
    start_rad += 2.0f * (float)M_PI;
  while (end_rad < 0)
    end_rad += 2.0f * (float)M_PI;
  while (start_rad >= 2.0f * (float)M_PI)
    start_rad -= 2.0f * (float)M_PI;
  while (end_rad >= 2.0f * (float)M_PI)
    end_rad -= 2.0f * (float)M_PI;

  // Handle wraparound (when going from compass, end_rad may be < start_rad)
  bool wraps = (end_rad < start_rad);

  int r = (int)radius;

  // Scan all pixels in bounding box
  for (int y = -r; y <= r; y++)
    {
      for (int x = -r; x <= r; x++)
        {
          // Check distance from center
          float dist_sq = (float)(x * x + y * y);
          if (dist_sq > radius * radius)
            continue;

          // Calculate angle of this pixel (math convention: 0=right, CCW)
          float pixel_angle = atan2f((float)(-y), (float)x);
          if (pixel_angle < 0)
            pixel_angle += 2.0f * (float)M_PI;

          // Check if pixel angle is within wedge
          bool in_wedge;
          if (wraps)
            {
              // Wedge crosses 0 boundary
              in_wedge = (pixel_angle >= start_rad || pixel_angle <= end_rad);
            }
          else
            {
              in_wedge = (pixel_angle >= end_rad && pixel_angle <= start_rad);
            }

          if (in_wedge)
            {
              draw_pixel(fb, cx + x, cy + y, color);
            }
        }
    }
}

void
draw_wedge_outline(framebuffer_t *fb,
                   int cx,
                   int cy,
                   float radius,
                   float start_angle_deg,
                   float end_angle_deg,
                   uint32_t color,
                   float thickness)
{
  // Convert to math convention for endpoint calculation
  float start_rad = compass_to_math_rad(start_angle_deg);
  float end_rad   = compass_to_math_rad(end_angle_deg);

  // Calculate edge endpoints
  int x1 = cx + (int)(radius * cosf(start_rad));
  int y1 = cy - (int)(radius * sinf(start_rad));
  int x2 = cx + (int)(radius * cosf(end_rad));
  int y2 = cy - (int)(radius * sinf(end_rad));

  // Draw two radial lines from center to edge
  draw_line(fb, cx, cy, x1, y1, color, thickness);
  draw_line(fb, cx, cy, x2, y2, color, thickness);

  // Draw arc connecting the edges
  draw_arc(fb, cx, cy, radius, start_angle_deg, end_angle_deg, color, thickness,
           0);
}

// ════════════════════════════════════════════════════════════
// ELLIPSE DRAWING IMPLEMENTATION
// ════════════════════════════════════════════════════════════

void
draw_ellipse_outline(framebuffer_t *fb,
                     int cx,
                     int cy,
                     float radius_x,
                     float radius_y,
                     uint32_t color,
                     float thickness)
{
  // Draw ellipse using parametric form with line segments
  int segments     = 64; // More segments for smooth ellipse
  float half_thick = thickness / 2.0f;

  for (int i = 0; i < segments; i++)
    {
      float t1 = (float)i / (float)segments * 2.0f * (float)M_PI;
      float t2 = (float)(i + 1) / (float)segments * 2.0f * (float)M_PI;

      int x1 = cx + (int)(radius_x * cosf(t1));
      int y1 = cy + (int)(radius_y * sinf(t1));
      int x2 = cx + (int)(radius_x * cosf(t2));
      int y2 = cy + (int)(radius_y * sinf(t2));

      draw_line(fb, x1, y1, x2, y2, color, thickness);
    }
}

void
draw_ellipse_arc(framebuffer_t *fb,
                 int cx,
                 int cy,
                 float radius_x,
                 float radius_y,
                 float start_angle_deg,
                 float end_angle_deg,
                 uint32_t color,
                 float thickness,
                 int segments)
{
  // Auto-calculate segments if not specified
  if (segments <= 0)
    {
      float angle_span = fabsf(end_angle_deg - start_angle_deg);
      segments         = (int)(angle_span / 3.0f);
      if (segments < 4)
        segments = 4;
      if (segments > 120)
        segments = 120;
    }

  float angle_step = (end_angle_deg - start_angle_deg) / (float)segments;

  for (int i = 0; i < segments; i++)
    {
      float angle1 = start_angle_deg + (float)i * angle_step;
      float angle2 = start_angle_deg + (float)(i + 1) * angle_step;

      // Convert to math convention
      float rad1 = compass_to_math_rad(angle1);
      float rad2 = compass_to_math_rad(angle2);

      // Calculate endpoints using ellipse radii
      int x1 = cx + (int)(radius_x * cosf(rad1));
      int y1 = cy - (int)(radius_y * sinf(rad1));
      int x2 = cx + (int)(radius_x * cosf(rad2));
      int y2 = cy - (int)(radius_y * sinf(rad2));

      draw_line(fb, x1, y1, x2, y2, color, thickness);
    }
}

void
draw_ellipse_wedge_filled(framebuffer_t *fb,
                          int cx,
                          int cy,
                          float radius_x,
                          float radius_y,
                          float start_angle_deg,
                          float end_angle_deg,
                          uint32_t color)
{
  // Ensure start < end for consistent iteration
  if (start_angle_deg > end_angle_deg)
    {
      float tmp       = start_angle_deg;
      start_angle_deg = end_angle_deg;
      end_angle_deg   = tmp;
    }

  // Convert angles to math convention
  float start_rad = compass_to_math_rad(start_angle_deg);
  float end_rad   = compass_to_math_rad(end_angle_deg);

  // Normalize to [0, 2*PI) range
  while (start_rad < 0)
    start_rad += 2.0f * (float)M_PI;
  while (end_rad < 0)
    end_rad += 2.0f * (float)M_PI;
  while (start_rad >= 2.0f * (float)M_PI)
    start_rad -= 2.0f * (float)M_PI;
  while (end_rad >= 2.0f * (float)M_PI)
    end_rad -= 2.0f * (float)M_PI;

  // Handle wraparound
  bool wraps = (end_rad < start_rad);

  int rx = (int)radius_x;
  int ry = (int)radius_y;

  // Scan all pixels in bounding box
  for (int y = -ry; y <= ry; y++)
    {
      for (int x = -rx; x <= rx; x++)
        {
          // Check if point is within ellipse using normalized coordinates
          float nx = (float)x / radius_x;
          float ny = (float)y / radius_y;
          if (nx * nx + ny * ny > 1.0f)
            continue;

          // Calculate angle of this pixel (math convention)
          float pixel_angle = atan2f((float)(-y), (float)x);
          if (pixel_angle < 0)
            pixel_angle += 2.0f * (float)M_PI;

          // Check if pixel angle is within wedge
          bool in_wedge;
          if (wraps)
            {
              in_wedge = (pixel_angle >= start_rad || pixel_angle <= end_rad);
            }
          else
            {
              in_wedge = (pixel_angle >= end_rad && pixel_angle <= start_rad);
            }

          if (in_wedge)
            {
              draw_pixel(fb, cx + x, cy + y, color);
            }
        }
    }
}

void
draw_ellipse_wedge_outline(framebuffer_t *fb,
                           int cx,
                           int cy,
                           float radius_x,
                           float radius_y,
                           float start_angle_deg,
                           float end_angle_deg,
                           uint32_t color,
                           float thickness)
{
  // Convert to math convention for endpoint calculation
  float start_rad = compass_to_math_rad(start_angle_deg);
  float end_rad   = compass_to_math_rad(end_angle_deg);

  // Calculate edge endpoints on ellipse
  int x1 = cx + (int)(radius_x * cosf(start_rad));
  int y1 = cy - (int)(radius_y * sinf(start_rad));
  int x2 = cx + (int)(radius_x * cosf(end_rad));
  int y2 = cy - (int)(radius_y * sinf(end_rad));

  // Draw two radial lines from center to edge
  draw_line(fb, cx, cy, x1, y1, color, thickness);
  draw_line(fb, cx, cy, x2, y2, color, thickness);

  // Draw elliptical arc connecting the edges
  draw_ellipse_arc(fb, cx, cy, radius_x, radius_y, start_angle_deg,
                   end_angle_deg, color, thickness, 0);
}

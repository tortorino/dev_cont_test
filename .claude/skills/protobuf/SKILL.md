---
name: protobuf
description: Helps with protobuf messages, updating proto submodules, and understanding the JonGUIState structure. Use when working with proto files, message fields, or state data.
allowed-tools: Read, Bash, Glob, Grep
---

# Protobuf Integration

Assists with protobuf messages and submodule management.

## Submodule Structure

Proto definitions are git submodules (auto-updated by protogen CI):

```
proto/c/   -> jettison_proto_c (C nanopb bindings)
proto/ts/  -> jettison_proto_typescript (TypeScript ts-proto)
```

**Source files**: `src/proto/` (synced from proto/c/)

## Updating Protos

```bash
# From host
./tools/devcontainer-build.sh proto

# Inside container
make proto
```

This updates submodules to latest and syncs to `src/proto/`.

## Key Message: JonGUIState

The main protobuf message containing all OSD state:

### actual_space_time (Primary Orientation)

```protobuf
message ActualSpaceTime {
  float azimuth;     // Navball yaw
  float elevation;   // Navball pitch
  float bank;        // Navball roll
  double latitude;   // Observer location
  double longitude;
  double altitude;
  int64 timestamp;   // For celestial calculations
}
```

### compass (Alternative Orientation)

```protobuf
message Compass {
  float azimuth;    // 0-360
  float elevation;  // -90 to 90
  float bank;       // -180 to 180
}
```

### rotary (Speed Indicators)

```protobuf
message Rotary {
  float azimuth_speed;    // Normalized -1.0 to 1.0
  float elevation_speed;  // Normalized -1.0 to 1.0
  bool is_moving;         // Show indicators only when true
}
```

### time (Timestamp Display)

```protobuf
message Time {
  int64 timestamp;  // Unix epoch for HH:MM:SS UTC
}
```

### rec_osd (Crosshair Offsets)

```protobuf
message RecOsd {
  int32 day_crosshair_offset_x;
  int32 day_crosshair_offset_y;
  int32 heat_crosshair_offset_x;
  int32 heat_crosshair_offset_y;
}
```

### Frame Timing

```protobuf
int64 system_monotonic_time_us;   // State update time
int64 frame_monotonic_day_us;     // Day camera frame time
int64 frame_monotonic_heat_us;    // Thermal camera frame time
```

## State Access

State is accessed via `src/osd_state.h` which hides protobuf details:

```c
float osd_state_get_azimuth(const osd_state_t *state);
float osd_state_get_elevation(const osd_state_t *state);
bool osd_state_is_moving(const osd_state_t *state);
// etc.
```

## Proto Files in src/proto/

- `jon_shared_data.pb.h` - Main JonGUIState
- `jon_shared_data_actual_space_time.pb.h` - Orientation
- `jon_shared_data_rotary.pb.h` - Speed data
- `jon_shared_data_rec_osd.pb.h` - Crosshair offsets
- `pb.h`, `pb_decode.c/h`, `pb_encode.h` - nanopb runtime

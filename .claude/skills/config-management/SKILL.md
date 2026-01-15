---
name: config-management
description: Helps with OSD JSON configuration files and schema. Use when editing config.json, modifying widget settings, changing fonts, or working with the JSON schema.
allowed-tools: Read, Edit, Glob, Grep
---

# Configuration Management

Assists with editing OSD configuration files and understanding the JSON schema.

## Config File Locations

Each variant has its own config:

```
resources/live_day.json
resources/live_thermal.json
resources/recording_day.json
resources/recording_thermal.json
```

**Schema**: `resources/schemas/osd_config.schema.json`

## Required Fields

The frontend validates configs against schema. These are **required**:

- `version` - Semantic version (e.g., "1.0.0")
- Per-widget `font` fields in timestamp, speed_indicators, variant_info

## Key Configuration Sections

### Crosshair

```json
"crosshair": {
  "enabled": true,
  "orientation": "vertical",  // or "diagonal"
  "color": { "r": 255, "g": 255, "b": 255, "a": 255 }
}
```

### Navball

```json
"navball": {
  "enabled": true,
  "skin": "5thHorseman_brown",  // 13 available skins
  "center_indicator": {
    "indicator": "circle"  // "circle", "rectangle", or "crosshair"
  }
}
```

### Speed Indicators

```json
"speed_indicators": {
  "enabled": true,
  "threshold": 0.05,           // Min normalized speed to show
  "max_speed_azimuth": 35.0,   // Display scale
  "max_speed_elevation": 35.0,
  "font": "liberation_sans_bold"
}
```

### Celestial Indicators

```json
"celestial_indicators": {
  "enabled": true,
  "visibility_threshold": -5.0,  // Show bodies slightly below horizon
  "scale": 1.0
}
```

### Timestamp

```json
"timestamp": {
  "enabled": true,   // false for live variants
  "font": "liberation_sans_bold"
}
```

## Available Fonts

- `liberation_sans_bold` - Default, clean sans-serif
- `b612_mono_bold` - Monospace, cockpit displays
- `share_tech_mono` - Technical monospace
- `orbitron_bold` - Futuristic display

## Variant Differences

| Setting | Live | Recording |
|---------|------|-----------|
| timestamp.enabled | false | true |
| navball.enabled | false | true |
| celestial_indicators.enabled | false | true |

Live variants are optimized for minimal rendering cost.

## Proto-Sourced Values (Runtime)

These come from protobuf state, NOT config:

- Navball orientation: `actual_space_time.azimuth/elevation/bank`
- Celestial positions: `actual_space_time.latitude/longitude/altitude/timestamp`
- Speed values: `rotary.azimuth_speed/elevation_speed/is_moving`
- Crosshair offset: `rec_osd.day_crosshair_offset_*/heat_crosshair_offset_*`

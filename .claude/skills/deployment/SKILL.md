---
name: deployment
description: Helps with deploying OSD packages to sych.local. Use when discussing deployment, hot-reload, frontend updates, or gallery updates.
allowed-tools: Read, Bash, Glob, Grep
---

# Deployment

Assists with deploying OSD packages to the sych.local server.

## Deployment Targets

### Full Deploy (Frontend + Gallery)

```bash
# Dev builds (recommended for development)
./tools/devcontainer-build.sh deploy

# Production builds (optimized)
./tools/devcontainer-build.sh deploy-prod
```

### Selective Deploy

Inside container or via `make`:

```bash
make deploy-frontend   # Live variants only (live_day + live_thermal)
make deploy-gallery    # Recording variant only (recording_day)
```

## What Gets Deployed

**Location**: `sych.local:/home/archer/web/osd/`

| File | Purpose |
|------|---------|
| `live_day.tar` | Frontend live day stream |
| `live_thermal.tar` | Frontend live thermal stream |
| `default.tar` | Gallery (recording_day) |
| `pip_override.json` | PiP view config overrides |

## Hot-Reload Workflow

1. Make changes to OSD source
2. Run `make deploy` (or `./tools/devcontainer-build.sh deploy`)
3. Frontend auto-detects package changes via ETag
4. Worker reloads within ~1 second

## Configuration

Environment variables in `.env`:

```bash
DEPLOY_HOST=sych.local
DEPLOY_USER=archer
```

## PiP Override

The `resources/pip_override.json` disables all widgets except crosshair for Picture-in-Picture views:

```json
{
  "variant_info": { "enabled": false },
  "speed_indicators": { "enabled": false },
  "timestamp": { "enabled": false },
  "navball": { "enabled": false },
  "celestial_indicators": { "enabled": false }
}
```

## Package Sizes

| Mode | WASM | Package |
|------|------|---------|
| Production | ~640KB | ~900KB |
| Dev | ~2.9MB | ~3MB |

Dev packages include debug symbols for easier troubleshooting.

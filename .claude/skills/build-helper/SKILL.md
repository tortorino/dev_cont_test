---
name: build-helper
description: Helps with WASM compilation, building, and packaging. Use when discussing build targets, Makefile commands, build modes, or WASM compilation issues.
allowed-tools: Read, Bash, Glob, Grep
---

# Build Helper

Assists with building, compiling, and packaging the Jettison WASM OSD plugin.

## Key Concepts

### Build Architecture

- **4 WASM variants**: live_day, live_thermal, recording_day, recording_thermal
- **2 build modes**: production (~640KB) and dev (~2.9MB with debug symbols)
- **Docker-first**: All builds run inside dev container

### Host Commands (Primary)

Always use the devcontainer wrapper from the host:

```bash
# Build all 4 production WASM
./tools/devcontainer-build.sh wasm

# Build all 4 dev WASM (debug symbols)
./tools/devcontainer-build.sh wasm-debug

# Build + sign packages
./tools/devcontainer-build.sh package      # production
./tools/devcontainer-build.sh package-dev  # dev

# Full CI pipeline
./tools/devcontainer-build.sh ci
```

### Inside Container (CLion/IDE)

If already in container shell:

```bash
make                     # Build recording_day
make all                 # Build all 4 variants
make all BUILD_MODE=dev  # Build with debug symbols
make package-all         # Build + sign all
make index               # Regenerate compile_commands.json
```

### Build Modes

| Mode | Size | Features |
|------|------|----------|
| production | ~640KB WASM, ~900KB package | `-Oz -flto -DNDEBUG`, no LOG_DEBUG/INFO |
| dev | ~2.9MB WASM, ~3MB package | `-O1 -g`, debug symbols, all logging |

### Variants

| Variant | Resolution | Mode | Stream |
|---------|-----------|------|--------|
| live_day | 1920x1080 | Live | Day |
| live_thermal | 900x720 | Live | Thermal |
| recording_day | 1920x1080 | Recording | Day |
| recording_thermal | 900x720 | Recording | Thermal |

### Quality Gates

Runs automatically before builds:
- `clang-format` - GNU Coding Standard
- `clang-tidy` - Warnings as errors

### Output Locations

- `build/*.wasm` - Compiled WASM binaries
- `dist/*.tar` - Signed packages
- `dist/*.png` - PNG snapshots
- `logs/` - Build logs

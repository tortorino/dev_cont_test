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
- **Unified interface**: `make` commands work from both host and container

### Build Commands

Use `make` from anywhere (host or container):

```bash
# Build all 4 production WASM
make all

# Build all 4 dev WASM (debug symbols)
make all-dev

# Build + sign packages
make package-all      # production
make package-all-dev  # dev

# Full CI pipeline
make ci
```

On the host, these automatically delegate to `./tools/devcontainer-build.sh`.

### Additional Host Commands

The wrapper script provides some extra commands:

```bash
./tools/devcontainer-build.sh shell    # Interactive container shell
./tools/devcontainer-build.sh exec "cmd"  # Run arbitrary command
```

### IDE Support (CLion)

Inside the container, additional targets are available:

```bash
make                     # Build recording_day only
make recording_day       # Build single variant
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

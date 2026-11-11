# Architecture

## Principles

1. Keep gameplay and simulation independent of Android, Windows, Linux, and macOS APIs.
2. Keep platform integration behind SDL3 and narrow engine interfaces.
3. Prefer explicit ownership, fixed-capacity pools, and predictable data layouts.
4. Separate game content from engine code.
5. Keep the first playable prototype small; expand only when profiling and gameplay require it.

## Planned layers

### Platform
Window lifecycle, input events, audio device access, timing, and platform file locations. SDL3 is the initial integration layer.

### Engine core
Application lifecycle, logging, frame timing, memory arenas/pools, resource handles, and configuration.

### Renderer
A custom 2D sprite renderer. The initial compatibility target is OpenGL ES 2.0 on Android. Renderer interfaces must not leak graphics API types into gameplay code.

### Simulation
Entity/component storage, transforms, movement, collision detection, combat, and AI. Simulation uses a fixed update step; rendering may interpolate between simulation states.

### World
Sector loading, local entity simulation, procedural content, stations, asteroids, and exploration.

### Game systems
Player progression, spacecraft modules, weapons, cargo, economy, missions, factions, and save/load.

## Initial repository modules

The directories will be introduced as their implementations become real. Avoid creating empty placeholder modules solely to make the tree look complete.

## Initial prototype

1. SDL3 application shell.
2. Input and fixed-step timing.
3. Pixel-art camera and sprite renderer.
4. Player ship movement.
5. Asteroids and collision.
6. One weapon and projectile pool.
7. One enemy and basic combat.
8. Local save/load.

## Platform strategy

Android is the primary shipping platform. Desktop builds are useful for faster iteration and debugging. Platform-specific code must remain isolated so the same gameplay source can be compiled for each target.

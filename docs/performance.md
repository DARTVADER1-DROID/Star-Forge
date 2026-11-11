# Performance Requirements

These are initial engineering budgets, not claims of measured performance. They must be validated on physical low-end Android hardware and revised using profiling data.

## Device target

- Minimum compatibility goal: Android devices with 1 GB total system RAM.
- Android is the primary platform.
- The game must be playable offline.
- Low-end devices take priority over optional visual effects.

## Initial budgets

| Metric | Initial target |
|---|---:|
| Frame rate on minimum target | 30 FPS |
| Frame rate on capable devices | 60 FPS |
| Game process working set | Aim for 100-150 MB |
| Short peak working-set ceiling | Aim to remain below 200 MB |
| Internal render resolution | Start at 320 x 180 |
| Simulation | Fixed timestep; decouple rendering |

The process-memory targets are ambitious and not guaranteed on every 1 GB Android device. Android itself, GPU allocations, audio buffers, and other processes affect available memory. Measure Java/native heap, PSS, and graphics memory where tooling permits.

## Required practices

- Use bounded pools for projectiles, particles, and frequently created entities.
- Avoid allocation and deallocation in hot update/render loops.
- Stream sector assets instead of loading the whole universe.
- Release resources when sectors are unloaded.
- Use texture atlases and batch compatible sprites.
- Cull off-screen objects.
- Use spatial partitioning for collision candidates.
- Reduce update frequency or simplify distant simulation.
- Make particle counts and effects scalable.
- Handle Android pause, resume, memory-pressure, and process-recreation behavior.
- Save atomically and recover from interrupted writes.

## Performance acceptance tests

- Long-duration playtest with no unbounded memory growth.
- Sector transitions without persistent resource accumulation.
- Combat stress test with many projectiles and enemies.
- Stable frame pacing, not merely a high average FPS.
- Thermal and battery checks during extended play.
- Test on at least one genuine low-memory device before claiming 1 GB compatibility.

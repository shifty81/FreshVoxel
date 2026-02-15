# Fresh Voxel Engine Architecture Diagram

## System Overview

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                         FRESH VOXEL ENGINE ARCHITECTURE                      │
│                                                                              │
│  Three Core Pillars for Ease of Development & Testing                       │
└─────────────────────────────────────────────────────────────────────────────┘

┌────────────────────────────────────┐
│  1. Component-Based Architecture   │
│     (What entities ARE)            │
├────────────────────────────────────┤
│                                    │
│  Entity (ID)                       │
│    ├─ HealthComponent              │
│    ├─ MovementComponent            │
│    ├─ TransformComponent           │
│    └─ RigidBody2DComponent         │
│                                    │
│  EntityManager                     │
│    ├─ createEntity()               │
│    ├─ addComponent()               │
│    └─ getEntitiesWithComponents() │
│                                    │
│  Systems                           │
│    ├─ CombatSystem                 │
│    ├─ Physics2DSystem              │
│    └─ Collision2DSystem            │
│                                    │
└────────────────────────────────────┘
              │
              │ Components provide data
              ▼
┌────────────────────────────────────┐
│  2. Event-Driven Architecture      │
│     (How systems COMMUNICATE)      │
├────────────────────────────────────┤
│                                    │
│  EventSystem (Singleton)           │
│    ├─ subscribe()                  │
│    ├─ emit()                       │
│    ├─ queueEvent()                 │
│    └─ processEvents()              │
│                                    │
│  Event Types                       │
│    ├─ entity_damaged               │
│    ├─ entity_died                  │
│    ├─ door_opened                  │
│    ├─ enemy_spotted                │
│    └─ noise_made                   │
│                                    │
│  EventData (key-value pairs)       │
│    ├─ set("damage", 50.0f)         │
│    └─ get<float>("damage")         │
│                                    │
└────────────────────────────────────┘
              │
              │ Events trigger reactions
              ▼
┌────────────────────────────────────┐
│  3. Data-Driven Design             │
│     (How values are CONFIGURED)    │
├────────────────────────────────────┤
│                                    │
│  Configuration Files (JSON)        │
│    ├─ character_stats.json         │
│    │    ├─ player: health, speed   │
│    │    └─ enemies: stats          │
│    ├─ weapon_stats.json            │
│    │    ├─ damage, range           │
│    │    └─ special_effects         │
│    └─ voxel_materials.json         │
│         └─ block properties        │
│                                    │
│  Hot Reload                        │
│    └─ Edit JSON → Instant update   │
│                                    │
└────────────────────────────────────┘
```

## Component-Based Architecture Flow

```
Player wants to create an enemy
         │
         ▼
┌─────────────────────────────┐
│ 1. Create Entity            │
│    Entity enemy =           │
│    entityManager            │
│    ->createEntity()         │
└─────────────────────────────┘
         │
         ▼
┌─────────────────────────────┐
│ 2. Add Components           │
│    addComponent(enemy,      │
│      HealthComponent)       │
│    addComponent(enemy,      │
│      MovementComponent)     │
│    addComponent(enemy,      │
│      AIComponent)           │
└─────────────────────────────┘
         │
         ▼
┌─────────────────────────────┐
│ 3. Systems Process          │
│    CombatSystem updates     │
│      all HealthComponents   │
│    MovementSystem updates   │
│      all MovementComponents │
└─────────────────────────────┘
```

## Event-Driven Communication Flow

```
Player shoots enemy
         │
         ▼
┌──────────────────────────────────┐
│ CombatSystem                     │
│   combatSystem                   │
│   ->applyDamage(enemy, 50.0f)    │
│                                  │
│   1. Update HealthComponent      │
│   2. Emit "entity_damaged" event │
└──────────────────────────────────┘
         │
         │ Event broadcast
         ├─────────────┬─────────────┬──────────────┐
         ▼             ▼             ▼              ▼
┌──────────────┐ ┌──────────┐ ┌────────────┐ ┌──────────┐
│ AudioSystem  │ │ Particle │ │ AISystem   │ │ UISystem │
│              │ │ System   │ │            │ │          │
│ Play damage  │ │ Blood    │ │ Alert      │ │ Show     │
│ sound        │ │ splatter │ │ nearby     │ │ damage   │
│              │ │          │ │ enemies    │ │ number   │
└──────────────┘ └──────────┘ └────────────┘ └──────────┘

All systems react independently, no direct coupling!
```

## Data-Driven Configuration Flow

```
Designer wants to balance enemies
         │
         ▼
┌────────────────────────────────┐
│ Edit config/character_stats.json│
│                                 │
│ "enemy_zombie": {               │
│   "health": 50.0,  ← Change     │
│   "speed": 2.5,    ← Change     │
│   "damage": 12.0   ← Change     │
│ }                               │
└────────────────────────────────┘
         │
         ▼
┌────────────────────────────────┐
│ Save file                       │
└────────────────────────────────┘
         │
         ▼
┌────────────────────────────────┐
│ Restart game (or hot-reload)    │
└────────────────────────────────┘
         │
         ▼
┌────────────────────────────────┐
│ New values loaded automatically │
│ No recompilation needed!        │
└────────────────────────────────┘
         │
         ▼
┌────────────────────────────────┐
│ Test new balance                │
│ Repeat until fun!               │
└────────────────────────────────┘
```

## Testing Architecture

```
┌──────────────────────────────────────────────────────────────┐
│                        Testing Pyramid                        │
└──────────────────────────────────────────────────────────────┘

                    /\
                   /  \         End-to-End
                  /E2E \        Manual playtesting
                 /      \       Complex scenarios
                /________\
               /          \
              /Integration \    System interactions
             /    Tests     \   CombatSystemTests.cpp
            /                \  Event-driven tests
           /___________________\
          /                     \
         /      Unit Tests       \  Component isolation
        /    Fast & Reliable      \ HealthComponentTests.cpp
       /                           \ MovementComponentTests.cpp
      /_____________________________\

Testing Benefits:
✓ Fast feedback (unit tests in milliseconds)
✓ Catch regressions early
✓ Confident refactoring
✓ Documentation through examples
```

## Component System Interaction Example

```
┌───────────────────────────────────────────────────────────────┐
│  Example: Enemy Takes Damage                                  │
└───────────────────────────────────────────────────────────────┘

Entity: Zombie (ID: 123)
  │
  ├─ HealthComponent
  │    ├─ currentHealth: 50.0
  │    └─ maxHealth: 50.0
  │
  ├─ MovementComponent
  │    ├─ maxSpeed: 2.5
  │    └─ velocity: (1, 0, 0)
  │
  └─ AIComponent
       └─ state: Chasing

Player attacks zombie
         │
         ▼
┌────────────────────────────────┐
│ CombatSystem                   │
│ applyDamage(zombie, 25.0f)     │
└────────────────────────────────┘
         │
         ├─ Update Component
         │      │
         │      ▼
         │  HealthComponent
         │  currentHealth: 50.0 → 25.0
         │
         └─ Emit Event
                │
                ▼
         EventSystem.emit("entity_damaged", {
           entity: 123,
           damage: 25.0,
           health: 25.0
         })
                │
                ├──────────────┬───────────────┬──────────────┐
                ▼              ▼               ▼              ▼
         ┌──────────┐   ┌──────────┐   ┌──────────┐   ┌──────────┐
         │ Audio    │   │ Particle │   │ AI       │   │ UI       │
         │ System   │   │ System   │   │ System   │   │ System   │
         └──────────┘   └──────────┘   └──────────┘   └──────────┘
         Play sound     Blood effect   Alert others   Show -25

Result: Multiple systems react to single event without direct coupling!
```

## Emergent Gameplay Example

```
┌───────────────────────────────────────────────────────────────┐
│  Emergent Behavior: Chain Reaction Explosions                 │
└───────────────────────────────────────────────────────────────┘

Simple Rules:
  1. Barrel has HealthComponent + ExplosiveComponent
  2. When barrel dies → emit "explosion" event
  3. Explosion damages nearby entities

Emergent Result:

Player shoots Barrel 1
         │
         ▼
Barrel 1 dies (health → 0)
         │
         ▼
"entity_died" event → Check for ExplosiveComponent
         │
         ▼
Emit "explosion" event (radius: 5.0)
         │
         ├─────────────────────────┬──────────────────────┐
         ▼                         ▼                      ▼
    Barrel 2                  Barrel 3               Enemy
    (in radius)               (in radius)            (in radius)
         │                         │                      │
         ▼                         ▼                      ▼
    Takes damage              Takes damage           Takes damage
         │                         │                      │
         ▼                         ▼                      ▼
    Dies → explodes!          Dies → explodes!       Dies
         │                         │
         └─────────┬───────────────┘
                   ▼
         More explosions!
         CHAIN REACTION!

Programmer wrote simple rules, complex behavior emerges!
```

## Development Workflow

```
┌───────────────────────────────────────────────────────────────┐
│  Rapid Iteration Cycle                                        │
└───────────────────────────────────────────────────────────────┘

┌──────────────┐
│ 1. PROTOTYPE │  Create component + basic system
│   (1 hour)   │  Focus: Make it work
└──────┬───────┘
       │
       ▼
┌──────────────┐
│ 2. PLAYTEST  │  Gather feedback
│   (30 min)   │  What feels good? What's broken?
└──────┬───────┘
       │
       ▼
┌──────────────┐
│ 3. ITERATE   │  Adjust JSON configs
│   (30 min)   │  No recompilation!
└──────┬───────┘
       │
       ▼
┌──────────────┐
│ 4. REPEAT    │  Iterate until fun
│              │  2-3 hours from idea to polished
└──────┬───────┘
       │
       ▼
┌──────────────┐
│ 5. POLISH    │  Refactor code
│   (1 hour)   │  Tests ensure nothing breaks
└──────────────┘

Total: ~3-4 hours from idea to production-ready feature!
```

## File Structure

```
fresh/
├── engine/ecs/                    # Component-Based Architecture
│   ├── Entity.h                    # Lightweight ID
│   ├── IComponent.h                # Component base class
│   ├── EntityManager.h             # ECS coordinator
│   ├── SystemBase.h                # System base class
│   ├── HealthComponent.h           # NEW: Health management
│   ├── MovementComponent.h         # NEW: Movement behavior
│   └── CombatSystem.h              # NEW: Combat with events
│
├── engine/scripting/              # Event-Driven Architecture
│   └── EventSystem.h               # Event pub-sub system
│
├── config/                         # Data-Driven Design
│   ├── character_stats.json        # NEW: Character properties
│   └── weapon_stats.json           # NEW: Weapon properties
│
├── tests/ecs/                      # Automated Testing
│   ├── HealthComponentTests.cpp    # NEW: 12 unit tests
│   ├── MovementComponentTests.cpp  # NEW: 11 unit tests
│   └── CombatSystemTests.cpp       # NEW: 12 integration tests
│
└── docs/architecture/              # Documentation
    ├── COMPONENT_BASED_ARCHITECTURE.md  # NEW: 11KB guide
    ├── EVENT_DRIVEN_ARCHITECTURE.md     # NEW: 17KB guide
    ├── DATA_DRIVEN_DESIGN.md            # NEW: 17KB guide
    └── README.md                        # NEW: Overview
```

## Benefits Summary

```
┌────────────────────────────────────────────────────────────┐
│  Benefits Achieved                                         │
└────────────────────────────────────────────────────────────┘

FOR PROGRAMMERS:
✓ Modular code (easy to understand)
✓ Decoupled systems (easy to modify)
✓ Reusable components (write once, use everywhere)
✓ Confident refactoring (tests catch breaks)
✓ Fast iteration (change → test in seconds)

FOR DESIGNERS:
✓ No coding required (edit JSON files)
✓ Instant feedback (hot-reload)
✓ Easy experimentation (try different values)
✓ A/B testing (compare configurations)

FOR TESTERS:
✓ Fast unit tests (milliseconds)
✓ Easy to write tests (components isolated)
✓ Good coverage (35+ new tests)
✓ Catch bugs early (automated regression tests)

FOR PLAYERS:
✓ More polish (faster iteration = better quality)
✓ Better balance (easy to tune)
✓ Emergent gameplay (complex interactions)
✓ Modding support (edit configs)
```

## Architecture Principles

1. **Separation of Concerns**
   - Components = Data
   - Systems = Logic
   - Events = Communication
   - Configs = Values

2. **Loose Coupling**
   - Components don't know about each other
   - Systems communicate via events
   - Easy to add/remove features

3. **High Cohesion**
   - Each component has single responsibility
   - Each system focuses on one task
   - Clear, focused code

4. **Open-Closed Principle**
   - Open for extension (add new components)
   - Closed for modification (existing code unchanged)

5. **Testability**
   - Components testable in isolation
   - Systems testable via event simulation
   - Fast, reliable tests

## Next Steps

To learn more about each pillar:

1. **Components**: Read [COMPONENT_BASED_ARCHITECTURE.md](COMPONENT_BASED_ARCHITECTURE.md)
2. **Events**: Read [EVENT_DRIVEN_ARCHITECTURE.md](EVENT_DRIVEN_ARCHITECTURE.md)
3. **Data**: Read [DATA_DRIVEN_DESIGN.md](DATA_DRIVEN_DESIGN.md)
4. **Iteration**: Read [../guides/ITERATIVE_DEVELOPMENT.md](../guides/ITERATIVE_DEVELOPMENT.md)
5. **Testing**: Read [../guides/TESTING_BEST_PRACTICES.md](../guides/TESTING_BEST_PRACTICES.md)

Or see [ARCHITECTURE_IMPLEMENTATION_SUMMARY.md](../history/ARCHITECTURE_IMPLEMENTATION_SUMMARY.md) for complete details.

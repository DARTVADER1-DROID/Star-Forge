# Star-Forge — Game Design Foundations

**Status:** Design draft based on currently agreed direction. Numerical balance, individual weapon definitions, and detailed galaxy generation rules remain open.

## 1. Core concept

Star-Forge is an original, offline-first, top-down 2D pixel-art space sandbox RPG. The player explores a galaxy, travels between star systems using faster-than-light (FTL) travel, visits planets, trades commodities, acquires and customizes ships, and eventually engages with combat and other RPG systems.

The galaxy is composed of distinct star systems. Ordinary interstellar travel is not available: reaching another system requires FTL travel. Each system contains at least two landable planets.

## 2. Ship taxonomy

Ships have two independent classification dimensions:

- **Type:** the hull archetype. It defines the ship's basic scale, structural envelope, mass/capacity potential, and baseline performance.
- **Class:** the operational specialization. It changes how the hull is configured and what trade-offs it makes.

The combination produces 18 initial type/class combinations (6 types × 3 classes). These are configurations of a shared system, not 18 separate hard-coded ship implementations.

### 2.1 Ship types

The six agreed ship types are:

1. **Runner** — prioritizes speed and agility; suited to rapid travel, courier work, scouting, and evasion.
2. **Destroyer** — a combat-oriented hull with substantial weapons and defensive potential.
3. **Corvette** — a smaller, relatively agile combat-capable hull.
4. **Frigate** — a medium multipurpose hull with room for a useful mix of equipment and weapons.
5. **Battleship** — a large capital hull with high structural strength and substantial weapon capacity.
6. **Flagship** — a strategic command-scale hull with extensive equipment and systems capacity.

These are role descriptions, not final numerical specifications. Exact hull sizes, masses, hardpoint layouts, and baseline statistics are to be defined during balancing.

### 2.2 Ship classes

**Civilian**
- Optimized for speed and fuel efficiency.
- General-purpose civilian equipment and useful cargo capacity.
- Limited combat capability and less protection than a military fit.

**Cargo**
- Strong emphasis on available cargo volume for transporting commodities between systems.
- Sacrifices armor and hull strength.
- Generally slower than comparable civilian or military configurations.
- Cargo capacity is a primary identity, not a minor bonus.

**Military**
- Optimized for weapon-layout capability, speed, and strength.
- Better combat protection and weapon support.
- Poor fuel efficiency and medium cargo capacity.
- Weapon mounting capability depends on the hull's actual hardpoints and supported mount sizes.

Class descriptions are design intent. Exact modifiers and restrictions are not yet locked.

## 3. Ship statistics

The ship-stat model should keep these concerns distinct:

| Group | Candidate statistics |
|---|---|
| Mobility | Maximum speed, acceleration, turn rate, maneuverability |
| Structure | Hull integrity, armor, shield capacity |
| Logistics | Cargo volume, fuel capacity, fuel consumption / efficiency |
| Weapons | Hardpoint count, mount size/type compatibility, weapon power budget |
| Engineering | Power generation, power consumption, module capacity |
| Operations | Crew capacity, maintenance and operating costs |

Important distinctions:
- Hull integrity is the amount of structural damage a ship can sustain.
- Armor mitigates incoming damage to the hull.
- Shields absorb damage before it reaches armor and hull.
- Cargo volume is not the same as cargo mass. Heavy cargo may affect ship mass, acceleration, and fuel use.
- A hardpoint is a physical attachment location; it is not itself a weapon or a damage statistic.

### 3.1 Stat composition

The planned model is data-driven:

1. Load the hull type's baseline characteristics.
2. Apply the selected ship class's modifiers and restrictions.
3. Apply the individual ship model's specific values and hardpoint layout.
4. Apply installed upgrades and equipment.
5. Calculate derived runtime values, such as acceleration, effective speed, and fuel consumption.

Earlier illustrative class multipliers and 1–10 archetype ratings are provisional examples only. They are **not approved balance values** and must not be treated as final game data.

## 4. Ship mass and weapon mounting

Every ship has a defined mass and a weapon-mounting layout. Weapon installation is constrained by both the ship's layout and its remaining available mass.

### 4.1 Mass budget

Ship mass is a core design and equipment constraint:
- Each hull has a base mass.
- The ship has a maximum equipment/load mass budget (exact terminology and formula to be finalized).
- Installed weapons add their own mass.
- Installing a weapon reduces the ship's remaining available mass by that weapon's mass.
- Other equipment and cargo may also contribute to total mass; the exact accounting rules remain to be specified.
- Available mass must never become negative. The installation system must reject an installation that exceeds the hull's permitted mass limit.
- Total mass feeds into derived mobility and fuel calculations, so fitting heavier weapons has consequences beyond the weapon slot itself.

The data model should distinguish at least: base hull mass, installed equipment mass, cargo mass, total mass, maximum permitted mass, and remaining available mass. Whether the maximum is a hard equipment limit or a general maximum operating mass needs a final design decision.

### 4.2 Weapon size classes

Weapons are divided into four mount/size classes:
1. **Small**
2. **Medium**
3. **Large**
4. **Extra-large**

A ship declares which weapon-size layouts it supports. A weapon can only be installed where the ship has a compatible layout/mount.

### 4.3 Weapon layout geometry

Each ship's layout definition specifies mounting information for the supported weapon sizes. A layout can include:
- Weapon size class.
- Mount position(s) on the hull.
- Firing arc angle for each mount or mount group.
- Mount orientation and any other geometry needed by the renderer and combat system.
- Maximum count or other mount restrictions, when applicable.

For example, a ship may have a Small-weapon layout with a defined firing arc. The player can browse the weapons market, select a compatible Small weapon, and install it into that layout. The weapon's visual attachment point and firing arc must match the ship's approved layout metadata.

Weapon size compatibility, physical mount geometry, and remaining mass are separate checks: all must pass before installation.

### 4.4 Weapon fusion / grouping rule

The current design states that only weapons of the same size class and compatible layout may be fused. The exact meaning and gameplay result of "fused" (for example, combining identical weapons into a linked mount, or merging them into one stronger weapon) is not yet defined. Preserve this as a design rule to clarify before implementation; do not assume fusion increases damage or changes weapon size.

## 5. Ship upgrades

Ships are upgradeable. Upgrades have distinctive, humorous/in-universe names and improve one or more ship statistics by a defined percentage.

Upgrade definitions should be data-driven and identify:
- Stable upgrade ID and display name.
- Applicable ship systems or compatible ship types/classes.
- Statistics affected.
- Percentage change for each affected statistic.
- Installation constraints, slot/power requirements, and costs (to be designed).
- Whether multiple copies or upgrade tiers can stack.

The percentage modifier should be explicit and inspectable in the ship configuration/UI. Stacking rules and whether modifiers are additive or multiplicative remain to be decided. Upgrades must respect hull limits so they improve a ship without erasing its archetype.

## 6. Galaxy and star systems

- The game world is an open galaxy made up of multiple star systems.
- Inter-system access requires FTL travel.
- Each star system contains two or more landable planets.
- Each planet has a population and a set of characteristics.
- Planet characteristics influence which commodities are available, produced, consumed, scarce, or desirable.
- Planets in the same system should be distinct places with their own economic and population profiles.

The exact FTL mechanics, map representation, travel costs/times, system generation, and planet trait catalogue have not yet been specified.

## 7. Fuel logistics and fleet range

Fuel is a consumable resource:
- FTL travel between star systems consumes fuel.
- Fuel is also a tradeable commodity that can be bought and sold through planetary markets.
- Fuel consumption must be accounted for when determining whether a ship or fleet can make an FTL jump.
- The exact FTL fuel formula, per-jump cost, tank capacity rules, and whether local/system travel consumes fuel remain open.

### 7.1 Fuel haulers

Cargo ships may include specialized **fuel haulers** designed to transport fuel for other ships.
- A fuel hauler carries fuel as dedicated fleet-support cargo.
- Fuel haulers can extend the effective FTL range of a fleet by supplying fuel to fleet members.
- Fuel haulers have their own class/configuration variants; their exact class taxonomy and relationship to the existing Civilian, Cargo, and Military classes are not yet defined.
- Fleet range should account for the fuel available across eligible fleet ships, fuel-hauler capacity, transfer rules, and the fuel required for the planned route.
- Fuel-hauler support should not imply infinite range: fuel capacity, consumption, and transfer limits still apply.

The design must later define whether fuel is transferred between ships before a jump, during a stop, or represented as a shared fleet fuel pool, and what happens if a fuel hauler is disabled or separated from the fleet.

## 8. Planetary economy and commodities

The initial commodity categories are:

1. Medicine
2. Food
3. Equipment
4. Metal
5. Luxury goods
6. Fuel

Each planet has:
- A population value.
- Planetary characteristics/traits.
- Commodity availability and market prices.

Commodity prices may differ from planet to planet based on their characteristics and economic conditions. This creates the basis for inter-system and interplanetary trading.

A future economy model can account for local production, consumption, population demand, scarcity, and trade supply. These are candidate pricing inputs, not finalized formulas. The initial design should keep commodity definitions separate from each planet's market stock and price.

## 9. Weapons market and ship mounting

- Weapons are purchasable through a weapons market.
- Weapons are attachable equipment, installed at compatible ship hardpoints.
- A ship's artwork and hardpoint metadata determine where a weapon is visually attached.
- Hull hardpoint definitions determine which weapon mount types/sizes can be installed.
- Weapon gameplay statistics and balancing are separate from sprite artwork and attachment coordinates.

The user will provide/document the weapon catalogue and weapon details later. Therefore, this document intentionally does not define individual weapons, damage values, firing rates, projectile properties, or final weapon-market rules.

## 10. Piracy, wrecks, and salvage

Piracy is a player-facing combat and economic activity. The player can attack other ships, destroy them, recover surviving cargo and equipment from the wreck, and sell recovered goods at planetary markets.

### 10.1 Ship destruction and wreck contents

- A destroyed ship can leave behind a wreck/salvage container in space.
- The wreck's cargo contents are generated from the cargo actually carried by that ship at the moment of destruction.
- Cargo loss/survival should be resolved by a defined destruction-and-salvage rule; do not assume every item always survives or that the wreck duplicates the original cargo.
- Wreck contents are independent of the player's cargo hold. A wreck may contain more than the player can collect.
- Wrecks may occasionally contain weapons that were installed on the destroyed ship. Weapon drops are not guaranteed; drop probability and weapon condition remain to be balanced.
- Recovered weapons remain individual equipment items with their own size class, mass, condition, and compatibility requirements. They must pass the normal ship-layout and available-mass checks before installation.

### 10.2 Loot collection

- The player approaches a wreck and hovers/positions their ship over the salvage to collect it.
- Collection transfers selected eligible items from the wreck into the player's cargo inventory.
- The collection system must enforce the player's available cargo volume and mass limits.
- If the player cannot carry everything, uncollected items remain in the wreck for a limited or otherwise defined period; persistence, despawn timing, and loss conditions are open design decisions.
- The player should be able to inspect the wreck contents and choose what to collect when capacity is limited.

### 10.3 Salvage economy

- Recovered trade commodities can be sold at planetary markets.
- Sale value uses the destination planet's current market price and stock/demand rules, rather than a fixed universal value.
- Recovered weapons can be sold through an appropriate weapons market or retained for installation, subject to compatibility.
- Piracy therefore connects combat, cargo management, ship fitting, and planetary trading into one gameplay loop.

### 10.4 Reputation and consequences

The existence of piracy is agreed, but its wider consequences are not yet specified. Possible systems include faction or local-security hostility, wanted status, patrol response, bounty/reputation changes, and restrictions at ports. Do not treat any of these as approved mechanics until designed.

## 11. Asset and data separation

The implementation should separate:
- **Visual asset metadata:** sprite path, dimensions, pivot, animation frames, hardpoint coordinates, engine attachment points, and collision geometry.
- **Hull/type data:** baseline ship characteristics and structural limits.
- **Class data:** class modifiers and equipment restrictions.
- **Upgrade data:** named percentage-based improvements and compatibility.
- **Weapon data:** weapon behaviour, statistics, price, and mount requirements.
- **World/economy data:** systems, planets, population, traits, commodity stock, and prices.

The engine should combine these definitions at runtime rather than embedding individual ships, planets, or weapon values in C source code. Sprite coordinates use the convention in [Asset Pipeline](asset-pipeline.md).

## 12. Open design decisions

The following are deliberately left open:
- Exact baseline statistics and balance values for all six ship types.\n- Whether maximum permitted mass is an equipment-only limit or includes cargo and fuel.\n- Whether weapon mass is the only equipment mass counted against the installation budget.\n- Exact weapon layout schema, mount positions, firing-arc conventions, and whether arcs are per mount or per layout.\n- Exact meaning and result of weapon fusion.\n- Fuel consumption per FTL jump and fuel tank/fleet range calculations.\n- Fuel-hauler variants/classes, fuel transfer mechanics, and fleet membership constraints.
- Exact numerical class modifiers and class/type restrictions.
- Upgrade names, upgrade catalogue, stacking rules, costs, and installation slots.
- FTL navigation, range, fuel/cost, and travel-time rules.
- Galaxy scale and system/planet generation.
- Planet characteristic/trait catalogue.
- Commodity production, consumption, stock simulation, and pricing formula.
- Weapons catalogue, weapon statistics, market rules, and mount-size taxonomy.
- Whether and how population changes over time.

## 13. Design constraints

- Preserve the offline-first requirement; the core galaxy, trading, and ship systems must not require a server.
- Keep content definitions data-driven and versionable.
- Avoid hardcoding numerical balance in engine code.
- Make the system efficient for low-memory Android devices; do not simulate every distant planet and system at full detail.
- Treat all numerical examples as provisional until explicitly approved and playtested.

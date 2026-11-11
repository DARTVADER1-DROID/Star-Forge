# Data catalog layout

Definitions are grouped by gameplay domain so new content can be added without growing one shared catalog file.

- `economy/items.json`: commodities and equipment metadata, prices and cargo volume.
- `ships/designs.json`: reusable player and NPC hull designs.
- `ships/roles.json`: NPC role behaviour and starting cargo manifests.
- `world/systems.json`: star-system identity, FTL links, local map metadata and population settings.
- `world/planets/`: reserved for reusable planet definitions and per-planet service/landing configuration.
- `world/maps/`: reserved for map-specific layouts, regions and navigation metadata.
- `services/`: reserved for service catalogs such as refuelling, trade, weapons markets, missions and shipyards.

Every definition must have a stable `id`. Systems should reference planet IDs, ships should reference design IDs, and service availability should reference service IDs. Keep display names separate from IDs so content can be renamed without breaking saves or links.

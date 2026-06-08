# 3D Print Settings — Self-Balancing Robot

**Printer:** Creality Ender 3 Max (300 x 300 x 340 mm build volume)
**Filament:** eSUN PETG (Gray)
**Nozzle:** 0.4 mm (stock brass)
**Slicer:** Cura / PrusaSlicer / OrcaSlicer — settings below are slicer-agnostic

---

## General PETG Settings

| Parameter | Value |
|-----------|-------|
| Layer height | 0.2 mm |
| First layer height | 0.28 mm |
| Line width | 0.44 mm |
| Wall count (perimeters) | 3 |
| Top layers | 4 |
| Bottom layers | 4 |
| Infill density | 25% |
| Infill pattern | Gyroid |
| Nozzle temperature | 240 °C |
| First layer nozzle temp | 240 °C |
| Bed temperature | 70 °C |
| First layer bed temp | 75 °C |
| Print speed | 50 mm/s |
| First layer speed | 25 mm/s |
| Wall speed | 40 mm/s |
| Travel speed | 150 mm/s |
| Retraction distance | 6.5 mm |
| Retraction speed | 25 mm/s |
| Cooling fan | 30–50% (off for first 2 layers) |
| Z-hop | 0.2 mm (enabled) |
| Adhesion | None (clean glass + glue stick) |

---

## Per-Part Settings

Parts that need different settings from the general profile are noted below. All others use the general settings above.

### Structural / Leg Parts (high stress)

| STL File | Qty | Infill | Walls | Orientation | Notes |
|----------|:---:|:------:|:-----:|-------------|-------|
| `left top leg.stl` | 1 | 40% | 4 | Flat on widest face | Load-bearing arm — extra strength |
| `left bottom leg .stl` | 1 | 40% | 4 | Flat on widest face | Load-bearing arm — extra strength |
| `right top leg .stl` | 1 | 40% | 4 | Flat on widest face | Mirror of left top leg |
| `right bottom leg.stl` | 1 | 40% | 4 | Flat on widest face | Mirror of left bottom leg |

### Body Panels

| STL File | Qty | Infill | Walls | Orientation | Notes |
|----------|:---:|:------:|:-----:|-------------|-------|
| `Body_Side_Panel.stl` | 1 | 25% | 3 | Flat (outer face down) | Right side panel |
| `Body_Side_Panel left.stl` | 1 | 25% | 3 | Flat (outer face down) | Left side panel |
| `front flat.stl` | 1 | 25% | 3 | Flat | Front panel — thin part, print slow (40 mm/s) |
| `front left.stl` | 1 | 25% | 3 | Flat | Front left panel |
| `back.stl` | 1 | 25% | 3 | Flat | Rear panel |
| `back right.stl` | 1 | 25% | 3 | Flat | Rear right panel |

### Chassis / Floors

| STL File | Qty | Infill | Walls | Orientation | Notes |
|----------|:---:|:------:|:-----:|-------------|-------|
| `top.stl` | 1 | 25% | 3 | Flat (top face up) | Floor 1 — mounts Pi 5, ESP32, IMU, display |
| `bottom.stl` | 1 | 20% Lines | 3 | Flat (top face up) | Floor 2 — low stress, faster print |
| `floor 2.stl` | 1 | 25% | 3 | Flat | Internal floor divider |

---

## Support Settings

| Parameter | Value |
|-----------|-------|
| Support type | Tree (Cura) / Organic (PrusaSlicer/OrcaSlicer) |
| Overhang threshold | 50° |
| Support density | 10% |
| Support Z distance | 0.2 mm (1 layer gap) |
| Support XY distance | 0.8 mm |
| Support interface | Enabled |
| Interface layers | 2 top / 1 bottom |
| Interface density | 80% |
| Interface pattern | Grid |
| Support material | Same (eSUN PETG Gray) |

### Which parts likely need supports

| Part | Supports? | Why |
|------|-----------|-----|
| Leg parts (all 4) | Likely yes | Pivot holes, bearing seats, or curved linkage geometry |
| Body side panels | Maybe | Depends on screw boss or mounting tab angles |
| Front/back panels | Probably no | Flat panels, print face-down |
| Floors (top, bottom, floor 2) | Probably no | Flat plates — print flat, no overhangs |

### Tips for supports with PETG

- **Tree/organic supports are strongly recommended** over normal supports. PETG supports are notoriously hard to remove — tree supports use less material and have smaller contact points.
- **Increase Z distance to 0.24 mm** if supports are fusing to the part surface. PETG is stickier than PLA.
- **Preview in slicer before printing** — check every part for unsupported overhangs. Rotate parts to minimize support usage where possible.
- **Support blockers** — if the slicer adds supports to small holes or features that can bridge fine, use manual support blockers to remove them.

---

## Ender 3 Max PETG Tips

- **Bed adhesion:** Clean glass bed with IPA, apply thin glue stick layer. PETG bonds aggressively to bare glass — glue stick acts as a release agent.
- **PTFE tube vs eSUN temps:** eSUN PETG is rated for 230–250 °C (optimal 245 °C), but the stock Bowden PTFE tube degrades above 240 °C. We cap at 240 °C as a compromise. If you see poor layer adhesion, upgrade to a Capricorn PTFE tube (rated to 260 °C) or an all-metal hotend so you can print at eSUN's recommended 245 °C.
- **Stringing:** PETG strings more than PLA. If stringing is excessive, increase retraction to 6 mm or reduce nozzle temp to 230 °C. Enable "combing" in Cura or "avoid crossing perimeters" in PrusaSlicer.
- **First layer:** Slightly increase Z-offset (+0.02–0.04 mm) vs PLA. PETG squished too flat will stick permanently to glass.
- **Cooling:** Keep fan low (30–50%). Too much cooling causes layer delamination with PETG. No fan on first 2 layers.
- **Enclosure:** Not required for Ender 3 Max, but if ambient temp is below 20 °C, use a draft shield or cardboard enclosure to prevent warping on large body panels.
- **Drying:** PETG absorbs moisture. If you hear popping/crackling during printing, dry filament at 65 °C for 4–6 hours before use.
- **Print order suggestion:** Print body panels and floors first (low risk), then legs last (structural, need dialed-in settings).

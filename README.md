
# GePatchUltra ,  Native-Res Renderer Hand-Off for Adrenaline

**Goal:** run PSP content (games, homebrew, optionally the PSP XMB) at the Vita’s native **960×544** with high performance and high fidelity, without changing game code.

## What this is (and isn’t)

* This project hooks the **PSP GE (Graphics Engine) display list** inside Adrenaline (the Vita’s PSP emulator), **captures GE commands**, and **hands them off** to a Vita-side renderer which replays them natively at 960×544.
* Think of it as a “**renderer hand-off**”: the PSP CPU still does PSP work; we **tunnel out** just the rendering and draw it natively on the Vita GPU.
* It’s **not** an emulator from scratch. It cooperates with Adrenaline and reuses well-documented GE behavior (and later, PPSSPP’s proven GPU logic where sensible).



## High-level architecture

```
[ PSP game / homebrew ]
          │
          ▼
  [Adrenaline PSP emu]
          │
   ┌───────────────────────────────────────────────────┐
   │ PSP-side PRX    (kernel plugin)                   │
   │  • Hook GE list enqueue / stall / finish          │
   │  • Resolve BASE/OFFSET and pack GE commands       │
   │  • Stream resources (verts/indices/textures/CLUT) │
   └───────────────────────────────────────────────────┘
          │  (Kermit transport)
          ▼
   ┌───────────────────────────────────────────────────┐
   │ Vita renderer │  (userland app/plugin)            │
   │  • Reconstruct GE state                           │
   │  • Replay draws via GXM/vitaGL at 960×544         │
   │  • Present once per GE FINISH/vblank              │
   └───────────────────────────────────────────────────┘
          │
          ▼
      [Vita display]
```

* **PSP side (PRX)**: walks GE lists (no destructive edits), packets up commands + resource refs, and streams them out.
* **Vita side**: receives the stream, manages texture/RT caches, and renders using the Vita GPU.


## Roadmap (milestones & checkboxes)

* [x] **M0: Device bootstrap** – PSP PRX compiles, loads, and ACKs on device.
* [ ] **M1: Vita bridge alive** – Build/install a Vita app (or plugin) that logs once.
* [ ] **M2: Transport v1** – Reliable packet stream (frame begin/end, GE_CMD, resource refs/data) with ACK/windowing.
* [ ] **M3: Renderer bootstrap** – GXM/vitaGL init at 960×544; vsync’d clear/present.
* [ ] **M4: Minimal GE subset** – THROUGH vertices, pos decoding (16/F32), TRIANGLES/STRIP/FAN/LINES; draw a triangle.
* [ ] **M5: Texture path** – Tex formats (565/5551/4444/8888, CLUT4/8), filters/wrap, texcoord decoding; draw a textured quad.
* [ ] **M6: Depth/blend/alpha/stencil** – Common fixed-function parity for typical scenes.
* [ ] **M7: Render-to-texture & transfers** – Framebuffer textures, `TRANSFER*` GPU blits, alias tracking.
* [ ] **M8: Control flow & sync** – SIGNAL/CALL/JUMP/RET behavior; FINISH→present once; multi-list frames.
* [ ] **M9: Performance pass** – Texture/vertex dedup, sub-region uploads, batching; HUD telemetry.
* [ ] **M10: Compatibility sweep** – Tough titles, toggles/fallbacks, VSH overlay quirks documented.



## Does this make **all** of Adrenaline 960×544?

* We upscale **PSP content** (games, homebrew, and, if you enable the plugin for VSH, the PSP XMB).
* The **Vita app shell / LiveArea** is already native.
* Enable in `vsh.txt` if you want the XMB rendered natively too (note: the VSH overlay menu may be invisible until later milestones address overlays).



## Repository layout

```
ge-patch-ultra/
  common/            # protocol + framing (for host tests and later Kermit)
  host_mock/         # local TCP sender/receiver for early testing
  pspemu_plugin/     # PSP kernel PRX (GePatch-based)
  vita_bridge/       # simple Vita app used in M1 to prove Vita-side is alive
  CMakeLists.txt     # builds host_mock (optional)
  README.md
```



## Building (macOS on Apple Silicon using Docker)

> You don’t need to install toolchains natively. We use Docker images for PSP and Vita builds.

### 1) Prereqs

```bash
# One-time pulls (amd64 images run fine on Apple Silicon via emulation)
docker pull --platform linux/amd64 pspdev/pspdev:latest
docker pull --platform linux/amd64 vitasdk/vitasdk:latest
```

### 2) Build the PSP PRX (Milestone 0)

From the repo root:

```bash
cd pspemu_plugin
# Make sure your Makefile does NOT have a Windows "all:" copy rule at the top.
docker run --rm -it --platform linux/amd64 \
  -v "$PWD":/work -w /work pspdev/pspdev:latest \
  bash -lc 'export PSPDEV=/usr/local/pspdev; export PATH=$PSPDEV/bin:$PATH; make -j$(nproc)'
# -> produces ge_patch.prx
```

### 3) Install the PRX on your Vita

1. On the Vita, open **VitaShell** → **START** → set **USB device = ux0:** → **SELECT** to mount.
2. Copy `pspemu_plugin/ge_patch.prx` to:

   ```
   ux0:/pspemu/seplugins/ge_patch.prx
   ```
3. Ensure these files exist:

   ```
   ux0:/pspemu/seplugins/game.txt   ->  ms0:/seplugins/ge_patch.prx 1
   # Optional (PSP XMB at native res):
   ux0:/pspemu/seplugins/vsh.txt    ->  ms0:/seplugins/ge_patch.prx 1
   ```
4. Launch **Adrenaline** → run a PSP app (e.g., PSP Filer).
5. Verify Milestone 0: in VitaShell, open:

   ```
   ux0:/pspemu/ge_ack.txt   # should contain: GePatch module_start OK
   ```

### 5) (Optional) Build the host mock

From repo root:

```bash
cmake -S . -B build
cmake --build build
# Binaries: build/host_mock/ge_sender, build/host_mock/ge_receiver
```

Run receiver in one terminal and sender in another to see framed packet exchange:

```bash
./build/host_mock/ge_receiver
./build/host_mock/ge_sender
```



## Using it

* **Games:** ensure `game.txt` enables the PRX. Launch Adrenaline → your games.
* **PSP XMB (optional):** enable the PRX in `vsh.txt`.
* **Until M3+** there’s no visible native rendering yet, Milestone 0/1 are “endpoints alive”. Visual native rendering begins when the Vita renderer starts drawing in M3+.



## Contributing

* Keep commits small and testable (one opcode/state per commit is ideal).
* Prefer primary sources for GE behavior; when reusing PPSSPP logic, preserve license headers.
* Add a **before/after** screenshot or trace snippet to PR descriptions.



## License & credits

This project includes code derived from:

* **GePatch** by TheFloW (GPL-2.0) and PSPSDK BSD-licensed parts.
* Future renderer stages may reuse portions of **PPSSPP** (GPL-2.0+).

Accordingly, this repository is distributed under **GPL-2.0 (or later)**. See LICENSE files for details.

**Thanks:** TheFloW, PPSSPP contributors, and the wider PSP/Vita research community for the GE specs and prior art.



## FAQ

**Q: Will this make everything on Vita 960×544?**
A: It makes **PSP content** native: games, homebrew, and (optionally) the PSP XMB if enabled via `vsh.txt`. The Vita’s own UI is already native.

**Q: Why not just upscale the PSP framebuffer?**
A: Pure upscale blurs; this approach **replays geometry and pixels natively**, enabling sharper IQ and future upgrades (e.g., better filtering).

**Q: Is performance good?**
A: Milestones M9/M10 focus on bandwidth reductions, batching, and GPU blits; the design aims for stable vsync in typical games.

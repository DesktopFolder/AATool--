# trAAcker
## A cross-platform Minecraft 1.16.1 advancements tracker

This application is a reimplementation of a subset of the utilities provided by [CTM's AATool](https://github.com/DarwinBaker/AATool) to be cross-platform.

See [the todo list](todo.md) for a potentially more comprehensive list of currently planned / WIP features.

**Implementation status**:
- [ ] Automatic focused instance detection
    - [ ] Windows
    - [x] macOS - functional, does not work with watches yet.
    - [ ] Linux
- [ ] Automatic world detection
- [x] Integrated file update detection
- [ ] **Overlay Feature Parity**
    - [x] Renders all 80 advancements and the normal set of criteria (biomes, foods, etc)
    - [x] Can update to reflect the game state given an advancements JSON file
    - [x] Automatic updating to the most recent file in a saves/ folder
    - [x] Configurable rendering positions / sizes
    - [x] Text rendering for advancement names
    - [ ] Frames for advancements
    - [ ] Full automatic updating support
    - [ ] Configurable fonts / text display? *not currently planned, but aliasing and size can be configured*
- [ ] **Main window features**
    - [ ] Default (CTM) layout
    - [ ] Automatic layout (resizing)
    - [ ] Fully configurable layouts
- [ ] Reminders / guide window
    - [ ] It exists
- [ ] Full Voice Acting
    - [ ] This is a joke, trust me :)

## Defining Advancement Manifests

The base advancement manifest is located in advancements.json. However, more advancements can be added to this file to add advancements to the tracker.

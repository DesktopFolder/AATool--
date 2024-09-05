# Actual Design Notes I Guess

Top-level polled function is InstanceProvider::poll(). This gets us the 'active instance' based on log file date. Supports instance detection, etc.

Now that we have the active instance, we need to feed it to other things.

WorldProvider::poll(...)? ok, yeah. We will return `true` from this when we get any update.

Basic flow:
- press a key (p)
- load all advancements based on that


# Linux things

https://tronche.com/gui/x/xlib/input/XGetInputFocus.html


# New Design Notes

- key providers can be pub/subbed?
- need to separate out subsystems / subapps

subapps:
- reminders
- map
- statistics tracker
- overlay
- advancements lister

# Old Design Notes (Redesign / Refactor / Foundation Development)

Primary todolist:
- Background for main advancements in overlay
- Basic layout in main window
- Get only advancement file from advancements folder for instance

- "Advancement Status" rich object fed into display adaptors
- What is an advancement?
    - Key: category/shortname (FQN)
    - Basic data: Icon (texture ptr?) / Short Name (str) / Category (str)
    - Criterion data: Missing / Completed? Icons optional?

# Core / Refactor ToDo

- ResourceManager refactor!
    - Store textures at different resolutions, with a standardized search/lookup method.
    - Normalize all the different texture loading solutions.

# Initial Todo List (mostly old... I guess)

1. Basic animation on turntables
    - Render background sprites
2. AdvancementsFileProvider <- file stuff only done here
3. AdvancementsProvider

## Features

- F3+C + Alt1... -> initial keypress opens "current coords, press [keybind] for... xyz" in tracker? configuration string for coords/nether coords?
- Checklist based tiered/configurable helper/tutorial system
- ALERT: Biome X missing [you have entered: BIOME y]
- ALERT: Food X missing
- https://github.com/kwhat/libuiohook

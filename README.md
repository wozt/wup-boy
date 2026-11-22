# WUP Boy

Game Boy and Game Boy Color games on the Wii U GamePad, for the [Homebrew Menu](https://github.com/ravensclawx/wup-homebrew).
Get it in the GamePad's **App Store**.

<p align="center"><img src="docs/images/wup-boy-menu.png" width="427" alt="WUP Boy in the Homebrew Menu"> <img src="docs/images/wup-boy-empty.png" width="427" alt="No games yet"></p>
<p align="center"><img src="docs/images/wup-boy-playing.png" width="427" alt="Playing"> <img src="docs/images/wup-boy-pause.png" width="427" alt="Paused"></p>

## Use

- **Your games**: WUP Boy comes without games. Turn on File Sharing in the menu's Settings and copy `.gb` and `.gbc`
  files to `apps/wup_boy/roms` (FTP). Games up to 1 MB fit.
- **Play**: A and B (X and Y too), **+** Start, **−** Select, the D-pad or the left stick.
- **HOME** pauses: Resume, Save State, Load State, Restart, Quit.
- Hold **ZR** to fast forward.
- The game's own save (its battery memory) is kept at every pause and when you quit, in `apps/wup_boy/saves`,
  with the save state next to it.

## How it is made

Two apps in one folder: the list of games (`main.c`, in the Homebrew Menu's look: libwup's kit) and the player it starts
with a game's path (`player/main.c`), which draws on the GamePad's 8-bit layer so the whole game fits in memory. The
emulation is [Peanut-GB](https://github.com/deltabeard/Peanut-GB) (its `cgb` branch) with minigb_apu for sound, both MIT,
fetched by the build.

Build it with [libwup](https://github.com/ravensclawx/wup-homebrew/tree/main/libwup), the GamePad SDK:

```
git clone https://github.com/ravensclawx/wup-homebrew
git clone https://github.com/ravensclawx/wup-boy
cd wup-boy
LIBWUP=../wup-homebrew/libwup python3 package.py      # -> build/wup_boy.zip (both apps, as the App Store installs them)
```

`testrom.py` writes a small test game of its own (a scrolling pattern) for trying things in the emulator.

GPL-3.0-or-later ([LICENSE](LICENSE)), as libwup. Peanut-GB and minigb_apu: MIT.

Game Boy and Game Boy Color are trademarks of Nintendo; this is not affiliated with Nintendo. Use it with games you own.

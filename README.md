<p align="center">
  <img src="assets/logo.png" width="130" height="130" alt="Pixel Prism Logo" />
</p>

<h1 align="center">discord-wordle-destroyer</h1>

<p align="center">
  A frosted-glass Wordle solver in C built with Raylib to stop losing to my friends on Discord.
</p>

<p align="center">
  <a href="https://github.com/lunizura/discord-wordle-destroyer/actions/workflows/build.yml">
    <img src="https://github.com/lunizura/discord-wordle-destroyer/actions/workflows/build.yml/badge.svg" alt="Build Status">
  </a>
  <img src="https://img.shields.io/badge/Language-C99-00599C?style=flat-square&logo=c" alt="Language">
  <img src="https://img.shields.io/badge/GUI-Raylib%206.0-white?style=flat-square" alt="Raylib">
  <img src="https://img.shields.io/badge/Platform-Windows-0078D6?style=flat-square&logo=windows" alt="Platform">
  <img src="https://img.shields.io/badge/License-MIT-blue?style=flat-square" alt="License">
</p>

---

## Why I made this

Made this during the break after graduating high school while waiting for college to start. My friends were running Wordle games in our Discord server every night and consistently beating me, so I decided to use the free time to finally learn some C, do something with Shannon entropy, and build a desktop solver so I'd stop losing.

---

## What it does

* **Frosted Glass UI:** Dark indigo background with floating ambient light orbs and translucent glass panels.
* **120 FPS animations:** 3D tile flips, springy hover effects, and responsive typing feedback.
* **Shannon Entropy:** Calculates the mathematically best guess by seeing which word cuts down the remaining candidates the most.
* **Two wordlists:** 
  * *NYT Mode:* The 2,315 curated words for normal daily Wordle.
  * *Discord Wide:* 14,855 Scrabble words, because Discord bots like to pick weird obscure words.
* **Procedural audio:** Key clicks, tile pops, flip sounds, and victory chime are synthesized in C math at runtime. No external audio files.
* **Single `.exe`:** Standalone ~2 MB binary. No install needed.

---

## How to use

1. Type your guess on the board (or click one of the recommendations on the right).
2. Click the tiles to match what Discord gave you:
   * **Gray** = not in word
   * **Yellow** = wrong spot
   * **Green** = correct
3. Press **Enter** or click **`[SUBMIT GUESS ↵]`**.
4. The sidebar recalculates and shows you the next best words. Click any recommendation to auto-fill it.
5. If playing against a bot using obscure words, click the mode toggle at the top right to switch to **Discord Wide**.

---

## Download (Windows)

Grab the latest pre-built `wordle_prism.exe` from [GitHub Releases](https://github.com/lunizura/discord-wordle-destroyer/releases) (or download the `wordle-prism-windows` artifact from [GitHub Actions](https://github.com/lunizura/discord-wordle-destroyer/actions/workflows/build.yml)).

---

## Building from source (Windows)

Needs GCC (MinGW):

```cmd
git clone https://github.com/lunizura/discord-wordle-destroyer.git
cd discord-wordle-destroyer
build.bat
```

Outputs `wordle_prism.exe` in ~2 seconds (`build.bat` automatically downloads the matching Raylib 6.0 MinGW SDK into `raylib/` if not present).

### Running Solver Unit Tests

```cmd
gcc -O2 -Wall -Wextra -std=c99 -Isrc tests/test_solver.c src/solver.c src/words.c -lm -o test_solver.exe
test_solver.exe
```

---

## License

MIT.

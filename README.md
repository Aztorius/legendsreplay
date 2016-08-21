# LegendsReplay
A League of Legends replay software.

| Platforms | Build status |
|-----------|--------------|
| Linux     | [![Build Status](https://travis-ci.org/Aztorius/legendsreplay.svg?branch=master)](https://travis-ci.org/Aztorius/legendsreplay) |
| Windows   | [![Build status](https://ci.appveyor.com/api/projects/status/bbbh5654c5uif049?svg=true)](https://ci.appveyor.com/project/Aztorius/legendsreplay) |

Available for **Windows** and **Linux**

## Features

- All servers supported (EUW, EUNE, NA, JP, KR, OCE, BR, LAN, LAS, RU, TR, PBE)
- Available in different languages (English, French, Spanish, Deutch, Portuguese)
- Show servers status
- List all featured games
- Launch spectator mode for featured games (Windows only)
- Record featured games
- Record multiple games at the same time (multithreading)
- Autorecord current playing game (with openreplay servers, Windows only)
- Replay recorded games (Windows only)

### [Website](http://aztorius.github.io/legendsreplay)

## Build

Qt >= 5.6 is required.
C++14 is required.

- ``` git submodule init ```
- ``` git submodule update ```
- ``` cd qhttp ```
- ``` ./update-dependencies.sh ```
- ``` cd .. ```
- ``` qmake LegendsReplay.pro ```

### [Technical infos about the spectator API](https://gist.github.com/Aztorius/e428be6515b19fd24823754b72038e1b)

## Report issues

All issues must be in English.
**An issue can report a translation problem, a software bug or a feature request.**

LegendsReplay isn't endorsed by Riot Games and doesn't reflect the views or opinions of Riot Games or anyone officially involved in producing or managing League of Legends. League of Legends and Riot Games are trademarks or registered trademarks of Riot Games, Inc. League of Legends © Riot Games, Inc.

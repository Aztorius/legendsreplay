# LegendsReplay
A League of Legends replay software.

### Current Version : Stable 1.1.2

Available for **Windows** and **Linux**

## Features

- All servers supported (EUW, EUNE, NA, JP, KR, OCE, BR, LAN, LAS, RU, TR, PBE)
- Show servers status
- List all featured games
- Launch spectator mode for featured games
- Record featured games
- Record multiple games at the same time (multithreading)
- Autorecord current playing game (with openreplay servers)
- Replay recorded games

## TODO

- Improve GUI interface

### [Website](http://aztorius.github.io/legendsreplay)

## Build

Qt5 is required.

- ``` git submodule init && git submodule update ```
- Go to qhttp folder and run ``` ./update-dependencies.sh ```
- Open LegendsReplay.pro and build src project with QtCreator

### [Technical infos about the spectator API](https://gist.github.com/Aztorius/e428be6515b19fd24823754b72038e1b)

### Please report issues and send pull requests to improve this software

LegendsReplay isn't endorsed by Riot Games and doesn't reflect the views or opinions of Riot Games or anyone officially involved in producing or managing League of Legends. League of Legends and Riot Games are trademarks or registered trademarks of Riot Games, Inc. League of Legends © Riot Games, Inc.

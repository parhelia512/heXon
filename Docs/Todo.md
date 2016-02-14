#### Improvement of code architecture
- Seperate PilotData as class/struct
	* Overload << and >> 
- SpawnMaster should probably work with templates

Remove score leaks

#### New features
- Kernel (Enemy)
- 2 Player Lobby
	* Two step game start: Join (button) + ready (ship)
    * Top panel: Instructions
    * Bottom panel: Game type selection (single, battle, coop)
- Baphomech boss (trophy: coin)
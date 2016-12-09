[:four_leaf_clover:] (http://www.luckeyproductions.nl/) ![hēXōn logo](https://raw.githubusercontent.com/LucKeyProductions/heXon/master/Docs/Guide/images/heXonBanner.png)
### Summary
<<<<<<< HEAD
hēXōn is a free and open source twin-stick-shooter created using the [Urho3D](http://urho3d.github.io) game engine.  
=======
hēXōn is a free and open source twin-stick-shooter created using the [Urho3D](http://urho3d.github.io) game engine.
>>>>>>> custom-components
To score high you must fly well, avoiding the Notyous and destroying them with your Whack-o-Slack blast battery. Your firepower can be increased by collecting five apples. Picking up five hearts in a row will charge your shield.  
All edges of the hexagonal arena are connected to its opposite like portals, making for a puzzlingly dangerous playing field that might take some experience to wrap your head around.

### Screenshots
![heXon screenshot](https://raw.githubusercontent.com/LucKeyProductions/heXon/master/Screenshots/Screenshot_Sun_Jun__5_02_51_53_2016.png)
![heXon screenshot](https://raw.githubusercontent.com/LucKeyProductions/heXon/master/Screenshots/Screenshot_Sun_Jun__5_03_02_18_2016.png)

### Installation
#### 64-bit Linux
Visit [hēXōn's itch.io page](http://luckeyproductions.itch.io/hexon) and hit **Download Now**.

#### Compiling from source on Linux
1. Run CloneMakeUrho3D.sh; This script will clone the Urho3D game engine into your heXon folder, install its dependencies and compile it.
2. Now you can compile heXon using qmake. Either open heXon.pro using QtCreator and run it or run `qmake heXon.pro` and then `make` from within the heXon folder.
3. Add symlinks with `ln -s` to the Resources, Urho3D/bin/Data and Urho3D/bin/CoreData folders where the executable is ran (build folder).

### Pickups
* Golden apple / Provides 23 points. Collect five apples in a row to get a weapon upgrade.
* Heart / Heals half of your max life. Collect five hearts in a row to get a shield upgrade.

### Enemies
* Razors / Mostly harmless in small numbers. Don't fly into them though. 5 points on destruction.
* Spires / These sturdy towers launch player seeking foo fighters that should be evaded. 10 points when obliterated.

### Noteworthy
Explosions repel non-projectiles.
Touching the edge of the net will send most object to the opposite side.

### Controls
#### Controller
hēXōn is best played with a controller that has at least two analog sticks. These days, most controllers work perfectly well out-of-the-box on Linux. Make sure it is connected before you start the game.
#### Keyboard
* Movement / WASD
* Firing / Numpad or IJKL
* Pause / P

### Platforms of development
* Xubuntu GNU/Linux 64-bit
* ...

### Tools
[Urho3D](http://urho3d.github.io), [QtCreator](http://wiki.qt.io/Category:Tools::QtCreator), [Blender](http://www.blender.org/), [Inkscape](http://inkscape.org/), [GIMP](http://gimp.org), [SuperCollider](http://supercollider.github.io/), [Audacity](http://web.audacityteam.org/)

### Soundtrack
Alien Chaos - Disorder
from [Discovering the Code](http://www.ektoplazm.com/free-music/alien-chaos-discovering-the-code)

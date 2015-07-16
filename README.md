#:four_leaf_clover: heXon
###Summary
heXon is a free and open source dual stick arcade shooter created using the Urho3D game engine.

###Installation
1. After cloning the heXon repository place three symlinks in the heXon folder pointing to the root folder of the Urho3D game engine and both it's Data and CoreData folders. Running CloneMakeUrho3D.sh will clone Urho3D directly into your heXon folder and compile it.
2. Now you can compile heXon using qmake. Either open heXon.pro using QtCreator and run it or execture the following command in a terminal:
```qmake heXon.pro```

###Pickups
* Golden apple / Provides 23 points. Collect five apples in a row to get a weapon upgrade.
* Heart / Heals half of your max life. Collect five hearts in a row to get a shield upgrade.

###Enemies
* Razors / Mostly harmless in small numbers. Don't fly into them though. 5 points on destruction.
* Spires / These sturdy towers launch player seeking foo fighters that should be evaded. 10 points when obliterated.

###Noteworthy
Explosions repel non-projectiles.
Touching the edge of the net will send most object to the opposite side.

###Controls
####Controller
Preferably a SIXAXIS or other game controller with to analog sticks.
####Keyboard
Movement / WASD
Firing / Numpad or IJKL
Pause / P

###Platforms of development
* Xubuntu GNU/Linux 64-bit
* ...

###Screenshot
![heXon screenshot](https://raw.githubusercontent.com/LucKeyProductions/heXon/master/Screenshots/Screenshot_Wed_Jul__1_20_20_27_2015.png)

###Tools
Urho3D, QtCreator, Blender, Inkscape, GIMP, SuperCollider, Audacity

###Soundtrack
Zentrix - Warp Drive
from [Alternate Frequency](http://www.ektoplazm.com/free-music/alternate-frequency)

Alien Chaos - Disorder
from [Discorvering the Code](http://www.ektoplazm.com/free-music/alien-chaos-discovering-the-code)

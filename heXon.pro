LIBS += ../heXon/Urho3D/lib/libUrho3D.a \
    -lpthread \
    -ldl \
    -lGL

QMAKE_CXXFLAGS += -std=c++14

INCLUDEPATH += \
    ../heXon/Urho3D/include \
    ../heXon/Urho3D/include/Urho3D/ThirdParty \

TEMPLATE = app
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    apple.cpp \
    bubble.cpp \
    bullet.cpp \
    chaoball.cpp \
    chaoflash.cpp \
    chaomine.cpp \
    chaozap.cpp \
    door.cpp \
    effect.cpp \
    enemy.cpp \
    explosion.cpp \
    flash.cpp \
    heart.cpp \
    hexocam.cpp \
    hitfx.cpp \
    inputmaster.cpp \
    line.cpp \
    luckey.cpp \
    mastercontrol.cpp \
    multix.cpp \
    muzzle.cpp \
    pickup.cpp \
    player.cpp \
    razor.cpp \
    sceneobject.cpp \
    seeker.cpp \
    spawnmaster.cpp \
    spire.cpp \
    splatterpillar.cpp \
    TailGenerator.cpp \
    tile.cpp \
    tilemaster.cpp \
    pilot.cpp

HEADERS += \
    apple.h \
    bubble.h \
    bullet.h \
    chaoball.h \
    chaoflash.h \
    chaomine.h \
    chaozap.h \
    door.h \
    effect.h \
    enemy.h \
    explosion.h \
    flash.h \
    heart.h \
    hexocam.h \
    hitfx.h \
    inputmaster.h \
    line.h \
    luckey.h \
    mastercontrol.h \
    multix.h \
    muzzle.h \
    pickup.h \
    player.h \
    razor.h \
    sceneobject.h \
    seeker.h \
    spawnmaster.h \
    spire.h \
    splatterpillar.h \
    TailGenerator.h \
    tile.h \
    tilemaster.h \
    pilot.h

OTHER_FILES += \
    Docs/Todo.md \

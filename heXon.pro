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
    *.cpp \

HEADERS += \
    *.h \

OTHER_FILES += \
    Docs/Todo.md \

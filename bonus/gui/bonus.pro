QT       += core gui widgets
TARGET    = TrainGUI
TEMPLATE  = app
CONFIG   += c++17
DESTDIR   = ../../bin

# ─── Project source layout ───────────────────────────────────────────────────
INCLUDEPATH += \
    ../../src/Edge \
    ../../src/Node \
    ../../src/RailNetwork \
    ../../src/Train \
    ../../src/Event \
    ../../src/TrainFactory \
    ../../src/IPathfinding \
    ../../src/DijkstraPathfinding \
    ../../src/InputHandler \
    ../../src/OutputManager \
    ../../src/Simulation \
    ../../src/ISimulationObserver \
    ../../src/FileOutputObserver \
    ../../src/GraphExporter \
    ../../src/TerminalAnimDisplay

# ─── Shared simulation library sources ────────────────────────────────────────
SOURCES += \
    ../../src/Node/Node.cpp \
    ../../src/RailNetwork/RailNetwork.cpp \
    ../../src/Train/Train.cpp \
    ../../src/Event/Event.cpp \
    ../../src/TrainFactory/TrainFactory.cpp \
    ../../src/DijkstraPathfinding/DijkstraPathfinding.cpp \
    ../../src/InputHandler/InputHandler.cpp \
    ../../src/OutputManager/OutputManager.cpp \
    ../../src/Simulation/Simulation.cpp \
    ../../src/FileOutputObserver/FileOutputObserver.cpp \
    ../../src/GraphExporter/GraphExporter.cpp \
    ../../src/TerminalAnimDisplay/TerminalAnimDisplay.cpp

# ─── GUI sources ──────────────────────────────────────────────────────────────
SOURCES += \
    main.cpp \
    MainWindow.cpp \
    NetworkScene.cpp \
    SimulationWorker.cpp \
    SimDashboard.cpp

HEADERS += \
    MainWindow.hpp \
    NetworkScene.hpp \
    SimulationWorker.hpp \
    SimDashboard.hpp

OBJECTS_DIR = ../../objs/gui
MOC_DIR     = ../../objs/gui/moc

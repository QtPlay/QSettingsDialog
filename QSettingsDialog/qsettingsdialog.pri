

INCLUDEPATH += $$PWD/QSettingsDialog
INCLUDEPATH += $$PWD/QSettingsDialog/core
INCLUDEPATH += $$PWD/QSettingsDialog/dialogui
INCLUDEPATH += $$PWD/QSettingsDialog/loaders
INCLUDEPATH += $$PWD/QSettingsDialog/variantwidgets

win32:CONFIG(release, debug|release):
    LIBS += -L$$OUT_PWD/../../QSettingsDialog/release/ -lQSettingsDialog
else:win32:
    CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../QSettingsDialog/debug/ -lQSettingsDialog
else:mac {
        QMAKE_LFLAGS += -F$$OUT_PWD/../../QSettingsDialog/
        QMAKE_LFLAGS += '-Wl,-rpath,\'@executable_path/../../../../../QSettingsDialog\''
        LIBS += -F$$OUT_PWD/../../QSettingsDialog/ -framework QSettingsDialog
} else:unix:
    LIBS += -L$$OUT_PWD/../../QSettingsDialog/ -lQSettingsDialog

#-------------------------------------------------
#
# Project created by QtCreator 2016-06-02T21:32:33
#
#-------------------------------------------------

TEMPLATE = lib
QT       += widgets
CONFIG += c++14

TARGET = QSettingsDialog
VERSION = 1.0.0

win32 {
#	QMAKE_TARGET_COMPANY = "Skycoder42"
#	QMAKE_TARGET_PRODUCT = "QSettingsDialog"
#	QMAKE_TARGET_DESCRIPTION = $$QMAKE_TARGET_PRODUCT
#	QMAKE_TARGET_COPYRIGHT = "Felix Barz"

	CONFIG += skip_target_version_ext
} else:mac {
#	QMAKE_TARGET_BUNDLE_PREFIX = "com.Skycoder42."
	QMAKE_FRAMEWORK_BUNDLE_NAME = "QSettingsDialog"

	CONFIG += lib_bundle
	QMAKE_LFLAGS_SONAME = -Wl,-install_name,@rpath/
	QMAKE_LFLAGS += '-Wl,-rpath,\'@executable_path/../Frameworks\''
} else:unix {
	QMAKE_LFLAGS += '-Wl,-rpath,\'.\''
	QMAKE_LFLAGS += '-Wl,-rpath,\'\$$ORIGIN/../lib\''
	QMAKE_LFLAGS += '-Wl,-rpath,\'\$$ORIGIN/lib\''
}

DEFINES += QSETTINGSDIALOG_LIBRARY

HEADERS += qsettingsdialog_global.h \
	qtexception.h

SOURCES += \
	qtexception.cpp \
    qsettingsdialog_global.cpp

RESOURCES += \
    qsettingsdialog_res.qrc

DISTFILES += \
	$$PWD/classmodel.qmodel \
    qsettingsdialog.pri \
    generateIncludes.bat \
    doc/QSettingsDialog.doxy \
    doc/images/dialogwidget_button.png \
    doc/images/structure_core.svg \
    doc/modules_snippet.html \
    doc/images/structure_dialogui.svg \
    doc/images/structure_global.svg \
    doc/images/structure_loaders.svg \
    doc/images/structure_variantwidgets.svg \
    QSettingsDialog.pri

include(./submodules/submodules.pri)
#parts
include(./core/core.pri)
include(./dialogui/dialogui.pri)
include(./variantwidgets/variantwidgets.pri)
include(./loaders/loaders.pri)

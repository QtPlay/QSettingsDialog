#include "qsettingsentry.h"
#include "qsettingsentry_p.h"
#include <QThread>

#define d this->d_ptr

QSettingsEntry::QSettingsEntry(int displaytype, QSettingsLoader *loader) :
	d_ptr(new QSettingsEntryPrivate(displaytype, loader))
{}

QSettingsEntry::QSettingsEntry(int displaytype, QSettingsLoader *loader, const QString &name, bool optional, const QString &tooltip, const UiPropertyMap &properties) :
	d_ptr(new QSettingsEntryPrivate(displaytype, loader))
{
	d->name = name;
	d->optional = optional;
	d->tooltip = tooltip;
	d->properties = properties;
}

QSettingsEntry::~QSettingsEntry()
{
#ifndef QT_NO_DEBUG
//	Q_ASSERT_X(d->refCounter == 0, Q_FUNC_INFO, "settings entry destroyed with active loaders!");
#endif
}

QString QSettingsEntry::entryName() const
{
	return d->name;
}

void QSettingsEntry::setEntryName(const QString &name)
{
	d->name = name;
}

bool QSettingsEntry::isOptional() const
{
	return d->optional;
}

void QSettingsEntry::setOptional(bool optional)
{
	d->optional = optional;
}

QString QSettingsEntry::tooltip() const
{
	return d->tooltip;
}

void QSettingsEntry::setTooltip(const QString &tooltip)
{
	d->tooltip = tooltip;
}

UiPropertyMap QSettingsEntry::uiProperties() const
{
	return d->properties;
}

void QSettingsEntry::setUiProperties(const UiPropertyMap &properties)
{
	d->properties = properties;
}

void QSettingsEntry::setUiProperty(const QString &name, const QVariant &value)
{
	d->properties.insert(name, value);
}

int QSettingsEntry::displaytype()
{
	return d->displaytype;
}

QSettingsLoader *QSettingsEntry::getLoader()
{
#ifndef QT_NO_DEBUG
	d->refCounter++;
#endif
	return d->loader.data();
}

void QSettingsEntry::freeLoader(QSettingsLoader *)
{
#ifndef QT_NO_DEBUG
	d->refCounter--;
#endif
}

void QSettingsEntryPrivate::LoaderDeleter::cleanup(QSettingsLoader *loader)
{
	if(loader && loader->isAsync()) {
		auto async = loader->async();
		if(async->thread() == QThread::currentThread())
			async->deleteLater();
		else
			QMetaObject::invokeMethod(async, "deleteLater", Qt::QueuedConnection);
	} else
		delete loader;
}

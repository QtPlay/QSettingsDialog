#ifndef CONTAINERELEMENT_H
#define CONTAINERELEMENT_H

#include <QString>
#include <QIcon>
#include <QSharedPointer>
#include "sortedmap.h"
#include "specialgroupmap.h"
#include "qsettingsentry.h"
#include "exceptions.h"
#include <QAtomicPointer>
#include <qsettingsdialog.h>

class QSettingsContainer;
struct SettingsGroup
{
	QAtomicPointer<QSettingsContainer> locker;

	QString name;
	QString tooltip;
	bool isOptional;

	SortedMap<int, QSettingsEntry> entries;

	inline SettingsGroup(const QString &name) :
		name(name),
		tooltip(),
		isOptional(false),
		entries()
	{}

	inline void testNotLocked() const {
		if(this->locker != nullptr)
			throw ContainerLockedException();
	}
	inline void testNotLocked(QSettingsContainer *locker) const {
		if(this->locker != nullptr && this->locker != locker)
			throw ContainerLockedException();
	}

	static inline QSharedPointer<SettingsGroup> createDefaultGroup() {
		return QSharedPointer<SettingsGroup>(new SettingsGroup(QSettingsDialog::tr("General")));
	}
};

struct SettingsSection
{
	QString name;
	QIcon icon;
	QString tooltip;

	QSharedPointer<SettingsGroup> defaultGroup;
	SpecialGroupMap groups;

	inline SettingsSection(const QString &name) :
		name(name),
		icon(),
		tooltip(),
		defaultGroup(),
		groups()
	{}

	static inline QSharedPointer<SettingsSection> createDefaultSection() {
		return QSharedPointer<SettingsSection>(new SettingsSection(QSettingsDialog::tr("General")));
	}
};

struct SettingsCategory
{
	QString name;
	QIcon icon;
	QString tooltip;

	QSharedPointer<SettingsSection> defaultSection;
	SortedMap<QString, SettingsSection> sections;

	inline SettingsCategory(const QString &name) :
		name(name),
		icon(),
		tooltip(),
		defaultSection(),
		sections()
	{}

	static inline QSharedPointer<SettingsCategory> createDefaultCategory() {
		auto cat = new SettingsCategory(QSettingsDialog::tr("General Settings"));
		cat->icon = QIcon(QStringLiteral(":/QSettingsDialog/icons/settings.ico"));
		return QSharedPointer<SettingsCategory>(cat);
	}
};

struct SettingsRoot
{
	QSharedPointer<SettingsCategory> defaultCategory;
	SortedMap<QString, SettingsCategory> categories;
};

#endif // CONTAINERELEMENT_H
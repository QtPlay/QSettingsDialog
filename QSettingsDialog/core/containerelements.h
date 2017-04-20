#ifndef CONTAINERELEMENT_H
#define CONTAINERELEMENT_H

#include <QString>
#include <QIcon>
#include <QSharedPointer>
#include "sortedmap.h"
#include "specialgroupmap.h"
#include "qsettingsentry.h"
#include "exceptions.h"
#include <qsettingsdialog.h>

class QSettingsContainer;
struct SettingsGroup
{
    QString id;
	int displayId;
	QString name;
	QString tooltip;
	bool isOptional;

	SortedMap<int, QSettingsEntry> entries;

	inline SettingsGroup(int displayId, const QString &name) :
        id(name),
        displayId(displayId),
		name(name),
		tooltip(),
		isOptional(false),
		entries()
	{}
};

struct SettingsSection
{
    QString id;
	QString name;
	QIcon icon;
	QString tooltip;

	SpecialGroupMap groups;

	inline SettingsSection(const QString &name) :
        id(name),
        name(name),
		icon(),
		tooltip(),
		groups()
	{}

	static inline QSharedPointer<SettingsSection> createDefaultSection() {
        auto section = new SettingsSection(".");
        section->name = QSettingsDialog::tr("General");
        return QSharedPointer<SettingsSection>(section);
	}
};

struct SettingsCategory
{
    QString id;
	QString name;
	QIcon icon;
	QString tooltip;

	QSharedPointer<SettingsSection> defaultSection;
	SortedMap<QString, SettingsSection> sections;

	inline SettingsCategory(const QString &name) :
        id(name),
        name(name),
		icon(),
		tooltip(),
		defaultSection(),
		sections()
	{}

	static inline QSharedPointer<SettingsCategory> createDefaultCategory() {
        auto cat = new SettingsCategory(QString("."));
        cat->name = QSettingsDialog::tr("General Settings");
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

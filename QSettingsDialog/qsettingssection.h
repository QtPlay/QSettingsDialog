#ifndef QSETTINGSSECTION_H
#define QSETTINGSSECTION_H

#include "qsettingsdialog_global.h"
#include <QString>
#include <QIcon>
#include <QList>
class QSettingsGroup;
class QTabBar;

class QSETTINGSDIALOGSHARED_EXPORT QSettingsSection
{
	friend class QSettingsCategory;

public:
	QString name() const;
	QIcon icon() const;

	void setName(const QString &name);
	void setIcon(const QIcon &icon);

	QList<QSettingsGroup*> groups(bool includeDefault = false) const;
	QSettingsGroup *groupAt(int index) const;
	int groupIndex(QSettingsGroup *group) const;

	QSettingsGroup *insertGroup(int index, const QString &name);
	inline QSettingsGroup *addGroup(const QString &name) {
		return this->insertGroup(this->grps.size(), name);
	}

	void deleteGroup(int index);
	bool deleteGroup(QSettingsGroup *group);

	void moveGroup(int from, int to);

	QSettingsGroup *defaultGroup();

private:
	QTabBar *tabBar;
	int tabIndex;
	QWidget *contentWidget;

	QSettingsGroup *defaultGrp;
	QList<QSettingsGroup*> grps;

	QSettingsSection(QTabBar *tabBar, int tabIndex, QWidget *contentWidget);
	~QSettingsSection();
	void updateIndex(int tabIndex);
};

#endif // QSETTINGSSECTION_H

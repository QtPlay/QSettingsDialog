#ifndef SETTINGSPATHEDIT_H
#define SETTINGSPATHEDIT_H

#include "qsettingswidget.h"
#include <qpathedit.h>

class SettingsPathEdit : public QSettingsWidget<QPathEdit>
{
public:
	SettingsPathEdit(QWidget *parent = nullptr);

	void setValue(const QVariant &value) override;
	QVariant getValue() const override;
	void resetValue() override;
};

class SettingsDirEdit : public QSettingsWidget<QDirEdit>
{
public:
    SettingsDirEdit(QWidget *parent = nullptr);

    void setValue(const QVariant &value) override;
    QVariant getValue() const override;
    void resetValue() override;
};

class SettingsIconEdit : public QSettingsWidget<QPathEdit>
{
	Q_OBJECT
	Q_PROPERTY(bool asQIcon READ asQIcon WRITE setAsQIcon NOTIFY asQIconChanged)

public:
	SettingsIconEdit(QWidget *parent = nullptr);

	void setValue(const QVariant &value) override;
	QVariant getValue() const override;
	void resetValue() override;
	bool asQIcon() const;

public slots:
	void setAsQIcon(bool asQIcon);

signals:
	void asQIconChanged(bool asQIcon);

private slots:
	void updateIcon(const QString &path);

private:
	bool m_asQIcon;
};

#endif // SETTINGSPATHEDIT_H

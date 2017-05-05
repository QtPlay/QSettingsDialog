#ifndef SETTINGSENUMCOMBOBOX_H
#define SETTINGSENUMCOMBOBOX_H

#include "qsettingswidget.h"
#include <QComboBox>
#include <QMetaEnum>
#include <QButtonGroup>

class QSETTINGSDIALOGSHARED_EXPORT SettingsEnumComboBox : public QSettingsWidget<QComboBox>
{
	Q_OBJECT

	Q_PROPERTY(bool translated READ translated WRITE setTranslated NOTIFY translatedChanged)

public:
	SettingsEnumComboBox(const QMetaEnum &metaEnum, QWidget *parent = nullptr);

	// QSettingsWidgetBase interface
	void setValue(const QVariant &value) override;
	QVariant getValue() const override;
	void resetValue() override;

	bool translated() const;

public slots:
	void setTranslated(bool translated);

signals:
	void translatedChanged(bool translated);

protected:
	QMetaEnum metaEnum;
	bool m_translated;
};

class SettingsEnumRadioList : public QSettingsWidget<QWidget>
{
	Q_OBJECT

	Q_PROPERTY(bool translated READ translated WRITE setTranslated NOTIFY translatedChanged)

public:
	SettingsEnumRadioList(const QMetaEnum &metaEnum, QWidget *parent = nullptr);

	// QSettingsWidgetBase interface
	void setValue(const QVariant &value) override;
	QVariant getValue() const override;
	void resetValue() override;

	bool translated() const;

public slots:
	void setTranslated(bool translated);

signals:
	void translatedChanged(bool translated);

protected:
	QMetaEnum metaEnum;
	QButtonGroup *checkGroup;
	bool m_translated;
};

class SettingsEnumEditWrapper : public QSettingsWidget<QWidget>
{
public:
	SettingsEnumEditWrapper(const QMetaEnum &metaEnum, QWidget *parent = nullptr);

	// QSettingsWidgetBase interface
	void initialize(const UiPropertyMap &uiPropertyMap) override;
	bool hasValueChanged() const override;
	void resetValueChanged() override;
	void setValue(const QVariant &value) override;
	QVariant getValue() const override;
	void resetValue() override;
	bool searchExpression(const QRegularExpression &regex) override;

private:
	QMetaEnum metaEnum;
	QSettingsWidgetBase *current;
};

#endif // SETTINGSENUMCOMBOBOX_H

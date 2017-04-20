#ifndef SETTINGSSPINBOX_H
#define SETTINGSSPINBOX_H

#include "qsettingswidget.h"
#include "qsettingswidgetfactory.h"
#include <QSpinBox>

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

class SettingsSpinBox : public QSettingsWidget<QSpinBox>
{
public:
	SettingsSpinBox(QWidget *parent = nullptr);

	// QSettingsWidgetBase interface
	void setValue(const QVariant &value) override;
	QVariant getValue() const override;
	void resetValue() override;
};

class SettingsDoubleSpinBox : public QSettingsWidget<QDoubleSpinBox>
{
public:
	SettingsDoubleSpinBox(QWidget *parent = nullptr);

    void initialize(const UiPropertyMap &uiPropertyMap) override;

	// QSettingsWidgetBase interface
	void setValue(const QVariant &value) override;
	QVariant getValue() const override;
	void resetValue() override;
};

class SpinBoxFactory : public QSettingsWidgetFactory
{
public:
	SpinBoxFactory(int max, int min);

	// QSettingsWidgetFactory interface
	QSettingsWidgetBase *createWidget(QWidget *parent) final;

private:
    int max;
    int min;
};

class DoubleSpinBoxFactory : public QSettingsWidgetFactory
{
public:
	DoubleSpinBoxFactory(double max, double min);

	// QSettingsWidgetFactory interface
	QSettingsWidgetBase *createWidget(QWidget *parent) final;

private:
    double max;
    double min;
};

#endif // SETTINGSSPINBOX_H

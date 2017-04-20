#include "settingsspinbox.h"

SettingsSpinBox::SettingsSpinBox(QWidget *parent) :
	QSettingsWidget(parent)
{}

void SettingsSpinBox::setValue(const QVariant &value)
{
	this->QSpinBox::setValue(value.toInt());
}

QVariant SettingsSpinBox::getValue() const
{
	return this->value();
}

void SettingsSpinBox::resetValue()
{
	this->setValue(0);
}



SettingsDoubleSpinBox::SettingsDoubleSpinBox(QWidget *parent) :
	QSettingsWidget(parent)
{}

void SettingsDoubleSpinBox::initialize(const UiPropertyMap &uiPropertyMap) {
    QSettingsWidget<QDoubleSpinBox>::initialize(uiPropertyMap);
}

void SettingsDoubleSpinBox::setValue(const QVariant &value)
{
	this->QDoubleSpinBox::setValue(value.toDouble());
}

QVariant SettingsDoubleSpinBox::getValue() const
{
	return this->value();
}

void SettingsDoubleSpinBox::resetValue()
{
	this->setValue(0.0);
}



SpinBoxFactory::SpinBoxFactory(int max, int min) :
	max(max),
	min(min)
{}

QSettingsWidgetBase *SpinBoxFactory::createWidget(QWidget *parent)
{
	SettingsSpinBox *box = new SettingsSpinBox(parent);
	box->setRange(this->min, this->max);
	return box;
}



DoubleSpinBoxFactory::DoubleSpinBoxFactory(double max, double min) :
	max(max),
	min(min)
{}

QSettingsWidgetBase *DoubleSpinBoxFactory::createWidget(QWidget *parent)
{
	SettingsDoubleSpinBox *box = new SettingsDoubleSpinBox(parent);
	box->setRange(this->min, this->max);
	return box;
}

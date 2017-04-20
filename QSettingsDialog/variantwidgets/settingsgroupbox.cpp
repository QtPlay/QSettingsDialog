#include "settingsgroupbox.h"
#include "checkinghelper.h"

SettingsGroupBox::SettingsGroupBox(QWidget *parent) :
	QSettingsGroupWidget(parent),
	layout(new QFormLayout(this))
{
	this->setLayout(this->layout);
}

void SettingsGroupBox::setName(const QString &name)
{
	this->setTitle(name);
}

void SettingsGroupBox::setTooltip(const QString &tooltip)
{
	this->setToolTip(tooltip);
}

void SettingsGroupBox::setOptional(bool optional)
{
	this->setCheckable(optional);
}

bool SettingsGroupBox::isChecked() const
{
	if(this->isCheckable())
		return this->QGroupBox::isChecked();
	else
		return true;
}

void SettingsGroupBox::setChecked(bool checked)
{
	if(this->isCheckable())
		this->QGroupBox::setChecked(checked);
}


void SettingsGroupBox::addWidgetWithLabel(QWidget *label, QWidget *content)
{
	this->layout->addRow(label, content);
}

void SettingsGroupBox::takeWidgetWithLabel(QWidget *label, QWidget *content) {
    auto indexL = this->layout->indexOf(label); this->layout->takeAt(indexL);
    auto indexF = this->layout->indexOf(content); this->layout->takeAt(indexF);
}

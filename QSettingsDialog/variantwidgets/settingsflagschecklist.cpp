#include "settingsflagschecklist.h"
#include <QVBoxLayout>
#include <QCheckBox>
#include <QCoreApplication>

SettingsFlagsCheckList::SettingsFlagsCheckList(const QMetaEnum &metaEnum, QWidget *parent) :
	QSettingsWidget(parent),
	metaEnum(metaEnum),
	checkGroup(new QButtonGroup(this)),
	currentFlags(0),
	m_translated(false)
{
	this->checkGroup->setExclusive(false);
    connect(this->checkGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(updateFlags(int)));

	auto layout = new QVBoxLayout(this);
	layout->setContentsMargins(QMargins());
	this->setLayout(layout);

	for(int i = 0; i < this->metaEnum.keyCount(); i++) {
		auto box = new QCheckBox(QString::fromLocal8Bit(this->metaEnum.key(i)), this);
		layout->addWidget(box);
		this->checkGroup->addButton(box, this->metaEnum.value(i));
	}

	this->setTranslated(true);
}

void SettingsFlagsCheckList::setValue(const QVariant &value)
{
	this->currentFlags = value.toInt();
	this->reloadFlags();
}

QVariant SettingsFlagsCheckList::getValue() const
{
	return this->currentFlags;
}

void SettingsFlagsCheckList::resetValue()
{
	this->currentFlags = 0;
	this->reloadFlags();
}

bool SettingsFlagsCheckList::translated() const
{
	return m_translated;
}

void SettingsFlagsCheckList::setTranslated(bool translated)
{
	if (m_translated == translated)
		return;
	m_translated = translated;

	for(int i = 0; i < this->metaEnum.keyCount(); i++) {
		QString text;
		if(translated)
			text = QCoreApplication::translate(this->metaEnum.name(), this->metaEnum.key(i));
		else
			text = QString::fromLocal8Bit(this->metaEnum.key(i));

		auto box = this->checkGroup->button(this->metaEnum.value(i));
		box->setText(text);
	}

	emit translatedChanged(translated);
}


void SettingsFlagsCheckList::updateFlags(int value)
{
	if(this->checkGroup->button(value)->isChecked())
		this->currentFlags |= value;
	else
		this->currentFlags &= ~value;
	this->reloadFlags();
}

void SettingsFlagsCheckList::reloadFlags()
{
	for(int i = 0; i < this->metaEnum.keyCount(); i++) {
		auto value = this->metaEnum.value(i);
		this->checkGroup->button(value)->setChecked((this->currentFlags & value) == value);
	}
}

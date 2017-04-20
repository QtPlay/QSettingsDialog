#include "settingsenumcombobox.h"
#include <QVBoxLayout>
#include <QRadioButton>
#include <QCoreApplication>

SettingsEnumComboBox::SettingsEnumComboBox(const QMetaEnum &metaEnum, QWidget *parent) :
	QSettingsWidget(parent),
	metaEnum(metaEnum),
	m_translated(false)
{
	this->setTranslated(true);
}

void SettingsEnumComboBox::setValue(const QVariant &value)
{
	auto intVal = value.toInt();
	for(int i = 0; i < this->metaEnum.keyCount(); i++) {
		if(this->metaEnum.value(i) == intVal) {
			this->setCurrentIndex(i);
			break;
		}
	}
}

QVariant SettingsEnumComboBox::getValue() const
{
	return this->metaEnum.value(this->currentIndex());
}

void SettingsEnumComboBox::resetValue()
{
	this->setCurrentIndex(-1);
}

bool SettingsEnumComboBox::translated() const
{
	return m_translated;
}

void SettingsEnumComboBox::setTranslated(bool translated)
{
	if (m_translated == translated)
		return;
	m_translated = translated;

	auto cIndex = this->currentIndex();
	QStringList enumValues;
	for(int i = 0; i < this->metaEnum.keyCount(); i++) {
		QString text;
		if(translated)
			text = QCoreApplication::translate(this->metaEnum.name(), this->metaEnum.key(i));
		else
			text = QString::fromLocal8Bit(this->metaEnum.key(i));
		enumValues.append(text);
	}
	this->clear();
	this->addItems(enumValues);
	this->setCurrentIndex(cIndex);

	emit translatedChanged(translated);
}



SettingsEnumRadioList::SettingsEnumRadioList(const QMetaEnum &metaEnum, QWidget *parent) :
	QSettingsWidget(parent),
	metaEnum(metaEnum),
	checkGroup(new QButtonGroup(this)),
	m_translated(false)
{
	this->checkGroup->setExclusive(true);

	auto layout = new QVBoxLayout(this);
	layout->setContentsMargins(QMargins());
	this->setLayout(layout);

	for(int i = 0; i < this->metaEnum.keyCount(); i++) {
		auto box = new QRadioButton(QString::fromLocal8Bit(this->metaEnum.key(i)), this);
		layout->addWidget(box);
		this->checkGroup->addButton(box, this->metaEnum.value(i));
	}

	this->setTranslated(true);
}

void SettingsEnumRadioList::setValue(const QVariant &value)
{
	auto id = value.toInt();
	this->checkGroup->button(id)->setChecked(true);
}

QVariant SettingsEnumRadioList::getValue() const
{
	return this->checkGroup->checkedId();
}

void SettingsEnumRadioList::resetValue()
{
	this->checkGroup->checkedButton()->setChecked(false);
}

bool SettingsEnumRadioList::translated() const
{
	return m_translated;
}

void SettingsEnumRadioList::setTranslated(bool translated)
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



SettingsEnumEditWrapper::SettingsEnumEditWrapper(const QMetaEnum &metaEnum, QWidget *parent) :
	QSettingsWidget(parent),
	metaEnum(metaEnum),
	current(new SettingsEnumComboBox(metaEnum, this))
{
	auto layout = new QVBoxLayout(this);
	layout->setContentsMargins(QMargins());
	this->setLayout(layout);
	layout->addWidget(this->current->asWidget());
}

void SettingsEnumEditWrapper::initialize(const UiPropertyMap &uiPropertyMap)
{
	if(uiPropertyMap.contains(QStringLiteral("showAsRadio"))) {
		auto asRadio = uiPropertyMap.value(QStringLiteral("showAsRadio")).toBool();
		this->current->asWidget()->deleteLater();
		if(asRadio)
			this->current = new SettingsEnumRadioList(this->metaEnum, this);
		else
			this->current = new SettingsEnumComboBox(this->metaEnum, this);
		this->layout()->addWidget(this->current->asWidget());
    } else {
        this->current->asWidget()->deleteLater();
        this->current = new SettingsEnumComboBox(this->metaEnum, this);
        this->layout()->addWidget(this->current->asWidget());
    }

	this->current->initialize(uiPropertyMap);
}

bool SettingsEnumEditWrapper::hasValueChanged() const
{
	return this->current->hasValueChanged();
}

void SettingsEnumEditWrapper::resetValueChanged()
{
	this->current->resetValueChanged();
}

void SettingsEnumEditWrapper::setValue(const QVariant &value)
{
	this->current->setValue(value);
}

QVariant SettingsEnumEditWrapper::getValue() const
{
	return this->current->getValue();
}

void SettingsEnumEditWrapper::resetValue()
{
	this->current->resetValue();
}

bool SettingsEnumEditWrapper::searchExpression(const QRegularExpression &regex)
{
	return this->current->searchExpression(regex);
}

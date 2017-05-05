#include "settingspathedit.h"
#include <QImageReader>
#include <QStandardPaths>

SettingsPathEdit::SettingsPathEdit(QWidget *parent) :
	QSettingsWidget(parent)
{
	this->setEditable(true);
}

void SettingsPathEdit::setValue(const QVariant &value)
{
	this->setPath(value.toString(), true);
}

QVariant SettingsPathEdit::getValue() const
{
	return this->path();
}

void SettingsPathEdit::resetValue()
{
	this->clear();
}

SettingsDirEdit::SettingsDirEdit(QWidget *parent) :
    QSettingsWidget(parent)
{
    this->setEditable(true);
}

void SettingsDirEdit::setValue(const QVariant &value)
{
    this->setPath(value.toString(), true);
}

QVariant SettingsDirEdit::getValue() const
{
    return this->path();
}

void SettingsDirEdit::resetValue()
{
    this->clear();
}

SettingsIconEdit::SettingsIconEdit(QWidget *parent) :
	QSettingsWidget(parent)
{
	this->setEditable(false);
	this->setStyle(QPathEdit::JoinedButton, QLineEdit::LeadingPosition);
	QStringList sList;
	foreach(auto data, QImageReader::supportedMimeTypes())
		sList.append(QString::fromUtf8(data));
	sList.append(QStringLiteral("application/octet-stream"));
	this->setMimeTypeFilters(sList);
	this->setDefaultDirectory(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));

	connect(this, &SettingsIconEdit::pathChanged,
			this, &SettingsIconEdit::updateIcon);
}

void SettingsIconEdit::setValue(const QVariant &value)
{
	if(this->m_asQIcon) {
		this->clear();
		this->setDialogButtonIcon(value.value<QIcon>());
	} else
		this->setPath(value.toString(), false);
}

QVariant SettingsIconEdit::getValue() const
{
	if(this->m_asQIcon)
		return this->dialogButtonIcon();
	else
		return this->path();
}

void SettingsIconEdit::resetValue()
{
	this->clear();
	this->resetDialogButtonIcon();
}

bool SettingsIconEdit::asQIcon() const
{
	return m_asQIcon;
}

void SettingsIconEdit::setAsQIcon(bool asQIcon)
{
	if (m_asQIcon == asQIcon)
		return;

	m_asQIcon = asQIcon;
	emit asQIconChanged(asQIcon);
}

void SettingsIconEdit::updateIcon(const QString &path)
{
	this->setDialogButtonIcon(QIcon(path));
}

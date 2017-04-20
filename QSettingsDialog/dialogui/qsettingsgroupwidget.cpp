#include "qsettingsgroupwidget.h"
#include <QCheckBox>
#include <QLabel>
#include "settingsdisplaydialog.h"

QSettingsGroupWidgetBase::~QSettingsGroupWidgetBase() {}

void QSettingsGroupWidgetBase::addWidgetRaw(QSharedPointer<QSettingsEntry> entry, QWidget *content, bool hasError)
{
	QWidget *label = nullptr;
	if(entry->isOptional()) {
		auto optBox = new QCheckBox(this->asWidget());
		optBox->setText(entry->entryName() + SettingsDisplayDialog::tr(":"));
		QObject::connect(optBox, &QCheckBox::toggled,
						 content, &QWidget::setEnabled);
		label = optBox;
	} else {
		auto cLabel = new QLabel(this->asWidget());
		cLabel->setText(entry->entryName() + SettingsDisplayDialog::tr(":"));
		label = cLabel;
	}
	this->entryMap.insert(entry, {label, dynamic_cast<QSettingsWidgetBase*>(content)});

	label->setToolTip(entry->tooltip());
	if(content->toolTip().isNull())
		content->setToolTip(entry->tooltip());
	if(hasError)
		label->setEnabled(false);
	else
		content->setEnabled(!entry->isOptional());

	this->addWidgetWithLabel(label, content);
}

void QSettingsGroupWidgetBase::rmWidgetRaw(QSharedPointer<QSettingsEntry> entry, QWidget *content, bool hasError) {
    Q_UNUSED(hasError);
    Q_UNUSED(content);
    auto it = this->entryMap.find(entry);
    Q_ASSERT(it != this->entryMap.end());
    this->takeWidgetWithLabel(it->first, it->second->asWidget());
    delete it->first;
    delete it->second;
    this->entryMap.erase(it);
}

void QSettingsGroupWidgetBase::setEntryChecked(QSharedPointer<QSettingsEntry> entry, bool checked)
{
	auto box = dynamic_cast<QCheckBox*>(this->entryMap.value(entry).first);
	if(box)
		return box->setChecked(checked);
}

void QSettingsGroupWidgetBase::setEntryLabelEnabled(QSharedPointer<QSettingsEntry> entry, bool enabled)
{
	auto widget = this->entryMap.value(entry).first;
	if(widget)
		widget->setEnabled(enabled);
}

bool QSettingsGroupWidgetBase::isEntryChecked(QSharedPointer<QSettingsEntry> entry) const
{
	auto box = dynamic_cast<QCheckBox*>(this->entryMap.value(entry).first);
	if(box)
		return box->isChecked();
	else
		return true;
}

bool QSettingsGroupWidgetBase::searchExpression(const QRegularExpression &regex, const QString &searchStyleSheet)
{
	bool someFound = false;

	foreach(auto element, this->entryMap) {
		auto &label = element.first;
		auto &content = element.second;

		if(!regex.pattern().isEmpty() &&
		   (regex.match(label->property("text").toString()).hasMatch() ||
			(content && content->searchExpression(regex)))) {
			label->setStyleSheet(searchStyleSheet);
			someFound = true;
		} else
			label->setStyleSheet(QString());
	}

	return someFound;
}

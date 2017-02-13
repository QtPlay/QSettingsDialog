#include "testentry.h"
#include <QDebug>
#include "swidget.h"

TestEntry::TestEntry(bool optional, bool working, QVariant data) :
	QSettingsLoaderEntry(working ? 0 : -1),
	origData(data),
	data(data)
{
	this->setEntryName("Test Entry");
	this->setOptional(optional);
	this->setTooltip("Baum == 42");
}

TestEntry::~TestEntry() {
//    qDebug() << "delete entry";

}

bool TestEntry::load(QVariant & data, bool & userEdited)
{
	userEdited = !this->data.isNull();
	data = this->data;
	return true;
}

bool TestEntry::save(const QVariant &data)
{
	qDebug() << "SAVING from" << this->data << "to" << data;
	this->data = data;
	return true;
}

bool TestEntry::reset()
{
	qDebug() << "RESETTING to" << this->origData;
	this->data = this->origData;
	return true;
}

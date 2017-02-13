#ifndef TESTENTRY_H
#define TESTENTRY_H

#include <qsettingsloaderentry.h>

class TestEntry : public QSettingsLoaderEntry
{
public:
	TestEntry(bool optional, bool working, QVariant data = QVariant());

    bool load(QVariant & data, bool & userEdited) override;
	bool save(const QVariant &data) override;
	bool reset() override;

private:
	QVariant origData;
	QVariant data;
};

#endif // TESTENTRY_H

#ifndef SETTINGSENGINE_H
#define SETTINGSENGINE_H

#include <QObject>
#include <QSignalMapper>
#include "qsettingsentry.h"
#include "qsettingsloader.h"
#include "qsettingswidget.h"
#include "checkinghelper.h"

class SettingsEngine : public QObject
{
	Q_OBJECT
public:
	explicit SettingsEngine(QObject *parent = nullptr);
	~SettingsEngine();

	void addEntry(QSharedPointer<QSettingsEntry> entry,
				  QSettingsWidgetBase *currentWidget,
				  CheckingHelper *checkingHelper);

    void rmEntry(QSharedPointer<QSettingsEntry> entry,
                  QSettingsWidgetBase *currentWidget,
                  CheckingHelper *checkingHelper);

public slots:
	void startLoading();
	void startSaving();
	void startResetting();

	void abortOperation();

signals:
	void progressMaxChanged(int max);
	void progressValueChanged(int value);

	void operationCompleted(int errorCount);
	void operationAborted();

private slots:
	void entryLoaded(bool successful, const QVariant &data, bool isUserEdited);
	void entrySaved(bool successful);
	void entryResetted(bool successful);

private:
	struct EntryInfoBase {
		QSharedPointer<QSettingsEntry> entry;
        QSettingsWidgetBase *currentWidget ;
        CheckingHelper *checkingHelper ;
	};
	template<class TLoader>
	struct EntryInfo : public EntryInfoBase {
		TLoader *currentLoader;

		inline EntryInfo(QSharedPointer<QSettingsEntry> entry,
				  QSettingsWidgetBase *currentWidget,
				  CheckingHelper *checkingHelper,
				  TLoader *currentLoader) :
			EntryInfoBase({entry, currentWidget, checkingHelper}),
			currentLoader(currentLoader)
		{}
	};

	QList<EntryInfo<QSimpleSettingsLoader>> simpleEntries;
	QList<EntryInfo<QAsyncSettingsLoader>> asyncEntries;

	QHash<QObject*, int> activeAsyncs;
	int currentCount;
	int errorCount;

	void updateEntry(EntryInfoBase &entry, const QVariant &data, bool isUserEdited);
	void disableEntry(EntryInfoBase &entry);
	void updateProgress(bool increment);
	void tryLoadComplete();
};

#endif // SETTINGSENGINE_H

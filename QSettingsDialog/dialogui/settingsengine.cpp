#include "settingsengine.h"

SettingsEngine::SettingsEngine(QObject *parent) :
	QObject(parent),
	simpleEntries(),
	asyncEntries(),
	activeAsyncs(),
	currentCount(0),
	errorCount(0)
{}

SettingsEngine::~SettingsEngine()
{
	foreach(auto entry, this->simpleEntries)
		entry.entry->freeLoader(entry.currentLoader);
	foreach(auto entry, this->asyncEntries)
		entry.entry->freeLoader(entry.currentLoader);
}

void SettingsEngine::addEntry(QSharedPointer<QSettingsEntry> entry, QSettingsWidgetBase *currentWidget, CheckingHelper *checkingHelper)
{
	auto loader = entry->getLoader();
	if(loader->isAsync()) {
		connect(loader->async(), &QAsyncSettingsLoader::loadDone,
				this, &SettingsEngine::entryLoaded,
				Qt::QueuedConnection);
		connect(loader->async(), &QAsyncSettingsLoader::saveDone,
				this, &SettingsEngine::entrySaved,
				Qt::QueuedConnection);
		connect(loader->async(), &QAsyncSettingsLoader::resetDone,
				this, &SettingsEngine::entryResetted,
				Qt::QueuedConnection);

		this->asyncEntries.append({
									  entry,
									  currentWidget,
									  checkingHelper,
									  loader->async()
								   });
	} else {
		this->simpleEntries.append({
									   entry,
									   currentWidget,
									   checkingHelper,
									   loader->simple()
								   });
	}
}

void SettingsEngine::rmEntry(QSharedPointer<QSettingsEntry> entry,
              QSettingsWidgetBase *currentWidget,
              CheckingHelper *checkingHelper) {
    auto loader = entry->getLoader();
    if( loader->isAsync()) {
        auto it = std::find_if(this->asyncEntries.begin(), this->asyncEntries.end(), [entry](const EntryInfo<QAsyncSettingsLoader> & item){
            return item.entry == entry;
        });
        Q_ASSERT(it != this->asyncEntries.end());
        this->asyncEntries.erase(it);
    } else {
        auto it = std::find_if(this->simpleEntries.begin(), this->simpleEntries.end(), [entry](const EntryInfo<QSimpleSettingsLoader> & item){
            return item.entry == entry;
        });
        Q_ASSERT(it != this->simpleEntries.end());
        this->simpleEntries.erase(it);
    }
}

void SettingsEngine::startLoading()
{
	if(!this->activeAsyncs.isEmpty())
		return;
	this->errorCount = 0;

	for(int i = 0, total = this->asyncEntries.size(); i < total; ++i) {
		EntryInfo<QAsyncSettingsLoader> &entry = this->asyncEntries[i];
		this->activeAsyncs.insert(entry.currentLoader, i);
		QMetaObject::invokeMethod(entry.currentLoader, "loadData", Qt::QueuedConnection);
	}
	this->currentCount = 0;
	emit progressMaxChanged(this->activeAsyncs.size());

	foreach(auto entry, this->simpleEntries) {
		bool edited = false;
		QVariant data;
		auto ok = entry.currentLoader->load(data, edited);
		if(ok)
			this->updateEntry(entry, data, edited);
		else
			this->disableEntry(entry);
	}

	this->tryLoadComplete();
	this->updateProgress(false);
}

void SettingsEngine::startSaving()
{
	if(!this->activeAsyncs.isEmpty())
		return;
	this->errorCount = 0;

	for(int i = 0, total = this->asyncEntries.size(); i < total; ++i) {
		EntryInfo<QAsyncSettingsLoader> &entry = this->asyncEntries[i];
		if(entry.checkingHelper->testChecked()) {
			if(entry.currentWidget->hasValueChanged() ||
			   entry.checkingHelper->checkedChanged()) {
				auto data = entry.currentWidget->getValue();
				this->activeAsyncs.insert(entry.currentLoader, i);
				QMetaObject::invokeMethod(entry.currentLoader, "saveData", Qt::QueuedConnection,
										  Q_ARG(QVariant, data));
			}
		} else if(entry.checkingHelper->checkedChanged()) {
			this->activeAsyncs.insert(entry.currentLoader, i);
			QMetaObject::invokeMethod(entry.currentLoader, "resetData", Qt::QueuedConnection);
		}
	}
	this->currentCount = 0;
	emit progressMaxChanged(this->activeAsyncs.size());

	foreach(auto entry, this->simpleEntries) {
		if(entry.checkingHelper->testChecked()) {
			if(entry.currentWidget->hasValueChanged() ||
			   entry.checkingHelper->checkedChanged()) {
				if(entry.currentLoader->save(entry.currentWidget->getValue())) {
					entry.currentWidget->resetValueChanged();
					entry.checkingHelper->resetInitState();
				} else
					this->errorCount++;
			}
		} else if(entry.checkingHelper->checkedChanged()) {
			if(entry.currentLoader->reset())
				entry.checkingHelper->resetInitState();
			else
				this->errorCount++;
		}
	}

	this->updateProgress(false);
}

void SettingsEngine::startResetting()
{
	if(!this->activeAsyncs.isEmpty())
		return;
	this->errorCount = 0;

	for(int i = 0, total = this->asyncEntries.size(); i < total; ++i) {
		EntryInfo<QAsyncSettingsLoader> &entry = this->asyncEntries[i];
		this->activeAsyncs.insert(entry.currentLoader, i);
		QMetaObject::invokeMethod(entry.currentLoader, "resetData", Qt::QueuedConnection);
	}
	this->currentCount = 0;
	emit progressMaxChanged(this->activeAsyncs.size());

	foreach(auto entry, this->simpleEntries) {
		if(!entry.currentLoader->reset())
			this->errorCount++;
	}

	this->updateProgress(false);
}

void SettingsEngine::abortOperation()
{
	if(!this->activeAsyncs.isEmpty()) {
		this->activeAsyncs.clear();
		emit operationAborted();
	}
}

void SettingsEngine::entryLoaded(bool successful, const QVariant &data, bool isUserEdited)
{
	if(this->activeAsyncs.contains(QObject::sender())) {
		auto index = this->activeAsyncs.take(QObject::sender());
		EntryInfo<QAsyncSettingsLoader> &entry = this->asyncEntries[index];

		if(successful)
			this->updateEntry(entry, data, isUserEdited);
		else
			this->disableEntry(entry);

		this->tryLoadComplete();
		this->updateProgress(true);
	}
}

void SettingsEngine::entrySaved(bool successful)
{
	if(this->activeAsyncs.contains(QObject::sender())) {
		auto index = this->activeAsyncs.take(QObject::sender());
		EntryInfo<QAsyncSettingsLoader> &entry = this->asyncEntries[index];

		if(successful) {
			entry.currentWidget->resetValueChanged();
			entry.checkingHelper->resetInitState();
		} else
			this->errorCount++;

		this->updateProgress(true);
	}
}

void SettingsEngine::entryResetted(bool successful)
{
	if(this->activeAsyncs.contains(QObject::sender())) {
		auto index = this->activeAsyncs.take(QObject::sender());
		EntryInfo<QAsyncSettingsLoader> &entry = this->asyncEntries[index];

		if(successful)
			entry.checkingHelper->resetInitState();
		else
			this->errorCount++;

		this->updateProgress(true);
	}
}

void SettingsEngine::updateEntry(EntryInfoBase &entry, const QVariant &data, bool isUserEdited)
{
	if(data.isValid())
		entry.currentWidget->setValue(data);
	else
		entry.currentWidget->resetValue();
	entry.currentWidget->resetValueChanged();

	if(isUserEdited)
		entry.checkingHelper->doCheck();
}

void SettingsEngine::disableEntry(SettingsEngine::EntryInfoBase &entry)
{
	entry.currentWidget->asWidget()->setEnabled(false);
	entry.checkingHelper->disableLabel();
	this->errorCount++;
}

void SettingsEngine::updateProgress(bool increment)
{
	if(increment)
		emit progressValueChanged(++this->currentCount);
	if(this->activeAsyncs.isEmpty())
		emit operationCompleted(this->errorCount);
}

void SettingsEngine::tryLoadComplete()
{
	if(this->activeAsyncs.isEmpty()) {
		foreach(auto entry, this->asyncEntries)
			entry.checkingHelper->resetInitState();
		foreach(auto entry, this->simpleEntries)
			entry.checkingHelper->resetInitState();
	}
}

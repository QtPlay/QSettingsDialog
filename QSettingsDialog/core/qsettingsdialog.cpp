#include "qsettingsdialog.h"
#include "qsettingsdialog_p.h"
#include "settingspathparser.h"
#include "qsettingscontainer.h"
#include "qasyncsettingscontainer.h"
#include <QDebug>

#include "qsettingswidgetdialogengine.h"
#include <algorithm>
#undef d
#define d this->d_ptr

QSettingsDialog::QSettingsDialog(QObject *parent) :
	QObject(parent),
	d_ptr(new QSettingsDialogPrivate(this, new QSettingsWidgetDialogEngine()))
{}

QSettingsDialog::QSettingsDialog(QSettingsDisplayEngine *engine, QObject *parent) :
	QObject(parent),
	d_ptr(new QSettingsDialogPrivate(this, engine))
{}

QSettingsDialog::~QSettingsDialog() {}

QSettingsDisplayEngine *QSettingsDialog::displayEngine() const
{
	return d->displayEngine.data();
}

void QSettingsDialog::setDisplayEngine(QSettingsDisplayEngine *engine)
{
	d->displayEngine.reset(engine);
}

QString QSettingsDialog::containerPath() const
{
	return SettingsPathParser::createPath(d->categoryId, d->sectionId, d->groupId);
}

QString QSettingsDialog::sectionContainerPath() const
{
	return SettingsPathParser::createPath(d->categoryId, d->sectionId);
}

QString QSettingsDialog::categoryId() const
{
	return d->categoryId;
}

QString QSettingsDialog::sectionId() const
{
	return d->sectionId;
}

QString QSettingsDialog::groupId() const
{
	return d->groupId;
}

void QSettingsDialog::setCategory(const QString &id, const QString &name, const QIcon &icon, const QString &tooltip)
{
	auto element = d->getCategory(id);
	Q_ASSERT(!element.isNull());
	if(!id.isEmpty()) {
		d->categoryId = id;
		d->sectionId = QLatin1Char('.');
		d->groupId.clear();
	}

	if(!name.isNull())
		element->name = name;
	if(!icon.isNull())
		element->icon = icon;
	if(!tooltip.isNull())
		element->tooltip = tooltip;
}

void QSettingsDialog::setSection(const QString &id, const QString &name, const QIcon &icon, const QString &tooltip)
{
	auto element = d->getSection(id);
	Q_ASSERT(!element.isNull());
	if(!id.isEmpty()) {
		d->sectionId = id;
		d->groupId.clear();
	}

	if(!name.isNull())
		element->name = name;
	if(!icon.isNull())
		element->icon = icon;
	if(!tooltip.isNull())
		element->tooltip = tooltip;
}

void QSettingsDialog::setGroup(const QString &id, int displayId, const QString &name, int optional, const QString &tooltip)
{
	auto element = d->getGroup(id);
	Q_ASSERT(!element.isNull());
	if(!id.isEmpty())
		d->groupId = id;

	if(displayId != -1)
		element->displayId = displayId;
	if(!name.isNull())
		element->name = name;
	if(!tooltip.isNull())
		element->tooltip = tooltip;
	if(optional >= 0)
		element->isOptional = (bool)optional;
}

void QSettingsDialog::unsetGroup()
{
	d->groupId.clear();
}

void QSettingsDialog::setContainer(const QString &containerPath)
{
	auto elements = SettingsPathParser::parseFullPath(containerPath);
	Q_ASSERT(elements.size() >= 2);
	if(!elements[0].isEmpty())
		d->categoryId = elements[0];
	if(!elements[1].isEmpty())
		d->sectionId = elements[1];
	if(elements.size() == 3) {
		if(!elements[2].isEmpty())
			d->groupId = elements[2];
	} else
		d->groupId.clear();
}

bool QSettingsDialog::removeContainer(const QString &containerPath)
{
	auto elements = SettingsPathParser::parsePartialPath(containerPath);
	switch(elements.size()) {
	case 1:
		if(elements[0] == QStringLiteral(".")) {
			d->rootElement->defaultCategory.reset();
			return true;
		} else {
			if(elements[0].isEmpty())
				elements[0] = d->categoryId;
			return d->rootElement->categories.removeId(elements[0]);
		}
	case 2:
	{
		auto category = d->getCategory(elements[0]);
		if(elements[1] == QStringLiteral(".")) {
			category->defaultSection.reset();
			return true;
		} else {
			if(elements[1].isEmpty())
				elements[1] = d->sectionId;
			return category->sections.removeId(elements[1]);
		}
	}
	case 3:
	{
		auto section = d->getSection(elements[1], elements[0]);
		if(elements[2] == QStringLiteral("."))
			throw new InvalidContainerPathException();
		else {
			if(elements[2].isEmpty())
				elements[2] = d->groupId;
			return section->groups.removeId(elements[2]);
		}
	}
	default:
		Q_UNREACHABLE();
	}
}

QSettingsContainer *QSettingsDialog::currentContainer(QObject *parent)
{
	if(d->groupId.isEmpty())
		return new QSectionSettingsContainer(this, SettingsPathParser::createPath(d->categoryId, d->sectionId), parent);
	else
		return new QGroupSettingsContainer(this, SettingsPathParser::createPath(d->categoryId, d->sectionId, d->groupId), parent);
}

QSettingsContainer *QSettingsDialog::currentContainerAsync(QObject *parent)
{
	return new QAsyncSettingsContainer(this, SettingsPathParser::createPath(d->categoryId, d->sectionId, d->groupId), parent);
}

QSettingsContainer *QSettingsDialog::currentSectionContainer(QObject *parent)
{
	return new QSectionSettingsContainer(this, SettingsPathParser::createPath(d->categoryId, d->sectionId), parent);
}

QSettingsContainer *QSettingsDialog::currentSectionContainerAsync(QObject *parent)
{
	return new QAsyncSettingsContainer(this, SettingsPathParser::createPath(d->categoryId, d->sectionId), parent);
}

int QSettingsDialog::appendEntry(QSettingsEntry *entry)
{
	if(d->groupId.isEmpty()) {
		auto section = d->getSection();
		Q_ASSERT(!section.isNull());

		auto id = d->getNextId();
		section->groups.append(id, entry);
		return id;
	} else {
		auto group = d->getGroup();
		Q_ASSERT(!group.isNull());

		auto id = d->getNextId();
		group->entries.append(id, entry);
		return id;
	}
}

int QSettingsDialog::appendEntry(const QString &containerPath, QSettingsEntry *entry)
{
	auto elements = SettingsPathParser::parseFullPath(containerPath);
	if(elements.size() == 2) {
		auto section = d->getSection(elements[1], elements[0]);
		Q_ASSERT(!section.isNull());

		auto id = d->getNextId();
		section->groups.append(id, entry);
		return id;
	} else {
		Q_ASSERT(elements.size() == 3);

		auto group = d->getGroup(elements[2], elements[1], elements[0]);
		Q_ASSERT(!group.isNull());

		auto id = d->getNextId();
		group->entries.append(id, entry);
		return id;
	}
}

int QSettingsDialog::prependEntry(QSettingsEntry *entry)
{
	if(d->groupId.isEmpty()) {
		auto section = d->getSection();
		Q_ASSERT(!section.isNull());

		auto id = d->getNextId();
		section->groups.prepend(id, entry);
		return id;
	} else {
		auto group = d->getGroup();
		Q_ASSERT(!group.isNull());

		auto id = d->getNextId();
		group->entries.prepend(id, entry);
		return id;
	}
}

int QSettingsDialog::prependEntry(const QString &containerPath, QSettingsEntry *entry)
{
	auto elements = SettingsPathParser::parseFullPath(containerPath);
	if(elements.size() == 2) {
		auto section = d->getSection(elements[1], elements[0]);
		Q_ASSERT(!section.isNull());

		auto id = d->getNextId();
		section->groups.prepend(id, entry);
		return id;
	} else {
		Q_ASSERT(elements.size() == 3);

		auto group = d->getGroup(elements[2], elements[1], elements[0]);
		Q_ASSERT(!group.isNull());

		auto id = d->getNextId();
		group->entries.prepend(id, entry);
		return id;
	}
}

QSharedPointer<QSettingsEntry> QSettingsDialog::getEntry(int id) const
{
	auto path = d->findEntryPath(id);
	auto elements = SettingsPathParser::parseFullPath(path);

	if(elements.size() == 2) {
		auto section = d->getSection(elements[1], elements[0]);
		Q_ASSERT(!section.isNull());
		return section->groups.valueId(id);
	} else {
		Q_ASSERT(elements.size() == 3);

		auto group = d->getGroup(elements[2], elements[1], elements[0]);
		Q_ASSERT(!group.isNull());
		return group->entries.valueId(id);
	}
}

QString QSettingsDialog::getEntryPath(int id) const
{
	return d->findEntryPath(id);
}

bool QSettingsDialog::removeEntry(int id)
{
	auto path = d->findEntryPath(id);
	auto elements = SettingsPathParser::parseFullPath(path);

	if(elements.size() == 2) {
		auto section = d->getSection(elements[1], elements[0]);
		Q_ASSERT(!section.isNull());
		return section->groups.removeId(id);
	} else {
		Q_ASSERT(elements.size() == 3);

		auto group = d->getGroup(elements[2], elements[1], elements[0]);
		Q_ASSERT(!group.isNull());
		return group->entries.removeId(id);
	}
}

QString QSettingsDialog::createContainerPath(QString category, QString section, QString group)
{
	return SettingsPathParser::createPath(category, section, group);
}

void QSettingsDialog::openSettings(QWidget *parentWindow)
{
	d->showDialog(false, parentWindow);
}

int QSettingsDialog::execSettings(QWidget *parentWindow)
{
	return d->showDialog(true, parentWindow);
}

void QSettingsDialog::completed(bool close)
{
	if(close) {
		d->currentDialog->deleteLater();
		d->currentDialog = nullptr;
	}
}



QSettingsDialogPrivate *QSettingsDialogPrivate::getPrivateInstance(QSettingsDialog *dialog)
{
	return dialog->d_ptr.data();
}

int QSettingsDialogPrivate::getNextId()
{
	return ++this->currentIdMax;
}

QSettingsDialogPrivate::QSettingsDialogPrivate(QSettingsDialog *q_ptr, QSettingsDisplayEngine *engine) :
    currentIdMax(0),
    q_ptr(q_ptr),
	rootElement(new SettingsRoot()),
	categoryId(QStringLiteral(".")),
	sectionId(QStringLiteral(".")),
    groupId(),
    displayEngine(engine),
    currentDialog(nullptr)
{}

QSharedPointer<SettingsCategory> QSettingsDialogPrivate::getCategory(QString categoryId)
{
	SettingsPathParser::validateId(categoryId, false);
	if(categoryId.isEmpty())
		categoryId = this->categoryId;

	if(categoryId == QLatin1String(".")) {
		if(this->rootElement->defaultCategory.isNull())
			this->rootElement->defaultCategory = SettingsCategory::createDefaultCategory();
		return this->rootElement->defaultCategory;
	} else {
		auto element = this->rootElement->categories.valueId(categoryId);
		if(element.isNull())
			element = this->rootElement->categories.append(categoryId, new SettingsCategory(categoryId));
		return element;
	}
}

QSharedPointer<SettingsSection> QSettingsDialogPrivate::getSection(QString sectionId, const QString &categoryId)
{
	auto category = this->getCategory(categoryId);
	Q_ASSERT(!category.isNull());

	SettingsPathParser::validateId(sectionId, false);
	if(sectionId.isEmpty())
		sectionId = this->sectionId;

	if(sectionId == QLatin1String(".")) {
		if(category->defaultSection.isNull())
			category->defaultSection = SettingsSection::createDefaultSection();
		return category->defaultSection;
	} else {
		auto element = category->sections.valueId(sectionId);
		if(element.isNull())
			element = category->sections.append(sectionId, new SettingsSection(sectionId));
		return element;
	}
}

QSharedPointer<SettingsGroup> QSettingsDialogPrivate::getGroup(QString groupId, const QString &sectionId, const QString &categoryId)
{
	auto section = this->getSection(sectionId, categoryId);
	Q_ASSERT(!section.isNull());

	SettingsPathParser::validateId(groupId, false);
	if(groupId.isEmpty())
		groupId = this->groupId;

	if(groupId == QLatin1String("."))
		throw InvalidContainerPathException();
	else {
		auto element = section->groups.valueId(groupId);
		if(element.isNull())
			element = section->groups.append(groupId, new SettingsGroup(0, groupId));
		return element;
	}
}

QString QSettingsDialogPrivate::findEntryPath(int id)
{
	auto categories = this->rootElement->categories;
	if(!this->rootElement->defaultCategory.isNull())
		categories.append(QStringLiteral("."), this->rootElement->defaultCategory);

	foreach(auto categoryElement, categories) {
		const QSharedPointer<SettingsCategory> &category = categoryElement.second;

		auto sections = category->sections;
		if(!category->defaultSection.isNull())
			sections.append(QStringLiteral("."), category->defaultSection);

		foreach(auto sectionElement, sections) {
			const QSharedPointer<SettingsSection> &section = sectionElement.second;

			if(section->groups.contains(id)) {
				return SettingsPathParser::createPath(categoryElement.first,
													  sectionElement.first,
													  QString());
			}

			auto groups = section->groups.createGroupMap();
			foreach(auto groupElement, groups) {
				const QSharedPointer<SettingsGroup> &group = groupElement.second;

				if(group->entries.contains(id)) {
					return SettingsPathParser::createPath(categoryElement.first,
														  sectionElement.first,
														  groupElement.first);
				}
			}

		}
	}

	return QString();
}

int QSettingsDialogPrivate::showDialog(bool asExec, QWidget *parentWindow)
{
	if(!this->currentDialog.isNull()) {
		qWarning() << "Only one settings dialog can be shown at a time";//NOTE exception?
		return -1;
	}

	auto instance = this->displayEngine->createInstance();
	instance->setParentWindow(parentWindow);
	instance->createUi(this->rootElement);
	this->currentDialog = dynamic_cast<QObject*>(instance);
	Q_ASSERT(this->currentDialog.data());

	QObject::connect(this->currentDialog.data(), SIGNAL(saved(bool)),
					 this->q_ptr, SIGNAL(saved(bool)));
	QObject::connect(this->currentDialog.data(), SIGNAL(resetted()),
					 this->q_ptr, SIGNAL(resetted()));
	QObject::connect(this->currentDialog.data(), SIGNAL(canceled()),
					 this->q_ptr, SIGNAL(canceled()));

	QObject::connect(this->currentDialog.data(), SIGNAL(saved(bool)),
					 this->q_ptr, SLOT(completed(bool)));
	QObject::connect(this->currentDialog.data(), SIGNAL(resetted()),
					 this->q_ptr, SLOT(completed()));
	QObject::connect(this->currentDialog.data(), SIGNAL(canceled()),
					 this->q_ptr, SLOT(completed()));

	if(asExec)
		return instance->exec();
	else {
		instance->open();
		return 0;
	}
}


//===================

std::list<QSettingsDialog::CbRec> QSettingsDialog::d_cbs;
bool QSettingsDialog::reg(const QString & id,
                          std::function<void(QSettingsDialog & dlg)> addEntry ) {
    auto it = std::find_if(d_cbs.begin(), d_cbs.end(),
                           [&id](QSettingsDialog::CbRec & rec)->bool{
            return id == rec.id;});

    if( it != d_cbs.end() )
        return false;
    d_cbs.push_back({id, addEntry});
    return true;
}

void QSettingsDialog::unreg(const QString & id) {
    auto it = std::find_if(d_cbs.begin(), d_cbs.end(),
                           [&id](QSettingsDialog::CbRec & rec)->bool{
            return id == rec.id;});

    if( it != d_cbs.end() )
        d_cbs.erase(it);
    else
        Q_ASSERT(0);
}

void QSettingsDialog::setup(QSettingsDialog & dlg) {
    for(auto & rec : d_cbs) {
        rec.cb(dlg);
    }
}

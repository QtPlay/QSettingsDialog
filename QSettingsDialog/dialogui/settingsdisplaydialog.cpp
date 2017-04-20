#include "settingsdisplaydialog.h"
#include "ui_settingsdisplaydialog.h"
#include <dialogmaster.h>
#include <QTabWidget>
#include <QScrollArea>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QPushButton>
#include <QWindow>
#include "settingsengine.h"
#include "qsettingsgroupwidget.h"
#include "qsettingsdialog.h"
#include "settingspathparser.h"
#include "qsettingsdialog_p.h"

#define TAB_CONTENT_NAME "tabContent_371342666"

SettingsDisplayDialog::SettingsDisplayDialog(QSettingsWidgetDialogEngine *dialogEngine) :
	QDialog(nullptr),
	dialogEngine(dialogEngine),
	engine(new SettingsEngine(this)),
	ui(new Ui::SettingsDisplayDialog),
	delegate(nullptr),
	maxWidthBase(0),
	currentMode(Idle),
	workingDialog(nullptr)
{
	ui->setupUi(this);
	this->ui->buttonBox->button(QDialogButtonBox::Ok)->setAutoDefault(false);
    this->ui->buttonBox->button(QDialogButtonBox::Ok)->setText(tr("OK"));
	this->ui->buttonBox->button(QDialogButtonBox::Cancel)->setAutoDefault(false);
    this->ui->buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));
	this->ui->buttonBox->button(QDialogButtonBox::Apply)->setAutoDefault(false);
    this->ui->buttonBox->button(QDialogButtonBox::Apply)->setText(tr("Apply"));
	this->ui->buttonBox->button(QDialogButtonBox::RestoreDefaults)->setAutoDefault(false);
    this->ui->buttonBox->button(QDialogButtonBox::RestoreDefaults)->setText(tr("Restore"));
	this->ui->buttonBox->button(QDialogButtonBox::Ok)->setDefault(true);

#ifdef Q_OS_OSX
	auto font = this->ui->titleLabel->font();
	font.setPointSize(16);
	this->ui->titleLabel->setFont(font);
#endif

	connect(this->ui->buttonBox, &QDialogButtonBox::clicked,
			this, &SettingsDisplayDialog::buttonBoxClicked);
	connect(this, &SettingsDisplayDialog::rejected,
			this, &SettingsDisplayDialog::canceled);

	int listSpacing = this->calcSpacing(Qt::Vertical);
	this->delegate = new CategoryItemDelegate(std::bind(&SettingsDisplayDialog::updateWidth, this, std::placeholders::_1),
											  this->ui->categoryListWidget->iconSize(),
											  qMax(qRound(listSpacing * (2./3.)), 1),
											  this);
	this->ui->categoryListWidget->setItemDelegate(this->delegate);
	this->ui->categoryListWidget->setSpacing(qMax(qRound(listSpacing / 3.), 1) - 1);

	int spacing = this->calcSpacing(Qt::Horizontal);
	this->ui->contentLayout->setSpacing(spacing);
	this->ui->categoryLineSpacer->changeSize(spacing,
											 0,
											 QSizePolicy::Fixed,
											 QSizePolicy::Fixed);

	//engine
	connect(this->engine, &SettingsEngine::operationCompleted,
			this, &SettingsDisplayDialog::engineFinished);
	connect(this->engine, &SettingsEngine::operationAborted,
			this, &SettingsDisplayDialog::completeAbort);
}

SettingsDisplayDialog::~SettingsDisplayDialog()
{
	delete ui;
}

void SettingsDisplayDialog::setParentWindow(QWidget *parent)
{
	this->setParent(parent);
	DialogMaster::masterDialog(this);
}

void SettingsDisplayDialog::setParentWindow(QWindow *parent)
{
	this->windowHandle()->setParent(parent);
	DialogMaster::masterDialog(this);
}

void SettingsDisplayDialog::createUi( QSharedPointer<SettingsRoot> root)
{
    this->elementRoot = root;
	if(elementRoot->defaultCategory)
		this->createCategory(elementRoot->defaultCategory);
	foreach(auto category, elementRoot->categories)
		this->createCategory(category.second);
	this->ui->contentStackWidget->addWidget(new QWidget(this->ui->contentStackWidget));

	this->resetListSize();
	this->ui->categoryListWidget->setCurrentRow(0);
}

void SettingsDisplayDialog::open()
{
	this->QDialog::open();
}

int SettingsDisplayDialog::exec()
{
	return this->QDialog::exec();
}

void SettingsDisplayDialog::showEvent(QShowEvent *ev)
{
	this->currentMode = Load;
    this->workingDialog = DialogMaster::createProgress(this, QString(tr("Loading settings…")), 1);
	this->workingDialog->setMaximum(0);
	this->workingDialog->setAutoClose(false);
	this->workingDialog->setAutoReset(false);

	connect(this->workingDialog, &QProgressDialog::canceled,
			this->engine, &SettingsEngine::abortOperation);
	connect(this->engine, &SettingsEngine::progressMaxChanged,
			this->workingDialog, &QProgressDialog::setMaximum);
	connect(this->engine, &SettingsEngine::progressValueChanged,
			this->workingDialog, &QProgressDialog::setValue);

	this->engine->startLoading();
	this->QDialog::showEvent(ev);
}

void SettingsDisplayDialog::startSaving(bool isApply)
{
	this->currentMode = isApply ? Apply : Save;
	this->workingDialog = DialogMaster::createProgress(this, tr("Saving settings…"), 1);
	this->workingDialog->setMaximum(0);
	this->workingDialog->setAutoClose(false);
	this->workingDialog->setAutoReset(false);

	connect(this->workingDialog, &QProgressDialog::canceled,
			this->engine, &SettingsEngine::abortOperation);
	connect(this->engine, &SettingsEngine::progressMaxChanged,
			this->workingDialog, &QProgressDialog::setMaximum);
	connect(this->engine, &SettingsEngine::progressValueChanged,
			this->workingDialog, &QProgressDialog::setValue);

	this->engine->startSaving();
}

void SettingsDisplayDialog::startResetting()
{
	this->currentMode = Reset;
	this->workingDialog = DialogMaster::createProgress(this, tr("Restoring default settings…"), 1);
	this->workingDialog->setMaximum(0);
	this->workingDialog->setAutoClose(false);
	this->workingDialog->setAutoReset(false);

	connect(this->workingDialog, &QProgressDialog::canceled,
			this->engine, &SettingsEngine::abortOperation);
	connect(this->engine, &SettingsEngine::progressMaxChanged,
			this->workingDialog, &QProgressDialog::setMaximum);
	connect(this->engine, &SettingsEngine::progressValueChanged,
			this->workingDialog, &QProgressDialog::setValue);

	this->engine->startResetting();
}

void SettingsDisplayDialog::engineFinished(int errorCount)
{
	if(this->workingDialog) {
		this->workingDialog->close();
		this->workingDialog->deleteLater();
		this->workingDialog = nullptr;
	}

	auto wasApply = false;
	switch (this->currentMode) {
	case Load:
		if(errorCount > 0) {
			auto res = DialogMaster::warning(this,
											 tr("Loading finished with errors. %1 entries failed to load their data.")
											 .arg(errorCount),
											 tr("Loading errors"),
											 QString(),
											 QMessageBox::Close | QMessageBox::Ignore,
											 QMessageBox::Close,
											 QMessageBox::Ignore);
			if(res == QMessageBox::Close)
				this->reject();
		}
		break;
	case Apply:
		wasApply = true;
	case Save:
		if(errorCount > 0) {
			auto res = DialogMaster::warning(this,
											 tr("Saving finished with errors. %1 entries failed to save their data.")
											 .arg(errorCount),
											 tr("Saving errors"),
											 QString(),
											 QMessageBox::Ignore | QMessageBox::Retry,
											 QMessageBox::Retry,
											 QMessageBox::Ignore);
			if(res == QMessageBox::Retry) {
				QMetaObject::invokeMethod(this, "startSaving", Qt::QueuedConnection,
										  Q_ARG(bool, wasApply));
			} else {
				emit saved(!wasApply);
				if(!wasApply)
					this->accept();
			}
		} else {
			emit saved(!wasApply);
			if(!wasApply)
				this->accept();
		}
		break;
	case Reset:
		if(errorCount > 0) {
			auto res = DialogMaster::warning(this,
											 tr("Restoring defaults finished with errors. %1 entries failed to reset their data.")
											 .arg(errorCount),
											 tr("Resetting errors"),
											 QString(),
											 QMessageBox::Ignore | QMessageBox::Retry,
											 QMessageBox::Retry,
											 QMessageBox::Ignore);
			if(res == QMessageBox::Retry)
				QMetaObject::invokeMethod(this, "startResetting", Qt::QueuedConnection);
			else if(res == QMessageBox::Ignore) {
				emit resetted();
				this->accept();
			}
		} else {
			emit resetted();
			this->accept();
		}
		break;
	default:
		break;
	}

	this->currentMode = Idle;
}

void SettingsDisplayDialog::completeAbort()
{
	if(this->workingDialog) {
		this->workingDialog->close();
		this->workingDialog->deleteLater();
		this->workingDialog = nullptr;
	}

	if(this->currentMode == Load)
		this->reject();
	this->currentMode = Idle;
}

void SettingsDisplayDialog::resetListSize()
{
	int max = this->ui->categoryListWidget->count();
	if(max <= 1) {
		this->ui->categoryContentWidget->hide();
		this->resize(this->width() - this->ui->categoryContentWidget->sizeHint().width(),
					 this->height());
	} else {
		auto width = this->ui->categoryListWidget->sizeHint().width();
		this->ui->categoryListWidget->setFixedWidth(width);
		this->maxWidthBase = width;
	}
}

void SettingsDisplayDialog::updateWidth(int width)
{
	if(width > this->maxWidthBase) {
		this->maxWidthBase = width;
		QStyle *style = this->ui->categoryListWidget->style();
		width += style->pixelMetric(QStyle::PM_ScrollBarExtent);
		width += style->pixelMetric(QStyle::PM_LayoutHorizontalSpacing);
		this->ui->categoryListWidget->setFixedWidth(width);
	}
}

void SettingsDisplayDialog::buttonBoxClicked(QAbstractButton *button)
{
	if(this->currentMode != Idle)
		return;

	switch(this->ui->buttonBox->standardButton(button)) {
	case QDialogButtonBox::Ok:
		this->startSaving(false);
		break;
	case QDialogButtonBox::Cancel:
		this->reject();
		break;
	case QDialogButtonBox::Apply:
		this->startSaving(true);
		break;
	case QDialogButtonBox::RestoreDefaults:
		if(DialogMaster::warning(this,
								 tr("All custom changes will be deleted and the defaults restored. "
									"<i>This cannot be undone!</i>"),
								 tr("%1?").arg(button->text()),
								 QString(),
								 QMessageBox::Yes | QMessageBox::No,
								 QMessageBox::Yes,
								 QMessageBox::No)
		   == QMessageBox::Yes) {
			this->startResetting();
		}
		break;
	default:
		Q_UNREACHABLE();
	}
}

void SettingsDisplayDialog::on_filterLineEdit_textChanged(const QString &searchText)
{
	auto regex = QRegularExpression::escape(searchText);
	regex.replace(QStringLiteral("\\*"), QStringLiteral(".*"));
	regex.replace(QStringLiteral("\\?"), QStringLiteral("."));
	this->searchInDialog(QRegularExpression(regex, QRegularExpression::CaseInsensitiveOption));
}

int SettingsDisplayDialog::calcSpacing(Qt::Orientation orientation)
{
	auto baseSize = this->style()->pixelMetric(orientation == Qt::Horizontal ?
												   QStyle::PM_LayoutHorizontalSpacing :
												   QStyle::PM_LayoutVerticalSpacing);
	if(baseSize < 0)
		baseSize = this->style()->layoutSpacing(QSizePolicy::DefaultType, QSizePolicy::DefaultType, orientation);
	if(baseSize < 0)
		baseSize = this->style()->layoutSpacing(QSizePolicy::LineEdit, QSizePolicy::LineEdit, orientation);
	if(baseSize < 0) {
#ifdef Q_OS_OSX
		baseSize = 10;
#else
		baseSize = 6;
#endif
	}

	return baseSize;
}

void SettingsDisplayDialog::createCategory(const QSharedPointer<SettingsCategory> &category)
{
	auto item = new QListWidgetItem();
	item->setText(category->name);
	item->setIcon(category->icon);
	item->setToolTip(category->tooltip.isNull() ? category->name : category->tooltip);
	auto tab = new QTabWidget();
	tab->setTabBarAutoHide(true);

	this->ui->contentStackWidget->addWidget(tab);
	this->ui->categoryListWidget->addItem(item);

    tab->setWindowFilePath(QSettingsDialog::createContainerPath(category->id));
    categories[QSettingsDialog::createContainerPath(category->id)]= tab;

	if(category->defaultSection)
		this->createSection(category->defaultSection, tab);
	foreach(auto section, category->sections)
		this->createSection(section.second, tab);

}
#include <QtGlobal>
void SettingsDisplayDialog::createSection(const QSharedPointer<SettingsSection> &section, QTabWidget *tabWidget)
{
	auto scrollArea = new QScrollArea();
	scrollArea->setWidgetResizable(true);
	scrollArea->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
	scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setAutoFillBackground(true);
    auto pal = scrollArea->palette();
#ifdef Q_OS_LINUX
	pal.setColor(QPalette::Window, tabWidget->palette().color(QPalette::Base));
#else
	pal.setColor(QPalette::Window, Qt::transparent);
#endif
    scrollArea->setPalette(pal);
    scrollArea->setFrameShape(QFrame::NoFrame);

	auto scrollContent = new QWidget(scrollArea);
	scrollContent->setObjectName(TAB_CONTENT_NAME);
	auto layout = new QFormLayout(scrollContent);
	scrollContent->setLayout(layout);
	scrollArea->setWidget(scrollContent);

    auto elements = SettingsPathParser::parseFullPath(tabWidget->windowFilePath());
    auto path = QSettingsDialog::createContainerPath(elements[0],  section->id);
    scrollContent->setWindowFilePath(path);
    sections[path] = layout;

	auto index = tabWidget->addTab(scrollArea, section->icon, section->name);
	tabWidget->tabBar()->setTabToolTip(index, section->tooltip.isNull() ? section->name : section->tooltip);

	foreach(auto group, section->groups) {
        if(group.first.type() == QVariant::Int)
			this->createEntry(group.second.second, scrollContent, layout);
		else
			this->createGroup(group.second.first, scrollContent, layout);
	}
}

void SettingsDisplayDialog::createGroup(const QSharedPointer<SettingsGroup> &group, QWidget *contentWidget, QFormLayout *layout)
{
	auto groupWidget = this->dialogEngine->createGroupWidget(group->displayId, contentWidget);
	groupWidget->setName(group->name);
	groupWidget->setTooltip(group->tooltip.isNull() ? group->name : group->tooltip);
	groupWidget->setOptional(group->isOptional);
	layout->addRow(groupWidget->asWidget());

    auto elements = SettingsPathParser::parseFullPath(contentWidget->windowFilePath());
    auto path = QSettingsDialog::createContainerPath(elements[0], elements[1], group->id);
    groupWidget->asWidget()->setWindowFilePath(path);
    groups[path] = groupWidget;

	foreach(auto entry, group->entries)
		this->createEntry(entry.second, groupWidget);

	if(group->isOptional)
		groupWidget->setChecked(false);
}

void SettingsDisplayDialog::createEntry(const QSharedPointer<QSettingsEntry> &entry, QWidget *sectionWidget, QFormLayout *layout)
{
	QWidget *content = nullptr;
	auto settingsWidget = this->dialogEngine->createWidget(entry->displaytype(), entry->uiProperties(), sectionWidget);
    settingsWidget->id = entry->id;
    if(settingsWidget)
		content = settingsWidget->asWidget();
	else
		content = this->createErrorWidget(sectionWidget);

	QWidget *label = nullptr;
	CheckingHelper *labelAsHelper = nullptr;
	if(entry->isOptional()) {
		auto optBox = new CheckingCheckBox(this);
		optBox->setText(entry->entryName() + tr(":"));
		QObject::connect(optBox, &QCheckBox::toggled,
						 content, &QWidget::setEnabled);
		label = optBox;
		labelAsHelper = optBox;
	} else {
		auto cLabel = new CheckingLabel(this);
		cLabel->setText(entry->entryName() + tr(":"));
		label = cLabel;
		labelAsHelper = cLabel;
	}

	label->setToolTip(entry->tooltip());
	if(content->toolTip().isNull())
		content->setToolTip(entry->tooltip());
	if(!settingsWidget)
		label->setEnabled(false);
	else
		content->setEnabled(!entry->isOptional());

	layout->addRow(label, content);
    entryWdgs[entry->id] = std::make_pair(label, content);
	if(settingsWidget)
		this->engine->addEntry(entry, settingsWidget, labelAsHelper);
}


void SettingsDisplayDialog::rmEntryWdg(const QSharedPointer<QSettingsEntry> &entry, QWidget *sectionWidget, QFormLayout *layout) {
    auto it = entryWdgs.find(entry->id);
    Q_ASSERT(it != entryWdgs.end());
    auto index = layout->indexOf(it->second.first); layout->takeAt(index);
    auto field = layout->indexOf(it->second.second); layout->takeAt(field);
    delete it->second.first;
    delete it->second.second;
    entryWdgs.erase(entry->id);
    this->engine->rmEntry(entry, nullptr, nullptr);
}

void SettingsDisplayDialog::createEntry(const QSharedPointer<QSettingsEntry> &entry, QSettingsGroupWidgetBase *group)
{
	auto groupWidget = group->asWidget();
	QWidget *content = nullptr;
	auto settingsWidget = this->dialogEngine->createWidget(entry->displaytype(), entry->uiProperties(), groupWidget);
    settingsWidget->id = entry->id;
	if(settingsWidget)
		content = settingsWidget->asWidget();
	else
		content = this->createErrorWidget(groupWidget);

	group->addWidgetRaw(entry, content, !settingsWidget);
	if(settingsWidget)
		this->engine->addEntry(entry, settingsWidget, new GroupCheckingHelper(group, entry));
}

void SettingsDisplayDialog::rmEntryWdg(const QSharedPointer<QSettingsEntry> &entry, QSettingsGroupWidgetBase *group) {
    group->rmWidgetRaw(entry, nullptr, false);
}

QWidget *SettingsDisplayDialog::createErrorWidget(QWidget *parent)
{
	auto content = new QWidget(parent);
	auto layout = new QHBoxLayout(content);
	layout->setContentsMargins(QMargins());
	content->setLayout(layout);

	auto iconLabel = new QLabel(content);
	iconLabel->setPixmap(this->style()->standardIcon(QStyle::SP_MessageBoxWarning).pixmap(16, 16));
	layout->addWidget(iconLabel);
	layout->setStretchFactor(iconLabel, 0);

	auto errorTextLabel = new QLabel(tr("<i>Failed to load element</i>"), content);
	errorTextLabel->setEnabled(false);
	layout->addWidget(errorTextLabel);
	layout->setStretchFactor(errorTextLabel, 1);

	return content;
}

void SettingsDisplayDialog::addEntry(int id, const QVector<QString> & elements, QSharedPointer<QSettingsEntry> entry) {

    //create category widget, section wdg, group wdg
    QSharedPointer<SettingsCategory> cat;
    auto catIt = categories.find(QSettingsDialog::createContainerPath(elements[0]));
    if( catIt == categories.end()) {
        foreach( auto  category, elementRoot->categories){
            if( category.second->id == elements[0]) {
                cat = category.second;
                this->createCategory(category.second);
                return;
            }
        }
        if( cat.isNull() ) {
            cat = elementRoot->defaultCategory;
        }
    } else {
        foreach( auto  category, elementRoot->categories){
            if( category.second->id == elements[0]) {
                cat = category.second;
            }
        }
        if( cat.isNull())
            cat = elementRoot->defaultCategory;
    }
    catIt = categories.find(QSettingsDialog::createContainerPath(elements[0]));
    Q_ASSERT(catIt != categories.end());
    auto tab = catIt->second;

    QSharedPointer<SettingsSection> sec;
    auto secIt = sections.find(QSettingsDialog::createContainerPath(elements[0], elements[1]));
    if( secIt == sections.end()) {
        foreach(auto  section, cat->sections) {
            if( section.second->id == elements[1]) {
                sec = section.second;
                this->createSection(sec, tab);
                return;
            }
        }
        if( sec.isNull() ) {
            sec=cat->defaultSection;
            this->createSection(sec, tab);
        }
    } else {
        foreach(auto  section, cat->sections) {
            if( section.second->id == elements[1]) {
                sec = section.second;
            }
        }
        if( sec.isNull())
            sec=cat->defaultSection;
    }

    secIt = sections.find(QSettingsDialog::createContainerPath(elements[0], elements[1]));
    Q_ASSERT(secIt != sections.end());
    auto layout = secIt->second;
    auto scrollContent = layout->parentWidget();

    QSharedPointer<SettingsGroup> grp;
    if( elements.size() == 3) {
        auto groupIt = groups.find(QSettingsDialog::createContainerPath(elements[0], elements[1], elements[2]));
        if( groupIt == groups.end() ){
            if( sec->groups.contains(elements[2])) {
                grp = sec->groups.valueId(elements[2]);
                this->createGroup(grp, scrollContent, layout);
                return;
            }/*
            if( sec->groups.size())
            foreach(auto group, sec->groups) {
                if( group.second.first->id == elements[2]) {
                    grp = group.second.first;
                    this->createGroup(group.second.first, scrollContent, layout);
                }
            }*/
        } else {
            ;
        }
    }

    if(elements.size() == 2) {
        if(!sec.data()) {
            auto index = QSettingsDialog::createContainerPath(elements[0], elements[1]);
            auto it = sections.find(index);
            Q_ASSERT(it != sections.end());
            auto layout = it->second;
            auto widget = layout->parentWidget();
            Q_ASSERT(layout && widget);
            createEntry(entry, widget, layout);
        }
    } else {
        if(!grp.data()) {
            Q_ASSERT(elements.size() == 3);
            auto index = QSettingsDialog::createContainerPath(elements[0], elements[1], elements[2]);
            auto it = groups.find(index);
            Q_ASSERT(it != groups.end());
            auto group = it->second;
            Q_ASSERT(group);
            createEntry(entry, group);
        }
    }
}

void SettingsDisplayDialog::rmEntry(int id, const QVector<QString> & elements,  QSharedPointer<QSettingsEntry> entry) {
    if(elements.size() == 2) {
        auto index = QSettingsDialog::createContainerPath(elements[0], elements[1]);
        auto it = sections.find(index);
        Q_ASSERT(it != sections.end());
        auto layout = it->second;
        auto widget = layout->parentWidget();
        Q_ASSERT(layout && widget);
        rmEntryWdg(entry, widget, layout);
    } else {
        Q_ASSERT(elements.size() == 3);
        auto index = QSettingsDialog::createContainerPath(elements[0], elements[1], elements[2]);
        auto it = groups.find(index);
        Q_ASSERT(it != groups.end());
        auto group = it->second;
        Q_ASSERT(group);
        rmEntryWdg(entry, group);
    }
}

void SettingsDisplayDialog::searchInDialog(const QRegularExpression &regex)
{
	for(int i = 0, max = this->ui->categoryListWidget->count(); i < max; ++i) {
		auto item = this->ui->categoryListWidget->item(i);
		auto tab = safe_cast<QTabWidget*>(this->ui->contentStackWidget->widget(i));

		if(this->searchInCategory(regex, tab) ||
		   regex.match(item->text()).hasMatch()) {
			item->setHidden(false);

			if(this->ui->categoryListWidget->currentRow() == -1)
				this->ui->categoryListWidget->setCurrentRow(i);
		} else {
			item->setHidden(true);

			if(this->ui->categoryListWidget->currentRow() == i) {
				auto found = false;
				for(int j = 0; j < max; j++) {
					if(!this->ui->categoryListWidget->item(j)->isHidden()){
						this->ui->categoryListWidget->setCurrentRow(j);
						found = true;
						break;
					}
				}
				if(!found) {
					this->ui->categoryListWidget->setCurrentRow(-1);
					this->ui->contentStackWidget->setCurrentIndex(max);
				}
			}
		}
	}
}

bool SettingsDisplayDialog::searchInCategory(const QRegularExpression &regex, QTabWidget *tab)
{
	auto someFound = false;

	for(int i = 0, max = tab->count(); i < max; ++i) {
		if(this->searchInSection(regex, tab->widget(i)->findChild<QWidget*>(TAB_CONTENT_NAME)) ||
		   regex.match(tab->tabText(i)).hasMatch()){
			tab->setTabEnabled(i, true);
			someFound = true;
		} else
			tab->setTabEnabled(i, false);
	}

	return someFound;
}

bool SettingsDisplayDialog::searchInSection(const QRegularExpression &regex, QWidget *contentWidget)
{
	auto someFound = false;

	auto layout = safe_cast<QFormLayout*>(contentWidget->layout());
	for(int i = 0, max = layout->rowCount(); i < max; ++i) {
		auto spanItem = layout->itemAt(i, QFormLayout::SpanningRole);
		if(spanItem) {
			auto subGroup = dynamic_cast<QSettingsGroupWidgetBase*>(spanItem->widget());
			someFound |= subGroup->searchExpression(regex, this->dialogEngine->searchStyleSheet());
		} else {
			auto label = layout->itemAt(i, QFormLayout::LabelRole)->widget();
			auto content = dynamic_cast<QSettingsWidgetBase*>(layout->itemAt(i, QFormLayout::FieldRole)->widget());
			someFound |= this->searchInEntry(regex, label, content);
		}
	}

	return someFound;
}

bool SettingsDisplayDialog::searchInEntry(const QRegularExpression &regex, QWidget *label, QSettingsWidgetBase *content)
{
	if(!regex.pattern().isEmpty() &&
	   (regex.match(label->property("text").toString()).hasMatch() ||
		(content && content->searchExpression(regex)))) {
		label->setStyleSheet(this->dialogEngine->searchStyleSheet());
		return true;
	} else {
		label->setStyleSheet(QString());
		return false;
	}
}



CategoryItemDelegate::CategoryItemDelegate(std::function<void (int)> updateFunc, const QSize &iconSize, int layoutSpacing, QObject *parent) :
	QStyledItemDelegate(parent),
	iconSize(),
	updateFunc(updateFunc)
{
	this->iconSize = iconSize + QSize(0, layoutSpacing);
}

QSize CategoryItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QSize size = this->QStyledItemDelegate::sizeHint(option, index);
	this->updateFunc(size.width());
	return size.expandedTo(this->iconSize);
}

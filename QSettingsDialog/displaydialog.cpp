#include "displaydialog.h"
#include "ui_displaydialog.h"
#include <QStyledItemDelegate>
#include <dialogmaster.h>
#include <functional>

#include <QDebug>
#include <QTimer>

class CategoryItemDelegate : public QStyledItemDelegate
{
public:
	CategoryItemDelegate(std::function<void(int)> updateFunc, QObject *parent = Q_NULLPTR);

	void setIconSize(const QSize &size);

	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE;
private:
	QSize extendSize;
	std::function<void(int)> updateFunc;
};

DisplayDialog::DisplayDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::DisplayDialog),
	delegate(new CategoryItemDelegate(std::bind(&DisplayDialog::updateWidth, this, std::placeholders::_1), this))
{
	ui->setupUi(this);
	DialogMaster::masterDialog(this);
	this->delegate->setIconSize(this->ui->categoryListWidget->iconSize());
	this->ui->categoryListWidget->setItemDelegate(this->delegate);

	int spacing = this->style()->pixelMetric(QStyle::PM_LayoutHorizontalSpacing);
	this->ui->contentLayout->setSpacing(spacing);
	this->ui->categoryLineSpacer->changeSize(spacing,
											 0,
											 QSizePolicy::Fixed,
											 QSizePolicy::Fixed);

	connect(this->ui->categoryListWidget, &QListWidget::iconSizeChanged,
			this->delegate, &CategoryItemDelegate::setIconSize);

	this->resetListSize();
}

DisplayDialog::~DisplayDialog()
{
	delete ui;
}

void DisplayDialog::insertItem(int index, QListWidgetItem *item, QTabWidget *content)
{
	this->ui->categoryListWidget->insertItem(index, item);
	this->ui->contentStackWidget->insertWidget(index, content);
	this->resetListSize();
}

void DisplayDialog::deleteItem(int index)
{
	delete this->ui->categoryListWidget->item(index);
	QWidget *w = this->ui->contentStackWidget->widget(index);
	this->ui->contentStackWidget->removeWidget(w);
	w->deleteLater();
	this->resetListSize();
}

void DisplayDialog::moveItem(int from, int to)
{
	QListWidgetItem *item = this->ui->categoryListWidget->takeItem(from);
	this->ui->categoryListWidget->insertItem(to, item);
	QWidget *w = this->ui->contentStackWidget->widget(from);
	this->ui->contentStackWidget->removeWidget(w);
	this->ui->contentStackWidget->insertWidget(to, w);
}

QSize DisplayDialog::iconSize() const
{
	return this->ui->categoryListWidget->iconSize();
}

void DisplayDialog::updateIconSize(const QSize &size)
{
	this->ui->categoryListWidget->setIconSize(size);
}

void DisplayDialog::resetListSize()
{
	int max = this->ui->categoryListWidget->count();
	if(max == 0)
		this->ui->categoryContentWidget->hide();
	else {
		this->maxWidthBase = this->ui->categoryListWidget->sizeHint().width();
		this->updateWidth(this->maxWidthBase);
		this->ui->categoryContentWidget->show();
	}
}

void DisplayDialog::updateWidth(int width)
{
	if(width > this->maxWidthBase) {
		this->maxWidthBase = width;
		QStyle *style = this->ui->categoryListWidget->style();
		width += style->pixelMetric(QStyle::PM_ScrollBarExtent);
		width += style->pixelMetric(QStyle::PM_LayoutHorizontalSpacing);
		this->ui->categoryListWidget->setFixedWidth(width);
	}
}

void DisplayDialog::on_buttonBox_clicked(QAbstractButton *button)
{
	switch(this->ui->buttonBox->standardButton(button)) {
	case QDialogButtonBox::Ok:
		this->accept();
		break;
	case QDialogButtonBox::Cancel:
		this->reject();
		break;
	case QDialogButtonBox::Apply:
		break;
	case QDialogButtonBox::RestoreDefaults:
		if(DialogMaster::warning(this,
								 tr("All custom changes will be delete and the defaults restored. "
									"This cannot be undone."),
								 tr("%1?").arg(button->text()),
								 QString(),
								 QMessageBox::Yes,
								 QMessageBox::No)
		   == QMessageBox::Yes) {
			this->done(2);
		}
		break;
	default:
		Q_UNREACHABLE();
	}
}



CategoryItemDelegate::CategoryItemDelegate(std::function<void (int)> updateFunc, QObject *parent) :
	QStyledItemDelegate(parent),
	extendSize(),
	updateFunc(updateFunc)
{}

void CategoryItemDelegate::setIconSize(const QSize &size)
{
	this->extendSize = size + QSize(2, 2);
}

QSize CategoryItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QSize size = this->QStyledItemDelegate::sizeHint(option, index);
	this->updateFunc(size.width());
	return size.expandedTo(this->extendSize);
}

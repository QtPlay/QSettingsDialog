#ifndef SETTINGSGROUPBOX_H
#define SETTINGSGROUPBOX_H

#include "qsettingsgroupwidget.h"
#include <QGroupBox>
#include <QFormLayout>

class SettingsGroupBox : public QSettingsGroupWidget<QGroupBox>
{
public:
	SettingsGroupBox(QWidget *parent = nullptr);

	// QSettingsGroupWidgetBase interface
	void setName(const QString &name) override;
	void setTooltip(const QString &tooltip) override;
	void setOptional(bool optional) override;
	bool isChecked() const override;
	void setChecked(bool checked) override;

protected:
	void addWidgetWithLabel(QWidget *label, QWidget *content) override;
    void takeWidgetWithLabel(QWidget *label, QWidget *content) override;

private:
	QFormLayout *layout;
};

#endif // SETTINGSGROUPBOX_H

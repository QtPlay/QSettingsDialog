#ifndef SETTINGSSLIDER_H
#define SETTINGSSLIDER_H

#include "qsettingswidget.h"
#include <QSlider>

class SettingsSlider : public QSettingsWidget<QSlider>
{
	Q_OBJECT

public:
	SettingsSlider(QWidget *parent = nullptr);

	// QSettingsWidgetBase interface
	void setValue(const QVariant &value) override;
	QVariant getValue() const override;
	void resetValue() override;

private slots:
	void showValue();
};

class SettingsSpinBox;
class SettingsSlider2 : public QSettingsWidget<QWidget> {
    Q_OBJECT
public:
    SettingsSlider2(QWidget *parent = nullptr);

    // QSettingsWidgetBase interface
    void setValue(const QVariant &value) override;
    QVariant getValue() const override;
    void resetValue() override;

    void initialize(const UiPropertyMap &uiPropertyMap) override ;

public slots:

    void sltValueChanged(int v);

private:
    SettingsSlider * slider = nullptr;
    SettingsSpinBox * label = nullptr;
};

#endif // SETTINGSSLIDER_H

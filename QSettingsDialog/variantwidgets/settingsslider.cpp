#include "settingsslider.h"
#include <QToolTip>
#include <QStyle>
#include <QLocale>
#include "settingsspinbox.h"

SettingsSlider::SettingsSlider(QWidget *parent) :
	QSettingsWidget(parent)
{
	this->setOrientation(Qt::Horizontal);

	connect(this, &SettingsSlider::sliderPressed,
			this, &SettingsSlider::showValue,
			Qt::QueuedConnection);
	connect(this, &SettingsSlider::sliderMoved,
			this, &SettingsSlider::showValue,
			Qt::QueuedConnection);
}

void SettingsSlider::setValue(const QVariant &value)
{
	this->QSlider::setValue(value.toInt());
}

QVariant SettingsSlider::getValue() const
{
	return this->value();
}

void SettingsSlider::resetValue()
{
	this->QSlider::setValue(0);
}

void SettingsSlider::showValue()
{
	QPoint point;
	if(this->orientation() == Qt::Horizontal) {
		auto pos = QStyle::sliderPositionFromValue(this->minimum(),
												   this->maximum(),
												   this->value(),
												   this->width() -
												   this->style()->pixelMetric(QStyle::PM_SliderLength));
		point = QPoint(pos,
					   this->style()->pixelMetric(QStyle::PM_SliderLength));
	} else {
		auto pos = QStyle::sliderPositionFromValue(this->minimum(),
												   this->maximum(),
												   this->value(),
												   this->height() -
												   this->style()->pixelMetric(QStyle::PM_SliderLength),
												   true);
		point = QPoint(this->style()->pixelMetric(QStyle::PM_SliderThickness),
					   pos - this->style()->pixelMetric(QStyle::PM_SliderLength));
	}
	point = this->mapToGlobal(point);
	QToolTip::showText(point, QLocale().toString(this->value()), this);
}

#include <QHBoxLayout>
SettingsSlider2::SettingsSlider2(QWidget *parent ) : QSettingsWidget<QWidget>(parent) {
    auto lyout = new QHBoxLayout(this);
    lyout->setMargin(0);
    lyout->setContentsMargins(0,0,0,0);

    slider = new SettingsSlider(this);
    label = new SettingsSpinBox(this);
    label->setMaximumWidth(100);
//    label->setSuffix("(Unit)");

    lyout->addWidget(slider);
    lyout->addWidget(label);

    connect(slider, SIGNAL(valueChanged(int)), this, SLOT(sltValueChanged(int)));
    connect(label, SIGNAL(valueChanged(int)), this, SLOT(sltValueChanged(int)));
}

void SettingsSlider2::sltValueChanged(int v) {
    slider->blockSignals(true);
    label->blockSignals(true);
    slider->setValue(v);
    label->setValue(v);
    slider->blockSignals(false);
    label->blockSignals(false);
}

void SettingsSlider2::initialize(const UiPropertyMap &uiPropertyMap)  {
    slider->initialize(uiPropertyMap);
    label->initialize(uiPropertyMap);
}

void SettingsSlider2::setValue(const QVariant &value) {
    slider->setValue(value);
}

QVariant SettingsSlider2::getValue() const {
    return slider->getValue();
}
void SettingsSlider2::resetValue() {
    slider->resetValue();
    label->resetValue();
}

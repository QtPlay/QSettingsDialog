#include <QApplication>
#include <QTranslator>
#include <QDebug>
#include <qsettingsdialog.h>
#include <QMessageBox>
#include "metawrapper.h"
#include "qsettingswidgetfactory.h"
#include "qsettingsextendedtypes.h"
#include "qsettingswidgetdialogengine.h"
#include "qsettingsdialogwidget.h"

class StateLoader : public QSimpleSettingsLoader
{
public:
	StateLoader(QVariant value = QVariant()) :
		value(value)
	{}

	bool load(QVariant &data, bool &userEdited) override {
		data = this->value;
		userEdited = true;
		return true;
	}
	bool save(const QVariant &data) override {
		qDebug() << data;
		this->value = data;
		return true;
	}
	bool reset() override {
		return true;
	}

private:
	QVariant value;
};

QSettingsDialog * ptrDlg = nullptr;



class TranslatorInjector : public QTranslator
{
public:
	QString translate(const char *context, const char *sourceText, const char *disambiguation, int n) const override {
		if(context == QStringLiteral("TestEnum") ||
		   context == QStringLiteral("TestFlags"))
			return QStringLiteral("tr.%1.%2").arg(context).arg(sourceText);
		else
			return QTranslator::translate(context, sourceText, disambiguation, n);
	}
};

#define ENTRY(metatype) new QSettingsEntry(metatype, new StateLoader(), QMetaType::typeName(metatype), false, #metatype)
#define ENTRY_VALUE(metatype, value) new QSettingsEntry(metatype, new StateLoader(value), QMetaType::typeName(metatype), false, #metatype)
#define ENTRY_PARAM(metatype, ...) new QSettingsEntry(metatype, new StateLoader(), QMetaType::typeName(metatype), false, #metatype, __VA_ARGS__)
#define ENTRY_VALUE_PARAM(metatype, value, ...) new QSettingsEntry(metatype, new StateLoader(value), QMetaType::typeName(metatype), false, #metatype, __VA_ARGS__)

class MessageBoxWidget : public QSettingsWidget<QMessageBox>
{
public:
    MessageBoxWidget(QWidget *parent = nullptr) :
        QSettingsWidget(parent)
    {
        this->setWindowTitle("Message-Box");
        this->setText("This is the simple message text");
        this->setDefaultButton(QMessageBox::Ok);
    }

    void setValue(const QVariant &) override {
        qDebug() << "hello ---- Dialog was canceled ----";
        static int i = 0;
        i++;
        i = i%2;
//        auto v = value.value<UploadOpt::Method>();
        switch(i) {
        case 0 :
            ptrDlg->appendEntry(ENTRY(QMetaType::QUuid));
            break;
        default:
            ptrDlg->appendEntry(ENTRY(QMetaType::QFont));
        }
    }
    QVariant getValue() const override {
        return QVariant();
    }
    void resetValue() override {}
};

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	TranslatorInjector tj;
	QApplication::installTranslator(&tj);

	REGISTER_FLAG_CONVERTERS(MetaWrapper::TestFlags);
    QSettingsWidgetDialogEngine::registerGlobalWidgetType<QSettingsDialogWidget<MessageBoxWidget>>(qMetaTypeId<MetaWrapper::TestEnum>());

	QSettingsDialog dialog;
    ptrDlg = & dialog;

	QObject::connect(&dialog, &QSettingsDialog::saved, [](bool close) {
		qDebug() << "---- Save completed" << (close ? "with" : "without") << "closing ----";
	});
	QObject::connect(&dialog, &QSettingsDialog::resetted, []() {
		qDebug() << "---- Reset completed ----";
	});
	QObject::connect(&dialog, &QSettingsDialog::canceled, []() {
		qDebug() << "---- Dialog was canceled ----";
	});

	dialog.setSection("standardMetaTypes");
	dialog.appendEntry(ENTRY(QMetaType::Bool));
	dialog.appendEntry(ENTRY(QMetaType::Int));
	dialog.appendEntry(ENTRY(QMetaType::UInt));
	dialog.appendEntry(ENTRY(QMetaType::Double));
	dialog.appendEntry(ENTRY(QMetaType::QString));
	dialog.appendEntry(ENTRY_PARAM(QMetaType::QByteArray, "placeholderText", QStringLiteral("This is a dynamic property")));
	dialog.appendEntry(ENTRY(QMetaType::Long));
	dialog.appendEntry(ENTRY(QMetaType::LongLong));
	dialog.appendEntry(ENTRY(QMetaType::Short));
	dialog.appendEntry(ENTRY(QMetaType::Char));
	dialog.appendEntry(ENTRY(QMetaType::ULong));
	dialog.appendEntry(ENTRY(QMetaType::ULongLong));
	dialog.appendEntry(ENTRY(QMetaType::UShort));
	dialog.appendEntry(ENTRY(QMetaType::SChar));
	dialog.appendEntry(ENTRY(QMetaType::UChar));
	dialog.appendEntry(ENTRY(QMetaType::Float));
	dialog.appendEntry(ENTRY(QMetaType::QDate));
	dialog.appendEntry(ENTRY(QMetaType::QTime));
	dialog.appendEntry(ENTRY(QMetaType::QColor));
	dialog.appendEntry(ENTRY(QMetaType::QStringList));
	dialog.appendEntry(ENTRY(QMetaType::QUrl));
	dialog.appendEntry(ENTRY(QMetaType::QDateTime));
	dialog.appendEntry(ENTRY(QMetaType::QFont));
	dialog.appendEntry(ENTRY(QMetaType::QKeySequence));
	dialog.appendEntry(ENTRY(QMetaType::QUuid));

	dialog.setSection("enumTypes");
	dialog.appendEntry(ENTRY_VALUE_PARAM(qMetaTypeId<MetaWrapper::TestEnum>(), MetaWrapper::Value3, "translated", false));
	dialog.appendEntry(ENTRY_VALUE_PARAM(qMetaTypeId<MetaWrapper::TestEnum>(), QVariant::fromValue(MetaWrapper::Value2), "showAsRadio", true));
	dialog.appendEntry(ENTRY_VALUE_PARAM(qMetaTypeId<MetaWrapper::TestFlags>(), MetaWrapper::Flag3, "translated", false));
	dialog.appendEntry(ENTRY_VALUE(qMetaTypeId<MetaWrapper::TestFlags>(), QVariant::fromValue<MetaWrapper::TestFlags>(MetaWrapper::Flag8)));

	dialog.setSection("extendedTypes");
	dialog.appendEntry(ENTRY_VALUE(qMetaTypeId<FilePath>(), FilePath("C:/baum.txt")));
	dialog.appendEntry(ENTRY_VALUE(qMetaTypeId<IntRange>(), IntRange(42)));
	dialog.appendEntry(ENTRY_VALUE(qMetaTypeId<HtmlText>(), HtmlText("<u>Baum</u> <b>==</b> <i>42</i>")));
	dialog.appendEntry(ENTRY_VALUE_PARAM(QMetaType::QIcon, ":/QSettingsDialog/icons/delete.ico", "asQIcon", false));
	dialog.appendEntry(ENTRY_VALUE_PARAM(QMetaType::QIcon, QIcon(":/QSettingsDialog/icons/add.ico"), "asQIcon", true));

	dialog.setSection("dialogTest");
    dialog.appendEntry(new QSettingsEntry(qMetaTypeId<MetaWrapper::TestEnum>(), new StateLoader(), "Message-Test"));

	dialog.openSettings();
	return a.exec();
}

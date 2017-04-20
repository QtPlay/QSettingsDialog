#include <QApplication>
#include <QDebug>
#include <QStyle>
#include <QThread>
#include <QtConcurrent>
#include <qsettingsdialog.h>
#include <qsettingscontainer.h>
#include <qasyncsettingscontainer.h>
#include <qsettingslayout.h>
#include <qsettingswidgetdialogengine.h>
#include "testentry.h"
#include "delayedtestentry.h"
#include "swidget.h"

QSettingsLoaderEntry * createItem(const QString & n) {
    class TestEntry : public QSettingsLoaderEntry
    {
    public:
        TestEntry(bool optional, bool working,
                  QVariant data = QVariant()) :
            QSettingsLoaderEntry(working ? 0 : -1),
            origData(data),
            data(data)
        {
            this->setEntryName("name");
            this->setOptional(optional);
            this->setTooltip("Baum == 42");
        }

        bool load(QVariant & data,
                  bool & userEdited) {
            userEdited = !this->data.isNull();
            data = this->data;
            return true;
        }

        bool save(const QVariant &data){
            qDebug() << "SAVING from" << this->data << "to" << data;
            this->data = data;
            return true;
        }

        bool reset() {
            qDebug() << "RESETTING to" << this->origData;
            this->data = this->origData;
            return true;
        }

    private:
        QVariant origData;
        QVariant data;
    };

    return new TestEntry(false, false);
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QSettingsWidgetDialogEngine::registerGlobalFactory(0, new SFactory(false));
    QSettingsWidgetDialogEngine::registerGlobalFactory(1, new SFactory(true));

    QSettingsDialog dialog;
    QThread thread;
    thread.start();

    /*\
|*| -> instead of global registering
|*| auto engine = (QSettingsWidgetDialogEngine*) dialog.displayEngine();
|*| engine->addFactory(0, new SFactory(false));
|*| engine->addFactory(1, new SFactory(true));
\*/

        QObject::connect(&dialog, &QSettingsDialog::saved, [](bool close) {
            qDebug() << "---- Save completed" << (close ? "with" : "without") << "closing ----";
        });
        QObject::connect(&dialog, &QSettingsDialog::resetted, []() {
            qDebug() << "---- Reset completed ----";
        });
        QObject::connect(&dialog, &QSettingsDialog::canceled, []() {
            qDebug() << "---- Dialog was canceled ----";
        });

    //container demo

    auto cb = [&thread](QSettingsDialog & dialog) {
        dialog.setCategory("baum", "Baum == 42");
        dialog.setSection(".");
        dialog.setSection("__", "Is it true?", QIcon(), "some tooltop");

        dialog.appendEntry(new TestEntry(false, true, "moreText"));

        dialog.setGroup("42", 0, "Yes it is!", true, "Even here...");
        dialog.appendEntry(new TestEntry(false, false));
        dialog.appendEntry(new TestEntry(false, true));
        dialog.appendEntry(new TestEntry(true, false));
        dialog.appendEntry(new TestEntry(true, true));

        // general test on typical appeareances of normal items
        dialog.setContainer("..");
        dialog.setSection("generalTest", "generalTest");
        dialog.appendEntry(new TestEntry(false, false));
        dialog.appendEntry(new TestEntry(false, true));
        dialog.appendEntry(new TestEntry(true, false));
        dialog.appendEntry(new TestEntry(true, true));

        dialog.setGroup("normal");
        dialog.appendEntry(new TestEntry(false, true));
        dialog.appendEntry(new TestEntry(false, true, "withFixedData"));
        dialog.appendEntry(new TestEntry(true, true));
        dialog.appendEntry(new TestEntry(true, true, "withOptionalData"));

        //checking test, does auto-checking work?
        dialog.setSection("checkingTest");
        dialog.setGroup("optional1", 0, "optional - no data", true);
        dialog.appendEntry(new TestEntry(false, true));
        dialog.appendEntry(new TestEntry(true, true));

        dialog.setGroup("optional2", 0, "optional - fixed data", true);
        dialog.appendEntry(new TestEntry(false, true, "fixed group data"));
        dialog.appendEntry(new TestEntry(true, true));

        dialog.setGroup("optional3", 0, "optional - optional subdata", true);
        dialog.appendEntry(new TestEntry(false, true));
        dialog.appendEntry(new TestEntry(true, true, "optional group data"));

        //section and group entries - mixed appeareance
        dialog.setSection("mixedEntriesTest");
        dialog.unsetGroup();
        dialog.appendEntry(new TestEntry(false, true));
        dialog.appendEntry(new TestEntry(false, true));

        dialog.setGroup("someGroup");
        dialog.appendEntry(new TestEntry(true, true, "LOOOOL"));
        dialog.appendEntry(new TestEntry(false, false));
        dialog.unsetGroup();
        dialog.appendEntry(new TestEntry(false, true, QVariant()));
        dialog.appendEntry(new TestEntry(true, false, QVariant()));

        //async test
        dialog.setSection("asyncTest");
        dialog.unsetGroup();

        dialog.appendEntry(new DelayedTestEntry("test500", 500));
        dialog.appendEntry(new DelayedTestEntry("test1000", 1000));
        dialog.appendEntry(new DelayedTestEntry("test1500", 1500));

        auto threaded = new DelayedTestEntry("testThreaded", 42);
        threaded->moveToThread(&thread);
        dialog.appendEntry(threaded);

        //
        dialog.setContainer("my/.");
        auto my = dialog.currentContainer();
        my->appendEntry(new TestEntry(false, true));

        dialog.setContainer("you/./dd");
        auto you = dialog.currentSectionContainer();
        you->appendEntry(new TestEntry(false, true));


        //container test
        dialog.setContainer("containerTest/./normal");
        dialog.appendEntry(new TestEntry(false, true));

        QGroupSettingsContainer container(&dialog, "containerTest/./normal");
        container.appendEntry(new TestEntry(true, true));
        auto rem = dialog.appendEntry(new TestEntry(false, true));
        container.insertEntry(1, new TestEntry(false, false));

        QSettingsContainer *container2 = dialog.currentContainer(&dialog);
        container2->removeEntry(rem);
        dialog.prependEntry(new TestEntry(false, true));

        QGroupSettingsContainer container3(&dialog, "containerTest/./normal_trans");
        container2->transferElement(0, &container3, 0);

        QSettingsContainer *sectionContainer1 = dialog.currentSectionContainer(&dialog);
        sectionContainer1->appendEntry(new TestEntry(false, true));
        QSectionSettingsContainer sectionContainer2(&dialog, dialog.sectionContainerPath());
        sectionContainer2.moveElement(0, 2);

        //async container test -> only shows on second run
        QAsyncSettingsContainer asyncContainer(&dialog, "containerTest/./async");
        QtConcurrent::run([&]() {
            asyncContainer.appendEntryAsync(new TestEntry(true, false));

            QAsyncSettingsContainer async2(&dialog, "containerTest/./async");
            qDebug() << "async count:" << async2.elementCount();
            async2.prependEntry(new TestEntry(true, false));
            auto res = async2.appendEntry(new TestEntry(false, true));
            async2.removeEntry(res);
        });
        asyncContainer.appendEntryAsync(new TestEntry(false, false));
        dialog.appendEntry("containerTest/./async", new TestEntry(true, true));

        //layout tests - dialog
        QSettingsLayout dialogLayout(&dialog);
        dialogLayout.createElement(1, "layoutCategory");
        dialogLayout.moveElement(0, 2);
        dialogLayout.moveElement(1, 0);

        //layout tests - category
        QSettingsLayout categoryLayout = dialogLayout.elementAt(1);
        categoryLayout.createElement(0, "elem0");
        categoryLayout.createElement(1, "elem1");
        categoryLayout.createElement(2, "elem2");
        categoryLayout.createElement(3, "elem3");
        categoryLayout.removeElement(2);
        categoryLayout.setName("Layout Test");
        try {
            categoryLayout.setOptional(true);
            Q_ASSERT(false);
        } catch(LayoutPropertyNotDefinedException e) {
            qDebug() << "Expected exception:" << e.what();
        }
        Q_ASSERT(categoryLayout.defaultElement(false).isNull());
        Q_ASSERT(!categoryLayout.defaultElement(true).isNull());

        //layout tests - section
        QSettingsLayout sectionLayout = categoryLayout.elementAt(2);
        sectionLayout.setName("Look here!");
        sectionLayout.setIcon(QApplication::style()->standardIcon(QStyle::SP_ComputerIcon));
        sectionLayout.createGroupElement(0, "group2");
        sectionLayout.createGroupElement(0, "group1", 0, QString(), true);
        sectionLayout.createGroupElement(0, "group0");
        try {
            sectionLayout.createElement(0, "42z");
            Q_ASSERT(false);
        } catch(LayoutPropertyNotDefinedException e) {
            qDebug() << "Expected exception:" << e.what();
        }

        QSettingsLayout groupLayout = sectionLayout.elementAt(0);
        groupLayout.setOptional(true);
        try {
            sectionLayout.createElement(0, "42z");
            Q_ASSERT(false);
        } catch(LayoutPropertyNotDefinedException e) {
            qDebug() << "Expected exception:" << e.what();
        }

        //layout tests - group
        QSettingsLayout transferToCategory = dialogLayout.elementAt(2);
        categoryLayout.transferElement(0, transferToCategory, 1);

        //test container from layout (group)
        QGroupSettingsContainer layoutGroupContainer(groupLayout);
        layoutGroupContainer.appendEntry(new TestEntry(false, true));
        layoutGroupContainer.appendEntry(new TestEntry(false, true));
        layoutGroupContainer.appendEntry(new TestEntry(false, true));

        //layout tests - entry
        QSettingsLayout entryLayout = groupLayout.elementAt(1);
        entryLayout.setName("The unique entry");
        entryLayout.setOptional(true);
        entryLayout.setTooltip("What is the answer to all question and the universe?");
        try {
            sectionLayout.createElement(0, "42z");
            Q_ASSERT(false);
        } catch(LayoutPropertyNotDefinedException e) {
            qDebug() << "Expected exception:" << e.what();
        }

        //test container from layout
        QSectionSettingsContainer layoutContainer(sectionLayout);
        layoutContainer.appendEntry(new TestEntry(false, false));
        layoutContainer.appendEntry(new TestEntry(false, false));
        sectionLayout.moveElement(4, 1);

        //reshow 2nd dialog
        auto reshowId = dialog.appendEntry("..", new TestEntry(true, true));
        QSettingsLayout reshowLayout(&dialog, "..", reshowId);
        Q_ASSERT(reshowLayout.layoutType() == QSettingsLayout::EntryLayout);
        reshowLayout.setName("Reshow");
        reshowLayout.setTooltip("Will show the dialog a second time");

        dialog.setCategory("MyLamd", "name");
        dialog.setSection(".");
        dialog.setGroup("42", 0, "Yes it is!", true, "Even here...");
        dialog.appendEntry(createItem("A"));
        dialog.appendEntry(createItem("B"));
        dialog.appendEntry(createItem("C"));
    };

    QSettingsDialog::reg("id", cb);
    QSettingsDialog::setup(dialog);
    dialog.openSettings(nullptr);
    int res = a.exec();
    QSettingsDialog::unreg("id");

    //	forever {
    //		QSharedPointer<TestEntry> rEntry = dialog.getEntry(reshowId).dynamicCast<TestEntry>();
    //		bool d;
    //		QVariant v;
    //		rEntry->load(v, d);
    //		if(v.isValid())
    //			res = dialog.execSettings();
    //		else
    //			break;
    //	}

    thread.quit();
    thread.wait();
    return res;
}

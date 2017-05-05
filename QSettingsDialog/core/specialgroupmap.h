#ifndef SPECIALGROUPMAP_H
#define SPECIALGROUPMAP_H

#include <QHash>
#include <QList>
#include <QSharedPointer>
#include <QVariant>
#include "sortedmap.h"
struct SettingsGroup;
class QSettingsEntry;

class SpecialGroupMap
{
public:
	typedef QPair<QSharedPointer<SettingsGroup>, QSharedPointer<QSettingsEntry>> Value;
	typedef QPair<QVariant, Value> Entry;

	SpecialGroupMap();
	~SpecialGroupMap();

	//global operations
	int size() const;
	bool hasIndex(int index) const;
	QVariant id(int index) const;
	int index(const QVariant &id) const;
	QList<Entry> entries() const;
	Entry entry(int index) const;
	bool remove(int index);
	Entry take(int index);
	void move(int indexFrom, int indexTo);

	//group base operations
	QSharedPointer<SettingsGroup> append(const QString &id, SettingsGroup *entry);
	void append(const QString &id, const QSharedPointer<SettingsGroup> &entry);
	QSharedPointer<SettingsGroup> prepend(const QString &id, SettingsGroup *entry);
	void prepend(const QString &id, const QSharedPointer<SettingsGroup> &entry);
	QSharedPointer<SettingsGroup> insert(int index, const QString &id, SettingsGroup *entry);
	void insert(int index, const QString &id, const QSharedPointer<SettingsGroup> &entry);

	int index(const QString &id) const;
	bool contains(const QString &id) const;

	QList<QString> groupKeys() const;
	QList<QSharedPointer<SettingsGroup>> groupValues() const;
	SortedMap<QString, SettingsGroup> createGroupMap() const;

	QSharedPointer<SettingsGroup> valueId(const QString &id) const;
	bool removeId(const QString &id);
	QSharedPointer<SettingsGroup> takeId(const QString &id);

	//custom operations
	QSharedPointer<QSettingsEntry> append(const int &id, QSettingsEntry *entry);
	void append(const int &id, const QSharedPointer<QSettingsEntry> &entry);
	QSharedPointer<QSettingsEntry> prepend(const int &id, QSettingsEntry *entry);
	void prepend(const int &id, const QSharedPointer<QSettingsEntry> &entry);
	QSharedPointer<QSettingsEntry> insert(int index, const int &id, QSettingsEntry *entry);
	void insert(int index, const int &id, const QSharedPointer<QSettingsEntry> &entry);

	int index(const int &id) const;
	bool contains(const int &id) const;

	QList<int> customKeys() const;
	QList<QSharedPointer<QSettingsEntry>> customValues() const;
	SortedMap<int, QSettingsEntry> createCustomMap() const;

	QSharedPointer<QSettingsEntry> valueId(const int &id) const;
	bool removeId(const int &id);
	QSharedPointer<QSettingsEntry> takeId(const int &id);

	class const_iterator
	{
		friend class SpecialGroupMap;
	public:
        /*typename*/ SpecialGroupMap::Entry operator*();
		bool operator!=(const const_iterator& other) const;
		const_iterator &operator++();

	private:
		const_iterator(const SpecialGroupMap &map, int index);

		const SpecialGroupMap &map;
		int index;
	};

	const_iterator begin() const;
	const_iterator end() const;

private:
	QHash<QString, QSharedPointer<SettingsGroup>> groups;
	QHash<int, QSharedPointer<QSettingsEntry>> customGroups;
	QVariantList totalOrder;

	void cleanTotal(const QVariant &value);
};

#endif // SPECIALGROUPMAP_H

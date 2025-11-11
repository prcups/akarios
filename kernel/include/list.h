#ifndef LIST_H
#define LIST_H

template <typename LVal>
class ListItem {
public:
    LVal Val;
    ListItem(const LVal & val);
    ListItem <LVal> *Next = nullptr;
    static void Add(ListItem <LVal> **current, ListItem <LVal>* newItem);
};

template<typename LVal>
ListItem<LVal>::ListItem(const LVal& val):Val(val){}

template<typename LVal>
void ListItem<LVal>::Add(ListItem<LVal>** current, ListItem<LVal>* newItem)
{
    newItem->Next = *current;
    *current = newItem;
}

#endif

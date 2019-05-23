#include "orderedMap.h"
#include "constants.h"
#include <iostream>

/******************************************************************************
 * default constructor
 ******************************************************************************/
OrderedMap::OrderedMap() = default;

/******************************************************************************
 * insert()
 * example: insert(make_pair("key", "item"));
 ******************************************************************************/
void OrderedMap::insert(std::pair<std::string, std::string> element)
{
    int index = find(element.first);
    if (index == ERROR) // if the key hasn't been added yet
        map.push_back(element);
    else // else, update w/ the new value
        map[index].second = element.second;
}

/******************************************************************************
 * find()
 * - returns the index for the given key
 * - returns ERROR if key not found
 ******************************************************************************/
int OrderedMap::find(const std::string& key)
{
    int pos = ERROR;
    for (unsigned int i = 0; i < map.size(); i++)
        if (map[i].first == key)
        {
            pos = i;
            break;
        }
    return pos;
}

/******************************************************************************
 * operator[key]
 * - returns a reference for the value in the given key
 ******************************************************************************/
std::string& OrderedMap::operator[](const std::string& key)
{
    int index = find(key);
    if (index == ERROR)
    {
        insert(make_pair(key, "")); // if key not found, insert key w/ blank val
        index = size() - 1;
    }
    return (map[index].second);
}

/******************************************************************************
 * operator[index]
 * - returns the key-value pair found in the given index
 ******************************************************************************/
OrderedMap::item& OrderedMap::operator[](unsigned int index)
{
    if (index >= size())
        std::cout << "WARNING in orderedMap!! Index out of bounds!\n";
    return map[index]; // WARNING!!! there's no boundary checking!!!
}

/******************************************************************************
 * hasKey()
 * - returns true if the given key exists
 ******************************************************************************/
bool OrderedMap::hasKey(const std::string& key)
{
    return find(key) != ERROR;
}

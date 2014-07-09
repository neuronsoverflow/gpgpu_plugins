#ifndef ORDEREDMAP_H
#define ORDEREDMAP_H

#include <utility> // pair()
#include <string>
#include <vector>

/******************************************************************************
* this class is kind of like std::map, except that it keeps the key-value pairs
* in the order they are inserted. It also only allows strings to be inserted.
******************************************************************************/
class OrderedMap
{
   public:
      // this is the kind of vector that the class contains
      typedef std::pair<std::string, std::string> item;

      OrderedMap();
      void insert(std::pair<std::string, std::string>);
      std::string& operator[] (const std::string& key);
      std::pair<std::string, std::string>& operator[] (unsigned int index);

      unsigned int size() const { return map.size(); }
      bool hasKey(const std::string& key);
      int find(const std::string& key); // returns the index for the key
      void clear() { map.clear(); }


      // iterator support :D
      typedef std::vector<item>::iterator iterator;
      typedef std::vector<item>::const_iterator const_iterator;
      iterator begin()             { return map.begin(); }
      const_iterator begin() const { return map.begin(); }
      iterator end()               { return map.end(); }
      const_iterator end()   const { return map.end(); }

   private:
      std::vector<item> map;
};

#endif


//
// CS251 Data Structures
// Hash Table
//

#include <assert.h>
#include <stdlib.h>
#include <string.h>

// Each hash entry stores a key, object pair
template <typename Data>
struct HashTableTemplateEntry {
  const char * _key;
  Data _data;
  HashTableTemplateEntry * _next;
};

// This is a Hash table that maps string keys to objects of type Data
template <typename Data>
class HashTableTemplate {
 public:
  // Number of buckets
  enum { TableSize = 2039};

  // Array of the hash buckets.
  HashTableTemplateEntry<Data> **_buckets;

  // Obtain the hash code of a key
  int hash(const char * key);

 public:
  HashTableTemplate();
  ~HashTableTemplate();

  // Add a record to the hash table. Returns true if key already exists.
  // Substitute content if key already exists.
  bool insertItem( const char * key, Data data);

  // Find a key in the dictionary and place in "data" the corresponding record
  // Returns false if key is does not exist
  bool find( const char * key, Data * data);

  // Removes an element in the hash table. Return false if key does not exist.
  bool removeElement(const char * key);

  // Returns the data that corresponds to this index.
  // Data is initialized to 0s if the entry does not exist
  Data operator[] (const char * &key);
};

template <typename Data>
int HashTableTemplate<Data>::hash(const char * key)
{
  int h = 0;
  const char * p = key;
  while (*p) {
    h += *p;
    p++;
  }
  return h % TableSize;
}

template <typename Data>
HashTableTemplate<Data>::HashTableTemplate()
{
  _buckets = new HashTableTemplateEntry<Data>*[TableSize];
  for(int i = 0;i < TableSize; i++)
  {
    _buckets[i] = NULL;
  }
}

template <typename Data>
bool HashTableTemplate<Data>::insertItem( const char * key, Data data)
{
  int index = hash(key);
  HashTableTemplateEntry<Data> *n;
  n = _buckets[index];
  // Look for key in the table
  while(n != NULL)
  {
    if(strcmp(key, n -> _key) == 0) // If found, update data
    {
      n -> _data = data;
      return true;
    }
    n = n -> _next;
  }

  // If we did not find one, create new one
  n = new HashTableTemplateEntry<Data>();
  n -> _key = strdup(key);
  n -> _data = data;
  n -> _next = _buckets[index];
  _buckets[index] = n;


  return false;
}

template <typename Data>
bool HashTableTemplate<Data>::find( const char * key, Data * data)
{
  int index = hash(key);
  HashTableTemplateEntry<Data> *n = _buckets[index];
  // Look for key in the table
  while(n != NULL)
  {
    if(strcmp(key, n -> _key) == 0) // If found, update data
    {
      *data = n -> _data;
      return true;
    }
    n = n -> _next;
  }
  // If we did not find one, return false
  return false;
}

template <typename Data>
Data HashTableTemplate<Data>::operator[] (const char * &key) {
  Data d;
  int index = hash(key);
  HashTableTemplateEntry<Data> *n = _buckets[index];
  // Look for key in the table
  while(n != NULL)
  {
    if(strcmp(key, n -> _key) == 0) // If found, update data
    {
      d = n -> _data;
      return d;
    }
    n = n -> _next;
  }
  _buckets[index] -> _data = 0;
  return _buckets[index] -> _data;
}

template <typename Data>
bool HashTableTemplate<Data>::removeElement(const char * key)
{
  int k = hash(key);
  HashTableTemplateEntry<Data> *n = _buckets[k];
  HashTableTemplateEntry<Data> *prev = NULL;
  // Find entry to remove
  while(n != NULL)
  {
    if(strcmp(n -> _key, key) == 0)
    {
      // entry found
      break;
    }
    prev = n;
    n = n -> _next;
  }
  if(n == NULL)
  {
    // Did not find key
    return false;
  }
  // Two cases for prev
  if(prev == NULL)
  {
    // prev points to first element in the list
    _buckets[k] = n -> _next;
    free((void*)(n -> _key));
    delete n;
  }
  else
  {
    // n is an internal node in the list
    prev -> _next = n -> _next;
    // we need to free the key because we allocated it with strdup
    free((void*)(n -> _key));
    delete n;
  }
  return true;
}

template <typename Data>
class HashTableTemplateIterator {
  int _currentBucket;
  HashTableTemplateEntry<Data> *_currentEntry;
  HashTableTemplate<Data> * _hashTable;
 public:
  HashTableTemplateIterator(HashTableTemplate<Data> * hashTable);
  bool next(const char * & key, Data & data);
};

template <typename Data>
HashTableTemplateIterator<Data>::HashTableTemplateIterator(HashTableTemplate<Data> * hashTable)
{
  _hashTable = hashTable;
  _currentEntry = NULL;
  _currentBucket = 0;
}

template <typename Data>
bool HashTableTemplateIterator<Data>::next(const char * & key, Data & data)
{
  if(_currentEntry != NULL && _currentEntry -> _next != NULL)
  {
    key = _currentEntry -> _next -> _key;
    data = _currentEntry -> _next -> _data;
    _currentEntry = _currentEntry -> _next;
    return true;
  }

  int i = _currentBucket + 1;
  while(i < 2039 && _hashTable -> _buckets[i] == NULL)
  {
    i++;
  }

  if((i < 2039 && _hashTable -> _buckets[i] != NULL))
  {
    key =  _hashTable -> _buckets[i] -> _key;
    data =  _hashTable -> _buckets[i] -> _data;
    _currentBucket = i;
    _currentEntry =  _hashTable -> _buckets[i];
    return true;
  }
  return false;
}

// Destructor for hash table. Deletes hash table
template <typename Data>
HashTableTemplate<Data>::~HashTableTemplate()
{
  for(int i = 0; i < TableSize; i++)
  {
    if(_buckets[i] != NULL)
    {
      HashTableTemplateEntry<Data> *n = _buckets[i];
      HashTableTemplateEntry<Data> *next = NULL;
      while(n != NULL)
      {
        next = n -> _next;
        free((void*)(n -> _key));
        delete n;
        n = next;
      }
    }
  }
  delete [] _buckets;
}

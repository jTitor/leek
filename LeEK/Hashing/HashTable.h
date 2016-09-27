#pragma once
#include "Datatypes.h"
#include "Memory/STLAllocHook.h"
#include "DataStructures/STLContainers.h"
#include "Hashing/Hash.h"

namespace
{
	const float DEF_LOAD_FAC = 0.66f;
	const float MIN_LOAD_FAC = 0.00000000001f;
	const size_t MIN_TABLE_SZ = 11;
}

namespace LeEK
{
	template <typename Key, typename Val>
	class HashTable
	{
	private:
		typedef Pair<Key, Val> tableEntry;
		typedef Vector<tableEntry> rawTable;
	public:
		//local typedefs
		typedef typename rawTable::iterator iterator;
		typedef typename rawTable::allocator_type allocator_type;
		typedef typename rawTable::const_iterator const_iterator;
		typedef typename rawTable::value_type value_type;
		typedef typename rawTable::size_type size_type;
	private:
		const Key INVALID_KEY;
		const Val INVALID_VAL;
		rawTable dataTable;
		size_t numUsed;
		size_t currSize;
		size_t numColls;
		//size_t lastAccessed;
		float maxLoadFactor;
		inline U32 getHash(const Key& key, U32 tableSz) const
		{
			return Hash(key) % tableSz;
		}
		inline U32 getHash(const Key& key) const
		{
			return Hash(key) % dataTable.size();
		}
		size_t getFixedSize(size_t sz) const
		{
			sz = Math::Max(sz, MIN_TABLE_SZ);
			if(sz % 2 == 0)
			{
				sz += 1;
			}
			return sz;
		}
		size_t getNextSize()
		{
			return (2*currSize) + 1;
		}
		size_t getPrevSize()
		{
			return (currSize - 1) / 2;
		}
		bool isValid(const tableEntry& entry) const
		{
			return entry.first != INVALID_KEY;// && entry.second != INVALID_VAL;
		}
		size_t getProbe(size_t startIdx, size_t probeLvl, size_t tableSz) const
		{
			return (startIdx + (probeLvl*probeLvl)) % tableSz;
		}
		bool tryInsert(rawTable& table, size_t& numElems, iterator it)
		{
			//if the table's full, probing won't work;
			//give up
			if(numElems == table.size())
			{
				return false;
			}
			tableEntry copy = tableEntry(it->first, it->second);
			size_t startIdx = getHash(it->first, table.size());
			size_t probeLvl = 0;
			size_t numSearched = 0;
			size_t idx = getProbe(startIdx, probeLvl, table.size());
			while(numSearched < table.size())
			{
				//is this an empty spot?
				if(table[idx].second == INVALID_VAL)
				{
					table[idx] = copy;
					++numElems;
					return true;
				}
				//do the keys match?
				else if(table[idx].first == it->first)
				{
					table[idx] = copy;
					++numElems;
					return true;
				}
				//otherwise we have to continue
				++probeLvl;
				++numSearched;
				idx = getProbe(startIdx, probeLvl, table.size());
			}
			//if we reached this point, we're screwed; give up
			return false;
			/*
			//is this an empty spot?
			if(table[idx].second == INVALID_VAL)
			{
				table[idx] = copy;
				++numElems;
				return true;
			}
			//do the keys match?
			else if(table[idx].first == it->first)
			{
				table[idx] = copy;
				++numElems;
				return true;
			}
			//otherwise, we have to probe
			else
			{
				size_t startIdx = idx;
				size_t probeLvl = 1;
				idx = getProbe(startIdx, probeLvl, table.size());
				while(idx != startIdx)
				{
					//is this an empty spot?
					if(table[idx].second == INVALID_VAL)
					{
						table[idx] = copy;
						++numElems;
						return true;
					}
					//do the keys match?
					else if(table[idx].first == it->first)
					{
						table[idx] = copy;
						++numElems;
						return true;
					}
					//otherwise we have to continue
					++probeLvl;
					idx = getProbe(startIdx, probeLvl, table.size());
				}
				//if we reached this point, we're screwed; give up
				return false;
			}
			*/
		}
		void copyValues(rawTable& srcTb, rawTable& destTb, size_t& numCopied)
		{
			for(iterator it = srcTb.begin(); it != srcTb.end(); ++it)
			{
				if(isValid(*it))
				{
					bool inserted = tryInsert(destTb, numCopied, it);
					L_ASSERT(inserted && "Failed to copy hashtable!");
				}
			}
		}
		void moveToTable(size_t newTableSz)
		{
			rawTable newTable = rawTable();
			newTableSz = Math::Max(newTableSz, MIN_TABLE_SZ);
			newTable.resize(newTableSz, tableEntry(INVALID_KEY, INVALID_VAL));
			//do reinsert
			size_t numCopied = 0;
			copyValues(dataTable, newTable, numCopied);
			//and set new table as our table
			dataTable = newTable;
			numUsed = numCopied;
			currSize = newTableSz;
		}
		void grow()
		{
			moveToTable(getNextSize());
		}
		/**
		Copies the table to a smaller table;
		note that it does not check that the new table
		can fit all of the old table's values.
		*/
		void shrink()
		{
			moveToTable(getPrevSize());
		}
		iterator findFirstAvailable( const Key& key )
		{
			size_t startIdx = getHash(key);
			size_t probeLvl = 0;
			size_t idx = getProbe(startIdx, probeLvl, dataTable.size());
			size_t numSearched = 0;
			while(numSearched < dataTable.size())
			{
				//is this an empty spot?
				if(dataTable[idx].second == INVALID_VAL)
				{
					return begin() + idx;
				}
				//do the keys match?
				else if(dataTable[idx].first == key)
				{
					return begin() + idx;
				}
				//otherwise we have to continue
				++numColls;
				++numSearched;
				++probeLvl;
				idx = getProbe(startIdx, probeLvl, dataTable.size());
			}
			return end();
		}
		/**
		Returns true if the table was rehashed, false otherwise.
		*/
		bool tryGrowTable()
		{
			if(load_factor() > max_load_factor())
			{
				grow();
				return true;
			}
			return false;
		}
		bool tryShrinkTable()
		{
			if(load_factor() < 0.3f*max_load_factor() && dataTable.size() > MIN_TABLE_SZ)
			{
				shrink();
				return true;
			}
			return false;
		}
		friend void tableSwap(HashTable& a, HashTable& b)
		{
			//screw it, we're using STL in the data structure implementation anyway
			using std::swap;

			swap(a.dataTable, b.dataTable);
			swap(a.numUsed, b.numUsed);
			swap(a.numColls, b.numColls);
			swap(a.maxLoadFactor, b.maxLoadFactor);
			swap(a.currSize, b.currSize);
		}
		/**
		Tries finding index to element with key.
		Returns index or maximum size of size_t (as -1)
		if the key couldn't be found.
		*/
		size_t doFind(const Key& key, const rawTable& table) const
		{
			size_t startIdx = getHash(key);
			size_t probeLvl = 0;
			size_t idx = getProbe(startIdx, probeLvl, table.size());
			size_t numSearched = 0;
			while(numSearched < table.size())
			{
				//do the keys match?
				if(table[idx].first == key)
				{
					return idx;
				}
				//otherwise we have to continue
				++numSearched;
				++probeLvl;
				idx = getProbe(startIdx, probeLvl, table.size());
			}
			return (size_t)-1;
		}
		size_t doFind(const Key& key) const
		{
			return doFind(key, dataTable);
		}
	public:
		HashTable(Key pInvalidKey = Key(), Val pInvalidVal = Val(), size_t minStartSz = MIN_TABLE_SZ, float pMaxLoadFac = DEF_LOAD_FAC) :
			INVALID_KEY(pInvalidKey), INVALID_VAL(pInvalidVal)
		{
			dataTable = rawTable();
			numUsed = 0;
			numColls = 0;
			if(pMaxLoadFac < MIN_LOAD_FAC || pMaxLoadFac > 1)
			{
				pMaxLoadFac = DEF_LOAD_FAC;
			}
			maxLoadFactor = pMaxLoadFac;
			minStartSz = getFixedSize(minStartSz);
			dataTable.resize(minStartSz, tableEntry(INVALID_KEY, INVALID_VAL));
			currSize = minStartSz;
		}
		HashTable(const HashTable& other) :
			INVALID_KEY(other.INVALID_KEY), INVALID_VAL(other.INVALID_VAL)
		{
			dataTable(other.dataTable);
			numUsed = other.numUsed;
			numColls = other.numColls;
			maxLoadFactor = other.maxLoadFactor;
			currSize = other.currSize;
		}
		size_t num_collisions() const
		{
			return numColls;
		}
		#pragma region Iterators
		iterator begin()
		{
			return dataTable.begin();
		}
		iterator end()
		{
			return dataTable.end();
		}
		iterator rbegin()
		{
			return dataTable.rbegin();
		}
		iterator rend()
		{
			return dataTable.rend();
		}
		#pragma region Const Iterators
		const_iterator begin()
		const {
			return dataTable.begin();
		}
		const_iterator end()
		const {
			return dataTable.end();
		}
		const_iterator rbegin()
		const {
			return dataTable.rbegin();
		}
		const_iterator rend()
		const {
			return dataTable.rend();
		}
		#pragma endregion
		#pragma endregion
		#pragma region Capacity
		bool empty()
		const
		{
			return size == 0;
		}
		std::size_t size()
		const 
		{
			return numUsed;//dataTable.size();
		}
		std::size_t capacity()
		const
		{
			return dataTable.size();//.capacity();
		}
		std::size_t max_size()
		const
		{
			return dataTable.max_size();
		}
		float load_factor()
		const
		{
			return (float)size() / (float)capacity();
		}
		float max_load_factor() const
		{
			return maxLoadFactor;
		}
		void max_load_factor(float val) const
		{
			val = Math::Clamp(val, MIN_LOAD_FAC, 1.0f);
			maxLoadFactor = val;
		}
		#pragma endregion
		#pragma region Lookup
		//Yes, these are supposed to take keys.
		//Respond accordingly.
		bool contains(const Key& key) const
		{
			return doFind(key) != (size_t)-1;
		}
		size_type count( const Key& key ) const
		{
			if(contains(key))
			{
				return 1;
			}
			return 0;
		}
		iterator find( const Key& key )
		{
			size_t resIdx = doFind(key);
			return resIdx != (size_t)-1 ? begin() + resIdx : end();
		}
		const_iterator find( const Key& key ) const
		{
			size_t resIdx = doFind(key);
			return resIdx != (size_t)-1 ? cbegin() + resIdx : cend();
		}
		#pragma endregion
		#pragma region Element Accessors
		/*
		#pragma region at
		Val& at( const Key& key )
		{
			auto result = dataTable.find(getHash(key));
			if()
			{
				++numUsed;
				return result;
			}
			return result;
		}
		const Val& at( const Key& key )
		const {
			const Val& result = dataTable.at(getHash(key));
			if(!contains(key))
			{
				return result;
			}
			++numUsed;
			return result;
		}
		#pragma endregion
		*/
		#pragma region []
		Val& operator[]( const Key& key )
		{
			auto result = findFirstAvailable(key);
			
			if(result == end())
			{
				L_ASSERT(false && "Failed to find entry in hash table!");
				//If this was at(), this is where we'd throw an exception.
				//Instead, just crash.
				return result->second;
			}
			//if the key didn't previously exist, notify table.
			if(!isValid(*result))
			{
				result->first = key;
				++numUsed;
				if(tryGrowTable())
				{
					return operator[](key);
				}
				return result->second;
			}
			return result->second;
		}
		Val& operator[]( const Key&& key )
		{
			auto result = findFirstAvailable(key);
			if(result == end())
			{
				L_ASSERT(false && "Failed to find entry in hash table!");
				//If this was at(), this is where we'd throw an exception.
				//Instead, just crash.
				return result->second;
			}
			//if the key didn't previously exist, notify table.
			if(!isValid(*result))
			{
				result->first = key;
				++numUsed;
				if(tryGrowTable())
				{
					return operator[](key);
				}
				return result->second;
			}
			return result->second;
		}
		#pragma endregion
		#pragma endregion
		#pragma region Modifiers
		void clear()
		{
			//dataTable.clear();
			for(auto it = dataTable.begin(); it != dataTable.end(); ++it)
			{
				it->first = INVALID_KEY;
				it->second = INVALID_VAL;
			}
			numUsed = 0;
		}
		//Insert just doesn't make much sense...
		/*
		#pragma region Insert Overloads
		std::pair<iterator, bool> insert( const value_type& value )
		{
			return dataTable.insert(value);
		}
		template <class P> 
		std::pair<iterator, bool> insert( P&& value )
		{
			return dataTable.insert(value);
		}
		iterator insert( iterator hint, const value_type& value )
		{
			return dataTable.insert(hint, value);
		}
		iterator insert( const_iterator hint, const value_type& value )
		{
			return dataTable.insert(hint, value);
		}
		template <class P> 
		iterator insert( const_iterator hint, P&& value )
		{
			return dataTable.insert(hint, value);
		}
		template< class InputIt >
		void insert( InputIt first, InputIt last )
		{
			dataTable.insert(first, last);
		}
		#pragma endregion
		*/
		#pragma region Erase Overloads
		//really not sure how to handle these.
		void erase( iterator position )
		{
			if(position == end())
			{
				return;
			}

			Key origKey = position->first;
			position->first = INVALID_KEY;
			position->second = INVALID_VAL;
			if(origKey != INVALID_KEY)
			{
				--numUsed;
				tryShrinkTable();
			}
		}
		/*
		iterator erase( const_iterator position )
		{
			return dataTable.erase(position);
		}
		*/
		void erase( iterator first, iterator last )
		{
			while(first != end() && first != last)
			{
				erase(first);
				++first;
			}
		}
		/*
		iterator erase( const_iterator first, const_iterator last )
		{
			return dataTable.erase(first, last);
		}
		*/
		size_type erase( const Key& key )
		{
			U32 hashed = getHash(key);
			//See if there's a valid entry for that key.
			iterator entry = find(key);
			if(entry == end())
			{
				return size();
			}
			//Invalidate this entry.
			entry->first = INVALID_KEY;
			entry->second = INVALID_VAL;
			--numUsed;
			tryShrinkTable();
			//update table size
			return size();
		}
		#pragma endregion
		void swap(HashTable& other)
		{
			dataTable.swap(other.dataTable);
		}
		#pragma endregion
		/*
		#pragma region Observers
		key_compare key_comp() const
		{
			return dataTable.key_comp();
		}
		value_compare value_comp() const
		{
			return dataTable.value_comp();
		}
		#pragma endregion
		*/
		HashTable& operator=(HashTable& other )
		{
			tableSwap(*this, other);
			return *this;
		}
		//HashTable& operator=( HashTable&& other );
		allocator_type get_allocator() const 
		{
			return dataTable.get_allocator();
		}
	};
}

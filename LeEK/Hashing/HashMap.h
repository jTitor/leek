#pragma once
#include "Datatypes.h"
#include "Memory/STLAllocHook.h"
#include "DataStructures/STLContainers.h"
#include "Hashing/Hash.h"
#include "Hashing/HashedString.h"

namespace LeEK
{
	/**
	Implementation of a hash table???
	For now, only hashes strings.
	*/
	template <typename T>
	class HashMap
	{
	private:
		typedef STLAllocHook<std::pair<const U32, T>> stlHook;
		typedef std::map<U32, T, std::less<U32>, stlHook> rawMap;
		//typedef std::map<U32, T, std::less<U32>, LeEK::STLAllocHook<std::pair<const U32, T>>> rawMap;
		//typedef std::map<U32, T> rawMap;
		rawMap mapData;
	public:
		HashMap(void)// : mapData()
		{
			//mapData = CustomNew<rawMap>(0, "dunno");
		}
		//~HashMap(void)
		//{
			//delete mapData;
		//}

		//local typedefs
		typedef typename rawMap::iterator iterator;
		typedef typename rawMap::allocator_type allocator_type;
		typedef typename rawMap::const_iterator const_iterator;
		typedef typename rawMap::value_type value_type;
		typedef typename rawMap::size_type size_type;
		typedef typename rawMap::key_compare key_compare;
		typedef typename rawMap::value_compare value_compare;

		#pragma region Element Accessors
		#pragma region at
		T& at( const String& key )
		{
			return mapData.at(Hash(key));
		}
		const T& at( const String& key )
		const {
			return mapData.at(Hash(key));
		}
		T& at( const U32& key )
		{
			return mapData.at(key);
		}
		const T& at( const U32& key )
		const {
			return mapData.at(key);
		}
		T& at( const HashedString& key )
		{
			return mapData.at(key.Value());
		}
		const T& at( const HashedString& key )
		const {
			return mapData.at(key.Value());
		}
		#pragma endregion
		#pragma region []
		T& operator[]( const U32& key )
		{
			return mapData.operator[](key);
		}
		T& operator[]( U32&& key )
		{
			return mapData.operator[](key);
		}
		T& operator[]( const String& key )
		{
			return mapData.operator[](Hash(key));
		}
		//TODO
		//not sure exactly how to form the hashed array
		//T& operator[]( String&& key )
		//{
		//	return mapData.operator[](key);
		//}
		T& operator[]( const HashedString& key )
		{
			return mapData.operator[](key.Value());//[key.Value()];
		}
		T& operator[]( HashedString&& key )
		{
			return mapData.operator[](key.Value());//[key.Value()];
		}
		#pragma endregion
		#pragma endregion
		#pragma region Iterators
		iterator begin()
		{
			return mapData.begin();
		}
		iterator end()
		{
			return mapData.end();
		}
		iterator rbegin()
		{
			return mapData.rbegin();
		}
		iterator rend()
		{
			return mapData.rend();
		}
		#pragma region Const Iterators
		const_iterator begin()
		const {
			return mapData.begin();
		}
		const_iterator end()
		const {
			return mapData.end();
		}
		const_iterator rbegin()
		const {
			return mapData.rbegin();
		}
		const_iterator rend()
		const {
			return mapData.rend();
		}
		#pragma endregion
		#pragma endregion
		#pragma region Capacity
		bool empty()
		const {
			return mapData.empty();
		}
		std::size_t size()
		const {
			return mapData.size();
		}
		std::size_t max_size()
		const {
			return mapData.max_size();
		}
		#pragma endregion
		#pragma region Modifiers
		void clear()
		{
			mapData.clear();
		}
		#pragma region Insert Overloads
		std::pair<iterator, bool> insert( const value_type& value )
		{
			return mapData.insert(value);
		}

		template <class P> 
		std::pair<iterator, bool> insert( P&& value )
		{
			return mapData.insert(value);
		}
		iterator insert( iterator hint, const value_type& value )
		{
			return mapData.insert(hint, value);
		}
		iterator insert( const_iterator hint, const value_type& value )
		{
			return mapData.insert(hint, value);
		}
		template <class P> 
		iterator insert( const_iterator hint, P&& value )
		{
			return mapData.insert(hint, value);
		}
		template< class InputIt >
		void insert( InputIt first, InputIt last )
		{
			mapData.insert(first, last);
		}
		//Doesn't seem to be a header for initializer_list in VC12?
		/*
		void insert( std::initializer_list<value_type> ilist )
		{
			mapData.insert(ilist);
		}*/
		#pragma endregion
		/*
		template< class... Args >
		std::pair<iterator,bool> emplace( Args&&... args )
		{
			return mapData.emplace(args);
		}
		template< class... Args >
		std::pair<iterator,bool> emplace_hint( const_iterator hint, Args&&... args )
		{
			return mapData.emplace_hint(hint, args);
		}
		*/
		#pragma region Erase Overloads
		void erase( iterator position )
		{
			mapData.erase(position);
		}
		iterator erase( const_iterator position )
		{
			return mapData.erase(position);
		}
		void erase( iterator first, iterator last )
		{
			mapData.erase(first, last);
		}
		iterator erase( const_iterator first, const_iterator last )
		{
			return mapData.erase(first, last);
		}
		//size_type erase( const key_type& key )
		//{
		//	return mapData.erase(key);
		//}
		size_type erase( const U32& key )
		{
			return mapData.erase(key);
		}
		size_type erase( const String& key )
		{
			return mapData.erase(Hash(key));
		}
		size_type erase( const HashedString& key )
		{
			return mapData.erase(key.Value());
		}
		#pragma endregion
		void swap(HashMap& other)
		{
			mapData.swap(other.mapData);
		}
		#pragma endregion
		#pragma region Lookup
		size_type count( const U32& key ) const 
		{
			return mapData.count(key);
		}
		size_type count( const String& key ) const
		{
			return count(Hash(key));
		}
		size_type count( const HashedString& key ) const 
		{
			return mapData.count(key.Value());
		}
		iterator find( const U32& key )
		{
			return mapData.find(key);
		}
		const_iterator find( const U32& key ) const
		{
			return mapData.find(key);
		}
		iterator find( const String& key )
		{
			return find(Hash(key));
		}
		const_iterator find( const String& key ) const
		{
			return find(Hash(key));
		}
		iterator find( const HashedString& key )
		{
			return mapData.find(key.Value());
		}
		const_iterator find( const HashedString& key ) const
		{
			return mapData.find(key.Value());
		}
		#pragma endregion
		#pragma region Observers
		key_compare key_comp() const
		{
			return mapData.key_comp();
		}
		value_compare value_comp() const
		{
			return mapData.value_comp();
		}
		#pragma endregion
		//HashMap& operator=( const HashMap& other )
		//{
		//	return new HashMap(other);
		//}
		//HashMap& operator=( HashMap&& other );
		allocator_type get_allocator() const 
		{
			return mapData.get_allocator();
		}
	};
}

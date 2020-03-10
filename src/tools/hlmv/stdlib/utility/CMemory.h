#ifndef CMEMORY_H
#define CMEMORY_H

#include <cstdlib>
#include <algorithm>
#include <cstdio>
#include <type_traits>

/**
* Lightweight memory management class
* Can store owned or non-owned memory
* If this owns the given memory, it assumes the memory was allocated as an array
*/
template<typename SIZE_TYPE>
class CMemory
{
public:
	typedef unsigned char DataType_t;
	typedef SIZE_TYPE size_type;

public:
	CMemory( const size_type uiSizeInBytes = 0 )
		: m_pMemory( nullptr )
		, m_uiSize( 0 )
		, m_fOwnsMemory( false )
	{
		Init( uiSizeInBytes );
	}

	template<typename T>
	CMemory( T* pMemory, const size_type uiSizeInBytes, bool fOwnsMemory = true )
		: m_pMemory( nullptr )
		, m_uiSize( 0 )
		, m_fOwnsMemory( false )
	{
		Init( pMemory, uiSizeInBytes, fOwnsMemory );
	}

	~CMemory()
	{
		Release();
	}

	bool HasMemory() const { return m_pMemory != nullptr; }

	const DataType_t* GetMemory() const { return m_pMemory; }

	DataType_t* GetMemory() { return m_pMemory; }

	size_type GetSize() const { return m_uiSize; }

	bool OwnsMemory() const { return m_fOwnsMemory; }

	template<typename T>
	CMemory& Init( T* pMemory, const size_type uiSizeInBytes, bool fOwnsMemory = true )
	{
		Release();

		if( pMemory )
		{
			m_pMemory = reinterpret_cast<DataType_t*>( const_cast<typename std::remove_const<T>::type*>( pMemory ) );
			m_uiSize = uiSizeInBytes;
			m_fOwnsMemory = fOwnsMemory;
		}

		return *this;
	}

	CMemory& Init( const size_type uiSizeInBytes )
	{
		Release();

		//0 means no memory allocation
		if( uiSizeInBytes > 0 )
		{
			//Make sure to allocate the size in bytes, not in number of objects of type
			m_pMemory = new DataType_t[ uiSizeInBytes / sizeof( DataType_t ) ];
		}

		m_uiSize = uiSizeInBytes;

		m_fOwnsMemory = true;

		return *this;
	}

	void Release()
	{
		if( m_fOwnsMemory )
		{
			delete[] m_pMemory;
			m_fOwnsMemory = false;
		}

		m_pMemory = nullptr;
		m_uiSize = 0;
		m_fOwnsMemory = false;
	}

	void Swap( CMemory& other )
	{
		if( this != &other )
		{
			std::swap( m_pMemory, other.m_pMemory );
			std::swap( m_uiSize, other.m_uiSize );
			std::swap( m_fOwnsMemory, other.m_fOwnsMemory );
		}
	}

	const DataType_t* Begin() const { return m_pMemory; }

	DataType_t* Begin() { return m_pMemory; }

	const DataType_t* End() const { return m_pMemory + m_uiSize; }

	DataType_t* End() { return m_pMemory + m_uiSize; }

	bool Resize( const size_type uiNewSizeInBytes )
	{
		if( !OwnsMemory() )
			return false;

		DataType_t* pNewMemory = new DataType_t[ uiNewSizeInBytes ];

		const size_type uiBytesToCopy = std::min( m_uiSize, uiNewSizeInBytes );

		//If we had no memory, we'll be copying 0 bytes; memset below takes care of the rest
		if( m_pMemory )
		{
			memcpy( pNewMemory, m_pMemory, uiBytesToCopy );
			delete[] m_pMemory;
		}

		if( uiBytesToCopy < uiNewSizeInBytes )
			memset( pNewMemory + uiBytesToCopy, 0, uiNewSizeInBytes - uiBytesToCopy );

		m_pMemory = pNewMemory;
		m_uiSize = uiNewSizeInBytes;

		return true;
	}

	bool Reserve( const size_type uiMinimumSizeInBytes )
	{
		if( uiMinimumSizeInBytes <= m_uiSize )
			return true;

		return Resize( uiMinimumSizeInBytes );
	}

private:
	DataType_t* m_pMemory;	//Memory that we own.
	size_type m_uiSize;		//Size, in bytes, of the memory that we own
	bool m_fOwnsMemory;		//Whether we own this memory

private:
	CMemory( const CMemory& ) = delete;
	CMemory& operator=( const CMemory& ) = delete;
};

#endif //CMEMORY_H
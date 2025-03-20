//////////////////////////////////////////////////////////////////////////////

#include <string>
#include <cstdint>
#include <cstring>
#include <vector>

//////////////////////////////////////////////////////////////////////////////

template <class T>
class FastQueue
{
public:
	FastQueue(const unsigned iLength)
	{
		if (iLength==0) return;
		this->m_data = new T[iLength];
		this->m_length = iLength;
	}

	~FastQueue()
	{
		if (this->m_data) {
			delete [] this->m_data;
		}
	}

	T * next()
	{
		return(&this->m_data[this->m_wr]);
	}

	bool next(std::vector<T*> & oEntryArray)
	{
		size_t size = oEntryArray.size();
		unsigned free = this->m_length - 1 - getNumAvail();
		if (free < size) {
			return false;
		}

		T * p;
		unsigned wr = this->m_wr;

		for (size_t u0=0; u0<size; u0++) {
			p = &this->m_data[wr];
			oEntryArray[u0] = p;
			wr  = (wr + 1) % this->m_length;
		}

		return true;
	}

	bool add(const uint32_t iCount)
	{
		unsigned wr = this->m_wr;
		unsigned rd = this->m_rd;
		for (size_t u0=0; u0<iCount; u0++) {
			wr  = (wr + 1) % this->m_length;
			if (wr == rd) {
				this->m_overflow += iCount;
				return(false);
			}
		}
		this->m_wr = wr;
		return(true);
	}

	bool add()
	{
		unsigned wr = this->m_wr;
		wr  = (wr + 1) % this->m_length;
		if (wr == this->m_rd) {
			this->m_overflow++;
			return(false);
		}
		this->m_wr = wr;
		return(true);
	}

	T * get()
	{
		if (this->m_wr == this->m_rd) {
			return(0);
		}
		return(&this->m_data[this->m_rd]);
	}

	bool get(std::vector<T*> & oEntryArray)
	{
		size_t size = oEntryArray.size();
		if (getNumAvail() < size) {
			return(false);
		}

		T * p;
		unsigned rd = this->m_rd;

		for (size_t u0=0; u0<size; u0++) {
			p = &this->m_data[rd];
			oEntryArray[u0] = p;
			rd  = (rd + 1) % this->m_length;
		}

		return(true);
	}

	bool release(const uint32_t iCount)
	{
		unsigned wr = this->m_wr;
		unsigned rd = this->m_rd;
		for (size_t u0=0; u0<iCount; u0++) {
			if (wr == rd) return(false);
			rd  = (rd + 1) % this->m_length;
		}
		this->m_rd = rd;
		return(true);
	}

	bool release()
	{
		if (this->m_wr == this->m_rd) {
			return(false);
		}
		unsigned rd = this->m_rd;
		rd  = (rd + 1) % this->m_length;
		this->m_rd = rd;
		return(true);
	}

	T * getAt(const unsigned iIndex)
	{
		if (iIndex == this->m_length) {
			return(0);
		}
		return(&this->m_data[iIndex]);
	}

	unsigned getNumAvail()
	{
		unsigned wr = this->m_wr;
		unsigned rd = this->m_rd;
		if (wr == rd) {
			return(0);
		}
		if (wr > rd) {
			return(wr -rd);
		}
		return this->m_length+wr-rd;
	}

	unsigned getOverflow() const
	{
		return this->m_overflow;
	}

	unsigned getLength() const
	{
		return this->m_length;
	}

private:
	T *		m_data = 0;
	unsigned	m_length = 0;
	unsigned	m_rd = 0;
	unsigned	m_wr = 0;
	unsigned	m_overflow = 0;
};

//////////////////////////////////////////////////////////////////////////////

 

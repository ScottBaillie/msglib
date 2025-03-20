#ifndef IPCMEM_H
#define IPCMEM_H 1

#include <string>
#include <stddef.h>

class IpcMem
{
public:
	IpcMem();

	IpcMem(	const size_t iSize,
		const std::string & iName,
		const bool iReadOnly,
		const bool iCreate,
		const bool iExclusive);

	~IpcMem();

	bool init(
		const size_t iSize,
		const std::string & iName,
		const bool iReadOnly,
		const bool iCreate,
		const bool iExclusive);

	bool init(
		const size_t iSize,
		const std::string & iName,
		bool & oCreated);

	bool close();

	bool unmap();

	bool unlink();

	void * getMemPtr();

private:
	std::string	m_name;
	bool		m_readOnly = false;
	size_t		m_size = 0;
	void *		m_memPtr = 0;
};

#endif

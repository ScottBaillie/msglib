////////////////////////////////////////////////////////////////

#ifndef MSGLIB_LINKEDLIST_H
#define MSGLIB_LINKEDLIST_H

////////////////////////////////////////////////////////////////

#include <vector>
#include <cstddef>

#include <stdint.h>

////////////////////////////////////////////////////////////////

namespace msglib {

////////////////////////////////////////////////////////////////

template<class T> class ListEntry
{
public:
	typedef ListEntry<T>* Ptr;

	const T &	getData() const {return(m_data);}
	T &		getData() {return(m_data);}

	Ptr		getPrev() const {return(m_prev);}
	Ptr		getNext() const {return(m_next);}

public:
//private: // TODO
	template<class T1> class List;
	friend class List<T>;

	Ptr	m_prev = 0;
	Ptr	m_next = 0;
	T	m_data;
};

////////////////////////////////////////////////////////////////

template<class T> class FindAllFunction
{
public:
	bool find(T & data) const {return true;}
};

////////////////////////////////////////////////////////////////

template<class T> class List
{
public:
	typedef ListEntry<T>* Ptr;
	typedef List<Ptr> ListPtr;
	typedef std::vector<Ptr> VecPtr;

	size_t		size() const {return m_size;}
	void		clear();

	Ptr		getHead() const {return m_head;}
	Ptr		getTail() const {return m_tail;}

	Ptr 		insertAfter(Ptr p);
	void 		insertAfter(Ptr p, const std::vector<Ptr> & plist);
	void 		insertAfter(Ptr p, const List<T> & plist);

	Ptr 		insertBefore(Ptr p);
	void 		insertBefore(Ptr p, const std::vector<Ptr> & plist);
	void 		insertBefore(Ptr p, const List<T> & plist);

	Ptr		remove(Ptr p);
	void		remove(const std::vector<Ptr> & plist) { for (auto p:plist) remove(p); }
	void		remove(const List<Ptr> & plist) { for(auto p=plist.getHead();p!=0;p=p->getNext()) remove(p->getData()); }

	// bool found = T1.find(const T& data);
	template<class T1> void findFwdAll(const T1 & findFunction, std::vector<Ptr> & plist);
	template<class T1> void findFwdFirst(const T1 & findFunction, std::vector<Ptr> & plist);
	template<class T1> void findRvsAll(const T1 & findFunction, std::vector<Ptr> & plist);
	template<class T1> void findRvsFirst(const T1 & findFunction, std::vector<Ptr> & plist);

	template<class T1> void findFwdAll(const T1 & findFunction,List<Ptr> & list);
	template<class T1> void findFwdFirst(const T1 & findFunction,List<Ptr> & list);
	template<class T1> void findRvsAll(const T1 & findFunction,List<Ptr> & list);
	template<class T1> void findRvsFirst(const T1 & findFunction,List<Ptr> & list);

	// T2.process(ListEntry<T>* p);
	template<class T1,class T2> void processFwd(const T1 & findFunction, const T2 & processFunction);
	template<class T1,class T2> void processRvs(const T1 & findFunction, const T2 & processFunction);

	void		copy(const std::vector<Ptr> & plist);
	void		copy(const List<T> & plist);

	List<T> & operator = (const List<T> & plist) {copy(plist);return(*this);}
	List<T> & operator = (const std::vector<Ptr> & plist) {copy(plist);return(*this);}

	List<T> & operator += (const List<T> & plist) {insertAfter(m_tail,plist);return(*this);}
	List<T> & operator += (const std::vector<Ptr> & plist) {insertAfter(m_tail,plist);return(*this);}

	bool operator == (const List<T> & plist) const;
	bool operator == (const std::vector<Ptr> & plist) const;
	bool operator != (const List<T> & plist) const {return(!((*this)==plist));}
	bool operator != (const std::vector<Ptr> & plist) const {return(!((*this)==plist));}

	operator bool () const {return(m_size!=0);}

private:
	Ptr	m_head = 0;
	Ptr	m_tail = 0;
	size_t	m_size = 0;
};

////////////////////////////////////////////////////////////////

template<typename T> void
List<T>::clear()
{
	ListEntry<T>* p = m_head;
	ListEntry<T>* pold = 0;

	while (p != 0) {
		pold = p;
		p = p->getNext();
		delete pold;
	}
	m_head = 0;
	m_tail = 0;
	m_size = 0;
}

////////////////////////////////////////////////////////////////

template<typename T> ListEntry<T>*
List<T>::insertAfter(ListEntry<T>* p)
{
	if (m_head==0) {
		if (p!=0) return 0;
		m_head = new ListEntry<T>;
		m_tail = m_head;
		m_size += 1;
		return m_head;
	}

	if (p==0) return 0;

	if (p==m_head) {
		if (p==m_tail) {
			auto ptr = new ListEntry<T>;
			ptr->m_prev = m_head;
			m_head->m_next = ptr;
			m_tail = ptr;
			m_size += 1;
			return ptr;
		} else {
			auto ptr = new ListEntry<T>;
			ptr->m_next = m_head->m_next;
			ptr->m_prev = m_head;
			m_head->m_next->m_prev = ptr;
			m_head->m_next = ptr;
			m_size += 1;
			return ptr;
		}
	}

	if (p==m_tail) {
		auto ptr = new ListEntry<T>;
		ptr->m_prev = m_tail;
		m_tail->m_next = ptr;
		m_tail = ptr;
		m_size += 1;
		return ptr;
	}

	auto ptr = new ListEntry<T>;
	ptr->m_next = p->m_next;
	ptr->m_prev = p;
	p->m_next->m_prev = ptr;
	p->m_next = ptr;
	m_size += 1;

	return ptr;
}

////////////////////////////////////////////////////////////////

template<typename T> ListEntry<T>*
List<T>::insertBefore(ListEntry<T>* p)
{
	if (m_head==0) {
		if (p!=0) return 0;
		m_head = new ListEntry<T>;
		m_tail = m_head;
		m_size += 1;
		return m_head;
	}

	if (p==0) return 0;

	if (p==m_head) {
		auto ptr = new ListEntry<T>;
		ptr->m_next = m_head;
		m_head->m_prev = ptr;
		m_head = ptr;
		m_size += 1;
		return ptr;
	}

	if (p==m_tail) {
		auto ptr = new ListEntry<T>;
		ptr->m_next = m_tail;
		ptr->m_prev = m_tail->m_prev;
		m_tail->m_prev->m_next = ptr;
		m_tail->m_prev = ptr;
		m_size += 1;
		return ptr;
	}

	auto ptr = new ListEntry<T>;
	ptr->m_next = p;
	ptr->m_prev = p->m_prev;
	p->m_prev->m_next = ptr;
	p->m_prev = ptr;
	m_size += 1;

	return ptr;
}

////////////////////////////////////////////////////////////////

template<typename T> void
List<T>::insertAfter(ListEntry<T>* p, const std::vector<Ptr> & plist)
{
	Ptr pnew = 0;
	for (auto i:plist) {
		pnew = insertAfter(p);
		if (pnew==0) return;
		pnew->getData() = i->getData();
		p = pnew;
	}
}

template<typename T> void
List<T>::insertAfter(ListEntry<T>* p, const List<T> & plist)
{
	Ptr pnew = 0;
	for (auto i=plist.getHead();i!=0;i=i->getNext()) {
		pnew = insertAfter(p);
		if (pnew==0) return;
		pnew->getData() = i->getData();
		p = pnew;
	}
}

template<typename T> void
List<T>::insertBefore(ListEntry<T>* p, const std::vector<Ptr> & plist)
{
	Ptr pnew = 0;
	for (auto i:plist) {
		pnew = insertBefore(p);
		if (pnew==0) return;
		pnew->getData() = i->getData();
		p = pnew;
	}
}

template<typename T> void
List<T>::insertBefore(ListEntry<T>* p, const List<T> & plist)
{
	Ptr pnew = 0;
	for (auto i=plist.getHead();i!=0;i=i->getNext()) {
		pnew = insertBefore(p);
		if (pnew==0) return;
		pnew->getData() = i->getData();
		p = pnew;
	}
}

////////////////////////////////////////////////////////////////

template<typename T> ListEntry<T>*
List<T>::remove(ListEntry<T>* p)
{
	if ((p==0)||(m_head==0)) return 0;

	ListEntry<T>* next = p->m_next;
	ListEntry<T>* prev = p->m_prev;

	if (p==m_head) {
		if (next==0) {
			m_head = 0;
			m_tail = 0;
		} else {
			m_head = next;
			m_head->m_prev = 0;
		}
		delete p;
		m_size -= 1;
		return m_head;
	}

	if (p==m_tail) {
		m_tail = prev;
		m_tail->m_next = 0;
		delete p;
		m_size -= 1;
		return m_tail;
	}

	prev->m_next = next;
	next->m_prev = prev;
	delete p;
	m_size -= 1;

	return next;
}

////////////////////////////////////////////////////////////////

//
// bool found = T1.find(const T& data);
//
// T2.process(ListEntry<T>* p);
//
template<class T,class T1,class T2> void
process(ListEntry<T>* p,const bool fwd,const uint32_t limit,const T1 & findFunction, const T2 & processFunction)
{
	if (p==0) return;

	uint32_t count = 0;

	while (p != 0) {

		if ((limit)&&(count > limit)) break;

		if (findFunction.find(p->getData())) {
			processFunction.process(p);
		}

		if (fwd) {
			p = p->getNext();
		} else {
			p = p->getPrev();
		}
		count++;
	}
}

////////////////////////////////////////////////////////////////

template<class T,class T1,class T2> void
processFwd(ListEntry<T>* p,const T1 & findFunction, const T2 & processFunction)
{
	process(p,true,0,findFunction,processFunction);
}


template<class T,class T1,class T2> void
processRvs(ListEntry<T>* p,const T1 & findFunction, const T2 & processFunction)
{
	process(p,false,0,findFunction,processFunction);
}

////////////////////////////////////////////////////////////////

//
// bool found = T1.find(const T& data);
//
template<class T,class T1> void
find(ListEntry<T>* p,const bool fwd,const bool all,const uint32_t limit,const T1 & findFunction,List<ListEntry<T>*> & list)
{
	if (p==0) return;

	uint32_t count = 0;

	while (p != 0) {

		if ((limit)&&(count > limit)) break;

		if (findFunction.find(p->getData())) {
			auto ptr = list.insertAfter(list.getTail());
			ptr->getData() = p;
			if (!all) break;
		}

		if (fwd) {
			p = p->getNext();
		} else {
			p = p->getPrev();
		}
		count++;
	}
}
	
////////////////////////////////////////////////////////////////

//
// bool found = T1.find(const T& data);
//
template<class T,class T1> void
find(ListEntry<T>* p,const bool fwd,const bool all,const uint32_t limit,const T1 & findFunction, std::vector<ListEntry<T>*> & plist)
{
	if (p==0) return;

	uint32_t count = 0;

	while (p != 0) {

		if ((limit)&&(count > limit)) break;

		if (findFunction.find(p->getData())) {
			plist.push_back(p);
			if (!all) break;
		}

		if (fwd) {
			p = p->getNext();
		} else {
			p = p->getPrev();
		}
		count++;
	}
}

////////////////////////////////////////////////////////////////

template<class T,class T1> void
findFwdAll(ListEntry<T>* p,const T1 & findFunction, std::vector<ListEntry<T>*> & plist)
{
	bool fwd = true;
	bool all = true;
	uint32_t limit = 0;
	find(p,fwd,all,limit,findFunction,plist);
}

template<class T,class T1> void
findFwdFirst(ListEntry<T>* p,const T1 & findFunction, std::vector<ListEntry<T>*> & plist)
{
	bool fwd = true;
	bool all = false;
	uint32_t limit = 0;
	find(p,fwd,all,limit,findFunction,plist);
}

template<class T,class T1> void
findRvsAll(ListEntry<T>* p,const T1 & findFunction, std::vector<ListEntry<T>*> & plist)
{
	bool fwd = false;
	bool all = true;
	uint32_t limit = 0;
	find(p,fwd,all,limit,findFunction,plist);
}

template<class T,class T1> void
findRvsFirst(ListEntry<T>* p,const T1 & findFunction, std::vector<ListEntry<T>*> & plist)
{
	bool fwd = false;
	bool all = false;
	uint32_t limit = 0;
	find(p,fwd,all,limit,findFunction,plist);
}

////////////////////////////////////////////////////////////////

template<class T> ListEntry<T>*
findHead(ListEntry<T>* p,const uint32_t limit=0)
{
	uint32_t count = 0;
	while (p != 0) {
		if ((limit)&&(count > limit)) return 0;
		if (p->getPrev()==0) return p;
		p = p->getPrev();
		count++;
	}
	return 0;
}

////////////////////////////////////////////////////////////////

template<class T> ListEntry<T>*
findTail(ListEntry<T>* p,const uint32_t limit=0)
{
	uint32_t count = 0;
	while (p != 0) {
		if ((limit)&&(count > limit)) return 0;
		if (p->getNext()==0) return p;
		p = p->getNext();
		count++;
	}
	return 0;
}

////////////////////////////////////////////////////////////////

template<class T> template<class T1> void
List<T>::findFwdAll(const T1 & findFunction, std::vector<ListEntry<T>*> & plist)
{
	return msglib::findFwdAll(m_head,findFunction,plist);
}

template<class T> template<class T1> void
List<T>::findFwdFirst(const T1 & findFunction, std::vector<ListEntry<T>*> & plist)
{
	return msglib::findFwdFirst(m_head,findFunction,plist);
}

template<class T> template<class T1> void
List<T>::findRvsAll(const T1 & findFunction, std::vector<ListEntry<T>*> & plist)
{
	return msglib::findRvsAll(m_tail,findFunction,plist);
}

template<class T> template<class T1> void
List<T>::findRvsFirst(const T1 & findFunction, std::vector<ListEntry<T>*> & plist)
{
	return msglib::findRvsFirst(m_tail,findFunction,plist);
}

////////////////////////////////////////////////////////////////

template<class T> template<class T1> void
List<T>::findFwdAll(const T1 & findFunction,List<ListEntry<T>*> & list)
{
	return msglib::find(m_head,true,true,0,findFunction,list);
}

template<class T> template<class T1> void
List<T>::findFwdFirst(const T1 & findFunction,List<ListEntry<T>*> & list)
{
	return msglib::find(m_head,true,false,0,findFunction,list);
}

template<class T> template<class T1> void
List<T>::findRvsAll(const T1 & findFunction,List<ListEntry<T>*> & list)
{
	return msglib::find(m_tail,false,true,0,findFunction,list);
}

template<class T> template<class T1> void
List<T>::findRvsFirst(const T1 & findFunction,List<ListEntry<T>*> & list)
{
	return msglib::find(m_tail,false,false,0,findFunction,list);
}

////////////////////////////////////////////////////////////////

template<class T> template<class T1,class T2> void
List<T>::processFwd(const T1 & findFunction, const T2 & processFunction)
{
	return msglib::processFwd(m_head,findFunction,processFunction);
}


template<class T> template<class T1,class T2> void
List<T>::processRvs(const T1 & findFunction, const T2 & processFunction)
{
	return msglib::processRvs(m_tail,findFunction,processFunction);
}

////////////////////////////////////////////////////////////////

template<typename T> bool
List<T>::operator==(const List<T> & plist) const
{
	if (plist.size()!=m_size) return false;
	auto ptr = m_head;
	for(auto p=plist.getHead();p!=0;p=p->getNext()) {
		if (p->getData() != ptr->getData()) return false;
		ptr = ptr->getNext();
	}
	return true;
}

template<typename T> bool
List<T>::operator==(const std::vector<Ptr> & plist) const
{
	if (plist.size()!=m_size) return false;
	auto ptr = m_head;
	for(auto p : plist) {
		if (p->getData() != ptr->getData()) return false;
		ptr = ptr->getNext();
	}
	return true;
}

////////////////////////////////////////////////////////////////

template<typename T> void
List<T>::copy(const std::vector<Ptr> & plist)
{
	clear();
	Ptr pnew = 0;
	Ptr p = m_head;
	for (auto i:plist) {
		pnew = insertAfter(p);
		if (pnew==0) return;
		pnew->getData() = i->getData();
		p = pnew;
	}
}

template<typename T> void
List<T>::copy(const List<T> & plist)
{
	clear();
	Ptr pnew = 0;
	Ptr  p = m_head;
	for (auto i=plist.getHead();i!=0;i=i->getNext()) {
		pnew = insertAfter(p);
		if (pnew==0) return;
		pnew->getData() = i->getData();
		p = pnew;
	}
}

////////////////////////////////////////////////////////////////

}

#endif

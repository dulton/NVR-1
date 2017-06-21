

#include "System/pool_allocator.h"

Pool::Pool(unsigned int sz)
:esize(sz < sizeof(Link) ? sizeof(Link) : sz),
ecount(ecount_default)
{
	head = 0;
	chunks = 0;
}

//free all chunks
Pool::~Pool()
{
	Chunk* n = chunks;
	while(n)
	{
		Chunk *p = n;
		n = n->next;
		delete p; //the data memory of 'chunk' will also be freed
	}
}

//allocate new 'chunk' and its space, organize it as a linked list of element of size 'esize'
void Pool::grow()
{
	//trace("Pool::grow %d bytes\n", esize * ecount + sizeof(Chunk));

	void *buff = new char[esize * ecount + sizeof(Chunk)];
	Chunk *n = new (buff) Chunk;
	n->mem = reinterpret_cast<char*>(n + 1);
	n->next = chunks;
	chunks = n;

	char *start = n->mem;
	char *last = &start[(ecount - 1) * esize];
	for (char *p = start; p < last; p += esize)
	{
		reinterpret_cast<Link*>(p)->next = reinterpret_cast<Link*>(p + esize);
	}
	reinterpret_cast<Link*>(last)->next = 0;
	head = reinterpret_cast<Link*>(start);
}

//if you want to change the defaut element count in a 'chunk',
//you the best call the function when initializing and only once
void Pool::setec(unsigned int n)
{
	mutex.Enter();
	ecount = n;
	mutex.Leave();
}

unsigned int Pool::getec()
{
	return ecount;
}

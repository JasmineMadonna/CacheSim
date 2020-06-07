#include "Cache.h"
#include <iostream>
#include <fstream>
#include <cmath>
#include <string>
#include <cstdlib>

int CacheSet::insert(int tag)
{
	bool hit = false;
	if (blocks.empty())
	{
		CacheBlock new_blk{ tag,0 };
		blocks.push_back(new_blk);
		return hit;
	}
	else
	{
		int hit_index;
		for (int i = 0; i < blocks.size(); i++)
		{
			if (blocks[i].tag == tag)
			{
				hit = true;
				hit_index = i;
				break;
			}
		}
		if (hit)
		{
			for (int i = 0; i < blocks.size(); i++)
			{
				if (blocks[i].lru < blocks[hit_index].lru)
					blocks[i].lru++;
			}
			blocks[hit_index].lru = 0;
		}
		else
		{
			if (blocks.size() < 4)
			{
				for (int i = 0; i < blocks.size(); i++)
				{
					blocks[i].lru++;
				}
				CacheBlock new_blk{ tag,0 };
				blocks.push_back(new_blk);
				return hit;
			}
			else
			{
				for (int i = 0; i < blocks.size(); i++)
				{
					if (blocks[i].lru == 3)
					{
						blocks[i].tag = tag;
						blocks[i].lru = 0;
					}
					else
						blocks[i].lru++;
				}
			}
		}
	}
	return hit;
}

/*For write through and write not allocate cache*/
int CacheSet::handleWT_WNA(int tag)
{
	bool hit = false;

	/*Check whether the block is in the cache*/
	int hit_index;
	for (int i = 0; i < blocks.size(); i++)
	{
		if (blocks[i].tag == tag)
		{
			hit = true;
			hit_index = i;
			break;
		}
	}
	if (hit)
	{
		for (int i = 0; i < blocks.size(); i++)
		{
			if (blocks[i].lru < blocks[hit_index].lru)
				blocks[i].lru++;
		}
		blocks[hit_index].lru = 0;
	}
	else
	{
		//Need this if there is cycles calculation
	}

	return hit;
}

/*Write back and write allocate cache*/
int CacheSet::handleWB_WA(int tag)
{
	bool hit = false;
	/*Check whether the block is in the cache*/
	int hit_index;
	for (int i = 0; i < blocks.size(); i++)
	{
		if (blocks[i].tag == tag)
		{
			hit = true;
			hit_index = i;
			break;
		}
	}
	if (hit) //Update LRU info
	{
		for (int i = 0; i < blocks.size(); i++)
		{
			if (blocks[i].lru < blocks[hit_index].lru)
				blocks[i].lru++;
		}
		blocks[hit_index].lru = 0;
	}
	else
	{
		if (blocks.size() < 4)
		{
			for (int i = 0; i < blocks.size(); i++)
			{
				blocks[i].lru++;
			}
			CacheBlock new_blk{ tag,0 };
			blocks.push_back(new_blk);
			return hit;
		}
		else
		{
			for (int i = 0; i < blocks.size(); i++)
			{
				if (blocks[i].lru == 3)
				{
					blocks[i].tag = tag;
					blocks[i].lru = 0;
				}
				else
					blocks[i].lru++;
			}
		}
	}

	return hit;
}

void CacheSet::printTags()
{
	for (auto it = blocks.begin(); it != blocks.end(); it++)
		std::cout << "Tag : " << (*it).tag << "		lru : " << (*it).lru << "\n";
}

BlockAddr Cache::decodeAddress(unsigned int addr)
{
	int block_offset = ((1 << 5) - 1) & addr;
	std::cout << "addr copy: " << addr << "block offset = " << block_offset << "\n";
	int index = (addr >> offset_bits)& ((1 << index_bits) - 1);
	std::cout << "addr copy: " << addr << "index = " << index << "\n";
	int tag = (addr >> (offset_bits + index_bits))& ((1 << tag_bits) - 1);
	std::cout << "addr copy: " << addr << "tag = " << tag << "\n";

	BlockAddr blk_addr{ tag, index, block_offset };
	return blk_addr;
}

void Cache::accessCache(unsigned int addr, char ls)
{
	BlockAddr blk_addr = decodeAddress(addr);
	//std::cout << "tag : " << blk_addr.tag << "		index = " << blk_addr.index << "\n";
	if (ls == 'l')
	{
		std::cout << "Calling insert \n";
		bool hit = sets[blk_addr.index].insert(blk_addr.tag);
		if (hit)
			load_hits++;
		else
			load_misses++;
	}
	else if (ls == 's')
	{
		bool hit = sets[blk_addr.index].handleWB_WA(blk_addr.tag);
		if (hit)
			store_hits++;
		else
			store_misses++;
	}
}

void Cache::printStats()
{
	std::cout << "Load Hits : " << load_hits << "\n";
	std::cout << "Load Misses : " << load_misses << "\n";
	std::cout << "Total loads : " << load_hits + load_misses << "\n";
	std::cout << "Store Hits : " << store_hits << "\n";
	std::cout << "Store Misses : " << store_misses << "\n";
	std::cout << "Total loads : " << store_hits + store_misses << "\n";
}

int main()
{
	int cache_size = 32 * 1024;
	int assoc = 4;
	int block_size = 32;

	//unsigned int addr[5][2]{ {'l',0x0000AA40},{'l', 0x0000AA40},{'l', 0x0000AA40} ,{'l', 0x0000F231},{'l', 0x0000AA40} };
	//Trace addr[5] = { Trace{'l',0x0000AA40}, Trace{'l',0x0000AA40}, Trace{'l',0x0000AA40}, Trace{'l',0x0000F231}, Trace{'l',0x0000AA40} };
	Trace addr[5] = { Trace{'s',0x0000AA40}, Trace{'l',0x0000AA40}, Trace{'s',0x0000AA40}, Trace{'s',0x0000F231}, Trace{'s',0x0000AA40} };
	Cache* cache = new Cache(cache_size, assoc, block_size);
	for (int i = 0; i < 5; i++)
	{
		std::cout << "ls = " << addr[i].ls << "		addr = " << addr[i].addr << "\n";
		cache->accessCache(addr[i].addr, addr[i].ls);
	}
	
	cache->printStats();

	return 0;
}
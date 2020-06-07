#pragma once
#include <vector>
#include <array>

struct CacheBlock
{
	int tag;
	int lru;
};

struct BlockAddr
{
	int tag;
	int index;
	int block_offset;
};

struct Trace
{
	char ls; // 's' refers to store and 'l' refers to load
	unsigned int addr;
};

class CacheSet
{
private:
	std::vector<CacheBlock> blocks;

public:
	int insert(int tag);
	int handleWT_WNA(int tag); //Write through and write no allocate policy
	int handleWB_WA(int tag); // Write Back and write alloacte
	void printTags();
};

class Cache
{
private:
	int cache_size;
	int assoc; 
	int block_size; // Gives the block size in KB
	int num_sets;
	int index_bits;
	int tag_bits;
	int offset_bits;
	//BlockAddr addr;
	std::vector<CacheSet> sets;

	//Statistics
	int load_hits;
	int load_misses;
	int store_hits;
	int store_misses;
	int cycles;

public:
	Cache(int cache_size, int assoc, int block_size)
		:cache_size(cache_size), assoc(assoc), block_size(block_size)
	{
		load_hits = 0;
		load_misses = 0;
		store_hits = 0;
		store_misses = 0;
		cycles = 0;

		num_sets = cache_size / (assoc * block_size);
		index_bits = static_cast<int> (log2(num_sets));
		offset_bits = static_cast<int> (log2(block_size));
		tag_bits = 32 - index_bits - offset_bits;

		sets.reserve(num_sets);
		for (int i = 0; i < num_sets; i++)
			sets.push_back(CacheSet());
	}
	BlockAddr decodeAddress(unsigned int addr);
	void accessCache(unsigned int addr, char ls);
	void printStats();
};



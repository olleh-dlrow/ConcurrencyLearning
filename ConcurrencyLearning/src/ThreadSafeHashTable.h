#ifndef THREADSAFEHASHTABLE_H
#define THREADSAFEHASHTABLE_H

#include <shared_mutex>
#include <list>
#include <vector>
#include <algorithm>
#include <optional>

template<typename K, typename V, typename Hash=std::hash<K> >
class ThreadSafeHashTable {
private:
	class BucketType {
	private:
		typedef std::pair<K, V> BucketValue;
		typedef std::list<BucketValue> BucketData;
		typedef typename BucketData::iterator BucketIterator;

	public:
		std::optional<V> get(const K& key) {
			std::shared_lock<std::shared_mutex> lk(mut);
			BucketIterator found_entry = find_entry(key);
			std::optional<V> opt_value;
			if (found_entry != data.end()) {
				opt_value = found_entry->second;
			}
			return opt_value;
		}

		void insert_or_update(const K& key, const V& value) {
			std::unique_lock<std::shared_mutex> lk(mut);
			BucketIterator found_entry = find_entry(key);
			if (found_entry == data.end()) {
				data.push_back(BucketValue(key, value));
			}
			else {
				found_entry->second = value;
			}
		}

		void erase(const K& key) {
			std::unique_lock<std::shared_mutex> lk(mut);
			BucketIterator found_entry = find_entry(key);
			if (found_entry != data.end()) {
				data.erase(found_entry);
			}
		}

	private:
		BucketIterator find_entry(const K& key) {
			return std::find_if(data.begin(), data.end(), [&](const BucketValue& item) {
				return item.first == key;
			});
		}

		BucketData data;
		mutable std::shared_mutex mut;
	};
public:
	ThreadSafeHashTable(int num_buckets = 17, const Hash& hasher_ = Hash()) :
	buckets(num_buckets), hasher(hasher_)
	{

	}

	std::optional<V> get(const K& key) {
		return buckets[get_hash(key)].get(key);
	}

	void insert_or_update(const K& key, const V& value) {
		buckets[get_hash(key)].insert_or_update(key, value);
	}

	void erase(const K& key) {
		buckets[get_hash(key)].erase(key);
	}

private:
	std::size_t get_hash(const K& key) const {
		return hasher(key) % buckets.size();
	}

	std::vector<BucketType> buckets;
	Hash hasher;
};

#endif // !THREADSAFEHASHTABLE_H

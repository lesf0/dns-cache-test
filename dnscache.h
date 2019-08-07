#ifndef DNSCACHE_H
#define DNSCACHE_H

#include <unordered_map>
#include <queue>
#include <string>
#include <mutex>
#include <exception>

template <size_t N>
class DNSCache
{
private:
	DNSCache() {}
	~DNSCache() {}
	DNSCache(DNSCache const&) = delete;
	DNSCache& operator=(DNSCache const&) = delete;

	typedef std::unordered_map<std::string, std::string> cache_type;

	cache_type m_cache;
	std::queue<cache_type::iterator> m_delete_queue;
	mutable std::mutex m_mutex;

public:
	static DNSCache& instance()
	{
		static DNSCache inst;
		return inst;
	}

	void update(const std::string& name, const std::string& ip)
	{
		std::lock_guard<std::mutex> lock(m_mutex);

		auto inserted = m_cache.insert({name, ip});

		if (inserted.second)
		{
			if (m_delete_queue.size() == N)
			{
				m_cache.erase(m_delete_queue.front());
				m_delete_queue.pop();
			}

			m_delete_queue.push(inserted.first);
		}
	}

	std::string resolve(const std::string& name) const
	{
		std::lock_guard<std::mutex> lock(m_mutex);

		auto found = m_cache.find(name);

		if (found == m_cache.end())
		{
			throw std::out_of_range(name);
		}

		return found->second;
	}
};

#endif
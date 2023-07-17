#pragma once

#include <iterator>
#include <mutex>
#include <unordered_map>
#include <vector>

template <typename _K, typename _V>
/**
 * @brief 线程安全的哈希表。
 *
 */
class ConcurrentHashMap {

  private:
    /**
     * @brief 仅允许单线程读写的哈希表。
     *
     */
    class SingleThreadHashMap {
      private:
        /**
         * @brief 互斥锁。
         *
         */
        std::mutex m_mtx;

        /**
         * @brief 线程不安全的哈希表。
         *
         */
        std::unordered_map<_K, _V> m_map;

      public:
        void put(const _K &key, _V value) {
            std::lock_guard<std::mutex> lock(m_mtx);
            m_map.emplace(key, value);
        }

        _V get(const _K &key) {
            std::lock_guard<std::mutex> lock(m_mtx);
            auto it = m_map.find(key);
            if (it != m_map.end()) {
                return it->second;
            }
            return nullptr;
        }

        const std::unordered_map<_K, _V> &getData() const { return m_map; }
    };

    /**
     * @brief 分片线程安全的哈希表。
     *
     */
    std::vector<SingleThreadHashMap *> m_vec;

    /**
     * @brief 分片掩码。
     *
     */
    const size_t m_mask;

    SingleThreadHashMap &get_shard(const _K &key) {
        // 位运算选择分片
        auto hash_code = std::hash<_K>(key);
        return m_vec[hash_code & m_mask];
    }

  public:
    ConcurrentHashMap(size_t num_shard)
        : m_mask(num_shard - 1), m_vec(num_shard) {}

    void put(const _K &key, _V value) {
        auto hashmap = get_shard(key);
        hashmap.put(key, value);
    }

    _V get(const _K &key) {
        auto hashmap = get_shard(key);
        return hashmap.get(key);
    }

    const std::vector<SingleThreadHashMap *> &getData() const { return m_vec; }
};
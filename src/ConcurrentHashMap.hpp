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
    std::vector<SingleThreadHashMap> m_vec;

    /**
     * @brief 分片掩码。
     *
     */
    const size_t m_mask;

    SingleThreadHashMap &getShard(const _K &key) {
        // 位运算选择分片
        std::hash<_K> hashFunc;
        auto hash_code = hashFunc(key);
        return m_vec[hash_code & m_mask];
    }

  public:
    ConcurrentHashMap(size_t shardNum)
        : m_vec(shardNum), m_mask(shardNum - 1) {}

    void put(const _K &key, _V value) { getShard(key).put(key, value); }

    _V get(const _K &key) { return getShard(key).get(key); }

    const std::vector<SingleThreadHashMap> &getData() const { return m_vec; }
};
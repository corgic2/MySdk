#pragma once
#include <vector>
#include <mutex>
#include <memory>
#include <cassert>
#include <algorithm>

/// <summary>
/// 内存块结构体
/// </summary>
template<typename T>
struct ST_MemoryBlock {
    T* m_data;           ///< 数据指针
    bool m_isUsed;       ///< 使用标志
    size_t m_blockSize;  ///< 块大小

    /// <summary>
    /// 构造函数
    /// </summary>
    ST_MemoryBlock()
        : m_data(nullptr)
        , m_isUsed(false)
        , m_blockSize(0)
    {
    }
};

/// <summary>
/// 内存池类
/// </summary>
template<typename T>
class MemoryPool {
public:
    /// <summary>
    /// 构造函数
    /// </summary>
    /// <param name="blockSize">块大小</param>
    /// <param name="initialBlocks">初始块数量</param>
    explicit MemoryPool(size_t blockSize = 1024, size_t initialBlocks = 16)
        : m_blockSize(blockSize)
    {
        for (size_t i = 0; i < initialBlocks; ++i) {
            AddBlock();
        }
    }

    /// <summary>
    /// 析构函数
    /// </summary>
    ~MemoryPool() {
        for (auto& block : m_blocks) {
            delete[] block.m_data;
        }
    }

    /// <summary>
    /// 分配内存
    /// </summary>
    /// <param name="size">需要的大小</param>
    /// <returns>内存指针</returns>
    T* Allocate(size_t size) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        // 查找可用块
        for (auto& block : m_blocks) {
            if (!block.m_isUsed && block.m_blockSize >= size) {
                block.m_isUsed = true;
                return block.m_data;
            }
        }

        // 没有找到合适的块，创建新块
        size_t newBlockSize = std::max(size, m_blockSize);
        AddBlock(newBlockSize);
        auto& newBlock = m_blocks.back();
        newBlock.m_isUsed = true;
        return newBlock.m_data;
    }

    /// <summary>
    /// 释放内存
    /// </summary>
    /// <param name="ptr">内存指针</param>
    void Deallocate(T* ptr) {
        if (!ptr) return;

        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto& block : m_blocks) {
            if (block.m_data == ptr) {
                block.m_isUsed = false;
                return;
            }
        }
    }

    /// <summary>
    /// 获取已分配的块数量
    /// </summary>
    /// <returns>块数量</returns>
    size_t GetBlockCount() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_blocks.size();
    }

    /// <summary>
    /// 获取已使用的块数量
    /// </summary>
    /// <returns>已使用块数量</returns>
    size_t GetUsedBlockCount() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return std::count_if(m_blocks.begin(), m_blocks.end(),
            [](const ST_MemoryBlock<T>& block) { return block.m_isUsed; });
    }

    /// <summary>
    /// 清理未使用的块
    /// </summary>
    void Cleanup() {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = std::remove_if(m_blocks.begin(), m_blocks.end(),
            [](const ST_MemoryBlock<T>& block) {
                if (!block.m_isUsed) {
                    delete[] block.m_data;
                    return true;
                }
                return false;
            });
        m_blocks.erase(it, m_blocks.end());
    }

private:
    /// <summary>
    /// 添加新的内存块
    /// </summary>
    /// <param name="blockSize">块大小</param>
    void AddBlock(size_t blockSize = 0) {
        if (blockSize == 0) blockSize = m_blockSize;
        
        ST_MemoryBlock<T> block;
        block.m_data = new T[blockSize];
        block.m_blockSize = blockSize;
        block.m_isUsed = false;
        m_blocks.push_back(block);
    }

private:
    std::vector<ST_MemoryBlock<T>> m_blocks;  ///< 内存块集合
    size_t m_blockSize;                       ///< 默认块大小
    mutable std::mutex m_mutex;               ///< 互斥锁
};

/// <summary>
/// 字符串内存池单例
/// </summary>
class StringMemoryPool {
public:
    /// <summary>
    /// 获取实例
    /// </summary>
    /// <returns>内存池实例</returns>
    static MemoryPool<char>& Instance() {
        static MemoryPool<char> instance(1024, 32);
        return instance;
    }

private:
    StringMemoryPool() = delete;
    StringMemoryPool(const StringMemoryPool&) = delete;
    StringMemoryPool& operator=(const StringMemoryPool&) = delete;
}; 
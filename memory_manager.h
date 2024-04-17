#pragma once

#include <cstddef>
#include <array>
#include <bitset>
#include <memory>
#include <type_traits>
#include <utility>

template <class Impl, std::size_t N_INSTANCES>
class MemoryManager
{
    using weak_handle_t = std::weak_ptr<Impl>;
    using handle_t = std::shared_ptr<Impl>;

public:

// need another construct_memory without idx parameter

    template <class... Args>
    static handle_t construct_memory(std::size_t idx, Args&&... args)
    {
        if (not is_valid_index(idx))
            return nullptr;

        if (is_constructed(idx))
            return nullptr;

        auto p = get_memory_addr(idx);
        if (not p)
            return nullptr;

        auto return_handle = handle_t{ new (p) Impl(std::forward<Args>(args)...),
                                    [](Impl* _p){std::destroy_at(_p);} };
        weak_handles[idx] = return_handle;

        return return_handle;
    }

    [[nodiscard]] static constexpr auto is_valid_index(std::size_t idx)
    {
        return idx < N_INSTANCES;
    } 


    [[nodiscard]] static bool is_constructed(std::size_t idx)
    {
        return is_valid_index(idx) ? 
            (not get_weak_handle(idx).expired()) : false;
    }

    [[nodiscard]] static auto is_constructed(void)
    {
        std::bitset<N_INSTANCES> ret;
        for (std::size_t i = 0 ; i < N_INSTANCES ; ++i)
            ret[i] = is_constructed(i);
        return ret;
    }

    [[nodiscard]] static handle_t get_handle(std::size_t idx)
    {
        return handle_t{get_weak_handle(idx)};
    }

private:

    [[nodiscard]] static constexpr std::byte* get_memory_addr(std::size_t idx)
    {
        if (not is_valid_index(idx))
            return nullptr;

        const auto offset = idx * sizeof(Impl);
        return memory_pool + offset;
    }

    [[nodiscard]] static constexpr weak_handle_t get_weak_handle(std::size_t idx)
    {
        return instance_handles.at(idx);
    }



    alignas(std::alignment_of_v<Impl>) static std::byte memory_pool[N_INSTANCES * sizeof(Impl)];

    static std::array<weak_handle_t, N_INSTANCES> weak_handles;
};

template <class Impl, std::size_t N_INSTANCES>
inline std::byte MemoryManager<Impl, N_INSTANCES>::memory_pool[N_INSTANCES * sizeof(Impl)]{};

template <class Impl, std::size_t N_INSTANCES>
alignas(std::alignment_of_v<Impl>) inline std::array<typename MemoryManager<Impl, N_INSTANCES>::weak_handle_t, N_INSTANCES> MemoryManager<Impl, N_INSTANCES>::weak_handles{};





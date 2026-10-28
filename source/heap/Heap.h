// Heap.h - generic binary heap written from scratch.
// Heap<T, std::less<T>>    -> Max Heap (largest on top)
// Heap<T, std::greater<T>> -> Min Heap (smallest on top)
#pragma once
#include <cstddef>
#include <functional>
#include <stdexcept>
#include <utility>
#include <vector>

template <typename T, typename Cmp = std::less<T>>
class Heap {
public:
    void push(const T& value) {
        data_.push_back(value);
        siftUp(data_.size() - 1);
    }

    const T& top() const {
        if (data_.empty()) throw std::out_of_range("Heap is empty");
        return data_.front();
    }

    void pop() {
        if (data_.empty()) throw std::out_of_range("Heap is empty");
        data_.front() = data_.back();
        data_.pop_back();
        if (!data_.empty()) siftDown(0);
    }

    std::size_t size() const { return data_.size(); }
    bool empty() const { return data_.empty(); }

    // Top-K without destroying this heap: works on a temporary copy.
    // If k > size(), returns every element.
    std::vector<T> topK(std::size_t k) const {
        Heap temp(*this);
        std::vector<T> out;
        while (out.size() < k && !temp.empty()) {
            out.push_back(temp.top());
            temp.pop();
        }
        return out;
    }

private:
    std::vector<T> data_;
    Cmp cmp_;

    void siftUp(std::size_t i) {
        while (i > 0) {
            std::size_t p = (i - 1) / 2;
            if (cmp_(data_[p], data_[i])) { std::swap(data_[p], data_[i]); i = p; }
            else break;
        }
    }

    void siftDown(std::size_t i) {
        const std::size_t n = data_.size();
        while (true) {
            std::size_t l = 2 * i + 1, r = l + 1, best = i;
            if (l < n && cmp_(data_[best], data_[l])) best = l;
            if (r < n && cmp_(data_[best], data_[r])) best = r;
            if (best == i) break;
            std::swap(data_[i], data_[best]);
            i = best;
        }
    }
};

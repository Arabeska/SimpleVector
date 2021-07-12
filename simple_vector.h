#pragma once

#include <cassert>
#include <initializer_list>
#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <utility>

#include "array_ptr.h"

class ReserveProxyObj {
public:
    ReserveProxyObj(size_t capacity_to_reserve)
        : capacity_reserve_(capacity_to_reserve) {
    }

    size_t GetCapacity() {
        return capacity_reserve_;
    }

private:
    size_t capacity_reserve_ = 0;
};

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value)
        : items_(size), size_(size), capacity_(size) {
        std::fill(items_.Get(), items_.Get() + size, value);
    }

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size)
        : SimpleVector(size, Type()) {
    }

    SimpleVector(std::initializer_list<Type> init)
        :items_(init.size()), size_(init.size()), capacity_(init.size()) {
        std::copy(init.begin(), init.end(), items_.Get());
    }

    SimpleVector(const SimpleVector& other) {
        assert(size_ == 0);
        SimpleVector<Type> cop(other.GetSize());
        std::copy((other.items_).Get(), ((other.items_).Get() + other.GetSize()), (cop.items_).Get());
        cop.capacity_ = other.capacity_;
        swap(cop);
    }

    SimpleVector(SimpleVector&& other) {
        assert(size_ == 0);
        items_ = std::move(other.items_);
        size_ = std::exchange(other.size_, 0);
        capacity_ = std::exchange(other.capacity_, 0);
    }

    SimpleVector(ReserveProxyObj obj) {
        Reserve(obj.GetCapacity());
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
        if (this != &rhs) {
            if (rhs.IsEmpty()) {
                Clear();
            }
            else {
                SimpleVector<Type> cop(rhs);
                swap(cop);
            }
        }
        return *this;
    }

    SimpleVector& operator=(SimpleVector&&) = default;

    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept {
        return size_;
    }

    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept {
        assert(index < size_);
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        assert(index < size_);
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        if (index >= size_) {
            using namespace std;
            throw std::out_of_range("index >= size"s);
        }
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        if (index >= size_) {
            using namespace std;
            throw std::out_of_range("index >= size"s);
        }
        return items_[index];
    }

    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
        return items_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
        return (items_.Get() + size_);
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
        return items_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
        return (items_.Get() + size_);
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept {
        return items_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
        return (items_.Get() + size_);
    }

    // Добавляет элемент в конец вектора
    // При нехватке места увеличивает вдвое вместимость вектора
    void PushBack(const Type& item) {
        if (size_ == capacity_) {
            ReCapacity(std::max(size_ * 2, static_cast<size_t>(1)));
        }
        *(end()) = item;
        size_++;
    }

    void PushBack(Type&& item) {
        if (size_ == capacity_) {
            ReCapacity(std::max(size_ * 2, static_cast<size_t>(1)));
        }
        *(end()) = std::move(item);
        size_++;
    }

    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
    Iterator Insert(ConstIterator pos, const Type& value) {
        assert(pos >= begin() && pos <= end());
        auto step_ = pos - begin();
        if (capacity_ == size_) {
            ReCapacity(std::max(size_ * 2, static_cast<size_t>(1)));
        }
        std::copy_backward(begin() + step_, end(), end() + 1);
        auto* it = begin() + step_;
        *it = std::move(value);
        size_++;
        return it;
    }

    Iterator Insert(ConstIterator pos, Type&& value) {
        assert(pos >= begin() && pos <= end());
        auto step_ = pos - begin();
        if (capacity_ == size_) {
            ReCapacity(std::max(size_ * 2, static_cast<size_t>(1)));
        }
        std::move_backward(begin() + step_, end(), end() + 1);
        auto* it = begin() + step_;
        *it = std::move(value);
        size_++;
        return it;
    }

    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack() noexcept {
        assert(!IsEmpty());
        size_--;
    }

    // Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos) {
        assert(pos >= begin() && pos < end());
        auto step_ = pos - begin();
        auto* it = begin() + step_;
        std::move((it + 1), end(), it);
        size_--;
        return (begin() + step_);
    }

    // Обменивает значение с другим вектором
    void swap(SimpleVector& other) noexcept {
        items_.swap(other.items_);
        std::swap(other.size_, size_);
        std::swap(other.capacity_, capacity_);
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {
        if (size_ < new_size) {
            if (new_size <= capacity_)
                std::fill(items_.Get() + size_, items_.Get() + new_size, Type());
            else
                ReCapacity(new_size * 2);
        }
        size_ = new_size;
    }

    void Reserve(size_t new_capacity) {
        if (new_capacity > capacity_) {
            ReCapacity(new_capacity);
        }
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        size_ = 0;
    }

private:
    ArrayPtr<Type> items_;
    size_t size_ = 0;
    size_t capacity_ = 0;

    void ReCapacity(size_t new_capacity) {
        ArrayPtr<Type> new_cont_(new_capacity);
        std::move(items_.Get(), items_.Get() + size_, new_cont_.Get());
        items_.swap(new_cont_);
        capacity_ = new_capacity;
    }

};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(rhs < lhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs < lhs;
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
}

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}
// вставьте сюда ваш код для класса SimpleVector
// внесите необходимые изменения для поддержки move-семантики
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
    ReserveProxyObj(size_t to_reserve)
        : to_reserve_(to_reserve)
    {
    }
    
    size_t GetReverse() const noexcept {
        return to_reserve_;
    }
private:
    size_t to_reserve_ = 0u;
};


ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

/* Конструкторы------------------------------------------------------------------------- */
    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size) {
        SimpleVector tmp(size, Type{});
		swap(tmp);
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value) {
        ArrayPtr<Type> tmp(size);
		items_.swap(tmp);
		size_ = size;
		capacity_ = size;
		std::fill(begin(), end(), value);
    }
    
    // Конструктор копирования
    SimpleVector(const SimpleVector& other) {
        SimpleVector<Type> tmp(other.size_);
        std::copy(other.begin(), other.end(), tmp.begin());
        swap(tmp);
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init) {
        SimpleVector<Type> tmp(init.size());
		std::copy(init.begin(), init.end(), tmp.begin());
		swap(tmp);
    }
    
    SimpleVector(const ReserveProxyObj reserve) {
        Reserve(reserve.GetReverse());
    }
    
    // Конструктор перемещения
    SimpleVector(SimpleVector&& other) noexcept 
        : items_(other.items_.Get())
    {
        size_ = std::exchange(other.size_, 0u);
        capacity_ = std::exchange(other.capacity_, 0u);
        other.items_ = nullptr;
    }
/* -------------------------------------------------------------------------Конструкторы */
/* Операторы---------------------------------------------------------------------------- */
    // Оператор присваивания
    SimpleVector& operator=(const SimpleVector& rhs) {
        if (this != &rhs) {
            SimpleVector tmp(rhs);
            swap(tmp);
        }
        return *this;
    }

    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept {
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        return items_[index];
    }
    
    // Перемещающий оператор присваивания
    SimpleVector& operator=(SimpleVector&& rhs) noexcept {
        items_.~ArrayPtr();
        items_ = rhs.items_;
        size_ = std::exchange(rhs.size_, 0u);
        capacity_ = std::exchange(rhs.capacity_, 0u);
        rhs.items_ = nullptr;
        return *this;
    }
/* ----------------------------------------------------------------------------Операторы */
    void Reserve(size_t new_capacity) {
        if (new_capacity > capacity_) {
            ArrayPtr<Type> new_items(new_capacity);
            std::copy(begin(), end(), new_items.Get());
            items_.swap(new_items);
            capacity_ = new_capacity;
        }
    }
    
    // Добавляет элемент в конец вектора
    // При нехватке места увеличивает вдвое вместимость вектора
    void PushBack(const Type& item) {
        if (size_ < capacity_) {
            items_[size_] = item;
        }
        else {
            SimpleVector<Type> new_vector((capacity_ == 0) ? 1 : capacity_ * 2);
            std::move(begin(), end(), new_vector.begin());
            items_.swap(new_vector.items_);
            items_[size_] = item;
            capacity_ = new_vector.capacity_;
        }
        ++size_;
    }
    
    void PushBack(Type&& item) {
        if (size_ < capacity_) {
            items_[size_] = std::move(item);
        }
        else {
            size_t new_capacity = (capacity_ == 0) ? 1 : capacity_ * 2;
            ArrayPtr<Type> new_items(new_capacity);
            std::move(begin(), end(), new_items.Get());
            items_.swap(new_items);
            items_[size_] = std::move(item);
            capacity_ = new_capacity;
        }
        ++size_;
    }
    
    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack() noexcept {
        size_ = (IsEmpty()) ? size_ : size_ - 1;
    }
    
    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
    Iterator Insert(ConstIterator pos, const Type& value) {
        if (size_ < capacity_) {
            std::copy_backward(const_cast<Iterator>(pos), end(), end() + 1);
            items_[pos - items_.Get()] = value;
            ++size_;
            return const_cast<Iterator>(pos);
        }
        else {
            const auto insert_pos = pos - begin();
            size_t new_capacity = (capacity_ == 0) ? 1 : capacity_ * 2;
            ArrayPtr<Type> tmp(new_capacity);
            std::copy(begin(), const_cast<Iterator>(pos), tmp.Get());
            tmp[insert_pos] = value;
            std::copy(const_cast<Iterator>(pos), end(), tmp.Get() + insert_pos + 1);
            items_.swap(tmp);
            ++size_;
			capacity_ = new_capacity;
            return Iterator{items_.Get() + insert_pos};
        }
    }
    
    Iterator Insert(ConstIterator pos, Type&& value) {
        if (size_ < capacity_) {
            std::move_backward(const_cast<Iterator>(pos), end(), end() + 1);
            items_[pos - items_.Get()] = std::move(value);
            ++size_;
            return const_cast<Iterator>(pos);
        }
        else {
            const auto insert_pos = pos - begin();
            size_t new_capacity = (capacity_ == 0) ? 1 : capacity_ * 2;
            ArrayPtr<Type> tmp(new_capacity);
            std::move(begin(), const_cast<Iterator>(pos), tmp.Get());
            tmp[insert_pos] = std::move(value);
            std::move(const_cast<Iterator>(pos), end(), tmp.Get() + insert_pos + 1);
            items_.swap(tmp);
            ++size_;
			capacity_ = new_capacity;
            return Iterator{items_.Get() + insert_pos};
        }
    }
    
    // Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos) {
        std::move(const_cast<Iterator>(pos + 1), end(), const_cast<Iterator>(pos));
        --size_;
        return const_cast<Iterator>(pos);
    }
    
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
        return 0u == size_;
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("out of range SimpleVector");
        }
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("out of range SimpleVector");
        }
        return items_[index];
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        size_ = 0u;
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {
        if (new_size <= size_) {
            size_ = new_size;
        }
        else if (new_size <= capacity_) {
            std::for_each(begin() + size_, begin() + new_size, [](auto &x) {x = Type();});
            size_ = new_size;
        }
        else {
            new_size = std::max(new_size, capacity_ * 2);
            ArrayPtr<Type> new_items(new_size);
            std::move(begin(), end(), new_items.Get());
            std::for_each(new_items.Get() + size_, new_items.Get() + new_size, [](auto &x) {x = Type();});
            items_.swap(new_items);
            size_ = capacity_ = new_size;
        }
    }
    
    // Обменивает значение с другим вектором
    void swap(SimpleVector& other) noexcept {
        items_.swap(other.items_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }
/* Итераторы---------------------------------------------------------------------------- */
    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
        return Iterator{items_.Get()};
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
        return Iterator{items_.Get() + size_};
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
        return ConstIterator{items_.Get()};
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
        return ConstIterator{items_.Get() + size_};
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept {
        return ConstIterator{items_.Get()};
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
        return ConstIterator{items_.Get() + size_};
    }
/* ----------------------------------------------------------------------------Итераторы */
private:
    ArrayPtr<Type> items_;
    size_t size_ = 0u;
    size_t capacity_ = 0u;
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
	return !(lhs < rhs) && !(rhs < lhs);
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
	return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
	return std::lexicographical_compare(lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
	return lhs == rhs || lhs < rhs;
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
	return !(lhs <= rhs);
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
	return !(lhs < rhs);
}

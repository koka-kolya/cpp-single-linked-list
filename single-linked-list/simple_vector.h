#pragma once

#include "array_ptr.h"

#include <algorithm>
#include <initializer_list>
#include <stdexcept>

class ReserveProxyObj {
public:
	ReserveProxyObj() noexcept = default;
	
	ReserveProxyObj(size_t capacity_to_reserve) 
	: capacity_to_reserve_(capacity_to_reserve)
	{
	}
	
	size_t GetCapacity() const noexcept {
		return capacity_to_reserve_;
	} 
private:
	size_t capacity_to_reserve_ = 0;
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
	
	SimpleVector(ReserveProxyObj capacity) {
		Reserve(capacity.GetCapacity());
		size_ = 0;
	}
	
    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size) 
	: 
	arr(size),
	size_(size),
	capacity_(size)	
	{
    }
	
    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value)
	: SimpleVector(size) {
		std::fill(begin(), end(), value);
    }
	
    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init) 
	: SimpleVector(init.size()) {
		std::move(init.begin(), init.end(), begin());
		//old in work only
    }
	
	//Конструктор копирования
	SimpleVector(const SimpleVector& other) {
		if (&other != this) {
			SimpleVector new_arr(other.size_);
			std::copy(other.begin(), other.end(), new_arr.begin());
			swap(new_arr);
		}
	}
	//move-constructor для некопируемых
	SimpleVector(SimpleVector&& rhs) 
	: arr(std::move(rhs.arr)),
	size_(std::exchange(rhs.size_, 0)),
	capacity_(std::exchange(rhs.capacity_, 0))
	{
	}
	
	//Оператор присваивания
	SimpleVector& operator=(const SimpleVector& rhs) {
		if (this != &rhs) {
			SimpleVector tmp(rhs);
			swap(tmp);
		}
		return *this;
	}
	// move- operator=
	SimpleVector& operator=(SimpleVector&& rhs) {
		if (this != &rhs) {
			SimpleVector tmp(std::move(rhs));
			swap(tmp);
		}
		return *this;
	}
	
	void Reserve(size_t new_capacity) {
		if (new_capacity > capacity_) { // игнор, если new_capacity меньше существущего
			size_t old_size = size_;
			SimpleVector new_arr(new_capacity);
			std::copy(begin(), end(), new_arr.begin());
			new_arr.size_ = old_size;
			swap(new_arr);
		}
	}

	// Добавляет элемент в конец вектора
	// При нехватке места увеличивает вдвое вместимость вектора
	void PushBack(const Type& item) {
		size_t old_capacity_ = capacity_;
		SimpleVector new_arr(size_ + 1);
		std::copy(begin(), end(), new_arr.begin());
		new_arr[size_] = item; // copy для констант
		new_arr.capacity_ = new_arr.size_ < capacity_ ? old_capacity_ : new_arr.capacity_;
//		if (new_arr.size_ < capacity_) new_arr.capacity_ = old_capacity_;
		swap(new_arr);
	}
	
	void PushBack(Type&& item) {
		size_t old_capacity_ = capacity_;
		SimpleVector new_arr(size_ + 1);
		std::move(begin(), end(), new_arr.begin());
		new_arr[size_] = std::move(item); // move для некопируемых элементов
		new_arr.capacity_ = new_arr.size_ < capacity_ ? old_capacity_ : new_arr.capacity_;
//		if (new_arr.size_ < capacity_) new_arr.capacity_ = old_capacity_;
		swap(new_arr);
	}
	
	// Вставляет значение value в позицию pos.
	// Возвращает итератор на вставленное значение
	// Если перед вставкой значения вектор был заполнен полностью,
	// вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
	Iterator Insert(ConstIterator pos, const Type& value) {
		size_t old_size = size_;
		Iterator it = Iterator(pos);
		auto current_pos = it - begin();
		size_t new_capacity = size_ == capacity_ ? capacity_ == 0 ? 1 : capacity_ * 2: capacity_;
		SimpleVector new_arr(new_capacity);
		new_arr.size_ = old_size + 1;
		std::copy(begin(), it, new_arr.begin());
		new_arr[current_pos] = value; // copy для констант
		std::copy_backward(it, end(), new_arr.end());
		swap(new_arr);
		it = begin() + current_pos;
		return it;
	}
	
	Iterator Insert(ConstIterator pos, Type&& value) {
		size_t old_size = size_;
		Iterator it = Iterator(pos);
		auto current_pos = it - begin();
		size_t new_capacity = size_ == capacity_ ? capacity_ == 0 ? 1 : capacity_ * 2: capacity_;
		SimpleVector new_arr(new_capacity);
		new_arr.size_ = old_size + 1;
		std::move(begin(), it, new_arr.begin());
		new_arr[current_pos] = std::move(value); // move для некопируемых
		std::move_backward(it, end(), new_arr.end());
		swap(new_arr);
		it = begin() + current_pos;
		return it;
	}
	
	// "Удаляет" последний элемент вектора. Вектор не должен быть пустым
	void PopBack() noexcept {
		assert(!IsEmpty());
		--size_;
	}
	
	// Удаляет элемент вектора в указанной позиции. copy-метод для конст
	Iterator Erase(ConstIterator pos) {
		Iterator it = Iterator(pos);
		auto current_pos = begin() + std::distance(begin(), it);
		if (size_ > 0) {
			std::copy(it + 1, end(), current_pos);
			--size_;
//			PrintArr(*this);
		}
		return begin() + std::distance(begin(), it);
	}
	
	// Удаляет элемент вектора в указанной позиции. move-метод для неконст
	Iterator Erase(Iterator pos) {
		Iterator it = Iterator(pos);
		auto current_pos = begin() + std::distance(begin(), it);
		if (size_ > 0) {
			std::move(it + 1, end(), current_pos);
			--size_;
		}
		return begin() + std::distance(begin(), it);
	}
		
	// Обменивает значение с другим вектором
	void swap(SimpleVector& other) noexcept {
		arr.swap(other.arr);
		std::swap(size_, other.size_);
		std::swap(capacity_, other.capacity_);
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
        return size_ == 0;
    }

    // Возвращает ссылку на элемент с индексом index
	Type& operator[](size_t index) noexcept {
		assert(index <= size_); 
		return arr[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
		assert(index <= size_);
		return arr[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
	Type& At(size_t index) {
		if (index >= size_) {
			throw std::out_of_range("Index is out of range");
		}
		return arr[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
		if (index >= size_) {
			throw std::out_of_range("Index is out of range");
		}
		return arr[index];
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
		size_t old_capacity = capacity_;
		ArrayPtr<Type> tmp; 
		arr.swap(tmp);
		capacity_ = old_capacity;
		size_ = 0;
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {
		if (new_size < size_) {
			size_ = new_size;
			return;
		}
		SimpleVector new_arr(new_size);
		std::move(begin(), end(), new_arr.begin());
		swap(new_arr);
		//old in work only
    }
	
    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
		return arr.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
		return arr.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
		return arr.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
		return arr.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept {
		return arr.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
		return arr.Get() + size_;
    }
private:
	ArrayPtr<Type> arr {};
	size_t size_ = 0;
	size_t capacity_ = 0;
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
	return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end()) && lhs.GetSize() == rhs.GetSize();
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
	return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
	return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), [](const auto& lhs, const auto& rhs){return lhs < rhs;});
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
	return (lhs < rhs) || (lhs == rhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
	return !(lhs < rhs) && lhs != rhs;
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
	return (lhs > rhs) || (lhs == rhs);
} 

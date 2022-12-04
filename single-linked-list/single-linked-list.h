#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <initializer_list>
#include <string>
#include <utility>
#include <vector>
#include <iterator>

template <typename Type>
class SingleLinkedList {
	// Узел списка
	struct Node {
		Node() = default;
		Node(const Type& val, Node* next = nullptr)
		: value(val)
		, next_node(next) {
		}
		Type value;
		Node* next_node = nullptr;
	};
	// Фиктивный узел, используется для вставки "перед первым элементом"
	size_t size_ = 0;
	Node head_;
	
public:
	
	SingleLinkedList() {};
	
	SingleLinkedList(std::initializer_list<Type> values) {
		InitializeByIterators(values.begin(), values.end());
	}
	
	SingleLinkedList(const SingleLinkedList& other) {
		InitializeByIterators(other.begin(), other.end());
	}
	
	SingleLinkedList& operator=(const SingleLinkedList& rhs) {
		if (this != &rhs) {
			SingleLinkedList tmp(rhs);
			swap(tmp);
		}
		return *this;
	}
	
	void swap(SingleLinkedList& other) noexcept {
		std::swap(other.size_, this->size_);
		std::swap(other.head_.next_node, this->head_.next_node);
	}
	
	[[nodiscard]] size_t GetSize() const noexcept {
		return size_;
	}
	
	[[nodiscard]] bool IsEmpty() const noexcept {
		return size_ == 0;
	}
	
	void PushFront(const Type& value) {
		head_.next_node = new Node(value, head_.next_node);
		++size_;
	}
	
	void Clear() noexcept {
		while(size_) {
			PopFront();
		}
	}
	
	~SingleLinkedList() {
		Clear();
	}
	
	// Шаблон класса «Базовый Итератор».
	template <typename ValueType>
	class BasicIterator {
		
		friend class SingleLinkedList;
		
		explicit BasicIterator(Node* node) 
		: node_(node)
		{
		}
		
	public:
		// Категория итератора — forward iterator
		// (итератор, который поддерживает операции инкремента и многократное разыменование)
		using iterator_category = std::forward_iterator_tag;
		// Тип элементов, по которым перемещается итератор
		using value_type = Type;
		// Тип, используемый для хранения смещения между итераторами
		using difference_type = std::ptrdiff_t;
		// Тип указателя на итерируемое значение
		using pointer = ValueType*;
		// Тип ссылки на итерируемое значение
		using reference = ValueType&;
		
		BasicIterator() = default;
		
		BasicIterator(const BasicIterator<Type>& other) noexcept
		: node_(other.node_)
		{
		}
		
		BasicIterator& operator=(const BasicIterator& rhs) = default;
		
		[[nodiscard]] bool operator==(const BasicIterator<const Type>& rhs) const noexcept {
			return node_ == rhs.node_;
		}
		
		[[nodiscard]] bool operator!=(const BasicIterator<const Type>& rhs) const noexcept {
			return node_ != rhs.node_;
		}
		
		[[nodiscard]] bool operator==(const BasicIterator<Type>& rhs) const noexcept {
			return node_ == rhs.node_;
		}
		
		[[nodiscard]] bool operator!=(const BasicIterator<Type>& rhs) const noexcept {
			return node_ != rhs.node_;
		}
		
		BasicIterator& operator++() noexcept {
			node_ = node_->next_node;
			return *this;
		}
		
		BasicIterator operator++(int) noexcept {
			auto prev_value(*this);
			++(*this);
			return prev_value;
		}
		
		[[nodiscard]] reference operator*() const noexcept {
			return node_->value;
		}
		
		[[nodiscard]] pointer operator->() const noexcept {
			return &(node_->value);
		}
		
	private:
		Node* node_ = nullptr;
	};
	
public:
	using value_type = Type;
	using reference = value_type&;
	using const_reference = const value_type&;
	
	using Iterator = BasicIterator<Type>;
	using ConstIterator = BasicIterator<const Type>;
	
	[[nodiscard]] Iterator before_begin() noexcept {
		return Iterator{ &head_ };
	}
	
	[[nodiscard]] ConstIterator cbefore_begin() const noexcept {
		return ConstIterator{ const_cast<Node*>(&(head_)) };
	}
	
	[[nodiscard]] ConstIterator before_begin() const noexcept {
		return ConstIterator{ const_cast<Node*>(&(head_)) };
	}
	
	[[nodiscard]] Iterator begin() noexcept {
		return Iterator{ head_.next_node };
	}
	
	[[nodiscard]] Iterator end() noexcept {
		auto it = head_.next_node;
		while (it) {
			it = it->next_node;
		}
		return Iterator{ it };
	}
	
	[[nodiscard]] ConstIterator begin() const noexcept {
		return ConstIterator{ head_.next_node };
	}
	
	[[nodiscard]] ConstIterator end() const noexcept {
		auto it = head_.next_node;
		while (it) {
			it = it->next_node;
		}
		return ConstIterator{ it };
	}
	
	[[nodiscard]] ConstIterator cbegin() const noexcept {
		return ConstIterator{head_.next_node};
	}
	
	[[nodiscard]] ConstIterator cend() const noexcept {
		auto it = head_.next_node;
		while (it) {
			it = it->next_node;
		}
		return ConstIterator{it};
	}
	
	Iterator InsertAfter(ConstIterator pos, const Type& value) {
		auto new_item = new Node(value);
		new_item->next_node = pos.node_->next_node;
		pos.node_->next_node = new_item;
		++size_;
		return Iterator { new_item };
	}
	
	Iterator EraseAfter(ConstIterator pos) noexcept {
		if (pos != end()) {
			auto tmp = pos.node_->next_node; // tmp сможем безопасно удалить
			if (tmp != nullptr) { // убеждаемся, что не получим исключения, обратившись к элементу через один от pos
				pos.node_->next_node = tmp->next_node;
			}
			delete tmp;
			--size_;
		}
		return Iterator {pos.node_->next_node};
	}
	
	void PopFront() {
		auto tmp = head_.next_node;
		head_.next_node = head_.next_node->next_node;
		delete tmp;
		--size_;
	}
	
	template <typename Iter>
	void InitializeByIterators (Iter begin, Iter end) {
		SingleLinkedList tmp;
		int size = static_cast<int>(std::distance(begin, end));
		const auto input_begin = begin;
		
		for (int i = size - 1; i >= 0; --i) {
			std::advance(begin, i);
			tmp.PushFront(*begin);
			begin = input_begin;
		}
		swap(tmp);
	}
	
}; 

template <typename Type>
void swap(SingleLinkedList<Type>& lhs, SingleLinkedList<Type>& rhs) noexcept {
	lhs.swap(rhs);
}

template <typename Type>
bool operator==(const SingleLinkedList<Type>& lhs, const SingleLinkedList<Type>& rhs) {
	if (lhs.GetSize() == rhs.GetSize()) {
		return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
	}
	return false;
}

template <typename Type>
bool operator!=(const SingleLinkedList<Type>& lhs, const SingleLinkedList<Type>& rhs) {
	return !(lhs == rhs);
}

template <typename Type>
bool operator<(const SingleLinkedList<Type>& lhs, const SingleLinkedList<Type>& rhs) {
	return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), [](const auto& lhs, const auto& rhs){ return lhs < rhs; });
}

template <typename Type>
bool operator<=(const SingleLinkedList<Type>& lhs, const SingleLinkedList<Type>& rhs) {
	return !(rhs < lhs);
}

template <typename Type>
bool operator>(const SingleLinkedList<Type>& lhs, const SingleLinkedList<Type>& rhs) {
	return rhs < lhs;
}

template <typename Type>
bool operator>=(const SingleLinkedList<Type>& lhs, const SingleLinkedList<Type>& rhs) {
	return (lhs == rhs) || (lhs > rhs);
}


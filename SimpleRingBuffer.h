#pragma once

#include <cassert>
#include <concepts>
#include <cstddef>
#include <iterator>
#include <ostream>

namespace ArcdpsExtension {
	/**
 * @tparam T The type that this buffer holds.
 */
	template<typename T, typename Allocator = std::allocator<T>>
	class RingBuffer {
	public:
		explicit RingBuffer(size_t pInitialCapacity) {
			mCapacityBegin = mAlloc.allocate(pInitialCapacity);
			// mCapacityBegin = static_cast<T*>(calloc(pInitialCapacity, sizeof(T)));
			mCapacityEnd = mCapacityBegin + pInitialCapacity;
			mCurrent = mCapacityBegin;
			mSizeEnd = mCapacityBegin;
		}

		virtual ~RingBuffer() {
			if (mCapacityBegin) {
				Clear();
				mAlloc.deallocate(mCapacityBegin, mCapacityEnd - mCapacityBegin);
			}
		}

		RingBuffer(const RingBuffer& pOther) {
			size_t size = pOther.Size();
			mCapacityBegin = mAlloc.allocate(size);
			mCapacityEnd = mCapacityBegin + size;
			mCurrent = mCapacityBegin;
			mSizeEnd = mCapacityBegin;

			for (const auto& other : pOther) {
				PushBack(other);
			}
		}

		RingBuffer(RingBuffer&& pOther) noexcept
			: mCurrent(pOther.mCurrent),
			  mSizeEnd(pOther.mSizeEnd),
			  mCapacityBegin(pOther.mCapacityBegin),
			  mCapacityEnd(pOther.mCapacityEnd),
			  mAlloc(std::move(pOther.mAlloc)) {
			pOther.mCapacityBegin = nullptr;
		}

		RingBuffer& operator=(const RingBuffer& pOther) {
			if (this == &pOther)
				return *this;
			size_t size = pOther.Size();
			mCapacityBegin = mAlloc.allocate(size);
			mCapacityEnd = mCapacityBegin + size;
			mCurrent = mCapacityBegin;
			mSizeEnd = mCapacityBegin;

			for (const auto& other : pOther) {
				PushBack(other);
			}
			return *this;
		}

		RingBuffer& operator=(RingBuffer&& pOther) noexcept {
			if (this == &pOther)
				return *this;
			mCurrent = pOther.mCurrent;
			mSizeEnd = pOther.mSizeEnd;
			mCapacityBegin = pOther.mCapacityBegin;
			mCapacityEnd = pOther.mCapacityEnd;
			mAlloc = std::move(pOther.mAlloc);
			pOther.mCapacityBegin = nullptr;
			return *this;
		}

		void PushBack(T&& pElement);
		void PushBack(const T& pElement);

		template<typename... Args>
		void EmplaceBack(const Args&... args);

		T& Back();
		const T& Back() const;

		void Clear();
		size_t Size() const;
		void Resize(size_t pNewCapacity);

		const T& operator[](size_t pNum) const;
		T& operator[](size_t pNum);

		class RingBufferIterator {
		public:
			using difference_type = std::ptrdiff_t;
			using value_type = T;
			using pointer = T*;
			using reference = T&;
			using iterator_category = std::random_access_iterator_tag;
			using iterator_concept = std::random_access_iterator_tag;

			RingBufferIterator() = default;

			RingBufferIterator(pointer pPtr, bool pBegin, bool pEnd, const RingBuffer<T, Allocator>* pParent)
				: mPtr(pPtr),
				  mBegin(pBegin),
				  mEnd(pEnd),
				  mParent(pParent) {}

			// COPY & MOVE
			RingBufferIterator(const RingBufferIterator& pOther) = default;
			RingBufferIterator(RingBufferIterator&& pOther) noexcept = default;
			RingBufferIterator& operator=(const RingBufferIterator& pOther) = default;
			RingBufferIterator& operator=(RingBufferIterator&& pOther) noexcept = default;

			// comparators
			bool operator==(const RingBufferIterator&) const = default;

			// Relation comparison relys on uintptr_t "underflow". It basically means, that the comparison breaks, when the size is bigger than `size_t / 2`.
			std::strong_ordering operator<=>(const RingBufferIterator& pOther) const {
				uintptr_t thisDiff = (uintptr_t) mPtr - (uintptr_t) (mParent->mCurrent);
				uintptr_t otherDiff = (uintptr_t) pOther.mPtr - (uintptr_t) (mParent->mCurrent);
				if ((mEnd && !pOther.mEnd) || (pOther.mBegin && !mBegin)) {
					return std::strong_ordering::greater;
				}
				if ((mBegin && pOther.mBegin) || (pOther.mEnd && !mEnd)) {
					return std::strong_ordering::less;
				}
				return thisDiff <=> otherDiff;
			}

			// dereferencable
			reference operator*() const { return *mPtr; }

			// helper
			pointer operator->() const { return mPtr; }

			// Prefix increment
			RingBufferIterator& operator++() {
				assert(mEnd == false && "Called ++ on an end iterator");
				mBegin = false;
				mPtr = mParent->advance(mPtr);
				if (mParent->mCurrent == mPtr) mEnd = true;
				return *this;
			}

			RingBufferIterator& operator--() {
				assert(mBegin == false && "Called -- on an begin iterator");
				mEnd = false;
				mPtr = mParent->retreat(mPtr);
				if (mParent->mCurrent == mPtr) mBegin = true;
				return *this;
			}

			// Postfix increment
			RingBufferIterator operator++(int) {
				RingBufferIterator tmp = *this;
				this->operator++();
				return tmp;
			}

			RingBufferIterator operator--(int) {
				RingBufferIterator tmp = *this;
				this->operator--();
				return tmp;
			}

			// calculation operators
			// with itself
			difference_type operator-(const RingBufferIterator& pOther) const {
				std::ptrdiff_t thisDiff = mPtr - mParent->mCurrent;
				std::ptrdiff_t otherDiff = pOther.mPtr - mParent->mCurrent;
				std::ptrdiff_t fullDiff = mParent->mSizeEnd - mParent->mCapacityBegin;
				if (thisDiff < 0 || (mEnd && thisDiff == 0)) {
					thisDiff += fullDiff;
				}
				if (otherDiff < 0 || (pOther.mEnd && thisDiff == 0)) {
					otherDiff += fullDiff;
				}
				return thisDiff - otherDiff;
			}

			// with numbers
			RingBufferIterator& operator+=(const difference_type pNum) {
				assert(*this - mParent->begin() + pNum <= mParent->end() - mParent->begin() && "tried to move iterator after end!");

				mBegin = false;
				mPtr += pNum;
				if (mPtr >= mParent->mSizeEnd) {
					mPtr = mPtr - mParent->mSizeEnd + mParent->mCapacityBegin;
				}
				if (mParent->mCurrent == mPtr) mEnd = true;
				return *this;
			}

			RingBufferIterator operator+(const difference_type pNum) const {
				RingBufferIterator it = *this;
				it += pNum;
				return it;
			}

			friend RingBufferIterator operator+(const difference_type pLhs, const RingBufferIterator& pRhs) {
				RingBufferIterator it = pRhs;
				it += pLhs;
				return it;
			}

			RingBufferIterator& operator-=(const difference_type pNum) {
				assert(*this - mParent->begin() - pNum >= 0 && "tried to move iterator before begin!");

				mEnd = false;
				mPtr -= pNum;
				if (mPtr < mParent->mCapacityBegin) {
					mPtr = (T*) ((uintptr_t) mPtr + (uintptr_t) mParent->mSizeEnd - (uintptr_t) mParent->mCapacityBegin);
				}
				if (mParent->mCurrent == mPtr) mBegin = true;
				return *this;
			}

			RingBufferIterator operator-(const difference_type pNum) const {
				RingBufferIterator it = *this;
				it -= pNum;
				return it;
			}

			// for some reason the const function has to return a non-const reference...
			// The standard says so, so we have to conform it: `{  j[n]  } -> std::same_as<std::iter_reference_t<I>>;` with `const I j`.
			// https://en.cppreference.com/w/cpp/iterator/random_access_iterator
			reference operator[](const difference_type pNum) const {
				return *(mParent->begin() + pNum);
			}

			/*
		 * Only use for debugging!
		 */
			friend std::ostream& operator<<(std::ostream& pOs, const RingBufferIterator& pObj) {
				return pOs
					   << "mPtr: " << pObj.mPtr
					   << " mBegin: " << pObj.mBegin
					   << " mEnd: " << pObj.mEnd
					   << " mParent: " << pObj.mParent;
			}

		private:
			pointer mPtr = nullptr;
			bool mBegin = false;
			bool mEnd = false;
			const RingBuffer<T>* mParent = nullptr;
		};


		static_assert(std::random_access_iterator<RingBufferIterator>);

		// pointer pPtr, bool pBegin, bool pEnd, RingBuffer* pParent
		RingBufferIterator begin() { return RingBufferIterator(mCurrent, true, mSizeEnd == mCapacityBegin, this); }
		RingBufferIterator end() { return RingBufferIterator(mCurrent, mSizeEnd == mCapacityBegin, true, this); }
		const RingBufferIterator begin() const { return RingBufferIterator(mCurrent, true, mSizeEnd == mCapacityBegin, this); }
		const RingBufferIterator end() const { return RingBufferIterator(mCurrent, mSizeEnd == mCapacityBegin, true, this); }
		const RingBufferIterator cbegin() const { return RingBufferIterator(mCurrent, true, mSizeEnd == mCapacityBegin, this); }
		const RingBufferIterator cend() const { return RingBufferIterator(mCurrent, mSizeEnd == mCapacityBegin, true, this); }
		RingBufferIterator rbegin() { return std::make_reverse_iterator(end()); }
		RingBufferIterator rend() { return std::make_reverse_iterator(begin()); }
		const RingBufferIterator rbegin() const { return std::make_reverse_iterator(end()); }
		const RingBufferIterator rend() const { return std::make_reverse_iterator(begin()); }
		const RingBufferIterator crbegin() const { return std::make_reverse_iterator(end()); }
		const RingBufferIterator crend() const { return std::make_reverse_iterator(begin()); }

	private:
		T* mCurrent;
		T* mSizeEnd;
		T* mCapacityBegin;
		T* mCapacityEnd;
		Allocator mAlloc;

		T* pushOne();
		[[nodiscard]] T* advance(T* pElem) const;
		[[nodiscard]] T* retreat(T* pElem) const;
	};
} // namespace ArcdpsExtension

// test if the range is also valid
static_assert(std::ranges::random_access_range<ArcdpsExtension::RingBuffer<uint64_t>>);
static_assert(std::ranges::random_access_range<const ArcdpsExtension::RingBuffer<uint64_t>>);

template<typename T, typename Allocator>
void ArcdpsExtension::RingBuffer<T, Allocator>::PushBack(T&& pElement) {
	T* elem = pushOne();
	new (elem) T(pElement);
}

template<typename T, typename Allocator>
void ArcdpsExtension::RingBuffer<T, Allocator>::PushBack(const T& pElement) {
	T* elem = pushOne();
	new (elem) T(pElement);
}

template<typename T, typename Allocator>
T* ArcdpsExtension::RingBuffer<T, Allocator>::pushOne() {
	// If first element
	if (mSizeEnd == mCapacityBegin) {
		++mSizeEnd;
		return mCapacityBegin;
	}

	if (mSizeEnd != mCapacityEnd) {
		T* current = mSizeEnd;
		++mSizeEnd;
		return current;
	}

	// advance begin and end
	T* current = mCurrent;
	mCurrent = advance(mCurrent);

	current->~T();
	return current;
}

template<typename T, typename Allocator>
T* ArcdpsExtension::RingBuffer<T, Allocator>::advance(T* pElem) const {
	auto val = pElem + 1;
	if (val >= mSizeEnd) return mCapacityBegin;
	return val;
}

template<typename T, typename Allocator>
T* ArcdpsExtension::RingBuffer<T, Allocator>::retreat(T* pElem) const {
	auto val = pElem;
	if (val <= mCapacityBegin) val = mSizeEnd;
	--val;
	return val;
}

template<typename T, typename Allocator>
template<typename... Args>
void ArcdpsExtension::RingBuffer<T, Allocator>::EmplaceBack(const Args&... args) {
	T* elem = pushOne();
	new (elem) T(args...);
}

template<typename T, typename Allocator>
T& ArcdpsExtension::RingBuffer<T, Allocator>::Back() {
	return *retreat(mCurrent);
}

template<typename T, typename Allocator>
const T& ArcdpsExtension::RingBuffer<T, Allocator>::Back() const {
	return *retreat(mCurrent);
}

template<typename T, typename Allocator>
void ArcdpsExtension::RingBuffer<T, Allocator>::Clear() {
	for (auto it = begin(); it != end(); ++it) {
		(*it).~T();
	}

	mCurrent = mSizeEnd = mCapacityBegin;
}

template<typename T, typename Allocator>
size_t ArcdpsExtension::RingBuffer<T, Allocator>::Size() const {
	return mSizeEnd - mCapacityBegin;
}

template<typename T, typename Allocator>
void ArcdpsExtension::RingBuffer<T, Allocator>::Resize(size_t pNewCapacity) {
	// backup old data
	T* current = mCurrent;
	T* sizeEnd = mSizeEnd;
	T* capacityBegin = mCapacityBegin;
	T* capacityEnd = mCapacityEnd;

	// allocate new memory
	mCapacityBegin = mAlloc.allocate(pNewCapacity);
	// mCapacityBegin = static_cast<T*>(calloc(pInitialCapacity, sizeof(T)));
	mCapacityEnd = mCapacityBegin + pNewCapacity;
	mCurrent = mCapacityBegin;
	mSizeEnd = mCapacityBegin;

	// iterate over old elements and move them
	// Also deconstruct them
	// if there are old elements
	T* val = current;
	while (sizeEnd != capacityBegin) {
		PushBack(std::move(*val));
		val->~T();

		val = val + 1;
		if (val >= sizeEnd) val = capacityBegin;
		if (val == current) break;
	}

	// deallocate old ram
	mAlloc.deallocate(capacityBegin, capacityEnd - capacityBegin);
}

template<typename T, typename Allocator>
const T& ArcdpsExtension::RingBuffer<T, Allocator>::operator[](size_t pNum) const {
	const auto var = begin() + pNum;
	return *var;
}

template<typename T, typename Allocator>
T& ArcdpsExtension::RingBuffer<T, Allocator>::operator[](size_t pNum) {
	auto var = begin() + pNum;
	return *var;
}

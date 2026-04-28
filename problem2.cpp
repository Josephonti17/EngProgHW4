
#include <iostream>
#include <stdexcept>
#include <initializer_list>

class NumberList {
public:
    //nconstructors / destructor 

    // Empty list
    NumberList() : size_(0), data_(nullptr) {}

    // Build from an initializer list so we can write NumberList{1, 2, 3}.
    NumberList(std::initializer_list<int> il) : size_(il.size()),
                                                data_(size_ ? new int[size_] : nullptr) {
        std::size_t i = 0;
        for (int v : il) {
            data_[i++] = v;
        }
    }

    // Copy constructor - deep copy
    NumberList(const NumberList& other) : size_(other.size_),
                                          data_(size_ ? new int[size_] : nullptr) {
        for (std::size_t i = 0; i < size_; ++i) {
            data_[i] = other.data_[i];
        }
    }

    // Copy-assignment - check for self-assign, free old buffer, deep-copy new.
    NumberList& operator=(const NumberList& rhs) {
        if (this == &rhs) return *this;

        delete[] data_;                     // free what we owned
        size_ = rhs.size_;
        data_ = size_ ? new int[size_] : nullptr;
        for (std::size_t i = 0; i < size_; ++i) {
            data_[i] = rhs.data_[i];
        }
        return *this;
    }

    // Destructor
    ~NumberList() {
        delete[] data_;
    }

    // observers 

    std::size_t size() const { return size_; }

    // subscript 
    // Throw on out-of-range to make bugs visible during testing.

    int& operator[](std::size_t i) {
        if (i >= size_) throw std::out_of_range("NumberList index out of range");
        return data_[i];
    }

    const int& operator[](std::size_t i) const {
        if (i >= size_) throw std::out_of_range("NumberList index out of range");
        return data_[i];
    }

    // ---- compound assignment / arithmetic ----
    // += appends rhs onto the end of *this. Allocate a new buffer big enough
    // for both, copy the old data, then copy rhs's data after it.

    NumberList& operator+=(const NumberList& rhs) {
        std::size_t newSize = size_ + rhs.size_;
        int* combined = newSize ? new int[newSize] : nullptr;

        for (std::size_t i = 0; i < size_; ++i)         combined[i] = data_[i];
        for (std::size_t j = 0; j < rhs.size_; ++j)     combined[size_ + j] = rhs.data_[j];

        delete[] data_;
        data_ = combined;
        size_ = newSize;
        return *this;
    }

    // ---- increment ----
    // Prefix: bump every element, return reference to *this.
    NumberList& operator++() {
        for (std::size_t i = 0; i < size_; ++i) {
            ++data_[i];
        }
        return *this;
    }

    // Postfix: take a dummy int, save the old state, increment, return old.
    NumberList operator++(int) {
        NumberList old(*this);
        ++(*this);
        return old;
    }

    // ---- friends (need access to size_/data_) ----

    friend bool operator==(const NumberList& a, const NumberList& b);
    friend bool operator<(const NumberList& a, const NumberList& b);
    friend std::ostream& operator<<(std::ostream& os, const NumberList& list);

private:
    std::size_t size_;
    int*        data_;
};

// ---- non-member operator definitions ----

// + uses += under the hood. Take the lhs by value so we get a copy we can
// modify and return — this avoids manually allocating a result buffer.
NumberList operator+(NumberList lhs, const NumberList& rhs) {
    lhs += rhs;
    return lhs;
}

bool operator==(const NumberList& a, const NumberList& b) {
    if (a.size_ != b.size_) return false;
    for (std::size_t i = 0; i < a.size_; ++i) {
        if (a.data_[i] != b.data_[i]) return false;
    }
    return true;
}

bool operator!=(const NumberList& a, const NumberList& b) {
    return !(a == b);
}

// Lexicographic less-than, same idea as comparing two strings character by
// character. First differing element decides the result; if one runs out
// first, the shorter list is "less than".
bool operator<(const NumberList& a, const NumberList& b) {
    std::size_t n = (a.size_ < b.size_) ? a.size_ : b.size_;
    for (std::size_t i = 0; i < n; ++i) {
        if (a.data_[i] < b.data_[i]) return true;
        if (a.data_[i] > b.data_[i]) return false;
    }
    return a.size_ < b.size_;
}

std::ostream& operator<<(std::ostream& os, const NumberList& list) {
    os << "[";
    for (std::size_t i = 0; i < list.size_; ++i) {
        os << list.data_[i];
        if (i + 1 < list.size_) os << ", ";
    }
    os << "]";
    return os;
}


// ---------------------------------------------------------------------------
// Driver

int main() {
    std::cout << "=== NumberList demo ===\n\n";

    NumberList a{1, 2, 3};
    NumberList b{4, 5};
    NumberList c{1, 2, 3};

    std::cout << "a = " << a << "\n";
    std::cout << "b = " << b << "\n";
    std::cout << "c = " << c << "\n\n";

    // Concatenation with +
    NumberList sum = a + b;
    std::cout << "a + b = " << sum << "\n";

    // Operator chaining: a += b + c (should be a += [4,5,1,2,3])
    NumberList aCopy(a);
    aCopy += b + c;
    std::cout << "a += b + c -> " << aCopy << "\n\n";

    // Equality / inequality / less-than
    std::cout << "(a == c) ? " << (a == c ? "yes" : "no") << "\n";
    std::cout << "(a == b) ? " << (a == b ? "yes" : "no") << "\n";
    std::cout << "(a != b) ? " << (a != b ? "yes" : "no") << "\n";
    std::cout << "(a <  b) ? " << (a <  b ? "yes" : "no") << "  (lexicographic)\n\n";

    // Subscript - read and write
    std::cout << "a[1] = " << a[1] << "\n";
    a[1] = 99;
    std::cout << "after a[1] = 99, a = " << a << "\n\n";

    // Reset a so the increment demo is cleaner
    a = NumberList{1, 2, 3};

    // Prefix vs postfix increment
    std::cout << "Before increment: a = " << a << "\n";
    NumberList preResult  = ++a;          // prefix - returns NEW value
    std::cout << "After ++a:        a = " << a << ", returned = " << preResult << "\n";

    NumberList postResult = a++;          // postfix - returns OLD value
    std::cout << "After a++:        a = " << a << ", returned = " << postResult
              << "  (returned value is pre-increment)\n\n";

    // Verify that copies are independent (Rule of Three sanity check)
    NumberList original{10, 20, 30};
    NumberList copy(original);
    copy[0] = -1;
    std::cout << "original = " << original << "  (should be unchanged)\n";
    std::cout << "copy     = " << copy     << "\n\n";

    // Self-assignment guard
    std::cout << "Self-assigning a = a ...\n";
    a = a;
    std::cout << "a = " << a << "  (still valid)\n";

    std::cout << "\nDone.\n";
    return 0;
}

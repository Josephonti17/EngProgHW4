// SimpleString manages its own char* buffer and acts like a value type.
// Because we're using dynamic memory, the synthesized copy ctor and
// copy-assignment would just copy the pointer, which would let two objects
// share the same buffer and double-free it at destruction. So we implement
// the Rule of Three by hand: copy ctor, copy-assignment, destructor.
//
// I went with the copy-and-swap approach for operator= because it handles
// self-assignment automatically and is exception-safe — if the new
// allocation throws, the original object is untouched.

#include <iostream>
#include <cstring>
#include <utility>   // std::swap

class SimpleString {
public:
    // ---- constructors / destructor ----

    // Default constructor - empty string (allocates a 1-byte buffer with '\0'
    // so data_ is never nullptr; keeps the rest of the code simpler).
    SimpleString() : size_(0), data_(new char[1]) {
        data_[0] = '\0';
    }

    // Construct from a C-string. We allocate our OWN memory and copy the
    // characters in — we don't keep a pointer to the caller's buffer.
    SimpleString(const char* s) {
        if (s == nullptr) {
            size_ = 0;
            data_ = new char[1];
            data_[0] = '\0';
        } else {
            size_ = std::strlen(s);
            data_ = new char[size_ + 1];
            std::strcpy(data_, s);
        }
    }

    // Copy constructor - deep copy. Allocate a fresh buffer and copy the
    // contents so the new object owns its own memory.
    SimpleString(const SimpleString& other) : size_(other.size_),
                                              data_(new char[other.size_ + 1]) {
        std::strcpy(data_, other.data_);
    }

    // Copy-assignment using copy-and-swap. The parameter `rhs` is taken by
    // value, which calls the copy constructor for us. Then we swap our
    // internals with the temporary's. When `rhs` goes out of scope, it takes
    // our old buffer with it — no leak, no self-assignment trouble.
    SimpleString& operator=(SimpleString rhs) {
        swap(*this, rhs);
        return *this;
    }

    // Destructor - free the buffer we own.
    ~SimpleString() {
        delete[] data_;
    }

    // observers

    std::size_t size() const { return size_; }

    // subscript operators 
    // Non-const version returns a reference so callers can modify a character.
    // Const version returns by value (or const ref) so a const SimpleString
    // can still be read but not written to.

    char& operator[](std::size_t i) {
        return data_[i];
    }

    const char& operator[](std::size_t i) const {
        return data_[i];
    }

    // equality

    friend bool operator==(const SimpleString& a, const SimpleString& b) {
        if (a.size_ != b.size_) return false;
        return std::strcmp(a.data_, b.data_) == 0;
    }

    friend bool operator!=(const SimpleString& a, const SimpleString& b) {
        return !(a == b);
    }

    // output

    friend std::ostream& operator<<(std::ostream& os, const SimpleString& s) {
        os << s.data_;
        return os;
    }

    // Free swap function used by operator=. Friend so it can touch privates.
    friend void swap(SimpleString& a, SimpleString& b) {
        using std::swap;
        swap(a.size_, b.size_);
        swap(a.data_, b.data_);
    }

private:
    std::size_t size_;
    char*       data_;   // owned - always points to a buffer of size_+1
};

// Driver - demonstrates that copies are independent and modifying one
// SimpleString doesn't bleed into another.
int main() {
    std::cout << "=== SimpleString demo ===\n\n";

    SimpleString a("hello");
    SimpleString b(a);          // copy ctor
    SimpleString c;
    c = a;                      // copy-assignment

    std::cout << "a = " << a << "  (size " << a.size() << ")\n";
    std::cout << "b = " << b << "  (size " << b.size() << ")\n";
    std::cout << "c = " << c << "  (size " << c.size() << ")\n\n";

    // Mutate b through operator[] and check that a and c are unaffected.
    // If the copy were shallow, this character would change in all three.
    std::cout << "Modifying b[0] from 'h' to 'J'...\n";
    b[0] = 'J';

    std::cout << "a = " << a << "   <-- should still be 'hello'\n";
    std::cout << "b = " << b << "   <-- should be 'Jello'\n";
    std::cout << "c = " << c << "   <-- should still be 'hello'\n\n";

    // Equality checks
    std::cout << "(a == c) ? " << (a == c ? "yes" : "no") << "\n";
    std::cout << "(a == b) ? " << (a == b ? "yes" : "no") << "\n";
    std::cout << "(a != b) ? " << (a != b ? "yes" : "no") << "\n\n";

    // Self-assignment - shouldn't blow up or corrupt the string
    std::cout << "Self-assigning a = a ...\n";
    a = a;
    std::cout << "a = " << a << "  (still valid)\n\n";

    // Const subscript - should compile and read fine
    const SimpleString d("readonly");
    std::cout << "d[0] (const access) = " << d[0] << "\n";
    std::cout << "d    = " << d << "\n";

    std::cout << "\nDone.\n";
    return 0;
}

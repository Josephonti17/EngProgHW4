// Bonus - SmartBuffer (Move Semantics & Performance)
// EE 5102 / EE 4953 - Homework 4
// Joseph
//
// SmartBuffer manages a dynamically allocated int array and follows the
// "Rule of Five": copy ctor, copy-assignment, move ctor, move-assignment,
// destructor. Each copy/move operation prints a message so we can watch
// when the compiler chooses to copy vs. move at runtime.
//
// Why moves matter:
//   Copying a SmartBuffer means allocating a new buffer and copying every
//   element - O(n) work plus a heap allocation. Moving just steals the
//   pointer from the source object - O(1), no allocation. For a vector
//   that resizes, or a function that returns by value, this is the
//   difference between fast and slow.
//
// Source object after a move:
//   The standard requires a moved-from object to be in a "valid but
//   unspecified" state. In practice we set its pointer to nullptr and its
//   size to 0, so the destructor still runs cleanly when the moved-from
//   object eventually goes out of scope.

#include <iostream>
#include <vector>
#include <utility>     // std::move

class SmartBuffer {
public:
    // ---- constructors / destructor ----

    explicit SmartBuffer(std::size_t n = 0, const std::string& tag = "")
        : size_(n), data_(n ? new int[n] : nullptr), tag_(tag) {
        for (std::size_t i = 0; i < size_; ++i) {
            data_[i] = static_cast<int>(i);
        }
        std::cout << "  [ctor]  built SmartBuffer \"" << tag_
                  << "\" of size " << size_ << "\n";
    }

    // Copy constructor - DEEP copy of the array.
    SmartBuffer(const SmartBuffer& other)
        : size_(other.size_),
          data_(other.size_ ? new int[other.size_] : nullptr),
          tag_(other.tag_ + "(copy)") {
        for (std::size_t i = 0; i < size_; ++i) {
            data_[i] = other.data_[i];
        }
        std::cout << "  [COPY]  copy-ctor from \"" << other.tag_
                  << "\" -> \"" << tag_ << "\"\n";
    }

    // Copy-assignment - free what we have, deep-copy from rhs.
    SmartBuffer& operator=(const SmartBuffer& rhs) {
        std::cout << "  [COPY]  copy-assign \"" << rhs.tag_
                  << "\" into \"" << tag_ << "\"\n";
        if (this == &rhs) return *this;

        delete[] data_;
        size_ = rhs.size_;
        data_ = size_ ? new int[size_] : nullptr;
        for (std::size_t i = 0; i < size_; ++i) {
            data_[i] = rhs.data_[i];
        }
        tag_ = rhs.tag_ + "(copy)";
        return *this;
    }

    // Move constructor - STEAL the pointer instead of copying. The noexcept
    // is important: std::vector will only use the move ctor during reallocation
    // if it's marked noexcept, otherwise it falls back to copying for
    // exception-safety reasons.
    SmartBuffer(SmartBuffer&& other) noexcept
        : size_(other.size_), data_(other.data_), tag_(std::move(other.tag_)) {
        // Leave the source in a valid, destructible state.
        other.size_ = 0;
        other.data_ = nullptr;
        std::cout << "  [MOVE]  move-ctor stole buffer (size " << size_ << ")\n";
    }

    // Move-assignment - free our own buffer, then steal from rhs.
    SmartBuffer& operator=(SmartBuffer&& rhs) noexcept {
        std::cout << "  [MOVE]  move-assign (size " << rhs.size_ << ")\n";
        if (this == &rhs) return *this;

        delete[] data_;
        size_ = rhs.size_;
        data_ = rhs.data_;
        tag_  = std::move(rhs.tag_);

        rhs.size_ = 0;
        rhs.data_ = nullptr;
        return *this;
    }

    ~SmartBuffer() {
        std::cout << "  [dtor]  destroying \"" << tag_
                  << "\" (size " << size_ << ")\n";
        delete[] data_;
    }

    std::size_t size() const { return size_; }
    const std::string& tag() const { return tag_; }

private:
    std::size_t size_;
    int*        data_;
    std::string tag_;     // a label so we can tell instances apart in output
};


// A factory that returns a SmartBuffer by value. Modern compilers usually
// elide the copy/move entirely (NRVO), but if elision doesn't happen the
// move ctor is what runs - never the copy ctor.
SmartBuffer makeBuffer(std::size_t n, const std::string& tag) {
    SmartBuffer local(n, tag);
    return local;
}


// ---------------------------------------------------------------------------
// Driver

int main() {
    std::cout << "=== SmartBuffer move-semantics demo ===\n\n";

    // ---- Part 1: return by value ----
    std::cout << "--- Part 1: return SmartBuffer by value from makeBuffer() ---\n";
    SmartBuffer fromFactory = makeBuffer(4, "factoryBuf");
    std::cout << "  (back in main, fromFactory.size = "
              << fromFactory.size() << ")\n\n";

    // ---- Part 2: explicit copy vs. explicit move ----
    std::cout << "--- Part 2: explicit copy vs. std::move ---\n";
    SmartBuffer original(5, "orig");
    std::cout << "Copy:\n";
    SmartBuffer copied = original;            // copy ctor
    std::cout << "Move:\n";
    SmartBuffer moved  = std::move(original); // move ctor; original is now empty
    std::cout << "  original.size after move = " << original.size()
              << " (expected 0)\n\n";

    // ---- Part 3: vector growth ----
    // Watch when push_back triggers a reallocation. Each reallocation copies
    // or MOVES every existing element to the new buffer. Because our move
    // ctor is noexcept, the vector chooses to move - which is exactly what
    // we want.
    std::cout << "--- Part 3: vector reallocations should MOVE, not copy ---\n";
    std::vector<SmartBuffer> bag;
    bag.reserve(2);   // start small so we can see growth happen

    std::cout << "push_back #1 (vector capacity = " << bag.capacity() << "):\n";
    bag.push_back(SmartBuffer(2, "v0"));
    std::cout << "push_back #2 (vector capacity = " << bag.capacity() << "):\n";
    bag.push_back(SmartBuffer(2, "v1"));
    std::cout << "push_back #3 -- this one forces reallocation:\n";
    bag.push_back(SmartBuffer(2, "v2"));

    std::cout << "\nVector now holds " << bag.size() << " buffers.\n\n";

    std::cout << "--- End of main, destructors run in reverse order ---\n";
    return 0;
}

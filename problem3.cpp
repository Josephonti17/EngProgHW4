// LibraryItem is an abstract base class with two pure virtuals:
//   lateFee(int days) - polymorphic late-fee calculation
//   clone()           - virtual copy constructor for safe duplication
//
// Two derived classes:
//   Book - flat $0.25/day late fee
//   DVD  - $1.00/day late fee, capped at $20.00 max
//
// Why a virtual destructor on the base?
//   Because we delete derived objects through LibraryItem* pointers in
//   main(). Without `virtual ~LibraryItem()`, only the base destructor
//   would run, leaking whatever the derived class allocated.
//
// Why clone() instead of just copying?
//   When you only have a LibraryItem*, you don't know what concrete type
//   it points to. clone() lets each derived class hand back a deep copy
//   of itself without slicing.
//
// Avoiding slicing:
//   We store LibraryItem* (pointers), never LibraryItem by value. Storing
//   by value would chop off the derived part of every object on the way
//   into the container.

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>

// Abstract base class

class LibraryItem {
public:
    LibraryItem(const std::string& title, const std::string& id)
        : title_(title), id_(id) {}

    // Virtual destructor - mandatory for a polymorphic base class.
    virtual ~LibraryItem() = default;

    // Pure virtual functions - make this an abstract class.
    virtual double        lateFee(int days) const = 0;
    virtual LibraryItem*  clone()           const = 0;

    // Helper for printing. Each derived class can override this to add its
    // own type-specific info, but the basic title/id stuff lives here.
    // operator<< below dispatches through this virtual function.
    virtual void print(std::ostream& os) const {
        os << "[" << id_ << "] " << title_;
    }

    const std::string& title() const { return title_; }
    const std::string& id()    const { return id_;    }

protected:
    std::string title_;
    std::string id_;
};

// Free operator<< dispatches to the virtual print() member, which is the
// trick that lets us print derived types polymorphically through a base
// reference. operator<< itself can't be virtual (it's not a member), but
// print() is.
std::ostream& operator<<(std::ostream& os, const LibraryItem& item) {
    item.print(os);
    return os;
}

// Derived: Book - flat per-day late fee

class Book : public LibraryItem {
public:
    Book(const std::string& title, const std::string& id, const std::string& author)
        : LibraryItem(title, id), author_(author) {}

    // Books charge $0.25 per day, no cap. Negative days clamped to zero.
    double lateFee(int days) const override {
        if (days <= 0) return 0.0;
        return 0.25 * days;
    }

    LibraryItem* clone() const override {
        return new Book(*this);     // uses the synthesized copy ctor
    }

    void print(std::ostream& os) const override {
        os << "Book   [" << id_ << "] \"" << title_ << "\" by " << author_;
    }

private:
    std::string author_;
};

// Derived: DVD - higher per-day fee, but capped

class DVD : public LibraryItem {
public:
    DVD(const std::string& title, const std::string& id, int runtimeMinutes)
        : LibraryItem(title, id), runtime_(runtimeMinutes) {}

    // $1.00 per day, but capped at $20.00 so the library doesn't bankrupt
    // someone who forgot a movie under their couch for a year.
    double lateFee(int days) const override {
        if (days <= 0) return 0.0;
        const double MAX_FEE = 20.00;
        double fee = 1.00 * days;
        return (fee < MAX_FEE) ? fee : MAX_FEE;
    }

    LibraryItem* clone() const override {
        return new DVD(*this);
    }

    void print(std::ostream& os) const override {
        os << "DVD    [" << id_ << "] \"" << title_ << "\" (" << runtime_ << " min)";
    }

private:
    int runtime_;
};

// Driver

int main() {
    std::cout << "=== Polymorphic Library Checkout ===\n\n";

    // Format dollar amounts with 2 decimal places throughout.
    std::cout << std::fixed << std::setprecision(2);

    // Build a heterogeneous collection of pointers. This is the
    // derived-to-base conversion: a Book* and a DVD* are implicitly
    // converted to LibraryItem* on the way in.
    std::vector<LibraryItem*> items;
    items.push_back(new Book("The C++ Programming Language", "B001", "Bjarne Stroustrup"));
    items.push_back(new Book("Effective Modern C++",          "B002", "Scott Meyers"));
    items.push_back(new DVD ("Hidden Figures",                "D001", 127));
    items.push_back(new DVD ("Apollo 13",                     "D002", 140));

    // Polymorphic print loop. operator<< takes a const LibraryItem& and
    // calls the virtual print() member, so the right overload runs for
    // each item even though the static type is just LibraryItem*.
    std::cout << "--- Catalog ---\n";
    for (const LibraryItem* p : items) {
        std::cout << "  " << *p << "\n";
    }

    // Polymorphic late-fee calculation. Same call site, different behavior
    // depending on whether it's a Book or DVD - this is dynamic binding.
    std::cout << "\n--- Late fees for 5 days ---\n";
    for (const LibraryItem* p : items) {
        std::cout << "  " << *p << "\n";
        std::cout << "      late fee: $" << p->lateFee(5) << "\n";
    }

    // Show the DVD cap kicking in - 30 days should hit the $20 ceiling.
    std::cout << "\n--- Late fees for 30 days ---\n";
    for (const LibraryItem* p : items) {
        std::cout << "  " << *p << "\n";
        std::cout << "      late fee: $" << p->lateFee(30) << "\n";
    }

    // Demonstrate clone() - polymorphic deep copy through a base pointer.
    // We don't know (and don't have to know) whether items[0] is a Book or
    // a DVD; clone() does the right thing.
    std::cout << "\n--- clone() demo ---\n";
    LibraryItem* duplicate = items[0]->clone();
    std::cout << "Original:  " << *items[0]  << "\n";
    std::cout << "Cloned:    " << *duplicate << "\n";
    std::cout << "(Different objects in memory: "
              << (items[0] != duplicate ? "yes" : "no") << ")\n";
    delete duplicate;

    // Clean up. Because LibraryItem has a virtual destructor, deleting
    // through a base pointer correctly runs the derived destructor first,
    // then the base one - no leaks.
    std::cout << "\nFreeing catalog...\n";
    for (LibraryItem* p : items) {
        delete p;
    }
    items.clear();

    std::cout << "Done.\n";
    return 0;
}

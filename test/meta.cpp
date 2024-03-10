#include "metafactory.h"

#include <string_view>
#include <charconv>
#include <iostream>
#include <cassert>

struct Default {
    static constexpr auto id = "None";
    int value = -1;
    constexpr Default() {}
    constexpr Default(std::string_view) {}
};

struct A {
    static constexpr auto id = "A";
    int value = 0;
    constexpr A(std::string_view raw) {
        std::from_chars(raw.begin(), raw.end(), value);
    }
};

struct B {
    static constexpr auto id = "B";
    int value{};
    constexpr B(std::string_view raw) {
        std::from_chars(raw.begin(), raw.end(), value);
    }
};

static constexpr auto same_id = []<typename T>(auto const& id)  {
    return (T::id == id);
};

static_assert([]{
    auto factory = make_meta_factory<A, B>(same_id);
    auto product = factory.produce_variant("A", "42");
    return std::get<A>(product).value == 42;    // Throws if A was not created
}());


struct Base {
    int value = -1;
    virtual void print() = 0;
    virtual ~Base() = default;
};

struct DerivedA : Base {
    static constexpr auto id = "A";
    void print() final { std::cout << "This is an A\n"; }
    constexpr DerivedA(std::string_view raw) {
        std::from_chars(raw.begin(), raw.end(), value);
    }
};

struct DerivedB : Base {
    static constexpr auto id = "B";
    void print() final { std::cout << "This is a B\n"; }
    constexpr DerivedB(std::string_view raw) {
        std::from_chars(raw.begin(), raw.end(), value);
    }
};


int main() {
    auto factory = make_meta_factory<DerivedA, DerivedB>(same_id);
    std::unique_ptr<Base> product = factory.produce_ptr_to<Base>("A", "42");
    product->print();
    assert(product->value == 42);
    assert(dynamic_cast<DerivedA*>(product.get()) != nullptr);
    return 0;
}

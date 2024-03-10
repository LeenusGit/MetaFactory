# Meta Factory

## Example Usage

### Using `std::variant` as polymorphic type

```c++
#include "metafactory.h"

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

```

### Using `std::unique_ptr` as polymorphic type

```c++
#include "metafactory.h"

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

static constexpr auto same_id = []<typename T>(auto const& id)  {
    return (T::id == id);
};

int main() {
    auto factory = make_meta_factory<DerivedA, DerivedB>(same_id);
    std::unique_ptr<Base> product = factory.produce_ptr_to<Base>("A", "42");
    product->print();
    assert(product->value == 42);
    assert(dynamic_cast<DerivedA*>(product.get()) != nullptr);
    return 0;
}
```

Output:

```
This is an A
```

## Implementation

```c++

#pragma once

#include <variant>
#include <memory>

template<typename F, typename ... Ts>
struct MetaFactory {

    F comparison_functor{};
    
    constexpr MetaFactory(F cmp) : comparison_functor{cmp} {}

    template <typename DefaultType>
    constexpr auto produce_variant_with_default(auto const& id, auto&& ... args) {
        std::variant<DefaultType, Ts...> res{};

        auto tmp = [&]<typename T> {
            if (comparison_functor.template operator()<T>(id)) {

                res.template emplace<T>(args...);
                return true;
            }
            return false;
        };

        (tmp.template operator()<Ts>() || ...);
        return res;
    }

    template <typename DefaultType = std::monostate>
    constexpr auto produce_variant(auto const& id, auto&& ... args) {
        return produce_variant_with_default<DefaultType>(id, args...);
    }

    template <typename BaseType>
    constexpr auto produce_ptr_to(auto const& id, auto&& ... args) {
        std::unique_ptr<BaseType> res{};

        auto tmp = [&]<typename T> {
            if (comparison_functor.template operator()<T>(id)) {

                res = std::make_unique<T>(args...);
                return true;
            }
            return false;
        };

        (tmp.template operator()<Ts>() || ...);
        return res;
    }
};

template<typename ... Ts>
constexpr auto make_meta_factory(auto func) {
    return MetaFactory<decltype(func), Ts...>{func};
}
```

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

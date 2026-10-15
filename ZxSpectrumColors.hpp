#ifndef ZX_SPECTRUM_COLORS_HPP
#define ZX_SPECTRUM_COLORS_HPP

#include <cstdint>
#include <type_traits>

class ZxSpectrumColor {
    template <uint8_t ColorValue>
    class Intrinsic {
    public:
        constexpr operator ZxSpectrumColor() const;
    };

public:
    static constexpr Intrinsic<0> Black{};
    static constexpr Intrinsic<1> Blue{};
    static constexpr Intrinsic<2> Red{};
    static constexpr Intrinsic<3> Magenta{};
    static constexpr Intrinsic<4> Green{};
    static constexpr Intrinsic<5> Cyan{};
    static constexpr Intrinsic<6> Yellow{};
    static constexpr Intrinsic<7> White{};

    constexpr ZxSpectrumColor() : value(0) {}

    constexpr explicit operator uint8_t() const { return value; }

private:
    constexpr explicit ZxSpectrumColor(uint8_t value) : value(value) {}

    uint8_t value;

    friend struct Ink;
    friend struct Paper;

    friend constexpr bool operator==(ZxSpectrumColor left, ZxSpectrumColor right) {
        return left.value == right.value;
    }

    friend constexpr bool operator!=(ZxSpectrumColor left, ZxSpectrumColor right) {
        return left.value != right.value;
    }

    friend constexpr bool operator<(ZxSpectrumColor left, ZxSpectrumColor right) {
        return left.value < right.value;
    }

    friend constexpr bool operator<=(ZxSpectrumColor left, ZxSpectrumColor right) {
        return left.value <= right.value;
    }

    friend constexpr bool operator>(ZxSpectrumColor left, ZxSpectrumColor right) {
        return left.value > right.value;
    }

    friend constexpr bool operator>=(ZxSpectrumColor left, ZxSpectrumColor right) {
        return left.value >= right.value;
    }
};

template <uint8_t ColorValue>
constexpr ZxSpectrumColor::Intrinsic<ColorValue>::operator ZxSpectrumColor() const {
    return ZxSpectrumColor(ColorValue);
}

struct Ink;
struct Paper;
class ZxSpectrumAttribute;

struct Ink {
    template <uint8_t ColorValue>
    struct Intrinsic {
        constexpr operator Ink() const;
        constexpr operator uint8_t() const { return ColorValue; }
    };

    static constexpr Intrinsic<0> Black{};
    static constexpr Intrinsic<1> Blue{};
    static constexpr Intrinsic<2> Red{};
    static constexpr Intrinsic<3> Magenta{};
    static constexpr Intrinsic<4> Green{};
    static constexpr Intrinsic<5> Cyan{};
    static constexpr Intrinsic<6> Yellow{};
    static constexpr Intrinsic<7> White{};

    constexpr Ink() = default;
    constexpr Ink(uint8_t encoded) : value(encoded & 0x07) {}
    constexpr Ink(ZxSpectrumColor color) : value(color.value & 0x07) {}

    constexpr operator ZxSpectrumColor() const { return ZxSpectrumColor(value); }
    constexpr operator uint8_t() const { return value; }

    constexpr Ink& operator=(ZxSpectrumColor color) {
        value = color.value & 0x07;
        return *this;
    }

    friend constexpr bool operator==(Ink left, Ink right) {
        return static_cast<ZxSpectrumColor>(left) == static_cast<ZxSpectrumColor>(right);
    }

    friend constexpr bool operator!=(Ink left, Ink right) {
        return static_cast<ZxSpectrumColor>(left) != static_cast<ZxSpectrumColor>(right);
    }

    friend constexpr bool operator<(Ink left, Ink right) {
        return static_cast<ZxSpectrumColor>(left) < static_cast<ZxSpectrumColor>(right);
    }

    friend constexpr bool operator<=(Ink left, Ink right) {
        return static_cast<ZxSpectrumColor>(left) <= static_cast<ZxSpectrumColor>(right);
    }

    friend constexpr bool operator>(Ink left, Ink right) {
        return static_cast<ZxSpectrumColor>(left) > static_cast<ZxSpectrumColor>(right);
    }

    friend constexpr bool operator>=(Ink left, Ink right) {
        return static_cast<ZxSpectrumColor>(left) >= static_cast<ZxSpectrumColor>(right);
    }

private:
    uint8_t value = 0;
};

template <uint8_t ColorValue>
constexpr Ink::Intrinsic<ColorValue>::operator Ink() const {
    return Ink(ColorValue);
}

struct Paper {
    template <uint8_t ColorValue>
    struct Intrinsic {
        constexpr operator Paper() const;
        constexpr operator uint8_t() const { return static_cast<uint8_t>(ColorValue << 3); }
    };

    static constexpr Intrinsic<0> Black{};
    static constexpr Intrinsic<1> Blue{};
    static constexpr Intrinsic<2> Red{};
    static constexpr Intrinsic<3> Magenta{};
    static constexpr Intrinsic<4> Green{};
    static constexpr Intrinsic<5> Cyan{};
    static constexpr Intrinsic<6> Yellow{};
    static constexpr Intrinsic<7> White{};

    constexpr Paper() = default;
    constexpr Paper(uint8_t encoded) : value(encoded & 0x38) {}
    constexpr Paper(ZxSpectrumColor color)
        : value(static_cast<uint8_t>((color.value & 0x07) << 3)) {}

    constexpr operator ZxSpectrumColor() const { return ZxSpectrumColor(value >> 3); }
    constexpr operator uint8_t() const { return value; }

    constexpr Paper& operator=(ZxSpectrumColor color) {
        value = static_cast<uint8_t>((color.value & 0x07) << 3);
        return *this;
    }

    friend constexpr bool operator==(Paper left, Paper right) {
        return static_cast<ZxSpectrumColor>(left) == static_cast<ZxSpectrumColor>(right);
    }

    friend constexpr bool operator!=(Paper left, Paper right) {
        return static_cast<ZxSpectrumColor>(left) != static_cast<ZxSpectrumColor>(right);
    }

    friend constexpr bool operator<(Paper left, Paper right) {
        return static_cast<ZxSpectrumColor>(left) < static_cast<ZxSpectrumColor>(right);
    }

    friend constexpr bool operator<=(Paper left, Paper right) {
        return static_cast<ZxSpectrumColor>(left) <= static_cast<ZxSpectrumColor>(right);
    }

    friend constexpr bool operator>(Paper left, Paper right) {
        return static_cast<ZxSpectrumColor>(left) > static_cast<ZxSpectrumColor>(right);
    }

    friend constexpr bool operator>=(Paper left, Paper right) {
        return static_cast<ZxSpectrumColor>(left) >= static_cast<ZxSpectrumColor>(right);
    }

private:
    uint8_t value = 0;
};

template <uint8_t ColorValue>
constexpr Paper::Intrinsic<ColorValue>::operator Paper() const {
    return Paper(static_cast<uint8_t>(ColorValue << 3));
}

struct Bright {
    template <bool Enabled>
    struct Intrinsic {
        constexpr operator Bright() const;
        constexpr operator uint8_t() const { return Enabled ? 0x40 : 0; }
        constexpr operator bool() const { return Enabled; }
    };

    static constexpr Intrinsic<true> On{};
    static constexpr Intrinsic<false> Off{};

    constexpr Bright() = default;
    constexpr Bright(bool enabled) : value(enabled ? 0x40 : 0) {}
    constexpr Bright(uint8_t encoded) : value(encoded & 0x40) {}

    constexpr operator uint8_t() const { return value; }
    constexpr operator bool() const { return value != 0; }

private:
    uint8_t value = 0;
};

template <bool Enabled>
constexpr Bright::Intrinsic<Enabled>::operator Bright() const {
    return Bright(Enabled);
}

struct Flash {
    template <bool Enabled>
    struct Intrinsic {
        constexpr operator Flash() const;
        constexpr operator uint8_t() const { return Enabled ? 0x80 : 0; }
        constexpr operator bool() const { return Enabled; }
    };

    static constexpr Intrinsic<true> On{};
    static constexpr Intrinsic<false> Off{};

    constexpr Flash() = default;
    constexpr Flash(bool enabled) : value(enabled ? 0x80 : 0) {}
    constexpr Flash(uint8_t encoded) : value(encoded & 0x80) {}

    constexpr operator uint8_t() const { return value; }
    constexpr operator bool() const { return value != 0; }

private:
    uint8_t value = 0;
};

template <bool Enabled>
constexpr Flash::Intrinsic<Enabled>::operator Flash() const {
    return Flash(Enabled);
}

class ZxSpectrumAttribute {
public:
    constexpr ZxSpectrumAttribute() = default;
    constexpr ZxSpectrumAttribute(uint8_t encoded) : value(encoded) {}
    constexpr ZxSpectrumAttribute(Ink ink);
    constexpr ZxSpectrumAttribute(Paper paper);

    constexpr operator uint8_t() const { return value; }

    constexpr ZxSpectrumAttribute& operator|=(uint8_t encoded) {
        value |= encoded;
        return *this;
    }

private:
    uint8_t value = 0;
};

constexpr ZxSpectrumAttribute::ZxSpectrumAttribute(Ink ink)
    : value(static_cast<uint8_t>(ink)) {
}

constexpr ZxSpectrumAttribute::ZxSpectrumAttribute(Paper paper)
    : value(static_cast<uint8_t>(paper)) {
}

namespace zx_spectrum_detail {

template <typename T>
struct IsAttributeField : std::false_type {};

template <>
struct IsAttributeField<Ink> : std::true_type {};

template <>
struct IsAttributeField<Paper> : std::true_type {};

template <uint8_t ColorValue>
struct IsAttributeField<Ink::Intrinsic<ColorValue>> : std::true_type {};

template <uint8_t ColorValue>
struct IsAttributeField<Paper::Intrinsic<ColorValue>> : std::true_type {};

template <>
struct IsAttributeField<Bright> : std::true_type {};

template <>
struct IsAttributeField<Flash> : std::true_type {};

template <bool Enabled>
struct IsAttributeField<Bright::Intrinsic<Enabled>> : std::true_type {};

template <bool Enabled>
struct IsAttributeField<Flash::Intrinsic<Enabled>> : std::true_type {};

template <>
struct IsAttributeField<ZxSpectrumAttribute> : std::true_type {};

template <typename T>
inline constexpr bool IsAttributeFieldV = IsAttributeField<std::decay_t<T>>::value;

}

template <typename Left, typename Right,
    std::enable_if_t<zx_spectrum_detail::IsAttributeFieldV<Left> &&
        zx_spectrum_detail::IsAttributeFieldV<Right>, int> = 0>
constexpr ZxSpectrumAttribute operator|(Left left, Right right) {
    return static_cast<uint8_t>(left) | static_cast<uint8_t>(right);
}

namespace zx_spectrum_detail {

template <typename Left, typename Right>
inline constexpr bool IsColorIntegerPairV =
    (std::is_convertible_v<Left, ZxSpectrumColor> && std::is_integral_v<Right>) ||
    (std::is_integral_v<Left> && std::is_convertible_v<Right, ZxSpectrumColor>);

template <typename T>
constexpr auto ColorIntegerValue(T value) {
    if constexpr (std::is_integral_v<T>) {
        return value;
    } else {
        return static_cast<uint8_t>(static_cast<ZxSpectrumColor>(value));
    }
}

}

template <typename Left, typename Right,
    std::enable_if_t<zx_spectrum_detail::IsColorIntegerPairV<Left, Right>, int> = 0>
constexpr bool operator==(Left left, Right right) {
    return zx_spectrum_detail::ColorIntegerValue(left) == zx_spectrum_detail::ColorIntegerValue(right);
}

template <typename Left, typename Right,
    std::enable_if_t<zx_spectrum_detail::IsColorIntegerPairV<Left, Right>, int> = 0>
constexpr bool operator!=(Left left, Right right) {
    return zx_spectrum_detail::ColorIntegerValue(left) != zx_spectrum_detail::ColorIntegerValue(right);
}

template <typename Left, typename Right,
    std::enable_if_t<zx_spectrum_detail::IsColorIntegerPairV<Left, Right>, int> = 0>
constexpr bool operator<(Left left, Right right) {
    return zx_spectrum_detail::ColorIntegerValue(left) < zx_spectrum_detail::ColorIntegerValue(right);
}

template <typename Left, typename Right,
    std::enable_if_t<zx_spectrum_detail::IsColorIntegerPairV<Left, Right>, int> = 0>
constexpr bool operator<=(Left left, Right right) {
    return zx_spectrum_detail::ColorIntegerValue(left) <= zx_spectrum_detail::ColorIntegerValue(right);
}

template <typename Left, typename Right,
    std::enable_if_t<zx_spectrum_detail::IsColorIntegerPairV<Left, Right>, int> = 0>
constexpr bool operator>(Left left, Right right) {
    return zx_spectrum_detail::ColorIntegerValue(left) > zx_spectrum_detail::ColorIntegerValue(right);
}

template <typename Left, typename Right,
    std::enable_if_t<zx_spectrum_detail::IsColorIntegerPairV<Left, Right>, int> = 0>
constexpr bool operator>=(Left left, Right right) {
    return zx_spectrum_detail::ColorIntegerValue(left) >= zx_spectrum_detail::ColorIntegerValue(right);
}

static_assert(!std::is_constructible_v<ZxSpectrumColor, uint8_t>);

#endif // ZX_SPECTRUM_ATTRIBUTE_HPP
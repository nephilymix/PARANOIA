#ifndef CRYPTER_HPP
#define CRYPTER_HPP
#include <array>
#include <cstdint>
#include <type_traits>

namespace ecry {

    constexpr uint64_t TG_MULTIPLIER = 6364136223846793005ULL;
    constexpr uint64_t TG_INCREMENT = 1442695040888963407ULL;

    __forceinline constexpr uint64_t next_random(uint64_t last) {
        return last * TG_MULTIPLIER + TG_INCREMENT;
    }

    template <typename T, size_t Size, uint64_t Seed>
    class XorStr {
    public:
        // Encrypt string at compile time
        __forceinline constexpr XorStr(const T(&str)[Size]) {
            uint64_t key = Seed;
            for (size_t i = 0; i < Size; i++) {
                key = next_random(key);
                storage_[i] = str[i] ^ static_cast<T>(key);
            }
        }

        // Decrypt string at runtime directly on the stack
        __forceinline T* get() {
            uint64_t key = Seed;
            for (size_t i = 0; i < Size; i++) {
                key = next_random(key);
                storage_[i] ^= static_cast<T>(key);
            }
            return storage_.data();
        }

        __forceinline constexpr size_t size() const {
            return Size;
        }

    private:
        std::array<T, Size> storage_{};
    };
}

#define ECRY_SEED ((__TIME__[7] - '0') * 1ull    + (__TIME__[6] - '0') * 10ull  + \
                   (__TIME__[4] - '0') * 60ull   + (__TIME__[3] - '0') * 600ull + \
                   (__LINE__ * 10000ull) + (__COUNTER__ * 100000ull))

// Macro forces compile-time encryption via constexpr, 
// then returns a temporary copy that decrypts itself on the stack.
#define ecrypt(str) ([]() { \
    constexpr auto crypted = ecry::XorStr< \
        std::remove_const_t<std::remove_reference_t<decltype(str[0])>>, \
        sizeof(str) / sizeof(str[0]), \
        ECRY_SEED \
    >(str); \
    return crypted; \
}().get())

#endif // !CRYPTER_HPP
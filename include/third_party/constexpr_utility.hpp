// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// SPDX-License-Identifier: MIT
// Copyright (c) 2019-2020 Daniil Goncharov <neargye@gmail.com>, Karaev Alexander <akaraevz@mail.ru>.
//
// Permission is hereby  granted, free of charge, to any  person obtaining a copy
// of this software and associated  documentation files (the "Software"), to deal
// in the Software  without restriction, including without  limitation the rights
// to  use, copy,  modify, merge,  publish, distribute,  sublicense, and/or  sell
// copies  of  the Software,  and  to  permit persons  to  whom  the Software  is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE  IS PROVIDED "AS  IS", WITHOUT WARRANTY  OF ANY KIND,  EXPRESS OR
// IMPLIED,  INCLUDING BUT  NOT  LIMITED TO  THE  WARRANTIES OF  MERCHANTABILITY,
// FITNESS FOR  A PARTICULAR PURPOSE AND  NONINFRINGEMENT. IN NO EVENT  SHALL THE
// AUTHORS  OR COPYRIGHT  HOLDERS  BE  LIABLE FOR  ANY  CLAIM,  DAMAGES OR  OTHER
// LIABILITY, WHETHER IN AN ACTION OF  CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE  OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <cstddef>
#include <cstring>

namespace third_party {

inline constexpr bool is_constant_evaluated() noexcept {
#if defined(__cpp_lib_is_constant_evaluated)
    return __builtin_is_constant_evaluated();
#else
    return true;
#endif
}

template <class _RawTy>
inline constexpr void trivial_copy(_RawTy* dest, const _RawTy* src, std::size_t count) {
    static_assert(std::is_trivial_v<_RawTy>);
    if (is_constant_evaluated()) {
#if defined(__clang__)
        static_cast<void>(__builtin_memcpy(dest, src, count));
#else
        if (dest != nullptr && src != nullptr && count != 0) {
            count /= sizeof(_RawTy);
            for (std::size_t i = 0; i < count; ++i) {
                dest[i] = src[i];
            }
        }
#endif
    } else {
        static_cast<void>(std::memcpy(dest, src, count));
    }
}

template <class _RawTy>
inline constexpr void trivial_move(_RawTy* dest, const _RawTy* src, std::size_t count) {
    static_assert(std::is_trivial_v<_RawTy>);
    if (is_constant_evaluated()) {
#if defined(__clang__)
        static_cast<void>(__builtin_memmove(dest, src, count));
#else
    if (dest != nullptr && src != nullptr && count != 0) {
        _RawTy* d = dest;
        count /= sizeof(_RawTy);
        if (dest > src) {
            const _RawTy* s = src + count;
            d += count;
            for (; count; --count) {
                *(--d) = *(--s);
            }
        } else if (dest < src) {
            for (; count; --count) {
                *(d++) = *(src++);
            }
        }
    }
#endif
    } else {
        static_cast<void>(std::memmove(dest, src, count));
    }
}

template <class _RawTy>
inline constexpr void trivial_fill(_RawTy* dest, _RawTy value, std::size_t count) {
    static_assert(std::is_trivial_v<_RawTy>);
    if (is_constant_evaluated()) {
        count /= sizeof(_RawTy);
        for (std::size_t i = 0; i < count; ++i) {
            dest[i] = value;
        }
    } else {
        static_cast<void>(std::memset(dest, static_cast<int>(value), count));
    }
}

} // namespace third_party

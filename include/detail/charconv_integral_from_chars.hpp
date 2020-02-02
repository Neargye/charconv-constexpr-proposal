// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// SPDX-License-Identifier: MIT
// Copyright (c) 2019 Daniil Goncharov <neargye@gmail.com>.
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

// Copyright (c) Microsoft Corporation.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// Changes:
// * add constexpr modifiers to '_Integer_from_chars' and remove '_NODISCARD'
// * add default initialize '_Risky_val' and '_Max_digit'
// * add static_assert(std::is_integral_v<_RawTy>)

#pragma once

#include <cstddef> // std::ptrdiff_t
#include <type_traits> // std::is_integral_v, std::is_signed_v, std::make_unsigned_t
#include "charconv_entity.hpp" // from_chars_result
#include "charconv_detail.hpp" // _Digit_from_char

namespace nstd {

template <class _RawTy>
constexpr from_chars_result _Integer_from_chars(const char* const _First, const char* const _Last, _RawTy& _Raw_value, const int _Base) noexcept {
    static_assert(std::is_integral_v<_RawTy>);
    _Adl_verify_range(_First, _Last);
    _STL_ASSERT(_Base >= 2 && _Base <= 36, "invalid base in from_chars()");

    bool _Minus_sign = false;

    const char* _Next = _First;

    if constexpr (std::is_signed_v<_RawTy>) {
        if (_Next != _Last && *_Next == '-') {
            _Minus_sign = true;
            ++_Next;
        }
    }

    using _Unsigned = std::make_unsigned_t<_RawTy>;

	// [smertig] gcc-9 warning: variable XXX set but not used
	[[maybe_unused]] constexpr _Unsigned _Uint_max    = static_cast<_Unsigned>(-1);
	[[maybe_unused]] constexpr _Unsigned _Int_max     = static_cast<_Unsigned>(_Uint_max >> 1);
	[[maybe_unused]] constexpr _Unsigned _Abs_int_min = static_cast<_Unsigned>(_Int_max + 1);

    _Unsigned _Risky_val = {}; //[neargye] default initialize for constexpr context. P1331 fix this?
    _Unsigned _Max_digit = {}; //[neargye] default initialize for constexpr context. P1331 fix this?

    if constexpr (std::is_signed_v<_RawTy>) {
        if (_Minus_sign) {
            _Risky_val = static_cast<_Unsigned>(_Abs_int_min / _Base);
            _Max_digit = static_cast<_Unsigned>(_Abs_int_min % _Base);
        } else {
            _Risky_val = static_cast<_Unsigned>(_Int_max / _Base);
            _Max_digit = static_cast<_Unsigned>(_Int_max % _Base);
        }
    } else {
        _Risky_val = static_cast<_Unsigned>(_Uint_max / _Base);
        _Max_digit = static_cast<_Unsigned>(_Uint_max % _Base);
    }

    _Unsigned _Value = 0;

    bool _Overflowed = false;

    for (; _Next != _Last; ++_Next) {
        const unsigned char _Digit = _Digit_from_char(*_Next);

        if (_Digit >= _Base) {
            break;
        }

        if (_Value < _Risky_val // never overflows
            || (_Value == _Risky_val && _Digit <= _Max_digit)) { // overflows for certain digits
            _Value = static_cast<_Unsigned>(_Value * _Base + _Digit);
        } else { // _Value > _Risky_val always overflows
            _Overflowed = true; // keep going, _Next still needs to be updated, _Value is now irrelevant
        }
    }

    if (_Next - _First == static_cast<std::ptrdiff_t>(_Minus_sign)) {
        return {_First, std::errc::invalid_argument};
    }

    if (_Overflowed) {
        return {_Next, std::errc::result_out_of_range};
    }

    if constexpr (std::is_signed_v<_RawTy>) {
        if (_Minus_sign) {
            _Value = static_cast<_Unsigned>(0 - _Value);
        }
    }

    _Raw_value = static_cast<_RawTy>(_Value); // implementation-defined for negative, N4713 7.8 [conv.integral]/3

    return {_Next, std::errc{}};
}

} // namespace nstd

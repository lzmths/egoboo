//********************************************************************************************
//*
//*    This file is part of Egoboo.
//*
//*    Egoboo is free software: you can redistribute it and/or modify it
//*    under the terms of the GNU General Public License as published by
//*    the Free Software Foundation, either version 3 of the License, or
//*    (at your option) any later version.
//*
//*    Egoboo is distributed in the hope that it will be useful, but
//*    WITHOUT ANY WARRANTY; without even the implied warranty of
//*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//*    General Public License for more details.
//*
//*    You should have received a copy of the GNU General Public License
//*    along with Egoboo.  If not, see <http://www.gnu.org/licenses/>.
//*
//********************************************************************************************

/// @file  egolib/math/Colour3.h
/// @brief Colours in real-valued, normalized RGB space.

#pragma once

#include <stdexcept>

namespace Ego {
	namespace Math {

		/**
		 * @brief
		 *	A colour in real-valued, normalized RGB space
		 * @author
		 *	Michael Heilmann
		 */
		struct Colour3f
		{
			/**
			 * @brief
			 *	The red component.
			 * @invariant
			 *	0.0f <= r <= 1.0f
			 */
			float _r;
			/**
			 * @brief
			 *	The green component.
			 * @invariant
			 *	0.0f <= r <= 1.0f
			 */
			float _g;
			/**
			 * @brief
			 *	The blue component.
			 * @invariant
			 *	0.0f <= r <= 1.0f
			 */
			float _b;

			/**
			 * @brief
			 *	Create a colour.
			 * @param other
			 *	the other colour
			 */
			Colour3f(const Colour3f& other)
				: _r(other._r),
				_g(other._g),
				_b(other._b)
			{
			}

			/**
			 * @brief
			 *	Create a color.
			 * @param r
			 *	the component value of the red component
			 * @param g
			 *	the component value of the green component
			 * @param b
			 *	the component value of the blue component
			 * @throws std::domain_error
			 *	if @a a, @a g or @a b a are not within the range of 0 (inclusive) and 1 (inclusive)
			 */
			Colour3f(float r, float g, float b)
			{
				if (r < 0.0f || r > 1.0f) {
					throw std::domain_error("red component outside bounds");
				}
				if (g < 0.0f || g > 1.0f) {
					throw std::domain_error("green component outside bounds");
				}
				if (b < 0.0f || b > 1.0f) {
					throw std::domain_error("blue component outside bounds");
				}
				_r = r;
				_g = g;
				_b = b;
			}

			/**
			 * @brief
			 *	Invert this colour value.
			 * @return
			 *	the inverted colour
			 * @remark
			 *	Given a colour  \f$(r,g,b)\f$ in real-valued, normalized RGB space,
			 *	then corresponding inverted colour is \f$(1-r,1-g,1-b)\f$. Inverting
			 *	a colour twice yields the same colour (modula floating-point precision).
			 * @remark
			 *	The corresponding inverted colour is also known as the complementary colour.
			 */
			Colour3f invert() const
			{
				return Colour3f(1.0f - _r, 1.0f - _g, 1.0f - _b);
			}
		};

	};
};
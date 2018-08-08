/*		 
 * Sux: Succinct data structures
 *
 * Copyright (C) 2007-2013 Sebastiano Vigna 
 *
 *  This library is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU Lesser General Public License as published by the Free
 *  Software Foundation; either version 3 of the License, or (at your option)
 *  any later version.
 *
 *  This library is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 *  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 *  for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <cassert>
#include <cstring>
#include "rs9/rank9.h"

rank9::rank9() : counts(nullptr) {}

rank9& rank9::operator=(rank9&& other) {
  bits = other.bits;
  num_words = other.num_words;
  num_counts = other.num_words;
  counts = other.counts;
  other.counts = nullptr;
  return *this;
}

rank9::rank9( const uint64_t * const bits, const uint64_t num_bits ) {
	this->bits = bits;
	num_words = ( num_bits + 63 ) / 64;
	num_counts = ( ( num_bits + 64 * 8 - 1 ) / ( 64 * 8 ) ) * 2;

  // Init rank structure
	counts = new uint64_t[ num_counts + 1 ]();

	uint64_t c = 0;
	uint64_t pos = 0;
	for( uint64_t i = 0; i < num_words; i += 8, pos += 2 ) {
		counts[ pos ] = c;
		c += __builtin_popcountll( bits[ i ] );
		for( int j = 1;  j < 8; j++ ) {
			counts[ pos + 1 ] |= ( c - counts[ pos ] ) << 9 * ( j - 1 );
			if ( i + j < num_words ) c += __builtin_popcountll( bits[ i + j ] );
		}
	}

	counts[ num_counts ] = c;

	assert( c <= num_bits );
}

rank9::~rank9() {
	if (counts) { delete [] counts; }
}


uint64_t rank9::rank( const uint64_t k ) {
	const uint64_t word = k / 64;
	const uint64_t block = word / 4 & ~1;
	const int offset = word % 8 - 1;
	return counts[ block ] + ( counts[ block + 1 ] >> ( offset + ( offset >> sizeof offset * 8 - 4 & 0x8 ) ) * 9 & 0x1FF ) + __builtin_popcountll( bits[ word ] & ( ( 1ULL << k % 64 ) - 1 ) );
}

uint64_t rank9::bit_count() {
	return num_counts * 64;
}

void rank9::print_counts() {}

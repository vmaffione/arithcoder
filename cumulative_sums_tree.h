/*
 *  Cumulative sums tree data structure
 *
 *  Copyright (C) 2012  Vincenzo Maffione
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

struct CST;
struct CST* CST_create(int num_codepoints);
void CST_destroy(struct CST* CST_p);
uint64_t cumulative_sums_lookup(struct CST* CST_p, unsigned int symbol);
void counter_increment(struct CST* CST_p, unsigned int symbol);

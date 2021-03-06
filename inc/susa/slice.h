/*
 * This file is part of Susa.

 * Susa is free software: you can redistribute it and/or modify
 * it under the terms of the Lesser GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * at your option) any later version.

 * Susa is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * Lesser GNU General Public License for more details.

 * You should have received a copy of the Lesser GNU General Public License
 * along with Susa.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file slice.h
 * @brief Slice type (definition and declation).
 *
 * A slice is a part of a matrix without copying its memory
 * content.
 *
 * @author Behrooz Kamary
 */

#ifndef SUSA_SLICE_H
#define SUSA_SLICE_H

#include <stdexcept>

namespace susa
{

/**
 * @brief slice_en is an enumeration of slicing options that is passed to slice constructor.
 */
enum class slice_en
{
    DAGGER      = 1001,
    UPPER_LEFT  = 2001,
    UPPER_RIGHT = 2002,
    LOWER_LEFT  = 2003,
    LOWER_RIGHT = 2004
};

template <typename T, slice_en M, typename Allocator> class slice;

template <typename T, slice_en M, typename Allocator> std::ostream& operator<<(std::ostream&, const slice <T, M, Allocator>&);

/**
 * @class slice
 * @brief <i>slice</i> type represents part of a <i>matrix</i> which its
 * reference is passed to slice's constructor at instanciation time without
 * performing a deep copy i.e. memory allocation.
 *
 * @ingroup TYPES
 *
 */
template <typename T, slice_en M = slice_en::DAGGER, typename Allocator = std::allocator <T>>
class slice
{

private:

  slice();

  matrix <T, Allocator>&  matx;
  size_t                  sizet_irow;
  size_t                  sizet_icol;
  size_t                  sizet_rows;
  size_t                  sizet_cols;
  size_t                  sizet_objects;

public:

  slice(const slice <T, M, Allocator>&) = delete;
  void operator=(const slice <T, M, Allocator>&) = delete;

  //! constructor
  slice(matrix<T, Allocator>& mat_arg, size_t sizet_row, size_t sizet_col)
  : matx(mat_arg)
  , sizet_irow(sizet_row)
  , sizet_icol(sizet_col)
  {

    if (M == slice_en::DAGGER)
    {
      sizet_rows    = matx.no_rows() - 1;
      sizet_cols    = matx.no_cols() - 1;
    }
    else if (M == slice_en::UPPER_LEFT)
    {
      sizet_rows    = sizet_row;
      sizet_cols    = sizet_col;
    }
    else if (M == slice_en::UPPER_RIGHT)
    {
      sizet_rows    = sizet_row;
      sizet_cols    = matx.no_cols()- sizet_col + 1;
    }
    else if (M == slice_en::LOWER_LEFT)
    {
      sizet_rows    = matx.no_rows() - sizet_row + 1;
      sizet_cols    = sizet_col;
    }
    else if (M == slice_en::LOWER_RIGHT)
    {
      sizet_rows    = matx.no_rows() - sizet_row + 1;
      sizet_cols    = matx.no_cols() - sizet_col + 1;
    }

    sizet_objects = sizet_rows * sizet_cols;
  }

  size_t size() const
  {
      return sizet_objects;
  }

  //! returns the value of a specific element
  T get(size_t sizet_elem) const
  {
      size_t row = sizet_elem % sizet_rows;
      size_t col = sizet_elem / sizet_rows;

      return matx.get(get_lindex(row, col));
  }

  //! returns the linear index of a specific (row, column) pair
  constexpr size_t get_lindex(size_t row, size_t col) const
  {
    if (row >= sizet_rows || col >= sizet_cols) return 0;

    if (M == slice_en::DAGGER)
    {
      if (sizet_irow <= row) row++;
      if (sizet_icol <= col) col++;
    }
    else if (M == slice_en::UPPER_LEFT)
    {
      // nothing to do
    }
    else if (M == slice_en::UPPER_RIGHT)
    {
      col += sizet_icol;
    }
    else if (M == slice_en::LOWER_LEFT)
    {
      row += sizet_irow;
    }
    else if (M == slice_en::LOWER_RIGHT)
    {
      row += sizet_irow;
      col += sizet_icol;
    }

    return (row + col * matx.no_rows());
  }

  //! () operator to set or get elements
  T& operator ()(size_t sizet_row, size_t sizet_col)
  {
    SUSA_ASSERT_MESSAGE(sizet_row < sizet_rows && sizet_col < sizet_cols, "the input arguments exceed slice size.");
    return matx.operator()(get_lindex(sizet_row, sizet_col));
  }

  //! () operator to set or get elements
  T operator ()(size_t sizet_row, size_t sizet_col) const
  {
    SUSA_ASSERT_MESSAGE(sizet_row < sizet_rows && sizet_col < sizet_cols, "the input arguments exceed slice size.");
    return matx.get(get_lindex(sizet_row, sizet_col));
  }

  //! () operator to set or get elements
  T& operator ()( size_t sizet_elem)
  {
    size_t row = sizet_elem % sizet_rows;
    size_t col = sizet_elem / sizet_rows;

    return matx.get(get_lindex(row, col));
  }

  //! () operator to set or get elements
  T operator ()( size_t sizet_elem) const
  {
    return get(sizet_elem);
  }

  template <typename L, typename R>
  friend bool operator==(const L& arg_left, const R& arg_right)
  {
    static_assert(std::is_same<L,matrix<T,Allocator>>::value || std::is_same<L,slice<T, M, Allocator>>::value);
    static_assert(std::is_same<R,matrix<T,Allocator>>::value || std::is_same<R,slice<T, M, Allocator>>::value);

    size_t sz_left  = arg_left.size();
    size_t sz_right = arg_right.size();

    if (sz_left != sz_right) return false;

    for (size_t indx = 0; indx < sz_left; indx++)
    {
      if (arg_left(indx) != arg_right(indx)) return false;
    }
    return true;
  }

  //! output stream
  friend std::ostream &operator<< <>(std::ostream& out_stream, const slice <T, M, Allocator>& slc_arg);

};

template <typename T, slice_en M, typename Allocator>
std::ostream &operator<<(std::ostream& out_stream, const slice<T, M, Allocator>& slc_arg)
{
  out_stream << "[";
  for (size_t sizet_row = 0; sizet_row < slc_arg.no_rows(); sizet_row++)
  {
    for (size_t sizet_col = 0; sizet_col < slc_arg.no_cols(); sizet_col++)
    {
      out_stream << slc_arg(sizet_row, sizet_col);
      if (sizet_col < slc_arg.no_cols() - 1)
        out_stream << " ";
    }
    if (sizet_row < slc_arg.no_rows() - 1)
      out_stream << "\n ";
  }
  out_stream << "]";
  return out_stream;
}

} // NAMESPACE SUSA
#endif  // SUSA_SLICE_H

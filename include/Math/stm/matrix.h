#ifndef STM_MATRIX_H
#define STM_MATRIX_H

#include "common.h"
#include "comparison.h"
#include "ranges.h"

namespace stm
{
	template<Number T, std::size_t Rows, std::size_t Columns>
	class matrix
	{
	public:
		using value_type = T;
		using underlying_num_type = underlying_num_t<T>;

		constexpr matrix() noexcept = default;
		constexpr matrix(const matrix&) noexcept = default;
		constexpr matrix(matrix&&) noexcept = default;
		constexpr matrix& operator=(const matrix&) noexcept = default;
		constexpr matrix& operator=(matrix&&) noexcept = default;
		~matrix() noexcept = default;

		constexpr matrix(const T* data) noexcept
		{
			std::copy(data, data + size(), data_);
		}

		constexpr static std::size_t rows() noexcept { return Rows; }
		constexpr static std::size_t columns() noexcept { return Columns; }
		constexpr static std::size_t size() noexcept { return Rows * Columns; }
		constexpr auto data() const noexcept { return data_; }

		constexpr static matrix diagonal(const vector<T, min_of(Rows, Columns)>& diagonal) noexcept
		{
			matrix out;
			for (auto i : up_to(min_of(Rows, Columns)))
				out[i][i] = diagonal[i];

			return out;
		}

		constexpr T* operator[](std::size_t row) noexcept { return &data_[row * Columns]; }
		constexpr const T* operator[](std::size_t row) const noexcept { return &data_[row * Columns]; }

		constexpr T& at(std::size_t row, std::size_t column)
		{
			if (row >= Rows) intern::out_of_bounds_except(row, Rows);
			if (column >= Columns) intern::out_of_bounds_except(column, Columns);

			return (*this)[row][column];
		}

		constexpr const T& at(std::size_t row, std::size_t column) const
		{
			if (row >= Rows) intern::out_of_bounds_except(row, Rows);
			if (column >= Columns) intern::out_of_bounds_except(column, Columns);

			return (*this)[row][column];
		}

		constexpr matrix<T, Columns, Rows> transpose() const noexcept
		{
			matrix<T, Columns, Rows> out;
			for (auto i : up_to(Rows))
			{
				for(auto j : up_to(Columns))
				{
					out[j][i] = (*this)[i][j];
				}
			}
			return out;
		}

	private:
		T data_[Rows * Columns]{};
	};

	template<Number T, std::size_t N>
	using sqmatrix = matrix<T, N, N>;

	template<Number T, std::size_t Rows, std::size_t Columns>
	constexpr matrix<T, Rows, Columns> eye() noexcept
	{
		matrix<T, Rows, Columns> out;
		for (auto i : up_to(min_of(Rows, Columns)))
			out[i][i] = T{1};

		return out;
	}

	template<Number T, std::size_t N>
	constexpr sqmatrix<T, N> identity() { return eye<T, N, N>(); }

	template<Number T, std::size_t M, std::size_t N, std::size_t K>
	constexpr matrix<T, M, K> matmul(const matrix<T, M, N>& lhs, const matrix<T, N, K>& rhs) noexcept
	{
		matrix<T, M, K> res;
		for (auto i : up_to(M))
		{
			for (auto j : up_to(N))
			{
				for (auto k : up_to(K))
				{
					res[i][k] += lhs[i][j] * rhs[j][k];
				}
			}
		}
		return res;
	}

	using mat4f = matrix<float, 4, 4>;
	using mat3f = matrix<float, 3, 3>;
	using mat2f = matrix<float, 2, 2>;
}

#endif /* STM_MATRIX_H */
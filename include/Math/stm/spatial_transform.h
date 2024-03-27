#pragma once

#include "matrix.h"
#include "quaternion.h"
#include "vector3.h"

namespace stm
{
    template<Float T>
    constexpr sqmatrix<T, 4> scale(const vector<T, 3>& scaleFactor) noexcept
	{
		sqmatrix<T, 4> temp;
		temp[0][0] = scaleFactor.x;
		temp[1][1] = scaleFactor.y;
		temp[2][2] = scaleFactor.z;
		temp[3][3] = 1;

		return temp;
	}

    template<Float T>
	constexpr sqmatrix<T, 4> scale(T xScale, T yScale, T zScale) noexcept
	{
		sqmatrix<T, 4> temp;
		temp[0][0] = xScale;
		temp[1][1] = yScale;
		temp[2][2] = zScale;
		temp[3][3] = 1;
		
		return temp;
	}

    template<Float T>
	constexpr sqmatrix<T, 4> translate(const vector<T, 3>& translateFactor) noexcept
	{
		sqmatrix<T, 4> temp = identity<float, 4>();
		temp[0][3] = translateFactor.x;
		temp[1][3] = translateFactor.y;
		temp[2][3] = translateFactor.z;
        temp[3][3] = 1;
		
        return temp;
	}

    template<Float T>
	constexpr sqmatrix<T, 4> translate(T xTrans, T yTrans, T zTrans) noexcept
	{
		sqmatrix<T, 4> temp = identity<float, 4>();
		temp[0][3] = xTrans;
		temp[1][3] = yTrans;
		temp[2][3] = zTrans;
        temp[3][3] = 1;
		
        return temp;
	}

    template<Float T>
	constexpr sqmatrix<T, 4> rotateX(T angleInRads) noexcept
	{
		sqmatrix<T, 4> temp = identity<float, 4>();
		temp[1][1] =  stm::cos(angleInRads);
		temp[1][2] = -stm::sin(angleInRads);
		temp[2][1] =  stm::sin(angleInRads);
		temp[2][2] =  stm::cos(angleInRads);
        temp[3][3] = 1.0f;
		
        return temp;
	}

    template<Float T>
	constexpr sqmatrix<T, 4> rotateY(T angleInRads) noexcept
	{
		sqmatrix<T, 4> temp = identity<float, 4>();
		temp[0][0] =  stm::cos(angleInRads);
		temp[0][2] =  stm::sin(angleInRads);
		temp[2][0] = -stm::sin(angleInRads);
		temp[2][2] =  stm::cos(angleInRads);
        temp[3][3] = 1;
		
        return temp;
	}

    template<Float T>
	constexpr sqmatrix<T, 4> rotateZ(T angleInRads) noexcept
	{
		sqmatrix<T, 4> temp = identity<float, 4>();
		temp[0][0] =  stm::cos(angleInRads);
		temp[0][1] = -stm::sin(angleInRads);
		temp[1][0] =  stm::sin(angleInRads);
		temp[1][1] =  stm::cos(angleInRads);
        temp[3][3] = 1.0f;
		
        return temp;
	}

    template<Float T>
	constexpr sqmatrix<T, 4> rotate(const vector<T, 3>& axis, T angleInRads) noexcept
	{
		sqmatrix<T, 4> temp;
		const auto sinA = stm::sin(angleInRads);
		const auto cosA = stm::cos(angleInRads);
		temp[0][0] = cosA + ((1.f - cosA) * (axis.x * axis.x));
		temp[0][1] = (axis.x * axis.y * (1 - cosA)) - (axis.z * sinA);
		temp[0][2] = (axis.x * axis.z * (1 - cosA)) + (axis.y * sinA);
		temp[1][0] = (axis.x * axis.y * (1 - cosA)) + (axis.z * sinA);
		temp[1][1] = cosA + ((1.f - cosA) * (axis.y * axis.y));
		temp[1][2] = (axis.y * axis.z * (1 - cosA)) - (axis.x * sinA);
		temp[2][0] = (axis.x * axis.z * (1 - cosA)) - (axis.y * sinA);
		temp[2][1] = (axis.y * axis.z * (1 - cosA)) + (axis.x * sinA);
		temp[2][2] = cosA + ((1.f - cosA) * (axis.z * axis.z));
		temp[3][3] = 1.0f;

		return temp;
	}

    template<Float T>
    constexpr vector<T, 3> rotate(const vector<T, 3>& vec, const vector<T, 3>& axis, T angle) noexcept
    {
        quaternion<T> func{ std::cos(angle / 2.0f), axis * std::sin(angle / 2.0f) };
        quaternion<T> out{ 0 , vec };
        out = (func * out) * func.inverse();
        return out.imaginary();
    }
	
    template<Float T>
    constexpr sqmatrix<T, 4> orthographic(T xleft, T xright, T ybottom, T ytop, T znear, T zfar) noexcept
	{
		sqmatrix<T, 4> temp;

		temp[0][0] = T{2} / (xright - xleft);
		temp[1][1] = T{2} / (ytop - ybottom);
		temp[2][2] = T{-2} / (zfar - znear);
		temp[0][3] = -(xright + xleft) / (xright - xleft);
		temp[1][3] = -(ytop + ybottom) / (ytop - ybottom);
		temp[2][3] = -(zfar + znear) / (zfar - znear);
		temp[3][3] = T{1};

		return temp;
	}

	template<Float T>
    constexpr sqmatrix<T, 4> perspective(T FOVRads, T aspectRatio, T zNear, T zFar) noexcept
	{
		sqmatrix<T, 4> temp;
		temp[0][0] = T{1} / (aspectRatio * stm::tan(FOVRads / T{2}));
		temp[1][1] = T{1} / stm::tan(FOVRads / T{2});
		temp[2][2] = zFar / (zFar - zNear);
        temp[3][2] = T{1};
        temp[2][3] = -zFar * zNear / (zFar - zNear);

		return temp;
	}

    template<Float T>
    constexpr sqmatrix<T, 4> lookAt(const vector<T, 3>& position,
                                    const vector<T, 3>& up,
                                    const vector<T, 3>& right) noexcept
    {
        auto direction = cross(up, right).unit();
        auto space = identity<float, 4>();
        
        space[0][0] = right.x;
        space[0][1] = right.y;
        space[0][2] = right.z;
        space[1][0] = up.x;
        space[1][1] = up.y;
        space[1][2] = up.z;
        space[2][0] = direction.x;
        space[2][1] = direction.y;
        space[2][2] = direction.z;
        
        return matmul(space, translate(-position));
    }

    template<Float T>
	constexpr sqmatrix<T, 4> lookAt(const vector<T, 3>& position,
                                    const vector<T, 3>& up,
                                    const vector<T, 3>& right,
                                    const vector<T, 3>& direction) noexcept
	{
		auto space = identity<float, 4>();
    
		space[0][0] = right.x;
		space[0][1] = right.y;
		space[0][2] = right.z;
		space[1][0] = up.x;
		space[1][1] = up.y;
		space[1][2] = up.z;
		space[2][0] = direction.x;
		space[2][1] = direction.y;
		space[2][2] = direction.z;
    
		return matmul(space, translate(-position));
	}
}
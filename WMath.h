/*
 * Copyright (c) 2014-2023, NVIDIA CORPORATION.  All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * SPDX-FileCopyrightText: Copyright (c) 2014-2021 NVIDIA CORPORATION
 * SPDX-License-Identifier: Apache-2.0
 */

/* Contact chebert@nvidia.com (Chris Hebert) for feedback */

#ifndef __H_WMATH_
#define __H_WMATH_

#pragma once

#include <float.h>
#include <math.h>
#include <stdint.h>
#include <vector>

template <typename T, int N>
struct Vec
{

  T m_data[N];

  /*Vec(){
		for (uint32_t i = 0; i < N; ++i){
			m_data[i] = 0.0;
		}
	}

	Vec(const Vec &inCopy){
		for (uint32_t i = 0; i < N; ++i){
			m_data[i] = inCopy.m_data[i];
		}
	}

	Vec<T, N> &operator=(const Vec<T, N> &rhs) const{
		Vec<T, N> out;
		for (uint32_t i = 0; i < N; ++i){
			out.m_data[i] = m_data[i];
		}
		return out;
	}*/

  Vec<T, N> operator+=(const Vec<T, N>& rhs)
  {
    for(int i = 0; i < N - 1; ++i)
    {
      m_data[i] = m_data[i] + rhs.m_data[i];
    }

    return *this;
  }

  Vec<T, N> operator-=(const Vec<T, N>& rhs)
  {
    for(int i = 0; i < N - 1; ++i)
    {
      m_data[i] = m_data[i] - rhs.m_data[i];
    }

    return *this;
  }

  Vec<T, N> operator+(const Vec<T, N>& rhs)
  {
    Vec<T, N> outVector = {0};
    for(int i = 0; i < N - 1; ++i)
    {
      outVector.m_data[i] = m_data[i] + rhs.m_data[i];
    }

    outVector.m_data[N - 1] = 1.0;
    return outVector;
  }

  T lengthSquared()
  {
    T out = 0;
    for(int i = 0; i < N - 1; ++i)
    {
      out += (m_data[i] * m_data[i]);
    }
    return out;
  }

  T length() { return sqrt(lengthSquared()); }

  void normalize()
  {
    T ln = length();
    if(ln == 0.0)
    {
      zero();
      return;
    }

    T w = m_data[N - 1];

    *this = *this / ln;
    if(N == 4)
      m_data[3] = w;
  }

  void zero()
  {
    for(int i = 0; i < N; ++i)
    {
      m_data[i] = 0;
    }
    m_data[N - 1] = 1.0;
  }

  Vec<T, N> operator-(const Vec<T, N>& rhs)
  {
    Vec<T, N> outVector = {0};
    for(int i = 0; i < N - 1; ++i)
    {
      outVector.m_data[i] = m_data[i] - rhs.m_data[i];
    }

    outVector.m_data[N - 1] = 1.0;
    return outVector;
  }

  T operator*(const Vec<T, N>& rhs)
  {
    float outValue = 0;

    for(int i = 0; i < N - 1; ++i)
    {
      outValue += m_data[i] * rhs.m_data[i];
    }

    return outValue;
  }

  Vec<T, N> operator*(const T& rhs)
  {
    Vec<T, N> out = {0};

    for(int i = 0; i < N - 1; ++i)
    {
      out.m_data[i] = m_data[i] * rhs;
    }
    out.m_data[N - 1] = 1.0;
    return out;
  }

  Vec<T, N> operator/(const T inValue)
  {
    Vec<T, N> outValue;
    if(inValue == 0.0)
    {
      outValue.zero();
      return outValue;
    }
    T inv = 1.0 / inValue;

    for(int i = 0; i < N - 1; ++i)
    {
      outValue.m_data[i] = m_data[i] * inv;
    }
    outValue.m_data[N - 1] = 1.0;

    return outValue;
  }

  Vec<T, N> operator%(const Vec<T, N>& rhs)
  {
    Vec<T, N> outVector;

    for(int i = 0; i < N - 1; ++i)
    {
      int i1 = (i + 1) % (N - 1);
      int i2 = (i + 2) % (N - 1);

      outVector.m_data[i2] = (rhs.m_data[i1] * m_data[i]) - (m_data[i1] * rhs.m_data[i]);
    }
    outVector.m_data[N - 1] = 1.0;

    return outVector;
  }

  Vec<T, N> project()
  {
    Vec<T, N> out;

    T invW = 1.0 / m_data[N - 1];

    for(uint32_t i = 0; i < N - 1; ++i)
    {
      out.m_data[i] = m_data[i] * invW;
    }

    out.m_data[N - 1] = 1;

    return out;
  }
};

typedef Vec<float, 2> Vec2f;
typedef Vec<float, 3> Vec3f;
typedef Vec<float, 4> Vec4f;


template <typename T>
struct Quaternion
{

  T i, j, k, r;

  Quaternion<T>()
  {
    i = 0.0;
    j = 0.0;
    k = 0.0;
    r = 0.0;
  }

  Quaternion<T>(const T inI, const T inJ, const T inK, const T inR)
  {
    i = inI;
    j = inJ;
    k = inK;
    r = inR;
  }

  Quaternion<T>(const Quaternion<T>& inCopy)
  {
    i = inCopy.i;
    j = inCopy.j;
    k = inCopy.k;
    r = inCopy.r;
  }

  Quaternion<T>& operator*(const Quaternion<T>& rhs)
  {
    Quaternion<T> out;

    out.r = rhs.r * r - rhs.i * i - rhs.j * j - rhs.k * k;
    out.i = rhs.r * i + rhs.i * r + rhs.j * k - rhs.k * j;
    out.j = rhs.r * j + rhs.j * r + rhs.k * i - rhs.i * k;
    out.k = rhs.r * k + rhs.k * r + rhs.i * j - rhs.j * i;

    return out;
  }

  Quaternion<T>& rotate(const Vec4f& inVector)
  {
    Quaternion<T> out = {inVector.m_data[0], inVector.m_data[1], inVector.m_data[2], 0.0};

    out = (*this) * out;

    out.normalize();

    return out;
  }

  float dot(const Quaternion<T>& rhs)
  {
    T out;


    Vec3f v0 = {i, j, k};

    Vec3f v1 = {rhs.i, rhs.j, rhs.k};

    out = (v0 * v1) + r + rhs.r;
    return out;
  }

  void normalize()
  {
    T d;

    d = dot(*this);
    if(d == 0)
    {
      r = 1;
      return;
    }

    d = 1.0 / sqrt(d);
    r = r * d;
    i = i * d;
    j = j * d;
    k = k * d;
  }
};


template <typename T, int N, int M>
struct Mat
{
  T m_data[N * M];

  T det()
  {
    T out = 0;
    for(int i = 0; i < N; ++i)
    {

      int i1 = (i + 1) % N;
      int i2 = (i + 2) % N;

      int n1 = N;
      int n2 = N + N;

      out += (((m_data[n1 + i1] * m_data[n2 + i2]) - (m_data[n1 + i2] * m_data[n2 + i1])) * m_data[i]);
    }
    return out;
  }

  T det4x4()
  {
    T out = 0;

    for(int i = 0; i < 4; ++i)
    {
      int secondRowIndices[3];
      int idx = 0;

      for(int j = 0; j < 4; ++j)
      {
        if(i == j)
          continue;
        secondRowIndices[idx++] = j;
      }
      int ad = (i % 2 == 0) ? 0 : 1;
      for(int j = 0; j < 3; ++j)
      {
        int j1 = (j + 1 + ad) % 3;
        int j2 = (j + 2 - ad) % 3;
        int n1 = N + N;
        int n2 = n1 + N;
        out += (((m_data[n1 + secondRowIndices[j1]] * m_data[n2 + secondRowIndices[j2]])
                 - (m_data[n1 + secondRowIndices[j2]] * m_data[n2 + secondRowIndices[j1]]))
                * m_data[N + secondRowIndices[j]] * m_data[i]);
      }
    }

    return out;
  }

  Mat<T, N, M>& operator*(const Mat<T, N, M>& rhs)
  {

    Mat<T, N, M> out = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    for(int y = 0; y < M; ++y)
    {

      for(int x = 0; x < N; ++x)
      {

        int idx = y * M + x;


        for(int i = 0; i < N; ++i)
        {
          out.m_data[idx] += (m_data[y * M + i] * rhs.m_data[i * N + x]);
        }
      }
    }

    return out;
  }

  Vec<T, N> operator*(const Vec<T, N>& rhs)
  {
    Vec<T, N> out = {0, 0, 0};

    for(int y = 0; y < M; ++y)
    {

      for(int i = 0; i < N; ++i)
      {
        out.m_data[y] += (m_data[y * M + i] * rhs.m_data[i]);
      }
    }

    return out;
  }

  Vec<T, N> operator()(const Vec<T, N>& rhs)
  {
    Vec<T, N> out = (*this) * rhs;
    return out;
  }

  Mat<T, N, M> operator()(const Mat<T, N, M>& rhs)
  {
    Mat<T, N, M> out = (*this) * rhs;
    return out;
  }

  Mat<T, N, M>& transpose()
  {
    Mat<T, N, M> out;

    for(int i = 0; i < N; ++i)
    {

      for(int j = 0; j < M; ++j)
      {

        out.m_data[i * N + j] = m_data[j * N + i];
      }
    }

    return out;
  }

  Mat<T, N, M>& inverse4x4()
  {
    Mat<T, N, M> out = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    int subMatrix[9];
    T   d = det4x4();

    if(d == 0.0)
      return out;

    T        invD = 1.0 / d;
    uint32_t cnt  = 0;
    for(int my = 0; my < 4; ++my)
    {
      for(int mx = 0; mx < 4; ++mx)
      {
        int sIdx = 0;
        for(int sy = 0; sy < 4; ++sy)
        {
          if(sy == my)
            continue;
          for(int sx = 0; sx < 4; ++sx)
          {
            if(sx == mx)
              continue;
            subMatrix[sIdx++] = (sy * 4 + sx);
          }
        }

        //submatrix complete.
        T     v  = 0;
        float dd = invD;
        for(int sx = 0; sx < 3; ++sx)
        {
          int x1 = (sx + 1) % 3;
          int x2 = (sx + 2) % 3;
          v += ((m_data[subMatrix[sx]] * m_data[subMatrix[3 + x1]] * m_data[subMatrix[6 + x2]])
                - (m_data[subMatrix[sx]] * m_data[subMatrix[3 + x2]] * m_data[subMatrix[6 + x1]]))
               * dd;
          //dd *= -1;
        }

        out.m_data[mx * 4 + my] = v * (((cnt % 2 == 0) || (v == 0.0)) ? 1.0 : -1.0);
        ++cnt;
      }
      ++cnt;
    }


    return out;
  }
};


typedef Mat<float, 3, 3>  Mat3x3f;
typedef Mat<float, 4, 4>  Mat4x4f;
typedef Quaternion<float> Quaternionf;

Mat4x4f identity4x4();

Mat4x4f rotate4x4(const Quaternion<float>& q);

Mat4x4f translate4x4(const Vec4f& inValue);
Mat4x4f translate4x4(float inX, float inY, float inZ);

Mat4x4f scale4x4(const Vec4f& inScale);
Mat4x4f scale4x4(const float inX, const float inY, const float inZ);


typedef Vec4f ColorRGBA;

template <typename T>
struct Plane
{
  T m_position;
  T m_normal;
};

typedef Plane<Vec4f> Plane4f;
typedef Plane<Vec3f> Plane3f;

template <typename T>
struct Sphere
{
  T m_position;
  T m_radius;
};

template <typename T>
struct QuadraticSolutions
{
  T    m_pos;
  T    m_neg;
  T    m_delta;
  bool m_has_real_solutions;
};

typedef QuadraticSolutions<float> QuadraticSolutionsf;

template <typename T>
struct Intersection
{
  float m_t;
  T     m_position;
  T     m_normal;
  T     v;
  int   m_index;
  void* m_data;
  void* m_tri;
  float m_delta;
  bool  m_found;
};

typedef Intersection<Vec3f> Intersection3f;
typedef Intersection<Vec4f> Intersection4f;

template <typename T>
QuadraticSolutions<T> quadraticSolver(T inA, T inB, T inC)
{

  QuadraticSolutions<T> outT;
  outT.m_delta = (inB * inB) - 4.0 * inA * inC;
  if(outT.m_delta < 0)
  {
    outT.m_has_real_solutions = false;
    return outT;
  }
  float ds                  = sqrt(outT.m_delta);
  outT.m_has_real_solutions = true;

  outT.m_pos = (-inB + ds) / (2.0 * inA);
  outT.m_neg = (-inB - ds) / (2.0 * inA);

  return outT;
}

template <typename T>
struct Ray
{
  T     m_position;
  T     m_direction;
  float m_min_t;
  float m_max_t;
};

typedef Ray<Vec4f> Ray4f;
typedef Ray<Vec3f> Ray3f;

template <typename T>
struct Triangle
{
  T m_vertices[3];
  T m_normals[3];
};

typedef Triangle<Vec3f>         Triangle3f;
typedef Triangle<Vec4f>         Triangle4f;
typedef std::vector<Triangle4f> TriangleList4f;

template <typename T>
inline bool intersect(Intersection<T>& inIntersection, Ray<T>& inRay, Triangle<T>& inTri)
{
  T p1;
  T p2;
  T p3;
  T e1;
  T e2;
  T s1;

  T n1, n2, n3;
  T ne1, ne2;


  float divisor;
  float invDivisor;

  T s2;

  float b1;
  float b2;
  float t;
  T     d;

  Vec2f uv0, uv1, uv2;
  Vec2f u1, u2;
  T     dp1;
  T     dp2;
  float det;
  float invDet;

  //if (!mBounds.intersect(inRay)) return;

  int i;
  int i1;

  int i2;
  int i3;

  int tCnt;
  i = 0;


  //if(!mVisible) return;


  p1 = inTri.m_vertices[0];
  p2 = inTri.m_vertices[1];
  p3 = inTri.m_vertices[2];

  n1 = inTri.m_normals[0];
  n2 = inTri.m_normals[1];
  n3 = inTri.m_normals[2];

  ne1 = n2 - n1;
  ne2 = n3 - n1;


  d = inRay.m_position - p1;
  //d.normalize();

  e1 = p2 - p1;
  e2 = p3 - p1;
  s1 = inRay.m_direction % e2;

  divisor = s1 * e1;

  if(divisor == 0.0)
    return false;

  invDivisor = 1.0 / divisor;

  b1 = (d * s1) * invDivisor;

  if(b1 < 0.0 || b1 > 1.0)
    return false;

  s2 = d % e1;
  b2 = (inRay.m_direction * s2) * invDivisor;

  if((b2 < 0.0) || (b2 > 1.0))
    return false;
  if(b1 + b2 > 1.0)
    return false;

  t = (e2 * s2) * invDivisor;

  if(t >= inRay.m_max_t - (1e-3f))
    return false;
  if(t <= (1e-3f))
    return false;
  if(t < inIntersection.m_t)
  {


    inIntersection.m_t     = t;
    inIntersection.m_found = true;


    inIntersection.m_normal = ne1 * b1 + ne2 * b2 + n1;

    inIntersection.m_position = e1 * b1 + e2 * b2 + p1;

    inIntersection.m_tri = (void*)&inTri;
    return true;
  }

  return false;
};

Mat4x4f perspectiveProjection4x4f(const float inFOV, const float inAspect, const float inNear, const float inFar);

namespace num {

template <typename T>
inline T clamp(const T inValue, const T inMin, const T inMax)
{
  return min(max(inValue, inMin), inMax);
}

}  // namespace num

namespace angle {

float d2rf(float ind);
float r2df(float inr);

}  // namespace angle

namespace basis {

Vec4f UP();
Vec4f LEFT();
Vec4f FWD();

}  // namespace basis

namespace bounds {

struct BBox
{
  Vec4f m_min{FLT_MAX, FLT_MAX, FLT_MAX, 0.0f};
  Vec4f m_max{FLT_MIN, FLT_MIN, FLT_MIN, 0.0f};

  BBox() {}

  void operator+=(const Vec4f& inV)
  {
    for(uint32_t i = 0; i < 3; ++i)
    {
      m_min.m_data[i] = fminf(m_min.m_data[i], inV.m_data[i]);
      m_max.m_data[i] = fmaxf(m_min.m_data[i], inV.m_data[i]);
    }
  }

  void operator+=(const BBox& inOther)
  {
    *this += inOther.m_min;
    *this += inOther.m_max;
  }

  Vec4f Center() { return (m_max + m_min) * 0.5; }

  Vec4f Dimensions() { return m_max - m_min; }

  float Width() { return m_max.m_data[0] - m_min.m_data[0]; }

  float Height() { return m_max.m_data[1] - m_min.m_data[1]; }

  float Depth() { return m_max.m_data[2] - m_min.m_data[2]; }
};


}  // namespace bounds

#endif

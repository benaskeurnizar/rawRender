#include <smmintrin.h>
#include <immintrin.h>
//#include <xmmintrin.h>
#include "simd_math.h"
#include <math.h>


/*
so here we try to process multiple triangles at once,my idea now to get a load of vertices at once ,for the matrixes,only the model changes,so we gonna get the vertices,store them by 8,do the model*view multiplication,send them back,then get them clipped do the proj multiplication and send them back.
for large objects with the same model,no problem should be caused right ? cause the model is the same so it wouldn't matter how many we store,we go with 8 to use the __m256.

 */

void mv8_mul_mat4_vec3(__m256 *x,__m256 *y,__m256 *z,float model[4][4]){
  __m256 m00 = _mm256_set1_ps(model[0][0]);
  __m256 m10 = _mm256_set1_ps(model[1][0]);
  __m256 m20 = _mm256_set1_ps(model[2][0]);
  __m256 m30 = _mm256_set1_ps(model[3][0]);
  
  __m256 m01 = _mm256_set1_ps(model[0][1]);
  __m256 m11 = _mm256_set1_ps(model[1][1]);
  __m256 m21 = _mm256_set1_ps(model[2][1]);
  __m256 m31 = _mm256_set1_ps(model[3][1]);

  __m256 m02 = _mm256_set1_ps(model[0][2]);
  __m256 m12 = _mm256_set1_ps(model[1][2]);
  __m256 m22 = _mm256_set1_ps(model[2][2]);
  __m256 m32 = _mm256_set1_ps(model[3][2]);
  
  // for x
  __m256 xm00 =  _mm256_mul_ps(*x,m00);
  __m256 ym10 =  _mm256_mul_ps(*y,m10);
  __m256 zm20 =  _mm256_mul_ps(*z,m20);

  // for y
  __m256 xm01 =  _mm256_mul_ps(*x,m01);
  __m256 ym11 =  _mm256_mul_ps(*y,m11);
  __m256 zm21 =  _mm256_mul_ps(*z,m21);

  __m256 xm02 =  _mm256_mul_ps(*x,m02);
  __m256 ym12 =  _mm256_mul_ps(*y,m12);
  __m256 zm22 =  _mm256_mul_ps(*z,m22);

  *x = _mm256_add_ps(_mm256_add_ps(_mm256_add_ps(ym10,zm20),xm00),m30);
  *y = _mm256_add_ps(_mm256_add_ps(_mm256_add_ps(ym11,zm21),xm01),m31);
  *z = _mm256_add_ps(_mm256_add_ps(_mm256_add_ps(ym12,zm22),xm02),m32);
}

void mv8_mul_mat4_vec3W(__m256 *x,__m256 *y,__m256 *z,__m256 *w, float model[4][4]){
  __m256 m00 = _mm256_set1_ps(model[0][0]);
  __m256 m10 = _mm256_set1_ps(model[1][0]);
  __m256 m20 = _mm256_set1_ps(model[2][0]);
  __m256 m30 = _mm256_set1_ps(model[3][0]);
  
  __m256 m01 = _mm256_set1_ps(model[0][1]);
  __m256 m11 = _mm256_set1_ps(model[1][1]);
  __m256 m21 = _mm256_set1_ps(model[2][1]);
  __m256 m31 = _mm256_set1_ps(model[3][1]);

  __m256 m02 = _mm256_set1_ps(model[0][2]);
  __m256 m12 = _mm256_set1_ps(model[1][2]);
  __m256 m22 = _mm256_set1_ps(model[2][2]);
  __m256 m32 = _mm256_set1_ps(model[3][2]);
			      
  __m256 m03 = _mm256_set1_ps(model[0][3]);
  __m256 m13 = _mm256_set1_ps(model[1][3]);
  __m256 m23 = _mm256_set1_ps(model[2][3]);
  __m256 m33 = _mm256_set1_ps(model[3][3]);
  
  
  // for x
  __m256 xm00 =  _mm256_mul_ps(*x,m00);
  __m256 ym10 =  _mm256_mul_ps(*y,m10);
  __m256 zm20 =  _mm256_mul_ps(*z,m20);

  // for y
  __m256 xm01 =  _mm256_mul_ps(*x,m01);
  __m256 ym11 =  _mm256_mul_ps(*y,m11);
  __m256 zm21 =  _mm256_mul_ps(*z,m21);

  __m256 xm02 =  _mm256_mul_ps(*x,m02);
  __m256 ym12 =  _mm256_mul_ps(*y,m12);
  __m256 zm22 =  _mm256_mul_ps(*z,m22);

  // for w
  __m256 xm03 =  _mm256_mul_ps(*x,m03);
  __m256 ym13 =  _mm256_mul_ps(*y,m13);
  __m256 zm23 =  _mm256_mul_ps(*z,m23);

  *x = _mm256_add_ps(_mm256_add_ps(_mm256_add_ps(ym10,zm20),xm00),m30);
  *y = _mm256_add_ps(_mm256_add_ps(_mm256_add_ps(ym11,zm21),xm01),m31);
  *z = _mm256_add_ps(_mm256_add_ps(_mm256_add_ps(ym12,zm22),xm02),m32);
  *w = _mm256_add_ps(_mm256_add_ps(_mm256_add_ps(ym13,zm23),xm03),m33);
}

void mv8_get_ndc_simd(__m256 *x,__m256 *y,int width,int height){
  // x = x*0.5*width +1
  // y = (1-y)*height*0.5
  __m256 half = _mm256_set1_ps(0.5);
  __m256 widthv = _mm256_set1_ps((float)width);
  __m256 heightv = _mm256_set1_ps((float)height);
  __m256 one = _mm256_set1_ps(1.0);
  __m256 onen = _mm256_set1_ps(-1.0);
  *x = _mm256_add_ps(*x,one);
  *x = _mm256_mul_ps(*x,half);
  *x = _mm256_mul_ps(*x,widthv);
  
  *y = _mm256_mul_ps(*y,onen);
  *y = _mm256_add_ps(*y,one);
  *y = _mm256_mul_ps(*y,heightv);
  *y = _mm256_mul_ps(*y,half);
}

void mv4_mul_mat4_vec3(__m128 *x,__m128 *y,__m128 *z,float model[4][4]){
  __m128 m00 = _mm_set1_ps(model[0][0]);
  __m128 m10 = _mm_set1_ps(model[1][0]);
  __m128 m20 = _mm_set1_ps(model[2][0]);
  __m128 m30 = _mm_set1_ps(model[3][0]);
  
  __m128 m01 = _mm_set1_ps(model[0][1]);
  __m128 m11 = _mm_set1_ps(model[1][1]);
  __m128 m21 = _mm_set1_ps(model[2][1]);
  __m128 m31 = _mm_set1_ps(model[3][1]);

  __m128 m02 = _mm_set1_ps(model[0][2]);
  __m128 m12 = _mm_set1_ps(model[1][2]);
  __m128 m22 = _mm_set1_ps(model[2][2]);
  __m128 m32 = _mm_set1_ps(model[3][2]);
  
  // for x
  __m128 xm00 =  _mm_mul_ps(*x,m00);
  __m128 ym10 =  _mm_mul_ps(*y,m10);
  __m128 zm20 =  _mm_mul_ps(*z,m20);

  // for y
  __m128 xm01 =  _mm_mul_ps(*x,m01);
  __m128 ym11 =  _mm_mul_ps(*y,m11);
  __m128 zm21 =  _mm_mul_ps(*z,m21);

  __m128 xm02 =  _mm_mul_ps(*x,m02);
  __m128 ym12 =  _mm_mul_ps(*y,m12);
  __m128 zm22 =  _mm_mul_ps(*z,m22);

  *x = _mm_add_ps(_mm_add_ps(_mm_add_ps(ym10,zm20),xm00),m30);
  *y = _mm_add_ps(_mm_add_ps(_mm_add_ps(ym11,zm21),xm01),m31);
  *z = _mm_add_ps(_mm_add_ps(_mm_add_ps(ym12,zm22),xm02),m32);
}

void mv4_mul_mat4_vec3W(__m128 *x,__m128 *y,__m128 *z,__m128 *w,float model[4][4]){
   __m128 m00 = _mm_set1_ps(model[0][0]);
  __m128 m10 = _mm_set1_ps(model[1][0]);
  __m128 m20 = _mm_set1_ps(model[2][0]);
  __m128 m30 = _mm_set1_ps(model[3][0]);
  
  __m128 m01 = _mm_set1_ps(model[0][1]);
  __m128 m11 = _mm_set1_ps(model[1][1]);
  __m128 m21 = _mm_set1_ps(model[2][1]);
  __m128 m31 = _mm_set1_ps(model[3][1]);

  __m128 m02 = _mm_set1_ps(model[0][2]);
  __m128 m12 = _mm_set1_ps(model[1][2]);
  __m128 m22 = _mm_set1_ps(model[2][2]);
  __m128 m32 = _mm_set1_ps(model[3][2]);

  __m128 m03 = _mm_set1_ps(model[0][3]);
  __m128 m13 = _mm_set1_ps(model[1][3]);
  __m128 m23 = _mm_set1_ps(model[2][3]);
  __m128 m33 = _mm_set1_ps(model[3][3]);
  
  // for x
  __m128 xm00 =  _mm_mul_ps(*x,m00);
  __m128 ym10 =  _mm_mul_ps(*y,m10);
  __m128 zm20 =  _mm_mul_ps(*z,m20);

  // for y
  __m128 xm01 =  _mm_mul_ps(*x,m01);
  __m128 ym11 =  _mm_mul_ps(*y,m11);
  __m128 zm21 =  _mm_mul_ps(*z,m21);

  __m128 xm02 =  _mm_mul_ps(*x,m02);
  __m128 ym12 =  _mm_mul_ps(*y,m12);
  __m128 zm22 =  _mm_mul_ps(*z,m22);

  // for w
  __m128 xm03 =  _mm_mul_ps(*x,m03);
  __m128 ym13 =  _mm_mul_ps(*y,m13);
  __m128 zm23 =  _mm_mul_ps(*z,m23);

  *x = _mm_add_ps(_mm_add_ps(_mm_add_ps(ym10,zm20),xm00),m30);
  *y = _mm_add_ps(_mm_add_ps(_mm_add_ps(ym11,zm21),xm01),m31);
  *z = _mm_add_ps(_mm_add_ps(_mm_add_ps(ym12,zm22),xm02),m32);
  *w = _mm_add_ps(_mm_add_ps(_mm_add_ps(ym13,zm23),xm03),m33);
}

void mv4_get_ndc_simd(__m128 *x,__m128 *y,int width,int height){
  // x = x*0.5*width +1
  // y = (1-y)*height*0.5
  __m128 half = _mm_set1_ps(0.5);
  __m128 widthv = _mm_set1_ps((float)width);
  __m128 heightv = _mm_set1_ps((float)height);
  __m128 one = _mm_set1_ps(1.0);
  __m128 onen = _mm_set1_ps(-1.0);
  *x = _mm_add_ps(*x,one);
  *x = _mm_mul_ps(*x,half);
  *x = _mm_mul_ps(*x,widthv);
  *y = _mm_mul_ps(*y,onen);
  *y = _mm_add_ps(*y,one);
  *y = _mm_mul_ps(*y,heightv);
  *y = _mm_mul_ps(*y,half);
}

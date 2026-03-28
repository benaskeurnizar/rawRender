#include "rasterizer.h"

int orient2d(int x1,int y1,int x2,int y2,int x,int y){
  return (x2-x1)*(y-y1) - (y2-y1)*(x-x1);
}

void fill_triangle(BitMap* Buffer,TriangleRS* tri,uint32_t color){
  int minX = tri->x1 < tri->x2 ? tri->x1 : tri->x2;
  minX = minX < tri->x3 ? minX : tri->x3;
  int maxX = tri->x1 > tri->x2 ? tri->x1 : tri->x2;
  maxX = maxX > tri->x3 ? maxX : tri->x3;
  int minY = tri->y1 < tri->y2 ? tri->y1 : tri->y2;
  minY = minY < tri->y3 ? minY : tri->y3;
  int maxY = tri->y1 > tri->y2 ? tri->y1 : tri->y2;
  maxY = maxY > tri->y3 ? maxY : tri->y3;

  // Clamp to buffer
  if(minX < 0) minX = 0;
  if(minY < 0) minY = 0;
  if(maxX >= Buffer->width)  maxX = Buffer->width-1;
  if(maxY >= Buffer->height) maxY = Buffer->height-1;

  // triangle setup
  int A01 = tri->y1 - tri->y2, B01 = tri->x2 - tri->x1;
  int A12 = tri->y2 - tri->y3, B12 = tri->x3 - tri->x2;
  int A20 = tri->y3 - tri->y1, B20 = tri->x1 - tri->x3;
  
  uint32_t* pixel = (uint32_t*)Buffer->Memory;
  int w0_row = orient2d(tri->x2,tri->y2,tri->x3,tri->y3,minX,minY);
  int w1_row = orient2d(tri->x3,tri->y3,tri->x1,tri->y1,minX,minY);
  int w2_row = orient2d(tri->x1,tri->y1,tri->x2,tri->y2,minX,minY);

  int area = orient2d(tri->x1,tri->y1,tri->x2,tri->y2,tri->x3,tri->y3);
  float invArea = 1.0f / area;

  float dZdx = (A12 * tri->z1 + A20 * tri->z2 + A01 * tri->z3) * invArea;
  float dZdy = (B12 * tri->z1 + B20 * tri->z2 + B01 * tri->z3) * invArea;

  float Z_row =
    (w0_row * tri->z1 +
     w1_row * tri->z2 +
     w2_row * tri->z3) * invArea;
  for(int y=minY;y<=maxY;y++){
    float Z = Z_row;
    int w0 = w0_row;
    int w1 = w1_row;
    int w2 = w2_row;
    for(int x=minX;x<=maxX;x++){
      
      if(w0 >= 0 && w1 >= 0 && w2 >= 0){
	if(Z < Buffer->ZBuffer[y*Buffer->width +x]){
	  pixel[y*Buffer->width +x] = color;
	  Buffer->ZBuffer[y*Buffer->width +x] = Z;
	}
      }
      // One step to the right
      w0 += A12;
      w1 += A20;
      w2 += A01;
      Z  += dZdx;
    }
    // One row step
    w0_row += B12;
    w1_row += B20;
    w2_row += B01;
    Z_row  += dZdy;
  }
}







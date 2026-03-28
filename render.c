#include <stdint.h>
#include "Game.h"
#include "MyMath.h"

LARGE_INTEGER win32_getWallClock(){
  LARGE_INTEGER result;
  QueryPerformanceCounter(&result);
  return result;
}

float win32_getSecondsElapsed(Engine* engine,LARGE_INTEGER start,LARGE_INTEGER end){
  float result =  ((float)(end.QuadPart -start.QuadPart)) / (float)engine->renderer.PerformanceFrequency.QuadPart;
  return result;
}

void ndc_to_screen(vec3 ndc,int width,int height,int* sx,int* sy){
  *sx = (int)((ndc[0] + 1.0f) * 0.5f * width);
  *sy = (int)((1.0f - ndc[1]) * 0.5f * height);
}
void Vector_IntersectPlane(vec3 plane_p, vec3 plane_n, vec3 lineStart, vec3 lineEnd, vec3 out)
{
    vec3_normalize(plane_n);

    float plane_d = -(plane_n[0]*plane_p[0] + plane_n[1]*plane_p[1] + plane_n[2]*plane_p[2]);
    float ad = lineStart[0]*plane_n[0] + lineStart[1]*plane_n[1] + lineStart[2]*plane_n[2];
    float bd = lineEnd[0]*plane_n[0] + lineEnd[1]*plane_n[1] + lineEnd[2]*plane_n[2];
    float t = (-plane_d - ad) / (bd - ad);

    vec3 lineDir;
    vec3_sub(lineEnd, lineStart, lineDir);
    vec3_mul_f(lineDir, t, lineDir);
    vec3_add(lineStart, lineDir, out);
}

float dist(vec3 p, vec3 plane_p, vec3 plane_n)
{
    return p[0]*plane_n[0] + p[1]*plane_n[1] + p[2]*plane_n[2]
         - (plane_n[0]*plane_p[0] + plane_n[1]*plane_p[1] + plane_n[2]*plane_p[2]);
}

int Triangle_ClipAgainstPlane(vec3 plane_p, vec3 plane_n, 
                              Triangle* in_tri, 
                              Triangle* out_tri1, Triangle* out_tri2)
{
    // --- Normalize plane normal ---
    vec3_normalize(plane_n);

    // --- Classify points relative to plane ---
    vec3* inside_points[3];  int nInsidePointCount = 0;
    vec3* outside_points[3]; int nOutsidePointCount = 0;

    float d0 = dist(in_tri->pt1, plane_p, plane_n);
    float d1 = dist(in_tri->pt2, plane_p, plane_n);
    float d2 = dist(in_tri->pt3, plane_p, plane_n);

    if (d0 >= 0) inside_points[nInsidePointCount++] = &in_tri->pt1;
    else         outside_points[nOutsidePointCount++] = &in_tri->pt1;

    if (d1 >= 0) inside_points[nInsidePointCount++] = &in_tri->pt2;
    else         outside_points[nOutsidePointCount++] = &in_tri->pt2;

    if (d2 >= 0) inside_points[nInsidePointCount++] = &in_tri->pt3;
    else         outside_points[nOutsidePointCount++] = &in_tri->pt3;

    // --- Case 1: All points outside ---
    if (nInsidePointCount == 0)
        return 0;

    // --- Case 2: All points inside ---
    if (nInsidePointCount == 3)
    {
        *out_tri1 = *in_tri;
        return 1;
    }

    // --- Case 3: 1 inside, 2 outside ---
    if (nInsidePointCount == 1 && nOutsidePointCount == 2)
    {
        vec3_copy(*inside_points[0], out_tri1->pt1);
        Vector_IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[0], out_tri1->pt2);
        Vector_IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[1], out_tri1->pt3);
        return 1;
    }

    // --- Case 4: 2 inside, 1 outside ---
    if (nInsidePointCount == 2 && nOutsidePointCount == 1)
    {
        vec3_copy(*inside_points[0], out_tri1->pt1);
        vec3_copy(*inside_points[1], out_tri1->pt2);
        Vector_IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[0], out_tri1->pt3);

        vec3_copy(*inside_points[1], out_tri2->pt1);
        vec3_copy(out_tri1->pt3, out_tri2->pt2);
        Vector_IntersectPlane(plane_p, plane_n, *inside_points[1], *outside_points[0], out_tri2->pt3);
        return 2;
    }

    return 0;
}


void drawLine(BitMap* Buffer,vec3 start,vec3 end,uint32_t color){
  int x0 = (int)roundf(start[0]), x1 = (int)roundf(end[0]);
  int y0 = (int)roundf(start[1]), y1 = (int)roundf(end[1]);
  int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
  int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
  int err = dx + dy;
  int steps = (dx > -dy) ? dx : -dy;  // max(|dx|,|dy|)
  int step  = 0;

  float z0 = start[2], z1 = end[2];
  uint32_t *pixels = (uint32_t*)Buffer->Memory;

  while (1) {
    float t = (steps > 0) ? (float)step / steps : 0.0f;
    float z = z0 + t * (z1 - z0);  // interpolated z
    
    // use z here (e.g. depth test, z-buffer write, shade, etc.)
    if((x0 >= Buffer->width || x0 <= 0) || (y0 >= Buffer->height-1 || y0 <= 0)){

    }else{
      if(z <= Buffer->ZBuffer[y0 * Buffer->width + x0]){
	pixels[y0 * Buffer->width + x0] = color;
	Buffer->ZBuffer[y0 * Buffer->width + x0] = z;
      }
    }
   

    if (x0 == x1 && y0 == y1) break;
    int e2 = 2 * err;
    if (e2 >= dy) { err += dy; x0 += sx;  }
    if (e2 <= dx) { err += dx; y0 += sy; }
    step++;
  }
}
void get_point_ndc(mat4 proj,vec3 point,int width,int height,int* x,int* y,float* z){
  vec4 result;
  float znear = 0.1;
  mult_mat4_vec3(proj,point,result);
  vec3 ndc;
  ndc[0] = result[0];
  ndc[1] = result[1];
  ndc[2] = result[2];
  
  //ndc_to_screen(ndc,width,height,x,y);
  //TODO: scale into view
  ndc[0]+=1.0f;//ndc[1]+=1.0f;
  ndc[0]*= 0.5f * ((float)width); //ndc[1]*= 0.5f * ((float)height);
  ndc[1] = (1.0f - ndc[1]) * 0.5f * height;  // notice minus here
  *z = result[2];
  //*z = ndc[2];
  *x = ndc[0];
  *y = ndc[1];
}
void drawTriangle(Engine* engine,Triangle* triangle,mat4 model,BitMap* Buffer){
  LARGE_INTEGER lastCounter_calc,lastCounter_render;
  
  vec3 pt1TR,pt2TR,pt3TR;
  mult_mat4_vec3R( triangle->pt1,model, pt1TR);
  mult_mat4_vec3R( triangle->pt2,model, pt2TR);
  mult_mat4_vec3R( triangle->pt3,model, pt3TR);

 
  vec3 pt1V,pt2V,pt3V;
  mult_mat4_vec3R( pt1TR,engine->world.camera.view, pt1V);
  mult_mat4_vec3R( pt2TR,engine->world.camera.view, pt2V);
  mult_mat4_vec3R( pt3TR,engine->world.camera.view, pt3V);

  
  // **BACKFACE CULLING - Add this check right after view transform**
  

  int nClipped=0;
  Triangle TriViewd;
  vec3_copy(pt1V,TriViewd.pt1);
  vec3_copy(pt2V,TriViewd.pt2);
  vec3_copy(pt3V,TriViewd.pt3);

  
  int width = engine->renderer.screenWidth;
  int height = engine->renderer.screenHeight;
  //if (vec3_dot(normal, TriViewd.pt1) < 0.0f)
  //return;
  Triangle clipped[2];
  nClipped = Triangle_ClipAgainstPlane((vec3){0.0,0.0,0.5}, (vec3){0.0,0.0,1.0},&TriViewd,&clipped[0],&clipped[1]);
  for (int i = 0; i < nClipped; i++) {
    QueryPerformanceCounter(&lastCounter_calc);
    int pt1x, pt1y, pt2x, pt2y, pt3x, pt3y;
    float z1, z2, z3;
    get_point_ndc(engine->renderer.proj,clipped[i].pt1, width,height, &pt1x, &pt1y, &z1);
    get_point_ndc(engine->renderer.proj,clipped[i].pt2, width,height, &pt2x, &pt2y, &z2);
    get_point_ndc(engine->renderer.proj,clipped[i].pt3, width,height, &pt3x, &pt3y, &z3);

    z1 = (z1 + 1.0f) * 0.5f;z2 = (z2 + 1.0f) * 0.5f;z3 = (z3 + 1.0f) * 0.5f;
    Triangle TriProjected;
    vec3_copy((vec3){(float)pt1x, (float)pt1y, z1}, TriProjected.pt1);
    vec3_copy((vec3){(float)pt2x, (float)pt2y, z2}, TriProjected.pt2);
    vec3_copy((vec3){(float)pt3x, (float)pt3y, z3}, TriProjected.pt3);

    // Prepare buffers
    Triangle listTriangles[16];
    Triangle newTriangles[16];
    listTriangles[0] = TriProjected;
    int nTriangles = 1;

    // Clip against each of 4 screen edges
    for (int p = 0; p < 4; p++) {
      int newCount = 0;
      for (int t = 0; t < nTriangles; t++) {
	Triangle out1, out2;
	int nOut = 0;

	switch (p) {
	case 0: // Top edge (y = 0)
	  nOut = Triangle_ClipAgainstPlane((vec3){0.0f, 0.0f, 0.0f}, (vec3){0.0f, 1.0f, 0.0f}, &listTriangles[t], &out1, &out2);
	  break;
	case 1: // Bottom edge (y = GHeight - 1)
	  nOut = Triangle_ClipAgainstPlane((vec3){0.0f, (float)height - 1, 0.0f}, (vec3){0.0f, -1.0f, 0.0f}, &listTriangles[t], &out1, &out2);
	  break;
	case 2: // Left edge (x = 0)
	  nOut = Triangle_ClipAgainstPlane((vec3){0.0f, 0.0f, 0.0f}, (vec3){1.0f, 0.0f, 0.0f}, &listTriangles[t], &out1, &out2);
	  break;
	case 3: // Right edge (x = GWidth - 1)
	  nOut = Triangle_ClipAgainstPlane((vec3){(float)width - 1, 0.0f, 0.0f}, (vec3){-1.0f, 0.0f, 0.0f}, &listTriangles[t], &out1, &out2);
	  break;
	}

	if (nOut == 1) newTriangles[newCount++] = out1;
	if (nOut == 2) { newTriangles[newCount++] = out1; newTriangles[newCount++] = out2; }
      }

      // Move new triangles to listTriangles for next plane
      nTriangles = newCount;
      for (int t = 0; t < nTriangles; t++)
	listTriangles[t] = newTriangles[t];
    }
    LARGE_INTEGER endCounter_calc;
    QueryPerformanceCounter(&endCounter_calc);
    int counterElapsed = (int)(endCounter_calc.QuadPart - lastCounter_calc.QuadPart);
    float MsElapsed = (win32_getSecondsElapsed(engine,lastCounter_calc,endCounter_calc) * 1000.0);
    engine->renderer.calc_time += MsElapsed;
    
    // Draw final triangles
    
    for (int j = 0; j < nTriangles; j++) {
      QueryPerformanceCounter(&lastCounter_render);
      Triangle Tri = listTriangles[j];
      uint32_t color = 0xFFFFFFFF;
      //TODO: draw the lines here
      drawLine(Buffer,Tri.pt1,Tri.pt2,color);
      drawLine(Buffer,Tri.pt2,Tri.pt3,color);
      drawLine(Buffer,Tri.pt3,Tri.pt1,color);

      LARGE_INTEGER endCounter_render;
      QueryPerformanceCounter(&endCounter_render);
      counterElapsed = (int)(endCounter_render.QuadPart - lastCounter_render.QuadPart);
      MsElapsed = (win32_getSecondsElapsed(engine,lastCounter_render,endCounter_render) * 1000.0);
      engine->renderer.render_time += MsElapsed;
    }
    
  }
}
void renderMesh(Engine* engine,BitMap* buffer,Mesh mesh){
  /*mat4_identity(mesh.model);
  mat4 RotX,RotZ,Trans;
  float angle = 1.0f + elapsed_time;
  mat4m_rotate_z(angle,RotZ);
  mat4m_rotate_x(angle,RotX);
  mat4_translation(Trans,0.0,0.0,3.0);
  mat4 rot_temp;
  mul_mat4_mat4(RotZ,RotX,rot_temp);
  mul_mat4_mat4(rot_temp,Trans,mesh.model);

  */
  for(int i=0;i<mesh.num_triangles;i++){
    drawTriangle(engine,&mesh.Triangles[i],mesh.model,buffer);
  }
}
void renderWorld(Engine* engine,BitMap* Buffer){
  for(int i=0;i<engine->world.num_meshes;i++){
    renderMesh(engine,Buffer,engine->world.meshes[i]);
  }
}

#include <stdint.h>
#include "Game.h"
#include "MyMath.h"
#include "optimization/mv_simd_math.c"
#include "rasterizer.c"


//NOTE : this is just for getting it to work,if this makes it optimized,then we gonna modify everything to be alligned with it,from structs to functions

/*
  the thing here is to draw vertices by 8 patches.
  what happens if we have less vertices than that ? well ther will be a case for 8 or less vertices,and it should not be that hard.
  {x1,y1,z1,x2,y2,z2,x3,y3,z3,x4,y4,z4,x5,y5,z5,x6,y6,z6,x7,y7,z7,x8,y8,z8   ,x9,y9,z9,x10,y10,z10,x11,y11,z11} count is 11
  123, 456 ,789
*/


void clip_triangle(
		   float ax, float ay, float az, float aw,
		   float bx, float by, float bz, float bw,
		   float cx, float cy, float cz, float cw,
		   float xc[9], float yc[9], float zc[9], float wc[9],
		   int* count);
  
void add_vertice(float* rx,float *ry,float *rz,float x,float y,float z,int* num_vertices){
  rx[*num_vertices] = x;
  ry[*num_vertices] = y;
  rz[*num_vertices] = z;
}
void mul_mat4_vec3SMD(float m[4][4], float vx, float vy, float vz, float *ox, float *oy, float *oz, float *ow);

int inside(float x, float y, float z, float w) {
    return x >= -w && x <= w && y >= -w && y <= w && z >= -w && z <= w;
}

int outside(float x, float y, float z, float w) {
  return (x < -w || x > w) && (y < -w || y > w) && ( z < -w || z > w);
}

float calculate_elapsed_ms(LARGE_INTEGER start,LARGE_INTEGER end,LARGE_INTEGER frequency){
  return ((float)(end.QuadPart - start.QuadPart)*1000.0) / frequency.QuadPart;
}

void draw_mesh_simd(Engine* engine,MeshS* mesh,mat4 model,BitMap* Buffer, float *x_mvp, float *y_mvp, float *z_mvp, float *w_mvp,LARGE_INTEGER frequency){
  vec3 lightDir;//TODO: this is dummy for now,will change
  lightDir[0] = engine->world.camera.lookdir[0];
  lightDir[1] = engine->world.camera.lookdir[1];
  lightDir[2] = engine->world.camera.lookdir[2];
  
  mat4 view ;
  memcpy(view, engine->world.camera.view, sizeof(mat4));
  mat4 proj ;
  memcpy(proj, engine->renderer.proj, sizeof(mat4));
  int total = mesh->vertex_count;
  int num_vertex8 = mesh->vertex_count*3 % 8;
  int index = 0;
  int width = engine->renderer.screenWidth;
  int height = engine->renderer.screenHeight;

  
  int total8 = (total / 8) * 8;
  LARGE_INTEGER last_counter_calc,last_counter_render,last_counter_clip;
  LARGE_INTEGER end_counter_calc,end_counter_render,end_counter_clip;
  QueryPerformanceCounter(&last_counter_calc);
  for(int i=0;i<total8;i+=8){
    // vec3 is a float[3] = {}
    float Xs[8] = {
      mesh->vertices[i+0][0], mesh->vertices[i+1][0],
      mesh->vertices[i+2][0], mesh->vertices[i+3][0],
      mesh->vertices[i+4][0], mesh->vertices[i+5][0],
      mesh->vertices[i+6][0], mesh->vertices[i+7][0]
    };
    float Ys[8] = {
      mesh->vertices[i+0][1], mesh->vertices[i+1][1],
      mesh->vertices[i+2][1], mesh->vertices[i+3][1],
      mesh->vertices[i+4][1], mesh->vertices[i+5][1],
      mesh->vertices[i+6][1], mesh->vertices[i+7][1]
    };
    float Zs[8] = {
      mesh->vertices[i+0][2], mesh->vertices[i+1][2],
      mesh->vertices[i+2][2], mesh->vertices[i+3][2],
      mesh->vertices[i+4][2], mesh->vertices[i+5][2],
      mesh->vertices[i+6][2], mesh->vertices[i+7][2]
    };
    
    __m256 x = _mm256_loadu_ps(Xs);
    __m256 y = _mm256_loadu_ps(Ys);
    __m256 z = _mm256_loadu_ps(Zs);
    mv8_mul_mat4_vec3(&x,&y,&z,model);
    //NOTE: here we have our vertices multiplied by the model matrix
    mv8_mul_mat4_vec3(&x,&y,&z,view);
    //NOTE: here we have our vertices multiplied by the proj matrix
    //TODO: now project the results
    __m256 w = _mm256_set1_ps(1.0);
    mv8_mul_mat4_vec3W(&x,&y,&z,&w,proj);
    //TODO: for each triangle here,clip it ,get it's NDC then render it
    float xp[8],yp[8],zp[8],wp[8];
    _mm256_storeu_ps(&(x_mvp[i]),x);
    _mm256_storeu_ps(&(y_mvp[i]),y);
    _mm256_storeu_ps(&(z_mvp[i]),z);
    _mm256_storeu_ps(&(w_mvp[i]),w);
    index = i+ 8;
  }
  int remaining = mesh->vertex_count - index;
  int num_vertex4 = remaining%4;
  int total4 = total8 + ((total - total8) / 4) * 4;
  for(int j=total8;j<total4;j+=4){
    float Xs[4] = {
      mesh->vertices[j+0][0], mesh->vertices[j+1][0],
      mesh->vertices[j+2][0], mesh->vertices[j+3][0]
    };
    float Ys[4] = {
      mesh->vertices[j+0][1], mesh->vertices[j+1][1],
      mesh->vertices[j+2][1], mesh->vertices[j+3][1]
    };
    float Zs[4] = {
      mesh->vertices[j+0][2], mesh->vertices[j+1][2],
      mesh->vertices[j+2][2], mesh->vertices[j+3][2]
    };
    __m128 x = _mm_loadu_ps(Xs);
    __m128 y = _mm_loadu_ps(Ys);
    __m128 z = _mm_loadu_ps(Zs);

    mv4_mul_mat4_vec3(&x,&y,&z,model);
    //NOTE: here we have our vertices multiplied by the model matrix
    mv4_mul_mat4_vec3(&x,&y,&z,view);
    //NOTE: here we have our vertices multiplied by the proj matrix
    //TODO: now project the results
    __m128 w = _mm_set1_ps(1.0);
    mv4_mul_mat4_vec3W(&x,&y,&z,&w,proj);
    //TODO: for each triangle here,clip it ,get it's NDC then render it
    float xp[4],yp[4],zp[4],wp[4];
    _mm_storeu_ps(&(x_mvp[j]),x);
    _mm_storeu_ps(&(y_mvp[j]),y);
    _mm_storeu_ps(&(z_mvp[j]),z);
    _mm_storeu_ps(&(w_mvp[j]),w);
    index = j+ 4;
  }
  remaining = mesh->vertex_count - index; // NOW : remaining <= 3
  for(int j=total4;j<total;j++){//NOTE: now simd here
    float ox,oy,oz,ow;
    mul_mat4_vec3SMD(model, mesh->vertices[j][0],mesh->vertices[j][1],mesh->vertices[j][2],&ox,&oy,&oz,&ow);
    mul_mat4_vec3SMD(view,ox,oy,oz,&ox,&oy,&oz,&ow);
    mul_mat4_vec3SMD(proj,ox,oy,oz,&ox,&oy,&oz,&ow);
    x_mvp[j] = ox;
    y_mvp[j] = oy;
    z_mvp[j] = oz;
    w_mvp[j] = ow;
    //j = j+1;
  }
  QueryPerformanceCounter(&end_counter_calc);
  engine->renderer.calc_time = calculate_elapsed_ms(last_counter_calc,end_counter_calc,frequency);
  //NOTE : now ,and if everything before is correct,we should have the transoformed x,y,z and w for each triangle,it is time for the clipping ndc then rendering
  int num_triangles = mesh->vertex_count/3;
  for(int i=0;i< mesh->vertex_count;i+=3){
    float xc[9]; float yc[9]; float zc[9];float wc[9];
    float normal[3] = {mesh->normals[i][0],mesh->normals[i][1],mesh->normals[i][2]};
    float nx = mesh->normals[i][0];
    float ny = mesh->normals[i][1];
    float nz = mesh->normals[i][2];

    // transform normal by model matrix (upper 3x3 only, no translation)
    float wnx = model[0][0]*nx + model[1][0]*ny + model[2][0]*nz;
    float wny = model[0][1]*nx + model[1][1]*ny + model[2][1]*nz;
    float wnz = model[0][2]*nx + model[1][2]*ny + model[2][2]*nz;

    float I = (-wnx * lightDir[0]) + (-wny * lightDir[1]) + (-wnz * lightDir[2]);
    //float I = (-normal[0] * lightDir[0]) + (-normal[1] * lightDir[1])+ (-normal[2] * lightDir[2]);
    if (I<0){
      I = 0.0;
    }
    int v = 255 *I; 
    
    int count = 0;
    if(outside(x_mvp[i], y_mvp[i], z_mvp[i], w_mvp[i]) &&
       outside(x_mvp[i+1], y_mvp[i+1], z_mvp[i+1], w_mvp[i+1]) &&
       outside(x_mvp[i+2], y_mvp[i+2], z_mvp[i+2], w_mvp[i+2])){
      continue;
    }else if(inside(x_mvp[i], y_mvp[i], z_mvp[i], w_mvp[i]) &&
	     inside(x_mvp[i+1], y_mvp[i+1], z_mvp[i+1], w_mvp[i+1]) &&
	     inside(x_mvp[i+2], y_mvp[i+2], z_mvp[i+2], w_mvp[i+2])) {
      // fully inside, skip clipping entirely
      count = 3;
      xc[0]=x_mvp[i];   yc[0]=y_mvp[i];   zc[0]=z_mvp[i];   wc[0]=w_mvp[i];
      xc[1]=x_mvp[i+1]; yc[1]=y_mvp[i+1]; zc[1]=z_mvp[i+1]; wc[1]=w_mvp[i+1];
      xc[2]=x_mvp[i+2]; yc[2]=y_mvp[i+2]; zc[2]=z_mvp[i+2]; wc[2]=w_mvp[i+2];

      float x0 = xc[0]/wc[0], y0 = yc[0]/wc[0];
      float x1 = xc[1]/wc[1], y1 = yc[1]/wc[1];
      float x2 = xc[2]/wc[2], y2 = yc[2]/wc[2];
      float z0 = zc[0]/wc[0];
      float z1 = zc[1]/wc[1];
      float z2 = zc[2]/wc[2];
      // viewport transform
      x0 = (x0+1)*0.5f*width; y0 = (1-y0)*0.5f*height;
      x1 = (x1+1)*0.5f*width; y1 = (1-y1)*0.5f*height;
      x2 = (x2+1)*0.5f*width; y2 = (1-y2)*0.5f*height;
      uint32_t color = 0xFFFFFFFF;
      color = (255 << 24) | (v << 16) | (v << 8) | v;
      vec3 pt1,pt2,pt3;
      pt1[0] = x0;pt1[1] =y0;pt1[2] = z0;
      pt2[0] = x1;pt2[1] = y1;pt2[2] = z1;
      pt3[0] = x2;pt3[1] = y2;pt3[2] = z2;
      QueryPerformanceCounter(&last_counter_render);
      TriangleRS triangle = {
	x0,x1,x2,y0,y1,y2,z0,z1,z2
      };
      fill_triangle(Buffer,&triangle,color);
      QueryPerformanceCounter(&end_counter_render);
      engine->renderer.render_time += calculate_elapsed_ms(last_counter_render,end_counter_render,frequency);
    }else{
      QueryPerformanceCounter(&last_counter_clip);
      clip_triangle(
		    x_mvp[i], y_mvp[i], z_mvp[i], w_mvp[i],
		    x_mvp[i+1], y_mvp[i+1], z_mvp[i+1], w_mvp[i+1],
		    x_mvp[i+2], y_mvp[i+2], z_mvp[i+2], w_mvp[i+2],
		    xc, yc, zc, wc,
		    &count);
      QueryPerformanceCounter(&end_counter_clip);
      engine->renderer.clip_time += calculate_elapsed_ms(last_counter_clip,end_counter_clip,frequency);
      if(count < 3) continue;
      for(int j=1;j<count-1;j++){
	// triangle: (0, i, i+1)
	// divide xc[0]/wc[0] etc -> NDC -> rasterize
	float x0 = xc[0]/wc[0], y0 = yc[0]/wc[0];
	float x1 = xc[j]/wc[j], y1 = yc[j]/wc[j];
	float x2 = xc[j+1]/wc[j+1], y2 = yc[j+1]/wc[j+1];
	float z0 = zc[0]/wc[0];
	float z1 = zc[j]/wc[j];
	float z2 = zc[j+1]/wc[j+1];
	// viewport transform
	x0 = (x0+1)*0.5f*width; y0 = (1-y0)*0.5f*height;
	x1 = (x1+1)*0.5f*width; y1 = (1-y1)*0.5f*height;
	x2 = (x2+1)*0.5f*width; y2 = (1-y2)*0.5f*height;
	uint32_t color = 0xFFFFFFFF;
	color = (255 << 24) | (v << 16) | (v << 8) | v;
	vec3 pt1,pt2,pt3;
	pt1[0] = x0;pt1[1] =y0;pt1[2] = z0;
	pt2[0] = x1;pt2[1] = y1;pt2[2] = z1;
	pt3[0] = x2;pt3[1] = y2;pt3[2] = z2;
	QueryPerformanceCounter(&last_counter_render);
	TriangleRS triangle = {
	  x0,x1,x2,y0,y1,y2,z0,z1,z2
	};
	fill_triangle(Buffer,&triangle,color);
	QueryPerformanceCounter(&end_counter_render);
	engine->renderer.render_time += calculate_elapsed_ms(last_counter_render,end_counter_render,frequency);
      }
    }
  }

  
}

void mul_mat4_vec3SMD(float m[4][4], float vx, float vy, float vz, float *ox, float *oy, float *oz, float *ow) {
    *ox = vx*m[0][0] + vy*m[1][0] + vz*m[2][0] + m[3][0];
    *oy = vx*m[0][1] + vy*m[1][1] + vz*m[2][1] + m[3][1];
    *oz = vx*m[0][2] + vy*m[1][2] + vz*m[2][2] + m[3][2];
    *ow = vx*m[0][3] + vy*m[1][3] + vz*m[2][3] + m[3][3];
}


/*
- in order to do the clipping we need the triangles right ? maybe we can convert our vectore back to triangle format,do the clipping then render
 */

void clip_triangle(
		   float ax, float ay, float az, float aw,
		   float bx, float by, float bz, float bw,
		   float cx, float cy, float cz, float cw,
		   float xc[9], float yc[9], float zc[9], float wc[9],
		   int* count)
{
  // two scratch buffers, swap after each plane
  float px[9],py[9],pz[9],pw[9];
  float qx[9],qy[9],qz[9],qw[9];

  // load triangle into first buffer
  px[0]=ax; py[0]=ay; pz[0]=az; pw[0]=aw;
  px[1]=bx; py[1]=by; pz[1]=bz; pw[1]=bw;
  px[2]=cx; py[2]=cy; pz[2]=cz; pw[2]=cw;
  int n = 3;

  // 6 planes: sign * component <= w
  // encoded as: d = w*ps + component*cs  (inside when d >= 0)
  // ps=1 always, cs=+1 or -1
  float cs[6] = {-1, 1, -1, 1, -1, 1};  // component sign
  // which component: 0=x,1=y,2=z
  int   ci[6] = { 0, 0,  1, 1,  2, 2};

  float (*src_x)[9] = (float(*)[9])&px;  // just use pointers
  // simpler: just use two named buffers and a flag

  float *sx=px,*sy=py,*sz=pz,*sw=pw;
  float *dx=qx,*dy=qy,*dz=qz,*dw=qw;

  for(int p=0;p<6;p++){
    int comp = ci[p];
    float sign = cs[p];
    int out = 0;

    for(int i=0;i<n;i++){
      int j = (i+1)%n;

      // pick component
      float Ac = comp==0?sx[i]:comp==1?sy[i]:sz[i];
      float Bc = comp==0?sx[j]:comp==1?sy[j]:sz[j];

      float dA = sw[i] + sign*Ac;
      float dB = sw[j] + sign*Bc;

      if(dA >= 0){
	dx[out]=sx[i]; dy[out]=sy[i];
	dz[out]=sz[i]; dw[out]=sw[i];
	out++;
      }
      if((dA >= 0) != (dB >= 0)){
	float t = dA/(dA-dB);
	dx[out] = sx[i] + t*(sx[j]-sx[i]);
	dy[out] = sy[i] + t*(sy[j]-sy[i]);
	dz[out] = sz[i] + t*(sz[j]-sz[i]);
	dw[out] = sw[i] + t*(sw[j]-sw[i]);
	out++;
      }
    }

    // swap buffers
    float *tmp;
    tmp=sx; sx=dx; dx=tmp;
    tmp=sy; sy=dy; dy=tmp;
    tmp=sz; sz=dz; dz=tmp;
    tmp=sw; sw=dw; dw=tmp;
    n = out;

    if(n < 3){ *count=0; return; }
  }

  // copy result out
  for(int i=0;i<n;i++){
    xc[i]=sx[i]; yc[i]=sy[i];
    zc[i]=sz[i]; wc[i]=sw[i];
  }
  *count = n;
}

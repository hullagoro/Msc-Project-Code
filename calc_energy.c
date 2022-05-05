#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

/* sums out1.raw and out2.raw pixel/pixel */

const uint32_t image_width  = 150;
const uint32_t image_height = 150;

// 0->49
// 50->90
// 91->240

float div_factor = 49 + (240-91); // number of images for both

 // torage type
typedef uint8_t storage_type;

size_t read_file(const char* filename,storage_type *&buf);
size_t write_file(const char* filename,storage_type *&buf,size_t length);
void add(storage_type* &dst,storage_type* src1,storage_type* src2,size_t len);
void div(storage_type* &dst,storage_type* src,size_t len);
storage_type sum(storage_type* src,size_t len);
storage_type sum_with_mean(storage_type* src,size_t len);
storage_type mean(storage_type* src,size_t len);
storage_type std_dev(storage_type* src,size_t len );
storage_type count(storage_type* src,size_t len);

int main(int argc,char** argv)
{

  if ( argc < 2 ) 
  {
    printf("Usage : ./calc_energy raw_filename\n");
    exit(1);
  }

  size_t file_size=0;

  //char ifname1[] = "out1.raw";

  storage_type* buf1;

  file_size = read_file(argv[1],buf1);
 
  size_t ele_size = file_size / sizeof(storage_type);

  storage_type pixel_count = count(buf1,ele_size);
  printf("pixel_count : %f\n",pixel_count);

  float sum_v = sum(buf1,ele_size);
  printf("sum : %f\n",sum_v);

  float mean_v = mean(buf1,ele_size);
  printf("mean : %f\n",mean_v);

  float sd = std_dev(buf1,ele_size);
  printf("std dev : %f\n",sd);

  const float pi = 3.14159;
  //const float delta_lambda = 500; // angstrom
  const float delta_lambda = 200.0; //50.0; // nm
  //const float delta_lambda = 0.0075;
  const float area = 486000000;
  const float pixel_multiply = pi * delta_lambda*area; // this will be multiplyed by the pixel value after subtraction

  double final = sum_v * pixel_multiply * 10000000; // ergs
  printf("final power (ergs): %E\n",final);

  printf("divide mean and std by number of images in a sequence\n");

  free(buf1);

  return 0;
}


storage_type count(storage_type* src,size_t len)
{
  storage_type _count=0;
  for(size_t i=0;i<=len;i++)
  {
    if ( src[i] > 0 )
      _count+=1;
  }
  return _count;
}

void add(storage_type* &dst, storage_type* src1,storage_type* src2,size_t len)
{
  for (size_t i=0;i<=len;i++)
  {
    dst[i] = src1[i] + src2[i];
    //dst[i] = src1[i]; // for test
  }
}

void div(storage_type* &dst, storage_type* src,size_t len)
{
  for (size_t i=0;i<=len;i++)
  {
    dst[i] = src[i] / div_factor;
    //dst[i] = src1[i]; // for test
  }
}


size_t read_file(const char* filename,storage_type *&buf)
{
  FILE* fp = fopen(filename,"r");
  fseek(fp, 0, SEEK_END); 
  size_t fsize = ftell(fp); 
  fseek(fp, 0, SEEK_SET);
 
  buf = (storage_type*)malloc(fsize);
  if (fread(buf, sizeof(storage_type), fsize/sizeof(storage_type), fp) != fsize/sizeof(storage_type))
  { fprintf(stderr,"error reading file %s\n",filename); exit(1);  }
  fclose(fp);
  return fsize;
}


storage_type sum(storage_type* src,size_t len)
{
  storage_type sum=0;
  for(int i=0;i<=len;i++)
  {
    sum += src[i];
  }
  return sum; 
}

storage_type sum_with_mean(storage_type* src,size_t len)
{

  storage_type* buf1;
  size_t fs = read_file("pixels_mean_13.raw",buf1);
  size_t es = fs / sizeof(storage_type);

  storage_type sum=0;
  for(int i=0;i<len;i++)
  {
    if ( src[i] - buf1[i] > 0)
      sum += src[i] - buf1[i];
  }
  free(buf1);
  return sum;

}

storage_type mean(storage_type* src,size_t len)
{
  return sum(src,len) / len;
}

storage_type std_dev(storage_type* src,size_t len )
{
  float mean_v = mean(src,len);

  storage_type sum_diff_sq = 0;
  for(int i=0;i<=len;i++)
  {
    sum_diff_sq += pow( (src[i] - mean_v) , 2 );
  }
  sum_diff_sq = sqrt(sum_diff_sq / len);
  return sum_diff_sq;
}

size_t write_file(const char* filename,storage_type *&buf,size_t length)
{
  FILE *fp = fopen(filename,"wb");
  if (fwrite(buf,sizeof(storage_type),length,fp) != length)
  { fprintf(stderr,"error writting file %s\n",filename); exit(1); }
  fclose(fp);
  return length;
}



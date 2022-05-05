#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

/* uses mask.raw to mask ( raw fits images on it ) and sum the pixels over specified raneg of files */
/* if specified , it will subtract a value from each pixel , the mean found for quite sun */
/* if specified , it will multiply a value from each pixel , area of pixel ?*/
/* also writes pixel_mean.raw which contains the mean of each pixel */

const uint32_t image_width  = 150;
const uint32_t image_height = 150;

// 0->50
// 51->90
// 91->240

int first_file=51;
int last_file=90;

/* subtract this value from each pixel */
const float pixel_subtract = 0; // 63885.0 - 4894; //4536.2;

// storage type
typedef uint8_t storage_type;

size_t read_file(const char* filename,storage_type *&buf);
size_t write_file(const char* filename,storage_type *&buf,size_t length);
void mul(storage_type* &dst,storage_type* src1,storage_type* src2,size_t len);
void add(storage_type* &dst,storage_type* src1,storage_type* src2,size_t len);

int main(int argc,char** argv)
{
  if (argc < 2 || argc > 3)
  {
    printf("Wrong number of arguments supplied (%d)\n",argc);
    printf("Usage:\n");
    printf("\t\t./create_mask first_file_number last_file_number\n\n");
    printf("fitst_file_number (int))\n");
    printf("last_file_number (int)\n");
    printf("all files must be in the same location as this executable");
    return 1;
  }

  first_file = atoi(argv[1]);
  last_file = atoi(argv[2]);

  size_t file_size=0;

  storage_type* mask;
  file_size = read_file("output.raw",mask);

  size_t ele_size = file_size / sizeof(storage_type);

  //init mask
  for(int i=0;i<ele_size;i++)
    mask[i] = mask[i] == 0 ? 0 : 1;
  
  storage_type* sum_out = (storage_type*)malloc(file_size); // assuming all files of same size

  //init sum
  for(int i=0;i<ele_size;i++)
    sum_out[i]=0;

  for(int j=first_file;j<=last_file;j++)
  {
    char ifname1[10];
    sprintf(ifname1,"%d",j); // file we working on.

    printf("Working on (%s)\n",ifname1);

    storage_type* tout = (storage_type*)malloc(file_size); // assuming all files of same size
    storage_type* buf1;
    file_size = read_file(ifname1,buf1);
    mul(tout,buf1,mask,ele_size);
    add(sum_out,tout,sum_out,ele_size);

    //for (int z=0;z<10;z++)
    //  printf("%f,",buf1[z]);

    free(buf1);
    free(tout);
  }


  printf("writting file out.raw\n");
  write_file("out.raw",sum_out,ele_size);


  free(sum_out);
  free(mask);
  return 0;
}

void add(storage_type* &dst, storage_type* src1,storage_type* src2,size_t len)
{
  /* will subtract mean value from src */
  /* dst is sum_out , src1 is frame, src2 is sum_out  */

  for (size_t i=0;i<=len;i++)
  {
    if ( src1[i] != 0 )
    {
      dst[i] = ((src1[i] - pixel_subtract)) + src2[i];
      //printf("%f,%f\n",src1[i] , dst[i]);
      //dst[i] = src1[i]; // for test
    }
  }
}


void mul(storage_type* &dst, storage_type* src1,storage_type* src2,size_t len)
{
  for (size_t i=0;i<=len;i++)
  {
    dst[i] = src1[i] * src2[i];
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




size_t write_file(const char* filename,storage_type *&buf,size_t length)
{
  FILE *fp = fopen(filename,"wb");
  if (fwrite(buf,sizeof(storage_type),length,fp) != length)
  { fprintf(stderr,"error writting file %s\n",filename); exit(1); }
  fclose(fp);
  return length;
}



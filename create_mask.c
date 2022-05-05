#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

//#define debug true
//#define verbose true
#ifdef verbose
#define verbose_print(x) printf x;
#else
#define verbose_print(x) ;
#endif
#define _line __LINE__
#ifdef debug
#define debug_point printf("%d:debug_point\n",_line);
#else
#define debug_point ;
#endif

const uint32_t image_width  = 150;
const uint32_t image_height = 150;
size_t sections_c = 100; // how many columns 
size_t sections_r = 100; // how many rows
uint32_t pixel_section_r = image_width  / sections_r; // pixels col's/row's per grid
uint32_t pixel_section_c = image_height / sections_c; // pixels col's/row's per grid
size_t file_size;

uint8_t th1 = 200; // intensity thershold ( if bigger switch to 0 ) ,  this is inverted
float th3 = -10; //-10; // subtractor mul ( need to be neg to inverse )
uint8_t th2 = 200; // intensity thershold ( if bigger , its flare stuff ) 

uint8_t first_fn = 50;
uint8_t last_fn = 80;


size_t map_size = sections_r * sections_c;
uint8_t* buf1;
uint8_t* mean_map1;
uint8_t* buf2;
uint8_t* mean_map2;
uint8_t* buf3;
uint8_t* mean_map3;
uint8_t* buf4;

size_t read_file(const char* filename,uint8_t *&buf);
size_t write_file(const char* filename,uint8_t *&buf,size_t length);
uint8_t find_max(uint8_t*& buf, size_t length);
uint8_t find_min(uint8_t*& buf, size_t length);
size_t sum_grid(size_t row,size_t col,uint8_t *&buf);
void create_map(uint8_t*& gmap, uint8_t*& buf);
uint32_t pixel_to_grid(uint32_t pix,uint32_t *grid_col,uint32_t *grid_row);
uint32_t grid_to_pixel(uint32_t grid_col,uint32_t grid_row);
void map_pixel_to_grid(uint32_t pix,uint32_t *grid_col,uint32_t *grid_row);
uint32_t map_grid_to_pixel(uint32_t grid_col,uint32_t grid_row);
bool map_check_neighbours_value(uint8_t* buf, uint32_t grid_col,uint32_t grid_row,uint8_t check_for);
bool map_check_neighbours_value(uint8_t* buf, uint32_t pix,uint8_t check_for);
uint32_t pixel_coord_to_1d(uint32_t pix_col,uint32_t pix_row);
bool pixel_check_neighbours_value(uint8_t* buf,uint32_t pix,uint8_t check_for);

int main(int argc,char** argv)
{

  if (argc < 2 || argc > 6)
  {
    printf("Wrong number of arguments supplied (%d)\n",argc);
    printf("Usage:\n");
    printf("\t\t./create_mask first_file_number last_file_number [threshold1] [threshold2] [threshold3]\n\n");
    printf("fitst_file_number (int))\n");
    printf("last_file_number (int)\n");
    printf("threshold1 default value is 200 (int)\n");
    printf("threshold2 default value is 200 (int)\n");
    printf("threshold3 default value is -10 (float)\n\n");
    printf("all files must be in the same location as this executable");
    return 1;
  }

  first_fn = atoi(argv[1]);
  last_fn = atoi(argv[2]);
  th1 = atoi(argv[3]);
  th2 = atoi(argv[4]);
  th3 = atof(argv[5]);

  verbose_print(("%d:Map Size:%d\n",_line,map_size));
  char ff[10];
  sprintf(ff,"%d",first_fn);
  file_size = read_file(ff,buf1);

  uint8_t* output = (uint8_t *)malloc(file_size); // our result image should be here.

  for (int file_n=first_fn;file_n<last_fn;file_n++) // more than 0, and less than 30, we dont care about first and last images ( use them for comparision )
  {  
    char ifname1[10];
    char ifname2[10];
    char ofname[10];
    sprintf(ifname1,"%d",file_n); // file we working on.
    sprintf(ifname2,"%d",file_n+1);
    sprintf(ofname,"out/%d",file_n);

    // we already have file0 , but lets create a map for it, this is useful if we wanna adjust map sizes in loop
    create_map(mean_map1,buf1);

    // buf points to previous file
    verbose_print(("%d:reading file %d\n",_line,file_n));
    read_file(ifname1,buf2); // allocates things itself ( current image )
    
    verbose_print(("%d:reading file %d\n",_line,file_n+1));
    read_file(ifname2,buf3); //  read ahead 

    // create maps
    verbose_print(("%d:creating map for %d\n",_line,file_n));
    create_map(mean_map2,buf2);
    verbose_print(("%d:creating map for %d\n",_line,file_n+1));
    create_map(mean_map3,buf3);

    buf4 = (uint8_t *)malloc(file_size); // our result image should be here.
   
  
    uint8_t* pattern_map = (uint8_t*)malloc(map_size);
    for(int i=0;i<map_size;i++)
    {
      if ( mean_map1[i] > mean_map2[i] && map_check_neighbours_value(mean_map2,i,mean_map1[i]) &&  map_check_neighbours_value(mean_map3,i,mean_map1[i]) )
      { pattern_map[i] = 255;  }
      else
      { pattern_map[i] = 0;    }
    }

      
    for(int i=0;i<file_size;i++)
    {
      uint32_t gcol,grow;
      uint32_t mpix = pixel_to_grid(i,&gcol,&grow);
      if(pattern_map[mpix] == 0)
        buf4[i] = 0;
      else
        buf4[i] = buf2[i] ;
    }

    uint8_t *tmp = (uint8_t*)malloc(file_size);
    for (int i=0;i<file_size;i++)
      tmp[i] = buf1[i] - buf2[i];

    // not we got the subbed version, and the running intensity/neighbour lets use them togetehr

    for (int i=0;i<file_size;i++)
    {
      if(buf4[i] > 0 && buf4[i] < 255)
        buf4[i] = 0;
      else
      {
        buf4[i] = tmp[i]*th3 - buf2[i]/2;
        if ( buf4[i] > th1 )
          buf4[i] = 0;
      }
      buf4[i] *= -1;
    }

    for(int i=0;i<file_size;i++)
    {
      if(buf4[i] >th2 )
      { buf4[i] = 255; }
      else
        buf4[i] = 0;
    }

    // write result to output
    if ( file_n != first_fn ) // skip first file
    {
      verbose_print(("%d:writting result file %s\n",_line,ofname));
      for (int i=0;i<file_size;i++)
        output[i] += buf4[i];
    }


    // copy current to previous for next comparision ( both buf and map )
    memcpy((void*)buf1,(void*)buf2,file_size);
    memcpy((void*)mean_map1,(void*)mean_map2,map_size);

    // free these we dont need them anymore
    debug_point
    free(buf2);
    debug_point
    free(mean_map2);
    debug_point
    free(buf3);
    free(mean_map3);
    debug_point
    free(buf4);
  }

  write_file("output.raw",output,file_size);
  
  free(output);
  free(buf1);
  free(mean_map1);
  return 0;
}



size_t read_file(const char* filename,uint8_t *&buf)
{
  FILE* fp = fopen(filename,"r");
  fseek(fp, 0, SEEK_END); 
  size_t fsize = ftell(fp); 
  fseek(fp, 0, SEEK_SET);
 
  debug_point
//  if(buf)
//    free(buf);

  debug_point
  buf = (uint8_t*)malloc(fsize);
  debug_point
  if (fread(buf, sizeof(*buf), fsize, fp) != fsize)
  { fprintf(stderr,"error reading file %s\n",filename); exit(1);  }
  fclose(fp);
  verbose_print (("%d:file %s read successfull (%d) bytes\n",_line,filename,fsize ));
  return fsize;
}


size_t write_file(const char* filename,uint8_t *&buf,size_t length)
{
  FILE *fp = fopen(filename,"wb");
  if (fwrite(buf,sizeof(*buf),length,fp) != length)
  { fprintf(stderr,"error writting file %s\n",filename); exit(1); }
  fclose(fp);
  verbose_print (("%d:file %s written successfull (%d) bytes\n",_line,filename,length ));
  return length;
}

/* find max */
uint8_t find_max(uint8_t*& buf, size_t length)
{
  uint8_t max=0;
  for (int j=0;j<length;j++)
    if ( buf[j] > max && buf[j] < 255 ) // we are excluding 255 values from our search
      max = buf[j];

  return max;
}
/* find min */
uint8_t find_min(uint8_t*& buf, size_t length)
{
  uint8_t min=255;
  for (int j=0;j<length;j++)
    if ( buf[j] < min && buf[j] > 0 ) // we are excluding 0 from our search
      min = buf[j];

  return min;
}

/* sum grid in image */
size_t sum_grid(size_t grid_row,size_t grid_col,uint8_t *&buf)
{
  float gsum=0;
  for (uint32_t col=0;col<pixel_section_r;col++)
  {
    for (uint32_t row=0;row<pixel_section_r;row++)
    {
      int V=  grid_col*pixel_section_c  + grid_row*image_width*pixel_section_r + col + (row*image_width);
      gsum += buf[V];
    }
  }
  return gsum;
}

/* create map from image based on global decl*/
void create_map(uint8_t*& gmap,uint8_t*& buf)
{
  gmap =(uint8_t*)malloc( map_size);
  for(uint32_t grid_col=0;grid_col<sections_c;grid_col++)
  {
    for(uint32_t grid_row=0;grid_row<sections_r;grid_row++)
    {
      float gsum = sum_grid(grid_row,grid_col,buf);
      gmap[grid_row*sections_r + grid_col] = gsum / (pixel_section_c*pixel_section_r);
    }
  }
  verbose_print(("%d:create map success\n",_line));
}

/* 1d image pixel, to grid 2d-coords and return 1d-coords */
uint32_t pixel_to_grid(uint32_t pix,uint32_t *grid_col,uint32_t *grid_row)
{
  uint32_t prow = pix / image_height; // pixel row
  uint32_t pcol = pix % image_width; // pixel col

  *grid_col = pcol / pixel_section_c; // which grid
  *grid_row = prow / pixel_section_r; // which grid
  return (*grid_col) + (*grid_row*sections_r);
}

/* grid coords to actual image 1d-2d-coords */
uint32_t grid_to_pixel(uint32_t grid_col,uint32_t grid_row)
{  return (grid_col*pixel_section_c + grid_row * image_width*pixel_section_r + grid_col + grid_row*image_width); }

/* map 1d-pixel to grid coords */
void map_pixel_to_grid(uint32_t pix,uint32_t *grid_col,uint32_t *grid_row)
{
  *grid_row = pix / sections_r;
  *grid_col = pix % sections_c;
}


/* map grid coords to 1d coords */
uint32_t map_grid_to_pixel(uint32_t grid_col,uint32_t grid_row)
{  return grid_row * sections_r + grid_col; }

/* check neighbours for value , retunrn true on first success otherwise false*/
bool map_check_neighbours_value(uint8_t* buf, uint32_t grid_col,uint32_t grid_row,uint8_t check_for)
{  
  uint32_t pixel_locations[3][3] = {
    { map_grid_to_pixel(grid_col-1,grid_row-1)  , map_grid_to_pixel(grid_col,grid_row-1) , map_grid_to_pixel(grid_col+1,grid_row-1)  },
    { map_grid_to_pixel(grid_col-1,grid_row)    , map_grid_to_pixel(grid_col,grid_row)   , map_grid_to_pixel(grid_col+1,grid_row)  },
    { map_grid_to_pixel(grid_col-1,grid_row+1)  , map_grid_to_pixel(grid_col,grid_row+1) , map_grid_to_pixel(grid_col+1,grid_row+1)  },
  };

  for(int row=0;row<3;row++)
  {
    for(int col=0;col<3;col++)
    {
      if ( pixel_locations[row][col] >= 0 && pixel_locations[row][col] < file_size)
      {
        if(buf[pixel_locations[row][col]] == check_for )
          return true;
      }
    }
  }
  return false;
}
/* same as above , but takes map 1d-pixel location and calls above */
bool map_check_neighbours_value(uint8_t* buf, uint32_t pix,uint8_t check_for)
{
  uint32_t grid_col,grid_row;
  map_pixel_to_grid(pix,&grid_col,&grid_row);
  return map_check_neighbours_value(buf,grid_col,grid_row,check_for);
}

/* convert pixel 2d coords to 1d coords */
uint32_t pixel_coord_to_1d(uint32_t pix_col,uint32_t pix_row)
{  return pix_row * image_width + pix_col; }

/* check neighbouring pixels values */
bool pixel_check_neighbours_value(uint8_t* buf,uint32_t pix,uint8_t check_for)
{
  // keep track of recursive calls
  static int counter=1;
  counter++;

  uint32_t row = pix / image_width;
  uint32_t col = pix % image_height;
  uint32_t pixel_locations[3][3] = {
    { pixel_coord_to_1d(col-1,row-1) ,pixel_coord_to_1d(col,row-1) , pixel_coord_to_1d(col+1,row-1) },
    { pixel_coord_to_1d(col-1,row) ,pixel_coord_to_1d(col,row) , pixel_coord_to_1d(col+1,row) },
    { pixel_coord_to_1d(col-1,row+1) ,pixel_coord_to_1d(col,row+1) , pixel_coord_to_1d(col+1,row+1) },
  };

  for(int irow=0;irow<3;irow++)
  {
    for(int icol=0;icol<3;icol++)
    {
      if ( pixel_locations[irow][icol] >= 0 && pixel_locations[irow][icol] < file_size)
      {
        if(buf[pixel_locations[irow][icol]] == check_for )
        { return true; }

        // oops , no good neighbours, do we need to recursivly check ?
        //if(counter <= despeckle_neighbours)
        //{ 
        //  if( pixel_check_neighbours_value(buf,pixel_locations[irow][icol],check_for) )
        //  {return true;}
        //}
      }
    }
  }
  return false;
}



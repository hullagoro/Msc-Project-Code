#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdint.h>
#include "fitsio.h"

typedef float storage_type;

int main(int argc, char *argv[])
{
    fitsfile *fptr;   /* FITS file pointer, defined in fitsio.h */
    int status = 0;   /* CFITSIO status value MUST be initialized to zero! */
    int bitpix, naxis, ii, anynul;
    long naxes[2] = {1,1}, fpixel[2] = {1,1};
    storage_type *pixels;
    char format[20], hdformat[20];

    if (argc < 2) {
      printf("Usage:  to_raw filename[ext][section filter] [output_filename]\n");
      printf("\n");
      printf("List the the pixel values in a FITS image \n");
      printf("\n");
      printf("Example: \n");
      printf("  to_raw image.fits                    - list the whole image\n");
      printf("  to_raw image.fits[100:110,400:410]   - list a section\n");
      printf("  to_raw table.fits[2][bin (x,y) = 32] - list the pixels in\n");
      printf("         an image constructed from a 2D histogram of X and Y\n");
      printf("         columns in a table with a binning factor = 32\n");
      printf(" size of double : %d\n",sizeof(double) );
      printf(" size of float : %d\n",sizeof(float) );
      printf(" size of TDouble : %d\n",sizeof(TDOUBLE) );
      printf(" size of TFloat : %d\n",sizeof(TFLOAT) );
      return(0);
    }

    if (!fits_open_file(&fptr, argv[1], READONLY, &status))
    {

        if (!fits_get_img_param(fptr, 2, &bitpix, &naxis, naxes, &status) )
        {
          if (naxis > 2 || naxis == 0)
             printf("Error: only 1D or 2D images are supported\n");
          else
          {
            // naxes[0] : row count
            // naxes[1] : col count
            // naxis dimensions of img
            //
            //
  /* acceptable types        
  TBYTE     unsigned char
  TSBYTE    signed char
  TSHORT    signed short
  TUSHORT   unsigned short
  TINT      signed int
  TUINT     unsigned int
  TLONG     signed long
  TLONGLONG signed 8-byte integer
  TULONG    unsigned long
  TFLOAT    double
  TDOUBLE   double
  */

            //
            /* get memory for 1 row */
            size_t fs = naxes[0] * naxes[1] * sizeof(storage_type) ; 
            printf("size of file (bytes) : %d\n",fs);
            pixels = (storage_type *) malloc(fs);

            if (pixels == NULL) {
                printf("Memory allocation error\n");
                return(1);
            }

            if (fits_read_pix(fptr, TFLOAT, fpixel, naxes[0] * naxes[1], NULL, pixels, NULL, &status) )  /* read all pixels */
            {
              printf("Error reasding image data\n"); 
              return(1);
            }


              FILE* fp=fopen(argv[2],"wb");
              fwrite(pixels,fs,1,fp);
              fclose(fp);


               //print first 10 value
              //for (int j=0;j<10;j++)
                //printf("%f,",pixels[j]);//,pixels[j*sizeof(storage_type)]);

            free(pixels);

          }
        }

        fits_close_file(fptr, &status);
    } 

    if (status) fits_report_error(stderr, status); /* print any error message */
    return(status);
}

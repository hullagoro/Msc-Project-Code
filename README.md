# Msc-Project-Code
Sample code for project report

all files are in ansi c and should compile without issue by typing the command "make" , in the exception of the file to_raw which requires the library fitsio

once compiled , all executables should be moved to the folder containing the raw FITS files and run in order 

NOTE: all raw files should have numerical names (in order) (e.g. 1 2 3 4 5 )

for the following we assume the files in the current folder are numbered 50-80 ( as in the test folder )

to create a mask : run the command ./create_mask 50 80 200 200 -10

this will create a mask by running through the files 50->80 , using threshold 1:200 , threshold 2:200 , threshold 3:-10 , and will create an output file
with the results "output.raw"

to apply the mask to the files : run the command ./apply_mask 50 80 

which will run the mask created on the files 50->80  and creates an output file names : out.raw

to calculate the energy emmitted : run the command ./calc_energy out.raw 
which will output all the energy calculations for the mentioned files .


NOTES TO KEEP IN MIND,
to keep the code simple and easy to compile, the resloution of the images is hard coded to 150x150 and only to 1byte images, the original code was modified to accept all formats, which is not the best practice , 
but that can be easily changed, this is due to lack of time to finish the report, but should show a good example of what is happeneing

To view the raw files : 

on viper you need to add the magick module using the command : module add image-magic/7.0.8-7/gcc-10.2.0

assuming the file you wish to view is "output.raw"

issue the command : magick display -depth 8 -size 150x150 gray:output.raw

I have already converted the files to png in the test/png folder for ease of view






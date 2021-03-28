
/*****************************************************************

 Copyright 2001   PIER LUCA MONTESSORO

 University of Udine
 ITALY

 montessoro@uniud.it
 www.montessoro.it

 This file is part of a freeware open source software package.
 It can be freely used (as it is or modified) as long as this
 copyright note is not removed.

******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bmp.hpp"


BITMAP ReadBitmap (FILE *fp)
{
   FILEHEADER fileheader;
   BMPHEADER bmpheader;
   COLORTRIPLE triple;
   BITMAP bitmap;
   int nstep;
   unsigned char fillbyte;
   int i,j, k, fill;

   /* read the headers */
   fread (&fileheader, sizeof(FILEHEADER), 1, fp);
   fread (&bmpheader, sizeof(BMPHEADER), 1, fp);

#ifdef BMPSHOWALL
   printf ("Following numbers are in hexadecimal representation\n");
   printf ("fileheader.ImageFileType = %x\n", fileheader.ImageFileType);
   printf ("fileheader.FileSize = %lx\n", fileheader.FileSize);
   printf ("fileheader.Reserved1 = %x\n", fileheader.Reserved1);
   printf ("fileheader.Reserved2 = %x\n", fileheader.Reserved2);
   printf ("fileheader.ImageDataOffset = %lx\n", fileheader.ImageDataOffset);

   printf ("bmpheader.HeaderSize = %lx\n", bmpheader.HeaderSize);
   printf ("bmpheader.ImageWidth = %lx\n", bmpheader.ImageWidth);
   printf ("bmpheader.ImageHeight = %lx\n", bmpheader.ImageHeight);
   printf ("bmpheader.NumberOfImagePlanes = %x\n",
           bmpheader.NumberOfImagePlanes);
   printf ("bmpheader.BitsPerPixel = %x\n", bmpheader.BitsPerPixel);
   printf ("bmpheader.CompressionMethod = %lx\n",
           bmpheader.CompressionMethod);
   printf ("bmpheader.SizeOfBitmap = %lx\n", bmpheader.SizeOfBitmap);
   printf ("bmpheader.HorizonalResolution = %lx\n",
           bmpheader.HorizonalResolution);
   printf ("bmpheader.VerticalResolution = %lx\n",
           bmpheader.VerticalResolution);
   printf ("bmpheader.NumberOfColorsUsed = %lx\n",
           bmpheader.NumberOfColorsUsed);
   printf ("bmpheader.NumberOfSignificantColors = %lx\n",
           bmpheader.NumberOfSignificantColors);
#endif

   if (fileheader.ImageFileType != BMPFILETYPE)
   {
     fclose (fp);
     printf ("Not a Windows bitmap file\n");
     exit (EXIT_FAILURE);
   }

   if (bmpheader.CompressionMethod != 0)
   {
     fclose (fp);
     printf ("Compressed images not supported\n");
     exit (EXIT_FAILURE);
   }

   switch (bmpheader.BitsPerPixel)
   {
      case 8:  /* 256 colors */
               fclose (fp);
               printf ("Color palette  not supported\n");
               exit (EXIT_FAILURE);
               break;

      case 16: /* 16 colors */
               fclose (fp);
               printf ("Color palette not supported\n");
               exit (EXIT_FAILURE);
               break;

       case 24: /* true colors */
               // fseek (fimg, fileheader.ImageDataOffset, 0);

               bitmap.pixel = (COLORTRIPLE *)
                  malloc (sizeof (COLORTRIPLE) *
                          bmpheader.ImageWidth * bmpheader.ImageHeight);
               if (bitmap.pixel == NULL)
               {
                  printf ("Memory allocation error\n");
                  exit (EXIT_FAILURE);
               }

               bitmap.width = bmpheader.ImageWidth;
               bitmap.height = bmpheader.ImageHeight;

               /* number of bytes is forced to be multiple of 4 */
               fill = bitmap.width % 4;

               for (i = 0; i < bitmap.height; i++)
               {
                  for (j = 0; j < bitmap.width; j++)
                  {
                     fread (&triple, sizeof(COLORTRIPLE), 1, fp);
                     nstep = j + (i * bitmap.width);
                     bitmap.pixel [nstep] = triple;
                  }
                  for (k = 0; k < fill; k++)
                     fread (&fillbyte, sizeof(unsigned char), 1, fp);

               }

#ifdef BMPSHOWALL
               printf ("%d pixels loaded\n", nstep + 1);
#endif
               break;

      default: /* unsupported format */
               printf ("Unsupported palette format\n");
               exit (EXIT_FAILURE);
               break;

   }

   /* save the headers read from the .bmp file */
   bitmap.fileheader = fileheader;
   bitmap.bmpheader = bmpheader;

   return bitmap;
}


void  WriteBitmap (BITMAP bitmap, FILE *fp)
{
   COLORTRIPLE triple;
   unsigned char fillbyte = 0;
   int nstep;
   int i, j, k, fill;


   fwrite (&bitmap.fileheader, sizeof (FILEHEADER), 1, fp);
   fwrite (&bitmap.bmpheader, sizeof (BMPHEADER), 1, fp);

   /* number of bytes in a row must be multiple of 4 */
   fill = bitmap.width % 4;

   for (i = 0; i < bitmap.height; i++)
   {
      for (j = 0; j < bitmap.width; j++)
      {
         nstep = j + (i * bitmap.width);
         triple = bitmap.pixel [nstep];
         fwrite (&triple, sizeof(COLORTRIPLE), 1, fp);
      }
      for (k = 0; k < fill; k++)
         fwrite (&fillbyte, sizeof(unsigned char), 1, fp);

   }

#ifdef BMPSHOWALL
   printf ("%d pixels written\n", nstep + 1);
#endif

   return;
}


void ReleaseBitmapData (BITMAP *bitmap)
{
   free ((*bitmap).pixel);
   (*bitmap).bmpheader.ImageHeight = (*bitmap).height = 0;
   (*bitmap).bmpheader.ImageWidth = (*bitmap).width = 0;
   (*bitmap).pixel = NULL;

   return;
}


BITMAP  CreateEmptyBitmap (dword height, dword width)
{
   BITMAP bitmap;

#ifdef BMPSHOWALL
   printf ("Creating empty bitmap %d x %d pixels\n", height, width);
#endif

   /* bitmap header */
   bitmap.fileheader.ImageFileType = BMPFILETYPE;   /* magic number! */
   bitmap.fileheader.FileSize = 14 + 40 + height * width * 3;
   bitmap.fileheader.Reserved1 = 0;
   bitmap.fileheader.Reserved2 = 0;
   bitmap.fileheader.ImageDataOffset = 14 + 40;

   /* bmp header */
   bitmap.bmpheader.HeaderSize = 40;
   bitmap.bmpheader.ImageWidth = bitmap.width = width;
   bitmap.bmpheader.ImageHeight = bitmap.height = height;
   bitmap.bmpheader.NumberOfImagePlanes = 1;
   bitmap.bmpheader.BitsPerPixel = 24;  /* the only supported format */
   bitmap.bmpheader.CompressionMethod = 0;  /* compression is not supported */
   bitmap.bmpheader.SizeOfBitmap = 0;  /* conventional value for uncompressed
                                          images */
   bitmap.bmpheader.HorizonalResolution = 0;  /* currently unused */
   bitmap.bmpheader.VerticalResolution = 0;  /* currently unused */
   bitmap.bmpheader.NumberOfColorsUsed = 0;  /* dummy value */
   bitmap.bmpheader.NumberOfSignificantColors = 0;  /* every color is
                                                       important */

   bitmap.pixel = (COLORTRIPLE *)
                  malloc (sizeof (COLORTRIPLE) * width * height);
   if (bitmap.pixel == NULL)
   {
      printf ("Memory allocation error\n");
      exit (EXIT_FAILURE);
   }

   return bitmap;
}
   

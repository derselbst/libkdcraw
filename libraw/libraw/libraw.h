/* -*- C++ -*-
 * File: libraw.h
 * Copyright 2008 Alex Tutubalin <lexa@lexa.ru>
 * Created: Sat Mar  8, 2008 
 *
 * LibRaw C++ interface
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#ifndef _LIBRAW_CLASS_H
#define _LIBRAW_CLASS_H

#include <limits.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>


#include "libraw_types.h"
#include "libraw_const.h"
#include "libraw_internal.h"
#include "libraw_alloc.h"

//#define DCRAW_VERBOSE

#ifdef __cplusplus
extern "C" 
{
#endif
DllDef    const char          *libraw_strerror(int errorcode);
    // LibRaw C API
DllDef    libraw_data_t       *libraw_init(unsigned int flags);
DllDef    int                 libraw_open_file(libraw_data_t*, const char *);
DllDef    int                 libraw_unpack(libraw_data_t*);
DllDef    int                 libraw_unpack_thumb(libraw_data_t*);
DllDef    void                libraw_recycle(libraw_data_t*);
DllDef    void                libraw_close(libraw_data_t*);

DllDef    void                libraw_set_memerror_handler(libraw_data_t*, memory_callback cb);
DllDef    void                libraw_set_dataerror_handler(libraw_data_t*,data_callback func);

    // DCRAW compatibility
DllDef    int                 libraw_adjust_sizes_info_only(libraw_data_t*);
DllDef    int                 libraw_dcraw_document_mode_processing(libraw_data_t*);
DllDef    int                 libraw_dcraw_ppm_tiff_writer(libraw_data_t* lr,const char *filename);
DllDef    int                 libraw_dcraw_thumb_writer(libraw_data_t* lr,const char *fname);
DllDef    int                 libraw_dcraw_process(libraw_data_t* lr);


#ifdef __cplusplus
}
#endif


#ifdef __cplusplus

class DllDef LibRaw
{
  public:
    libraw_data_t imgdata;
#ifdef DCRAW_VERBOSE
    int verbose;
#endif
    LibRaw(unsigned int flags = LIBRAW_OPTIONS_NONE)
        {
            double aber[4] = {1,1,1,1};
            unsigned greybox[4] =  { 0, 0, UINT_MAX, UINT_MAX };
#ifdef DCRAW_VERBOSE
            verbose = 1;
#endif
            bzero(&imgdata,sizeof(imgdata));
            bzero(&libraw_internal_data,sizeof(libraw_internal_data));
            mem_cb = (flags & LIBRAW_OPIONS_NO_MEMERR_CALLBACK) ? NULL:  &default_memory_callback;
            data_cb = (flags & LIBRAW_OPIONS_NO_DATAERR_CALLBACK)? NULL : &default_data_callback;
            memmove(&imgdata.params.aber,&aber,sizeof(aber));
            memmove(&imgdata.params.greybox,&greybox,sizeof(greybox));

            imgdata.params.bright=1;
            imgdata.params.use_camera_matrix=-1;
            imgdata.params.user_flip=-1;
            imgdata.params.user_black=-1;
            imgdata.params.user_sat=-1;
            imgdata.params.user_qual=-1;
            imgdata.params.output_color=1;
            imgdata.params.output_bps=8;
            imgdata.params.use_fuji_rotate=1;
            imgdata.parent_class = this;
            imgdata.progress_flags = 0;
#ifdef LIBRAW_THREADS
            tls.init();
#endif
        }
    int         main (int argc, char **argv);
    
    libraw_output_params_t*     output_params_ptr() { return &imgdata.params;}
    int                         open_file(const char *fname);
    int                         unpack(void);
    int                         unpack_thumb(void);

    int                         adjust_sizes_info_only(void);
    void                        set_memerror_handler(memory_callback cb) { mem_cb = cb; }
    void                        set_dataerror_handler(data_callback func) { data_cb = func;}
    // dcraw emulation
    int                         dcraw_document_mode_processing();
    int                         dcraw_ppm_tiff_writer(const char *filename);
    int                         dcraw_thumb_writer(const char *fname);
    int                         dcraw_process(void);

    // free all internal data structures
    void         recycle() 
        {
            if(libraw_internal_data.internal_data.input) 
                { 
                    fclose(libraw_internal_data.internal_data.input); 
                    libraw_internal_data.internal_data.input = NULL;
                }
#define FREE(a) do { if(a) { free(a); a = NULL;} }while(0)
            
            FREE(imgdata.image); 
            FREE(imgdata.thumbnail.thumb);
            FREE(libraw_internal_data.internal_data.meta_data);
            FREE(libraw_internal_data.output_data.histogram);
            FREE(imgdata.color.profile);
#undef FREE
            memmgr.cleanup();
            imgdata.thumbnail.tformat = LIBRAW_THUMBNAIL_UNKNOWN;
            imgdata.progress_flags = 0;
#ifdef LIBRAW_THREADS
            tls.init();
#endif
       }
    ~LibRaw(void) 
        { 
            recycle(); 
        }

    int FC(int row,int col) { return (imgdata.idata.filters >> ((((row) << 1 & 14) + ((col) & 1)) << 1) & 3);}

    int         fc (int row, int col)
        {
            static const char filter[16][16] =
                { { 2,1,1,3,2,3,2,0,3,2,3,0,1,2,1,0 },
                  { 0,3,0,2,0,1,3,1,0,1,1,2,0,3,3,2 },
                  { 2,3,3,2,3,1,1,3,3,1,2,1,2,0,0,3 },
                  { 0,1,0,1,0,2,0,2,2,0,3,0,1,3,2,1 },
                  { 3,1,1,2,0,1,0,2,1,3,1,3,0,1,3,0 },
                  { 2,0,0,3,3,2,3,1,2,0,2,0,3,2,2,1 },
                  { 2,3,3,1,2,1,2,1,2,1,1,2,3,0,0,1 },
                  { 1,0,0,2,3,0,0,3,0,3,0,3,2,1,2,3 },
                  { 2,3,3,1,1,2,1,0,3,2,3,0,2,3,1,3 },
                  { 1,0,2,0,3,0,3,2,0,1,1,2,0,1,0,2 },
                  { 0,1,1,3,3,2,2,1,1,3,3,0,2,1,3,2 },
                  { 2,3,2,0,0,1,3,0,2,0,1,2,3,0,1,0 },
                  { 1,3,1,2,3,2,3,2,0,2,0,1,1,0,3,0 },
                  { 0,2,0,3,1,0,0,1,1,3,3,2,3,2,2,1 },
                  { 2,1,3,2,3,1,2,1,0,3,0,2,0,2,0,2 },
                  { 0,3,1,0,0,2,0,3,2,1,3,1,1,3,1,3 } };
            
            if (imgdata.idata.filters != 1) return FC(row,col);
            return filter[(row+imgdata.sizes.top_margin) & 15][(col+imgdata.sizes.left_margin) & 15];
        }

  private:
    void*        malloc(size_t t)
        {
            void *p = memmgr.malloc(t);
            return p;
        }
    void*        calloc(size_t n,size_t t)
        {
            void *p = memmgr.calloc(n,t);
            return p;
        }
    void        free(void *p)
        {
            memmgr.free(p);
        }
    void        merror (void *ptr, const char *where)
        {
            if (ptr) return;
            if(mem_cb)(*mem_cb)(libraw_internal_data.internal_data.ifname,where);
            throw LIBRAW_EXCEPTION_ALLOC;
        }
    void        derror()
        {
            if (!libraw_internal_data.unpacker_data.data_error) 
                {
                    if (feof(libraw_internal_data.internal_data.input))
                        {
                            if(data_cb)(*data_cb)(libraw_internal_data.internal_data.ifname,-1);
                            throw LIBRAW_EXCEPTION_IO_EOF;
                        }
                    else
                        {
                            if(data_cb)(*data_cb)(libraw_internal_data.internal_data.ifname,
                                                  ftell(libraw_internal_data.internal_data.input));
                            throw LIBRAW_EXCEPTION_IO_CORRUPT;
                        }
                }
            libraw_internal_data.unpacker_data.data_error = 1;
        }

// data

#ifdef LIBRAW_THREADS
    LibRaw_TLS  tls;
#endif
    libraw_internal_data_t libraw_internal_data;
    decode      first_decode[2048], *second_decode, *free_decode;
    tiff_ifd_t  tiff_ifd[10];
    libraw_memmgr memmgr;

    LibRaw_constants rgb_constants;
    void        (LibRaw:: *write_thumb)(FILE *), 
                (LibRaw:: *write_fun)(FILE *);
    void        (LibRaw:: *load_raw)(),
                (LibRaw:: *thumb_load_raw)();
    memory_callback mem_cb;
    data_callback data_cb;

    void        kodak_thumb_loader();
    void        write_thumb_ppm_tiff(FILE *); // kodak
    void        foveon_thumb_loader (void); //Sigma

    
    // moved from implementation level to private: visibility
    void        identify();
    void        write_ppm_tiff (FILE *ofp);
    void        convert_to_rgb();
    void        kodak_ycbcr_load_raw();
    void        remove_zeroes();

// Iterpolators
    void        pre_interpolate();
    void        border_interpolate (int border);
    void        lin_interpolate();
    void        vng_interpolate();
    void        ppg_interpolate();
    void        ahd_interpolate();

// Image filters
    void        hat_transform (float *temp, float *base, int st, int size, int sc);
    void        wavelet_denoise();
    void        scale_colors();
    void        median_filter ();
    void        blend_highlights();
    void        recover_highlights();

    void        fuji_rotate();
    void        stretch();

// Thmbnail functions
    void        foveon_thumb (FILE *tfp);
    void        jpeg_thumb_writer (FILE *tfp,char *thumb,int thumb_length);
    void        jpeg_thumb (FILE *tfp);
    void        ppm_thumb (FILE *tfp);
    void        layer_thumb (FILE *tfp);
    void        rollei_thumb (FILE *tfp);
    void        kodak_thumb_load_raw();

    // utility for cut'n'pasted code
    void        foveon_decoder (unsigned size, unsigned code);
    unsigned    get4();

    int         flip_index (int row, int col);
    void        gamma_lut (uchar lut[0x10000]);


// == internal functions

#ifdef LIBRAW_LIBRARY_BUILD 
#include "internal/libraw_internal_funcs.h"
#endif

};

#endif // __cplusplus


#endif // _LIBRAW_CLASS_H
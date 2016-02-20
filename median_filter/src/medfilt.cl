/*
 *  median_filter: A demo showing CLTune applied to a median filter.
 *  Copyright (C) 2016 William John Shipman
 *  
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *  
 *  Created on: 16 Jan 2016
 *      Author: William John Shipman (@williamjshipman).
 */

//#pragma OPENCL EXTENSION cl_amd_printf: enable

//#include "medfilt.clh"

#if USE_LOCAL_MEM == 0

float median(
    read_only global float* img,
    const int id_x,
    const int id_y,
    const int width,
    const int height)
{
    float window[FILTER_WIDTH * FILTER_WIDTH];
    for (int wnd_y = 0; wnd_y < FILTER_WIDTH; wnd_y++)
    {
        for (int wnd_x = 0; wnd_x < FILTER_WIDTH; wnd_x++)
        {
            PIXEL(wnd_x, wnd_y, FILTER_WIDTH, window) = read_pixel(
                id_x+wnd_x-HALF_WIDTH,
                id_y+wnd_y-HALF_WIDTH,
                width,
                height,
                img);
        }
    }

    BubbleSort(window);
    
    return window[FILTER_WIDTH*FILTER_WIDTH/2];
}

__attribute__((reqd_work_group_size(TBX, TBY, 1)))
__kernel void medfilt(
    read_only global float* imgIn,
    write_only global float* imgOut,
    const int width,
    const int height)
{
    for (size_t id_y = get_global_id(1); id_y < height; id_y += get_global_size(1))
    {
        for (size_t id_x = get_global_id(0); id_x < width; id_x += get_global_size(0))
            PIXEL(id_x, id_y, width, imgOut) = median(imgIn, id_x, id_y, width, height);
	}
}

#elif USE_LOCAL_MEM == 1

#define LMEM_WIDTH (TBX + 2*HALF_WIDTH)
#define LMEM_HEIGHT (TBY + 2*HALF_WIDTH)

void load_local_memory(
    read_only global float* imgIn,
    local float* imgOut,
    const int tile_x,
    const int tile_y,
    const int width,
    const int height)
{
    for (int ly = get_local_id(1); ly < LMEM_HEIGHT; ly+=TBY)
    {
        for (int lx = get_local_id(0); lx < LMEM_WIDTH; lx+=TBX)
        {
            PIXEL(lx, ly, LMEM_WIDTH, imgOut) = read_pixel(
                tile_x+lx-HALF_WIDTH,
                tile_y+ly-HALF_WIDTH,
                width,
                height,
                imgIn);
            //printf("%u x %u lmem - %u,%u - %u,%u: %f\n", LMEM_WIDTH, LMEM_HEIGHT, lx, ly, tile_x+lx-HALF_WIDTH, tile_y+ly-HALF_WIDTH, PIXEL(lx, ly, LMEM_WIDTH, imgOut));
        }
    }

    barrier(CLK_LOCAL_MEM_FENCE);

    /*if ((get_local_id(0) == 0) & (get_local_id(1) == 0))
    {
        size_t idx = 0;
        for (size_t ly = 0; ly < LMEM_HEIGHT; ly++)
        {
            for (size_t lx = 0; lx < LMEM_WIDTH; lx++)
            {
                printf("%f,", imgOut[idx]);
                idx++;
            }
            printf("\n");
        }
    }*/
}

float median(
    read_only global float* img,
    local float* lmem,
    const int id_x,
    const int id_y,
    const int width,
    const int height)
{
    size_t lx = get_local_id(0);
    size_t ly = get_local_id(1);

    load_local_memory(img, lmem, get_group_id(0) * TBX, get_group_id(1) * TBY, width, height);

    float window[FILTER_WIDTH * FILTER_WIDTH];
    //printf("Filter window at %u,%u:\n", lx, ly);
    for (int wnd_y = 0; wnd_y < FILTER_WIDTH; wnd_y++)
    {
        for (int wnd_x = 0; wnd_x < FILTER_WIDTH; wnd_x++)
        {
            PIXEL(wnd_x, wnd_y, FILTER_WIDTH, window) = PIXEL(lx+wnd_x, ly+wnd_y, LMEM_WIDTH, lmem);
            /*printf("%u,%u, - %u,%u - %f,", lx+wnd_x, ly+wnd_y, wnd_x, wnd_y,
                window[wnd_x + wnd_y * FILTER_WIDTH]);*/
        }
        //printf("\n");
    }

    BubbleSort(window);

    /*printf("Filter window at %u,%u after sorting:\n", lx, ly);
    for (int wnd_y = 0; wnd_y < FILTER_WIDTH; wnd_y++)
    {
        for (int wnd_x = 0; wnd_x < FILTER_WIDTH; wnd_x++)
        {
            printf("%u,%u, - %u,%u - %f,", lx+wnd_x, ly+wnd_y, wnd_x, wnd_y,
                window[wnd_x + wnd_y * FILTER_WIDTH]);
        }
        printf("\n");
    }*/

    return window[FILTER_WIDTH*FILTER_WIDTH/2];
}

__attribute__((reqd_work_group_size(TBX, TBY, 1)))
__kernel void medfilt(
    read_only global float* imgIn,
    write_only global float* imgOut,
    const int width,
    const int height)
{
    /*if ((get_global_id(0) == 0) & (get_global_id(1) == 0))
    {
        for (size_t ly = 0; ly < height; ly++)
        {
            for (size_t lx = 0; lx < width; lx++)
                printf("%f,", PIXEL(lx, ly, width, imgIn));
            printf("\n");
        }
    }*/
    local float lmem[LMEM_WIDTH * LMEM_HEIGHT];
    for (size_t id_y = get_global_id(1); id_y < height; id_y += get_global_size(1))
    {
        for (size_t id_x = get_global_id(0); id_x < width; id_x += get_global_size(0))
            PIXEL(id_x, id_y, width, imgOut) = median(imgIn, lmem, id_x, id_y, width, height);
    }
}

#endif
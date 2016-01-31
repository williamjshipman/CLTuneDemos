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

#pragma OPENCL EXTENSION cl_amd_printf: enable

#define HALF_WIDTH 3
#define FILTER_WIDTH (HALF_WIDTH*2+1)

#define PIXEL(X, Y, W, IMG) (IMG[(X) + (Y)*(W)])

float read_pixel(const int x, const int y,
    const int w, const int h,
    global float* img)
{
    if ((x >= 0) & (y >= 0) & (x < w) & (y < h))
        return PIXEL(x, y, w, img);
    else
        return 0.0f;
}

void BubbleSort(float* window)
{
    for (size_t idx1 = 0; idx1 < FILTER_WIDTH*FILTER_WIDTH; idx1++)
    {
        for (size_t idx2 = 0; idx2 < idx1; idx2++)
        {
            if (window[idx2-1] > window[idx2])
            {
                // Swap.
                float temp = window[idx2];
                window[idx2] = window[idx2-1];
                window[idx2-1] = temp;
            }
        }
    }
}

#if USE_LOCAL_MEM == 0

float median(
    read_only global float* img,
    const size_t id_x,
    const size_t id_y,
    const size_t width,
    const size_t height)
{
    float window[FILTER_WIDTH * FILTER_WIDTH];
    for (size_t wnd_y = 0; wnd_y < FILTER_WIDTH; wnd_y++)
    {
        for (size_t wnd_x = 0; wnd_x < FILTER_WIDTH; wnd_x++)
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
    const size_t tile_x,
    const size_t tile_y,
    const size_t width,
    const size_t height)
{
    for (size_t ly = get_local_id(1); ly < LMEM_HEIGHT; ly+=TBY)
    {
        for (size_t lx = get_local_id(0); lx < LMEM_WIDTH; lx+=TBX)
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
    const size_t id_x,
    const size_t id_y,
    const size_t width,
    const size_t height)
{
    size_t lx = get_local_id(0);
    size_t ly = get_local_id(1);

    load_local_memory(img, lmem, get_group_id(0) * TBX, get_group_id(1) * TBY, width, height);

    float window[FILTER_WIDTH * FILTER_WIDTH];
    //printf("Filter window at %u,%u:\n", lx, ly);
    for (size_t wnd_y = 0; wnd_y < FILTER_WIDTH; wnd_y++)
    {
        for (size_t wnd_x = 0; wnd_x < FILTER_WIDTH; wnd_x++)
        {
            PIXEL(wnd_x, wnd_y, FILTER_WIDTH, window) = PIXEL(lx+wnd_x, ly+wnd_y, LMEM_WIDTH, lmem);
            /*printf("%u,%u, - %u,%u - %f,", lx+wnd_x, ly+wnd_y, wnd_x, wnd_y,
                window[wnd_x + wnd_y * FILTER_WIDTH]);*/
        }
        //printf("\n");
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
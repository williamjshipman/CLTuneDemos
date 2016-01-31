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

#define HALF_WIDTH 3
#define FILTER_WIDTH (HALF_WIDTH*2+1)

#define PIXEL(X, Y, W, IMG) (IMG[(X) + (Y)*(W)])

float read_pixel(const size_t x, const size_t y,
    const size_t w, const size_t h,
    read_only global float* img)
{
    if ((x >= 0) & (y >= 0) & (x < w) & (y < h))
        return PIXEL(x, y, w, img);
    else
        return 0.0f;
}

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

    return window[FILTER_WIDTH*FILTER_WIDTH/2];
}

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
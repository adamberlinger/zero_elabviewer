/*
 * Copyright 2016 Adam Berlinger
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */
#include "SpectralAnalysis.h"
#include <cmath>

/* TODO: use memcpy */

SpectralAnalysis::SpectralAnalysis(int dataSize){
    this->dataSize = dataSize;
    this->fftBuffer = fftw_alloc_complex(dataSize);
    this->inputBuffer = fftw_alloc_real(dataSize);
    this->resultBuffer = fftw_alloc_real(dataSize);
    this->useWindow = true;

    for(int i = 0;i < dataSize; ++i){
        this->inputBuffer[i] = 0.0;
    }

    plan = fftw_plan_dft_1d(dataSize, fftBuffer, fftBuffer, FFTW_FORWARD, FFTW_ESTIMATE);
}

void SpectralAnalysis::resize(int newDataSize){
    double *newInputBuffer = fftw_alloc_real(newDataSize);
    int maxSize = (newDataSize > dataSize)?dataSize:newDataSize;
    for(int i = 0;i < maxSize;++i){
        newInputBuffer[i] = inputBuffer[i];
    }

    for(int i = maxSize; i < newDataSize;++i){
        newInputBuffer[i] = 0.0;
    }

    this->lastSignalSize = (this->lastSignalSize > newDataSize)?newDataSize:this->lastSignalSize;

    fftw_free(inputBuffer);
    inputBuffer = newInputBuffer;
    fftw_free(fftBuffer);
    fftw_free(resultBuffer);

    this->dataSize = newDataSize;

    this->fftBuffer = fftw_alloc_complex(dataSize);
    this->resultBuffer = fftw_alloc_real(dataSize);

    fftw_destroy_plan(plan);
    plan = fftw_plan_dft_1d(dataSize, fftBuffer, fftBuffer, FFTW_FORWARD, FFTW_ESTIMATE);
}

const double* SpectralAnalysis::compute(const double* input,int size,double meanValue){

    int i;
    int maxSize = (size > dataSize)?dataSize:size;
    for(i = 0;i < maxSize;++i){
        inputBuffer[i] = (input[i] - meanValue);
    }

    for(i = maxSize;i < dataSize;++i){
        inputBuffer[i] = 0.0;
    }

    this->lastSignalSize = maxSize;

    return this->recompute();
}

const double* SpectralAnalysis::recompute(){
    /* TODO: recompute window */

    for(int i = 0;i < dataSize;++i){
        fftBuffer[i][0] = inputBuffer[i];
        fftBuffer[i][1] = 0.0;
    }

    if(useWindow){
        for(int i = 0;i < this->lastSignalSize;++i){
            fftBuffer[i][0] *= (0.54 - 0.46 * cos((2 * 3.1415926 * i) / (this->lastSignalSize - 1)));
        }
    }

    fftw_execute(plan);

    for(int i = 0;i < dataSize;++i){
        resultBuffer[i] = std::sqrt(fftBuffer[i][0] * fftBuffer[i][0] + fftBuffer[i][1] * fftBuffer[i][1]);
    }

    return this->resultBuffer;
}

SpectralAnalysis::~SpectralAnalysis(){
    fftw_destroy_plan(plan);
    fftw_free(this->fftBuffer);
    fftw_free(this->inputBuffer);
    fftw_free(this->resultBuffer);
}

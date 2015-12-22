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
#ifndef _SPECTRAL_ANALYSIS_H_
#define _SPECTRAL_ANALYSIS_H_

#include <stdint.h>

#include <fftw3.h>

class SpectralAnalysis {
protected:
    int dataSize;
    fftw_plan plan;
    double *inputBuffer;
    fftw_complex *fftBuffer;
    double *resultBuffer;
    bool useWindow;
    int lastSignalSize;
public:
    SpectralAnalysis(int dataSize);
    void enableWindow(bool value){this->useWindow = value;}
    const double* compute(const double* input,int size, double meanValue = 0);
    const double* recompute();
    void resize(int newDataSize);
    virtual ~SpectralAnalysis();
};

#endif /* _SPECTRAL_ANALYSIS_H_ */

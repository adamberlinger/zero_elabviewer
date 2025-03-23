# Zero eLab Viewer Community

Zero eLab Viewer Community is a PC program for displaying and
controlling external data acquisition system through standard
communication interface such as USB CDC class.

This project is a community fork of the official Zero eLab Viewer code base
developed by Adam Berlinger at
[@adamberlinger/zero_elabviewer](https://github.com/adamberlinger/zero_elabviewer).

## Supported devices

The application currently works with firmware running on STM32F042 and STM32L072,
ARM Cortex-M0/M0+ based devices. The communication is realized through embedded
USB device "crystal-less" peripheral. This allows a minimalistic design
based around a microcontroller and few external parts, such as voltage regulator,
decoupling capacitors and USB connector.

## Used libraries

The program uses these libraries:
* Qt framework (http://www.qt.io)
* QCustomPlot library (http://www.qcustomplot.com)
* fftw library (http://www.fftw.org)

All are available under GNU GPL or LGPL license.

## Running release on Linux

./run.sh should run the application
./debug_libs.sh can be used to produce debug output for tracking missing .so libraries

On Ubuntu, following package might be needed to run app properly:
```
sudo apt-get install libxcb-xinerama0
```

## Copyright

Developed as a part of a student project at Department of Measurement,
Faculty of Electrical Engineering,
Czech Technical University in Prague.

Similar projects can be found here: http://embedded.fel.cvut.cz/sdi

Copyright (C) 2016-2018  Adam Berlinger

## Future development

Currently the application is working with few issues, however some parts
of the code need to be rewritten in order to be readable and re-usable.
Also the firmware source code will be published in the near future
(as separate project).

## Feedback & Contribution

Anyone is welcomed to provide feedback or contribute to the projects.
As mentioned above, some parts of the code are not well written and require
refactoring which will have to be done probably by the author.
Also currently there are no clear coding standards defined.

Anyway if you find some bug, or have some idea for improving the application,
you can submit an issue on github.

## License

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

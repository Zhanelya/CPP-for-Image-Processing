C++ for Image Processing
This code is part of the coursework for C++ for Image Processing course. It involves writing an image processing program in c++.

###Main functionalities:

#####1. Loading image and basic filtering:

-loads and displays an image, pixel value rescaling and shifting;

-performs a convolution on the image forsmoothing the image;

-edge detection in the image;

-performs order statistic filtering;

-performs point processing;

#####2. Improving/altering the image contrast:

-thresholding;

-histogram equalisation;

#####3. Selecting a Region of Interest (ROI). Performing the image processing function on the ROI.

#####4. Menu system. Ability to perform a combination of image processing functions on the ROI, including an "undo" function.

How to run the program:

If you want to explore the project from your own machine, then g++ and wxwindows are easy to install under linux. To run the program, simply type ./window in your command line once you are in the correct directory. If you must use windows, cygwin is one option - it comes with gcc/g++ and wxwindows does work as well.Alternatively, it is very easy installing a dual boot linux machine at home. If you want to alter the code, use the Make file to compile the source into a runnable object and run it to start the program.

# Spatiotemporal Variance-guided Filtering
to explain myself the pipeline and maybe fix the bugs

do not follow what im saying
## Basic Idea
It gaussian filters each pixel by its variance, which is calculated by accumulation and the pixels around it.
## Steps
1. Temporally accumulating the path-traced inputs
2. Using these inputs to estimate the luminance variance
3. Using the luminance variance to control the multiple step a-trous wavelet filter
The paper states there will be some extra post-processing, which I don't assume is needed.
- It will apply a tone map onto the reconstructed image
- It will then apply a TAA (Temporal Anti-Aliasing) to the image
That will be the output of the SVGF.
## Details
I will go through each step of the filter in **figure 3**.
### Input textures
There are 2 types of input textures: The current frame data textures & the previous frame data.
#### Current frame data
1. Motion
2D vector of each pixel's movement prior to the previous frame in texture space (UV).
2. Normal
Normal of each pixel. This is given as an input as one of the checks whether the normal difference is small enough to blur.
3. Colour
Output the pathtracer has created. **I don't fully understand this yet**
4. Depth
Depth of each pixel. This is given as an input as one of the checks whether the depth difference is small enough to blur.
5. Mesh ID
The mesh ID of each pixel. This is given as an input to stop the filter from accidentally blurring 2 seperate objects together.
#### Previous frame data
1. Moments History
Moments accumulation textures. This is given for variance estimation.
2. Colour History
Colour accumulation textures. This is given for variance estimation.
3. Normal
Previous normal of each pixel. This is given as an input to compare between the input and output to see if TAA can be applied.
4. Depth
Previous depth of each pixel. This is given as an input to compare between the input and output to see if TAA can be applied.
5. Mesh ID
The previous mesh ID of each pixel. This is given as an input to compare between the input and output to see if TAA can be applied.
### Temporal Accumulation
The filter first temporally accumulates the colour texture and the moment textures. In figure 3, the textures I'm referring to are the Moments History and the Colour History textures. 
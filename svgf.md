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
### Input Textures
There are 2 types of input textures: The current frame data textures & the previous frame data.
#### Current Frame Data
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
#### Previous Frame Data
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
The filter first temporally accumulates the colour texture and the moment textures. In figure 3, the textures I'm referring to are the Moments History and the Colour History textures. After integration, the moment textures get outputted back into the previous frame data. The integrated colour texture does not.

The equation for accumulation is an exponential moving average. The equation is as follows:

$$C'_i = \alpha \cdot C_i + (1 - \alpha) \cdot C'_{i-1}$$

Where $\alpha$ is the control of the temporal fade, $C'_i$ is the integrated colour at the current frame $i$, and $C_i$ being the current frame data colour at frame $i$. The paper states $\alpha = 0.2$ works best.
#### Disocclusion Event Handling
I don't fully understand how this works yet, so don't follow my word.

Each pixel tests a 2x2 region around it (I don't know which pixels it is) and tests the depth, normals, and mesh IDs from the previous frame. This test is for a dissoclussion, where a pixel which has been occluded by another pixel and now has been unoccluded from camera movement. This is so the filter doesn't blur a completely different pixel from another one. They check the depth, normal, and mesh ID versus the prior frames' corresponding data. How they do it is by testing different thresholds of each test. In the case where a test fails, it will be considered a recently disoccluded pixel.

The thresholds will be represented as $\tau_n$ and $\tau_z$. Also, whenever handling a prior frame pixel, I will be representing the new 2D vector motion update pixel as $p_{i-1}$.

##### Normal test

The test is defined as the following:

$$n_i(p) \cdot n_{i-1}(p_{i-1}) > \tau_n$$

Where $\tau_n$ can be from 1 to -1.

##### Depth test

The test is defined as the following:

$$|z_i(p) - z_{i-1}(p_{i-1})| < \tau_z$$

Where $\tau_z$ can be from 0 to 1.

##### Mesh ID test

The test doesn't have a boolean equation, and it just checks if the current pixel's mesh ID equals to the prior mesh ID pixel.

If all of these tests pass, it is allowed to use the regular luminance variance equation. If it doesn't, it advances to a 3x3 tap filter **which I didn't learn about yet**.
### Variance Estimation
What SVGF uses is luminance variance estimation inside one of its weight factors. This affects the a-trous wavelet filter. 

We will represent the first and second non-accumulated moments as $\mu_1$ and $\mu_2$, and the accumulated moments will be $\mu'_1$ and $\mu'_2$.

Calculation of temporal variance, which is represented as $\sigma'^2_i$ is very simple, and the equation is as follows:
$$\sigma'^2_i = \mu'_{2_i} - \mu'^2_{1_i}$$
This will also be represented as the function: $\mathrm{Var}(\hat{c}(p))$.
#### Insufficient Data for Variance Estimation
I also don't fully understand how this works.

When a pixel gets disoccluded, it doesn't have enough moment data to make a good luminance variance estimation. To solve this, we use a different spatial approach to finding the luminance, which uses a 7x7 bilateral filter.

How we can choose this estimate versus the more accurate and faster equation is by checking if a pixel has been temporally accumulated for at least 4 frames. If not, use this fallback.

What the paper is implicitly telling you to do is to spatially weighted average a 7x7 pixel grid around you. The weight will be calculated by the edge-avoiding function. I will call this function $\mathrm{Var}_{\mathrm{spatial}}(p)$. Therefore, the equation for a pixel should be the following:

$$\mathrm{Var}_{\mathrm{spatial}}(p) = \frac{\sum_{q\in N_{7x7}}w(p,q) \cdot \mathrm{Var}(\hat{c}(q))}{\sum_{q\in N_{7x7}}w(p,q)}$$

Which is just how a weighted average is implemented.

### Edge-Avoiding A-Trous Wavelet Transform
One of the main ideas in this filter is that it goes through multiple variations, with $2^i$ spacing for certain kernels (that's just a-trous filtering), where $i$ is the iteration number.

Most of the edge avoidance is from a weight function, which is defined as the following: 
$$w_i(p, q) = w_z \cdot w_n \cdot w_l$$
$w_z$, $w_n$, and $w_l$ will be talked about later on.

A-trous filtering is used in the main equation that finds the output colour of the iteration. The equation is defined as followed:

$$\hat{c}_{i+1}(p) = \frac
{\sum_{q\in\Omega} h(q) \cdot w(p, q) \cdot \hat{c}_i(q)}
{\sum_{q\in\Omega} h(q) \cdot w(p, q)}$$

Where $\hat{c}_i(p)$ is the iteration colour, $\hat{c}_{i+1}(p)$ is the next iteration's colour, $\Omega$ being the gathered filter footprint which changes on the iteration (I think), and $h$ being a kernel which is defined as ${h = ( \frac{1}{16} , \frac{1}{4}, \frac{3}{8}, \frac{1}{4}, \frac{1}{16})}$. $p$ is the centre pixel and $q$ is a neighbouring pixel.
#### Tuning the Function
What tuning the function means is essentially outputting the variance for the next iteration to use, so therefore it's tuning it. However, what it's outputting is not the luminance variance. What it's talking about is the **propagated variance**.

Propagated variance calculation is similar to how the final colour of the iteration is calculated. The equation is defined as followed:

$$\mathrm{Var}(\hat{c}_{i+1}(p)) = \frac
{\sum_{q\in\Omega} h(q)^2 \cdot w(p, q)^2 \cdot \mathrm{Var}(\hat{c}_i(q))}
{(\sum_{q\in\Omega} h(q) \cdot w(p, q))^2}$$

Where $\mathrm{Var}(\hat{c}_i(p))$ is the propagated variance of an output colour at point $p$.

Propagated variance is calculated for the next iteration because of a few reasons. First of all, it optimizes luminance variance calculation ($\sigma'^2_i = \mu'_{2_i} - \mu'^2_{1_i}$), and it's much more correct than the luminance variance estimate.
#### Edge stopping function definitions
We will use 3 weight functions which use depth, normal, and luminance respective to the order $w_i(p, q)$ was defined in.

We will use 3 parameters which will tune the final output of the weight. These will be constant and set by the programmer, only to be used for debugging and looking for the best output. These functions are $\sigma_z$, $\sigma_n$, and $\sigma_l$. These parameters are $\sigma_z = 1$, $\sigma_n = 128$, and $\sigma_l = 4$.
##### Depth Weight
The function is defined as:

$$w_z = \mathrm{exp}(-\frac{|z(p) - z(q)|}{\sigma_z|\nabla z(p) \cdot (p - q)| + \epsilon})$$

Where $z(p)$ is the depth of a point, and $\nabla z(p)$ is the gradient/change of the point. $\epsilon$ is added to make sure division of 0 won't happen.
##### Normal Weight
The function is defined as:

$$w_n = \mathrm{max}(0, n(p) \cdot n(q))^{\sigma_n}$$

Where $n(p)$ is the normal of a point.
##### Luminance Weight
I don't fully understand this

The function is defined as:

$$w_l = \mathrm{exp}(-\frac{|l_i(p) - l_i(q)|}{\sigma_l \sqrt{g_{\mathrm{3x3}}(\mathrm{Var}(l_i(p))) + \epsilon}})$$

Where $l_i(p)$ is the luminance of a point, $Var(l_i(p))$ is the luminance variance of a point, and $g_{\mathrm{3x3}}$ blurs it. $\epsilon$ is, again, used to ensure division by 0 will not happen.
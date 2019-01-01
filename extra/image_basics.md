# Image Basics

## Color

- Hue: pure color, presented by a circle
- Saturation: purity of color, how far it is from pure white, which is every color combined
- Brightness: subjective perception, how far it is from grey
- Luminance: objective, cd/m2
- Contrast: 
- Sharpness: 

## Noise

Image noise is random variation of brightness or color information in images, and is usually an aspect of electronic noise.

- Gaussian noise
- Salt-and-pepper noise
- Shot noise
- Quantization noise
- Film grain
- Anisotropic noise
- Periodic noise

## Photography

### Backlight

The process of illuminating the subject from the back. The lighting instrument and the viewer face each other, with the subject in between, creating a **glowing** effect on the edges of subject, which helps to seperate the subject and the background. In photography, a back light (often the sun) that is about sixteen times more intense than the **key light** produces a **silhouette**.

### Dynamic range

Limits of luminance range that a given digital camera can capture, aka **reflectance range**.

### High-dynamic-range imaging

A HDR technique to reproduce greater dynamic range of luminosity than is possible with standard digital imaging or photographic techniques. The human eye adjusts constantly to adapt to a broad range of luminance, but most cameras do not. Two primary types of HDR images: **computer renderings** and images resulting from merging multiple LDR photographs.

### Tone mapping

A technique to map one set of colors to another to approximate the appearance of HDR images in a medium that has a more limited dynamic range. Global or local operators have been developed.

In some cases, tone mapped images are produced from a single exposure which is then manipulated with conventional processing tools to produce the inputs to the HDR image generation process. This avoids the artifacts that can appear when different exposures are combined, due to moving objects in the scene or camera shake. However, when tone mapping is applied to a single exposure in this way, the intermediate image has only normal dynamic range, and the amount of shadow or highlight detail that can be rendered is only that which was captured in the original exposure.

### Edge-preserving smoothing filter

An image processing technique that smooths away noise or textures while retaining sharp edges. Examples are the median, bilateral, guided, and anisotropic diffusion filters.

### Exposure fusion

A technique for blending multiple exposures of the same scene into a single image. As in high dynamic range imaging (HDRI or just HDR), the goal is to capture a scene with a higher dynamic range than the camera is capable of capturing with a single exposure.

However, because no HDR image is ever created during exposure fusion, it cannot be considered an HDR technique

### Image histogram

An image histogram is a type of histogram that acts as a graphical representation of the tonal distribution in a digital image.[1] It plots the number of pixels for each tonal value. By looking at the histogram for a specific image a viewer will be able to judge the entire tonal distribution at a glance.

### Histogram equalization

This method usually increases the global contrast of many images, especially when the usable data of the image is represented by close contrast values. Through this adjustment, the intensities can be better distributed on the histogram. This allows for areas of lower local contrast to gain a higher contrast. Histogram equalization accomplishes this by effectively spreading out the most frequent intensity values.

### Image enhancement

Process the image (e.g. contrast improvement, image sharpening...) so that it is better suited for further processing or analysis.

Image enhancement belongs to image **preprocessing** methods, and are based on subjective image quality criteria. No objective mathematical criteria are used for optimizing processing results. Performed by reversing the process that blurred the image. 

### Image restoration

The operation of taking a corrupt/noisy image and estimating the clean, original image. The objective of image restoration techniques is to reduce noise and recover resolution loss.

Image restoration is different from image enhancement in that the latter is designed to emphasize features of the image that make the image more pleasing to the observer, but not necessarily to produce realistic data from a scientific point of view. With image enhancement noise can effectively be removed by sacrificing some resolution, but this is not acceptable in many applications.

### multi-exposure

 The superimposition of two or more exposures to create a single image.

 **Exposure** is the amount of light per unit area (the image plane illuminance times the exposure time) reaching a photographic film or electronic image sensor, as determined by shutter speed, lens aperture and scene luminance.

### Flash/non-flash image pairs

illumination component

image component

image degradation

reflectance component

Retinex filtering

single image HDR

time-division/space-division acquisition

virtual multi-exposure illumination
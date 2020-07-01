# MPI Image Filter

A program that uses MPI to speed up the application of certain filter to a PGM or PNM image.

Check README (without .md) for detailed explanation of the implementation

## MPI Install

```
sudo apt-get update
sudo apt-get install lam4-dev mpich openmpi-bin
```

## Build

```
make
```

## Usage

Command

```
mpirun -np N ./tema3 image_in.pnm image_out.pnm filter1 filter2 ... filterX
```

- N - number of computing units
- filterX - filter to be applied to the input image
  - smooth
  - blur
  - sharpen
  - mean
  - emboss

Example

```
mpirun -np 4 ./tema3 baby-yoda.pnm baby-yoda-emboss.pnm emboss
```

---

## Filters

The program represents the filters as a 3x3 Matrix. Below you can find a detailed view of the filters available in the program:

### Smoothing filter

Improves the differences in images.

<img src="ColectiePoze/README_Images/smooth.png" height="150"/>

### Approximative Gaussian Blur filter

Reduces the background noise in the image.

<img src="ColectiePoze/README_Images/blur.png" height="150"/>

### Sharpen

Accentuates the details in the image.

<img src="ColectiePoze/README_Images/sharpen.png" height="150"/>

### Mean removal

Similar to "Sharpen" it improves the details in the image but Mean removal uses the values of the diagonal adjacent pixels

<img src="ColectiePoze/README_Images/mean.png" height="150"/>

### Emboss

<img src="ColectiePoze/README_Images/emboss.png" height="150"/>

---

## Usage examples

### Original image:

<img src="ColectiePoze/README_Images/darth-vader.png"/>

---

### Sharpen

```
mpirun -np 4 ./tema3 darth-vader.pgm dv-shapren.pgm sharpen

```

<img src="ColectiePoze/README_Images/dv-sharpen.png"/>

---

### Blur

```
mpirun -np 4 ./tema3 darth-vader.pgm dv-blur.pgm blur

```

<img src="ColectiePoze/README_Images/dv-blur.png"/>

---

### Mean

```
mpirun -np 4 ./tema3 darth-vader.pgm dv-mean.pgm mean

```

<img src="ColectiePoze/README_Images/dv-mean.png"/>

---

### Smooth

```
mpirun -np 4 ./tema3 darth-vader.pgm dv-smooth.pgm smooth

```

<img src="ColectiePoze/README_Images/dv-smooth.png"/>

---

### Extra smooth

```
mpirun -np 4 ./tema3 darth-vader.pgm dv-extra-smooth.pgm smooth smooth smooth

```

<img src="ColectiePoze/README_Images/dv-extra-smooth.png"/>

---

### Emboss

```
mpirun -np 4 ./tema3 darth-vader.pgm dv-emboss.pgm emboss

```

<img src="ColectiePoze/README_Images/dv-emboss.png"/>

---

### All

```
mpirun -np 4 ./tema3 darth-vader.pgm dv-all.pgm smooth blur sharpen mean emboss

```

<img src="ColectiePoze/README_Images/dv-all.png"/>

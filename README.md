# femton

## About

This project came about out of the impetus of exploring the triangulation for manipulable image represenation as an alternative to the traditional paradigm of pixel and vector graphics. In this conceptualization, an image is represented by colored triangles in a triangulation with movable vertices that can be inserted or removed to achieve adaptive resolution. By combining the neighborhood characteristic of the pixel grid with the free shape characteristic of polygons, a triangulation, along with proper re-triangulation, can be manipulated based on both the spatiality and the geometry of its visual elements, opening up for unique image transformations.

![brush](https://user-images.githubusercontent.com/2462565/180416854-f2599de5-6221-4109-952a-8b3808c57ae2.gif) ![smooth](https://user-images.githubusercontent.com/2462565/180481763-19affa20-f9e5-4e65-a7ef-776ec027b16d.gif) ![roll2_](https://user-images.githubusercontent.com/2462565/180505881-c258d626-7392-43fa-bfb9-fc96751aac7b.gif)


For the realization of a manipulable triangulation it is necessary to handle the occurence of intersecting edges which violate the triangulation condition when vertices are moved. In such cases, it turns out, re-triangulation is always possible by "flipping" edges between specific triangle pairs. For image representation, an appropriate re-triangulation can be arrived at by flipping edges in a way that is visually nondisruptive. A central part of the software of this project is the implementation of an algorithm which, in accordance with a principle of visual retention, performs re-triangulation as though the vertices had moved synchronously in continuous linear trajectories to their new positions.

## Introduction

### The principle of visual retention 

In an essential sense, the image representation of a static triangulation is unequivocally given. This is no longer the case when vertices are moved, inserted or removed, whereupon some decisions regarding the constituting of the triangulation, and by extension the representation, need to be made. Of paramount importance in these decisions is the consistency of representation, in other words, that the visual information, as presented to the retina, is retained throughout manipulation. This guiding notion, pertaining to the collection of more or less coerced constituting decisions in general, is here referred to as *the principle of visual retention*. It may be considered the central dogma, as it were, spanning this entire project.

### Contour and adaptive resolution

Contour can be defined as a path of edges where triangles have uniform color along either side of the path but distinct color across the path. Contour is an important concept because it is, in some regard, the basic element of visual information and because as a delimiter of representation it establishes, among other things, something analogous to pixel resolution only more adaptive: Image regions with uniform color can be consituted by large triangles while regions with more visual detail must be constituted by smaller triangles. The limit in detail, just like with a pixel raster, is ultimately determined by the size of the smallest contour edge unit.

![adaptive](https://user-images.githubusercontent.com/2462565/180307109-54b144eb-5e24-498d-9965-6e15350a1f75.png)

### Moving vertices

In the general case, when an edge is flipped, the visual impression of the triangulation will change because no reassignment of triangle colors can prevent contour from being disrupted. But in the case when an edge is flipped precisely at the incident of vertex interesection, and if the colors are reassigned in the proper way, contour will indeed remain the same before and after flipping. The principle of visual retention therefore suggests that edges should be flipped only when a vertex intersects it.

![flip_](https://user-images.githubusercontent.com/2462565/180372075-0bd6cf65-8d3e-4e38-962a-9111fd827604.gif) ![flip3](https://user-images.githubusercontent.com/2462565/180336545-fa470595-a978-44ef-96a6-c793ce9f47e2.gif)

In turn, in what order to flip edges also becomes a matter subsumed by the principle of visual retention. One intuitive order is given by treating vertices as moving synchronously along linear trajectories through time. An inductive method for re-triangulation can then be outlined: *Progress along the time line until some first triangle is intersected at some time t' and resolve the violation by flipping. The collection of triangles at t' is now a triangulation. Repeat the process progressing from t' and continue repeating until all vertices have moved to their new positions. The re-triangulation is now complete.* 

![github_1](https://user-images.githubusercontent.com/2462565/176647543-0d9f3e1a-3cee-460f-82ea-48246268ed56.gif) ![github_2](https://user-images.githubusercontent.com/2462565/176647568-434c5d02-c11f-48cb-a51a-3220212f12d2.gif) ![github_3](https://user-images.githubusercontent.com/2462565/176647584-3cccacb1-72d2-42f1-a289-094924395db3.gif)

More details about the re-triangulation algorithm are described [elsewhere](exposition.md).

### Removing vertices

The removal of a vertex from a triangulation necessarily prompts some form of re-triangulation. Incidentally, the removal of a vertex is also a solution to the corner case problem that vertices can become precisely superposed when re-triangulating moving vertices. The apparatus of removing a vertex can therefore be delegated by simply moving the vertex so that it superposes a neighbor vertex.

A typical application of vertex removal is removing those representationally redundant vertices whose adjacent triangles is of the same color.

[show removal of a redudant vertex]

### Inserting vertices

For a manipulable triangulation representation to exhibit basic capabilities for drawing, design and indeed image reproduction, there should be a way to support visual detail by introducing more vertices and triangles. Notably, this can be done such that the visual impression is retained if a triangle is split and the colors are assigned in the obvious way.

The applicability of triangle splitting during manipulation is demonstrated in the simple mechanic where contour edges that exceed some threshold length are continuously split.

![split](https://user-images.githubusercontent.com/2462565/180255443-846e5f72-0f0b-46ad-b97f-765df25eaf74.gif) ![refract2](https://user-images.githubusercontent.com/2462565/179385114-5fa6f233-335b-4615-a419-97b572f94db5.gif)

### Color gradients and smooth contour

In the basic framing with flatly rendered color triangles, color gradients are achieved in a similar fashion as they are with pixel graphics. Limited by resolution, i.e. some smallest edge length, a color transition is contituted by a mosaic of small triangles.

Seeing how the triangulation is, after all, a triangle mesh, a natural question to pose is whether there are alternative ways of rendering the mesh so as to achieve smoother gradients with coarser triangles. A different framing where color is linearly interpolated over the triangle faces was explored in a [master thesis](http://uu.diva-portal.org/smash/get/diva2:859026/FULLTEXT01.pdf) (pdf). Beziér curves, which as it turns out generalize to surfaces in the triangle, were also investigated, with inconclusive results. So far, the most encouraging results have been achieved with shader techniques combining guassian filters with max filters. In such a setup, in addition to making the amount of blur a property of vertices, a vertex parameter controlling the intensity of transition to the gaussian is also present. The finess then is that besides smoother gradients with coarser triangles, smoother but still sharp contours are also attainable.

![medianblur3](https://user-images.githubusercontent.com/2462565/178974317-63fd4cc6-b9fe-40d0-8564-b79eb8fd184a.jpg)

One should bare in mind that shading techniques amount to post-processing on rasterized renders. Importantly, the visual impression created by the shader is in the end based on the underlying tirangulation and is always merely a recasting of this information in a, perhaps, more pleasing way. The two-way interaction between the shader rasterizations and the triangulation representation is as of yet a much unexplored topic -- in so far it concerns the narrow scope of this project.

## Screenshots

![tumblr_4433e08e2f6b4087167ec823fbc00a08_cc0f846d_1280](https://user-images.githubusercontent.com/2462565/180423962-11adf262-d10b-4810-b828-6812a2005730.jpg)

![git_flashes](https://user-images.githubusercontent.com/2462565/176651031-2104efcf-75ef-4fa0-b751-da3a71ee17d1.gif) ![out](https://user-images.githubusercontent.com/2462565/180302797-72d971ea-a3a1-406d-9bfe-262af9587275.gif)

https://user-images.githubusercontent.com/2462565/176846043-6d02d548-be18-4a62-948f-526be59c24ca.mp4

https://user-images.githubusercontent.com/2462565/176846064-b3ab8c08-fae3-498d-9f98-2a06512b5dd5.mp4

## Additional resources

### Technical exposition

A technical exposition of the algorithm is found in [exposition.md](exposition.md).

### Blog

A project blog has been kept at [femton15.tumblr.com](http://femton15.tumblr.com).

### Master thesis (2015)

**Abstract:** *In this thesis the triangulation is treated as a general-purpose visual representation by investigation of various domain-specific methods such as triangulation interpolation, mesh flows, vertex neighborhood feature measures and re-triangulation for spatial transformations. Suggested new methods include an effective cost for image interpolation based on work by Sederberg et al. and a ridge-edge measure related to the Harris edge detector.*
[Link to pdf](http://uu.diva-portal.org/smash/get/diva2:859026/FULLTEXT01.pdf)

### Bachelor thesis (2013)

**Abstract:** *In as much as raster and vector graphics have complementary roles in digital imagery they both have limitations. In this paper, the two frameworks are in part bridged in the triangulation mesh where in particular the ideas of the spatial neighborhood and representation by geometrical primitives are combined. With a triangulation algorithm for preserving integrity of contour and color together with methods for introducing geometric detail and blending color, the end result is a configurable medium with qualities resembling those of physical paint, demonstrating potential as a viable alternative for graphics creation.*
[Link to pdf](http://uu.diva-portal.org/smash/get/diva2:654777/FULLTEXT01.pdf)



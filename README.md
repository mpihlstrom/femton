# femton

## About

This project came about with the exploration of manipulable image representation from first princples using the triangulation as an alternative to the traditional paradigm of pixel and vector graphics. In this conceptualization, an image is represented by colored triangles in a triangulation where along with proper re-triangulation vertices can be moved, inserted and removed. By combining the connected characteristic of the pixel grid with the free shape characteristic of polygons, such a triangulation differentiates itself by being manipulable both via the spatiality and the geometry of its visual elements.

![brush](https://user-images.githubusercontent.com/2462565/180416854-f2599de5-6221-4109-952a-8b3808c57ae2.gif) ![smooth](https://user-images.githubusercontent.com/2462565/180481763-19affa20-f9e5-4e65-a7ef-776ec027b16d.gif) ![roll2_](https://user-images.githubusercontent.com/2462565/180505881-c258d626-7392-43fa-bfb9-fc96751aac7b.gif)

For the realization of a manipulable triangulation it is necessary to handle the occurence of intersecting edges which violate the triangulation condition when vertices are moved. As it turns out, re-triangulation is always possible by "flipping" edges between certain triangle pairs. For image representation, an appropriate re-triangulation can be arrived at by flipping edges in a way that is visually nondisruptive. A central part of this project is the implementation of an algorithm which, in accordance with a principle of visual retention, performs re-triangulation as though the vertices had moved synchronously in linear trajectories to their new positions.

![cont75](https://user-images.githubusercontent.com/2462565/180953154-375f94bf-dfca-49aa-b097-31e57d9819e1.gif) ![cont75both](https://user-images.githubusercontent.com/2462565/180953164-a614ba30-fb1b-4e7e-b2eb-b6fb5609bc44.gif) ![cont75wf](https://user-images.githubusercontent.com/2462565/180953175-476c719b-8ac9-4fbd-85ae-6f996fd691f0.gif)

## Introduction

### Contour and the principle of visual retention

A 2D triangulation can be seen a mosaic of non-overlapping triangles covering some space. When the triangles are colored and insofar adjacent triangles share the same color, this space can also be seen as a jigsaw puzzle of polygon shapes. Importantly, then, the exterior of a polygon is at the same time the collected interior of the other polygons, and thus the entire space of shapes is connected. Modification to a shape is therefore just as much modification to the negative space surrounding it.

Contour can be defined as the boundary between polygons, or, as a path of edges where triangles have the same color along either side of the path but distinct color across the path. Contour is a helpful concept because, in some regard, it is the element of visual information and because as a delimiter of representation it establishes, among other things, something like adaptive resolution: only regions with contour need to be constituted by small edges, where the level of detail is ultimately limited by the smallest edge length.

| Image | Image + edges | Contour |
|-|-|-|
| ![1](https://user-images.githubusercontent.com/2462565/183255042-9247f1d4-ff94-42e2-b426-19dab05b4c9b.png) | ![1wf](https://user-images.githubusercontent.com/2462565/183255047-28dc16b8-ba01-49f7-a9fb-5a79f44bf409.png) | ![1contour](https://user-images.githubusercontent.com/2462565/183255049-c15bb8a4-51cc-44a3-9fe1-f33f81d28a90.png) |

The colored triangulation determines an image. When vertices are moved, inserted or removed, however, some decisions need to be made about the constituting of the triangulation and, by extention, about the image that is represented. Of specific concern in these decisions is the integrity of visual information, as mediated by the representation and as exhibited by contour in particular. This concern, pertaining to the overall collection of more or less coerced constituting decisions, is here referred to as *the principle of visual retention*. It may be considered the central dogma, as it were, spanning this entire project.

### Moving vertices

Moving a vertex of a triangulation will generally cause edges to intersect, an event where, strictly speaking, the triangulation ceases to be a triangulation. But any set of points can be triangulated and, furthermore, from a ceased triangulation a new triangulation can always be reached merely by "flipping" edges between adjacent triangles. Re-triangulation therefore can be reduced to the task of finding a proper and preferred way to flip edges.

![v0](https://user-images.githubusercontent.com/2462565/182787988-820410f2-eb0b-4326-9c6f-61734f386474.gif) ![v1](https://user-images.githubusercontent.com/2462565/182786169-7bd2f162-aa1d-4e50-83f6-1eee57beacd5.gif) ![v2](https://user-images.githubusercontent.com/2462565/182786178-261cad4a-85a7-4a5f-9019-6a1d30690ef8.gif)

When an edge is flipped, in the general case, the visual impression will change because no reassignment of triangle colors can prevent contour from being disrupted. However, in the case when an edge is flipped precisely at the incident of vertex intersection, and if the colors are reassigned in the proper way, contour will indeed be the same before and after flipping. The principle of visual retention therefore suggests that edges should be flipped only when a vertex intersects it.

![flip_2](https://user-images.githubusercontent.com/2462565/182556552-1ef6fa5f-ceac-4086-9a8b-2283589c695f.gif) ![flip3_2](https://user-images.githubusercontent.com/2462565/182556567-889ca307-6f90-4854-a03f-5c429fd082d5.gif) ![flippair2](https://user-images.githubusercontent.com/2462565/182549260-c87846c4-dc4c-420a-9944-9989dbac4473.gif)

In turn, in what order to flip edges also becomes a matter subsumed by the principle of visual retention. One intuitive order is given by treating vertices as moving synchronously along linear trajectories through time. A triangulation schema can then be outlined: *Progress along the time line until some first triangle is intersected at some time t' and resolve the violation by flipping. The collection of triangles at t' is now a triangulation. Repeat the process progressing from t' and continue repeating until all vertices have moved to their new positions. The re-triangulation is now complete.* This triangulation schema is here called *edge-semantic triangulation*. It is described in more detail in an [appendix](exposition.md).

![flashes](https://user-images.githubusercontent.com/2462565/181200969-9b0baef3-38ac-45f6-8086-b2ac84e5072f.gif) ![flashes_wf_1](https://user-images.githubusercontent.com/2462565/181201001-01e7ef4e-5a9a-4af6-94e2-47bc257f6884.gif) ![flashes_wf_2](https://user-images.githubusercontent.com/2462565/181259510-68bf3e04-fad5-4dd0-ba76-bd91915caa87.gif)

### Inserting and removing vertices

For a manipulable triangulation representation to show basic capabilities for image reproduction and design there should be a way to support visual detail by introducing more vertices and triangles. Notably, this can be done such that the visual impression is retained if a triangle is split and the colors are assigned in the obvious way. The applicability of triangle splitting during manipulation is demonstrated in the simple mechanic where contour edges that exceed some threshold length are continuously split.

| Vertex insertion | Contour splitting | Vertex removal |
| - | - | - |
| ![split](https://user-images.githubusercontent.com/2462565/180255443-846e5f72-0f0b-46ad-b97f-765df25eaf74.gif) | ![refract2](https://user-images.githubusercontent.com/2462565/179385114-5fa6f233-335b-4615-a419-97b572f94db5.gif) | ![remove_all](https://user-images.githubusercontent.com/2462565/183014499-30d09f35-31a1-41f2-bc40-33a87090eaab.gif) |

Removal of a vertex generally prompts a more intricate re-triangulation than insertion of a vertex. As it happens, vertex removal is also a solution to the problem that vertices can become precisely superposed when moving vertices. The apparatus of removing a vertex can therefore be delegated to moving the vertex so that it superposes a neighboring vertex. A typical application is the removal of visually redundant vertices, for instance a vertex inside a polygon shape.

### Alternative triangulations

Considering all the possible ways to flip edges in a given triangulation, it is easy to see that there are many ways to triangulate a set of points. Probably the most famous triangulation is the Delaunay triangulation, which can be constructed from any given triangulation by flipping those edges where the circle circumscribing one adjacent triangle contains the other adjacent triangle. The Delaunay triangulation always exists (that is, the flipping process halts) and is unique and, among other nice things, maximizes the smallest triangle angle.

Can the Delaunay triangulation be used for image representation? To anwser this question it must first be decided how triangle colors should be reassigned. By intepreting the color assignment of edge-semantic triangulation as, upon flipping, the smaller triangle taking on the color of the larger triangle (in a best attempt to stay true to the principle of visual retention), the same consistent logic can be used with the Delaunay triangulation. The two triangulation schemas for moving vertices can then be compared.

| Edge-semantic triangulation | Delaunay triangulation |
| - | - |
| ![tri_pvr](https://user-images.githubusercontent.com/2462565/182030887-979d5b5f-b97d-4337-abbf-712025e249fd.gif) | ![tri_del](https://user-images.githubusercontent.com/2462565/182030895-f7fad93a-f1ea-454f-8b89-860cdce6f014.gif) |
| Transformation + contour splitting |
|![est](https://user-images.githubusercontent.com/2462565/182153583-4b88591c-4ed3-4846-8f67-1f0f3df3fe9e.gif) | ![del](https://user-images.githubusercontent.com/2462565/182153597-28368483-40a5-4c64-9f00-dbe2a315b44c.gif) |

Comparing the results of a simple transformation with contour splitting, it becomes apparent that, for the purpose of manipulable image representation, the choice of triangulation is not arbitrary. The Delaunay triangulation does however still find a use if it is applied on non-contour edges only, since flipping such edges does not have any visual impact. Because also this kind of constrained Delaunay triangulation is unique, a 1-to-1 relationship between image and triangulation is thus ensured.

| No Delaunay | Constrained Delaunay | Percieved (in either case) |
| - | - | - |
| ![stroke_no](https://user-images.githubusercontent.com/2462565/182430474-5c508a34-a687-4504-b387-55dab4b53e0d.gif) | ![stroke_del](https://user-images.githubusercontent.com/2462565/182430483-3fc0b900-1d01-4fa7-96be-ae0fe02a6930.gif) | ![stroke_nowf](https://user-images.githubusercontent.com/2462565/182430493-c958a123-2c17-414f-ab8f-5b1875aa79de.gif) 
 
### Shading techniques

In the basic framing with flatly rendered color triangles, color gradients are achieved in a similar fashion as they are with pixel graphics. Limited by resolution, i.e., some smallest edge length, a color transition is constituted by a cluster of small triangles.

Seeing how the triangulation is, after all, a triangle mesh, a natural question to pose is whether there are alternative ways of rendering the mesh so as to achieve smoother gradients with coarser triangles. A different framing where color is linearly interpolated over the triangle faces was explored in a [master thesis (pdf)](http://uu.diva-portal.org/smash/get/diva2:859026/FULLTEXT01.pdf). Bézier curves, which as it turns out generalize to surfaces in the triangle, were also [investigated](https://femton15.tumblr.com/page/7), with inconclusive results. So far, the most encouraging results have been achieved with shader techniques combining guassian filters with max filters. In such a setup, in addition to making the amount of blur a property of vertices, a vertex parameter controlling the intensity of transition to the gaussian is also present. The finess then is that besides smoother gradients with coarser triangles, smoother but still sharp contours are also attainable.

| Flat rendering | Filter rendering |
| - | - |
| ![flat](https://user-images.githubusercontent.com/2462565/183244081-a5fc9a5b-0c84-4891-90f7-c6b6d154e591.png) | ![postproc](https://user-images.githubusercontent.com/2462565/183244083-b22cbc0c-cfb0-4a37-83c4-945dea8c90e6.png) |

One should bear in mind that shading techniques amount to post-processing on rasterized renders. Importantly, the visual impression created by the shader is in the end based on the underlying triangulation and is always merely a recasting of this information in a, perhaps, more pleasing way. The two-way interaction between the shader rasterizations and the triangulation representation is as of yet a much unexplored topic -- insofar it concerns the narrow scope of this project.

## Additional resources

### More media

https://user-images.githubusercontent.com/2462565/176846043-6d02d548-be18-4a62-948f-526be59c24ca.mp4

https://user-images.githubusercontent.com/2462565/176846064-b3ab8c08-fae3-498d-9f98-2a06512b5dd5.mp4

### Technical exposition

A technical exposition of the algorithm is found in [exposition.md](exposition.md).

### Blog

A project blog has been kept at [femton15.tumblr.com](http://femton15.tumblr.com).

An older project blog was kept at [https://femtondev.wordpress.com/](https://femtondev.wordpress.com/).

### Master thesis (2015)

**Abstract:** *In this thesis the triangulation is treated as a general-purpose visual representation by investigation of various domain-specific methods such as triangulation interpolation, mesh flows, vertex neighborhood feature measures and re-triangulation for spatial transformations. Suggested new methods include an effective cost for image interpolation based on work by Sederberg et al. and a ridge-edge measure related to the Harris edge detector.*
[Link to pdf](http://uu.diva-portal.org/smash/get/diva2:859026/FULLTEXT01.pdf)

### Bachelor thesis (2013)

**Abstract:** *In as much as raster and vector graphics have complementary roles in digital imagery they both have limitations. In this paper, the two frameworks are in part bridged in the triangulation mesh where in particular the ideas of the spatial neighborhood and representation by geometrical primitives are combined. With a triangulation algorithm for preserving integrity of contour and color together with methods for introducing geometric detail and blending color, the end result is a configurable medium with qualities resembling those of physical paint, demonstrating potential as a viable alternative for graphics creation.*
[Link to pdf](http://uu.diva-portal.org/smash/get/diva2:654777/FULLTEXT01.pdf)



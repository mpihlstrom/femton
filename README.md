# femton

## About

This software uses the triangulation as a data structure for 2D image representation and manipulation. Combining the free shape characteristic of polygons with the neighborhood characteristic of the pixel grid, a triangulation image can be manipulated based on both the geometry and the spatiality of its visual elements. It is for example possible to achieve various types of line smoothing by considering the edges surrounding a vertex and to create cellular automata rules by considering the colors of adjacent triangles.

Crucial for the realization of a manipulable triangulation is the avoidance of intersecting edges when vertices are moved. For image representation specifically, this can be achieved by "flipping" edges of those triangle pairs that cause minimal visual disruption. This software implements an algorithm which takes as input a triangulation together with some desired new positions of its vertices. The algorithm determines the resulting triangulation according to the principle of minimal visual disruption as though the vertices had moved in continuous straight lines to their new positions.

## Screenshots

https://user-images.githubusercontent.com/2462565/172197963-cff136c7-e16e-47b5-824d-3aeb6ca59165.mp4

https://user-images.githubusercontent.com/2462565/172198906-9aa0b879-1630-4c28-bb6a-1803f68c6a71.mp4

![tumblr_530d76cd260b1cae7065b925ae708e3f_c2a01d72_1280](https://user-images.githubusercontent.com/2462565/172197979-416f872c-b5e1-4392-9c47-e64fe25c1e32.jpg)

![200122-121850](https://user-images.githubusercontent.com/2462565/172198228-c9f4d12a-72bf-4b69-bafb-dc104a54bc2f.jpg)

## Additional resources

* A technical exposition of the algorithm is found in [exposition.md](exposition.md).

* A project blog has been kept at [femton15.tumblr.com](http://femton15.tumblr.com).

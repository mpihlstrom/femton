# femton

## About

This software uses the triangulation as a data structure for 2D image representation and manipulation. Combining the free shape characteristic of polygons with the neighborhood characteristic of the pixel grid, a triangulation image can be manipulated based on both the geometry and the spatiality of its visual elements. It is for example possible to achieve various types of line smoothing by considering the edges surrounding a vertex and to create cellular automata rules by considering the colors of adjacent triangles.

Crucial for the realization of a manipulable triangulation is the avoidance of intersecting edges when vertices are moved. For image representation specifically, this can be achieved by "flipping" edges of those triangle pairs that cause minimal visual disruption. This software implements an algorithm which takes as input a triangulation together with some desired new positions of its vertices. The algorithm determines the resulting triangulation according to the principle of minimal visual disruption as though the vertices had moved in continuous straight lines to their new positions.

## Additional resources

A project blog has been kept at [femton15.tumblr.com](http://femton15.tumblr.com).

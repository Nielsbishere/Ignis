# Availability of a technique

FT (Feature) specifies that the platform has to support the given feature. This can be the case with things like Raytracing, Compute, VR, etc.

EXT (Extension) specifies that the GPU has to support the given feature. This can be indirect rendering, etc.

The criticality of the FT or EXT is specified by the suffix:
0 = The technique should rarely be unsupported. If this can be supported, it has to be (even with a performance cost).
1 = The technique is API specific and might not be supported on lower tier devices (mobile / console).
2 = The technique is device specific (but still available in multiple vendors) and might not be supported on higher tier devices (desktop).
3 = The technique is vendor dependent.


Examples:

CMD_BUILD_ACCELERATION_STRUCTURE_FT_3
CMD_DRAW_INDIRECT_EXT_0
CMD_DRAW_INDEXED_INDIRECT_EXT_0
CMD_DISPATCH_FT_1
CMD_DISPATCH_INDIRECT_FT_EXT_1

Their performance impact can be queried using the Graphics class:

Supported: This is natively supported on the device.
Performance: It is supported, but by not natively
Unsupported: It won't run
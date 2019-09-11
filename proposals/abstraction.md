# Graphics abstraction

Since there's a lot of various graphics objects, the Graphics class has necessary templates to help with them. 

One of the drawbacks is that virtual functions aren't really possible and linking should be used instead. This means that implementation dependent functions would have to be specified by the implementation. Since these functions wouldn't stand out, the __impl define is used as a marker to show which functions need special treatment. This does provide a small performance gain, though it means the graphics API is stuck in the executable and can't be switched.


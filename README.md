# __Makefile_Tutorial__

## file structure

src/{
###.cpp
###.h
}
bin/{
###.o
}
./{
makefile
OUT.exe
}


counting "covered"
A face "covers" another face if at least 1 vertex intersects the other face.
Which face is covering which is found by checking distance from camera
to the point of intersection. This can be approximated by using distance the
z-value of a nearby vertex. The approximation will falter for large faces.
if face, a, covers face, b, then the a-coverIndex increments.
if face, c, covers face, a, then the a-coverIndex decrements.
A higher coverIndex means being draw later.
A lower coverIndex means being draw earlier.

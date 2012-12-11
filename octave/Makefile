all: MvpClass.oct _oct_project.oct

MvpClass.oct: MvpClass.cc ../src/mvp/Octave/ov-mvpobj-ref.cc
	mkoctfile -I../src $^
	rm MvpClass.o
	rm ov-mvpobj-ref.o

_oct_project.oct: _oct_project.cc
	mkoctfile $^
	rm _oct_project.o

VWDIR=/home/khusmann/projects/VisionWorkbench/build

test_oct_project: _oct_project.cc
	mkoctfile -DDEFTEST -I../src --link-stand-alone $^ -o test_oct_project -I$(VWDIR)/include -L$(VWDIR)/lib -lvwCore -lvwMath -lvwImage -lvwCamera -lboost_system -lboost_thread -lvwFileIO
	rm _oct_project.o

PRODUCT = QPlay2
ASMSOURCES = QPlay2.asm     \
             SPCHost.asm    \
             Video.asm      \
             MainScreen.asm \
             DrawText.asm   \
             TextBuffer.asm \
             initreg.asm

#-------------------------------------------------------------------------------
CL65	= ../../cc65/bin/cl65.exe
LD65	= ../../cc65/bin/ld65.exe
OBJECTS   = $(addprefix objs/,$(notdir $(ASMSOURCES:.asm=.o)))
LIBRARIES =
#-------------------------------------------------------------------------------

all :	$(OBJECTS) $(LIBRARIES)
	$(LD65) -o $(PRODUCT).smc --config $(PRODUCT).cfg --obj $(OBJECTS)

.SUFFIXES : .asm .o

objs/%.o : %.asm
	$(CL65) -t none -o objs/$*.o -c $*.asm

clean :
	rm -f objs/*.o
	rm -f *.smc

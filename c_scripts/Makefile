IDIR=/media/theboocock/data/Dropbox/PHDTHESIS/Single-CellRNASEQ/eqtls/single_cell_utilities/htslib/
CC=gcc
CFLAGS=-I$(IDIR) -L$(LDIR)
LDIR=/media/theboocock/data/Dropbox/PHDTHESIS/Single-CellRNASEQ/eqtls/single_cell_utilities/htslib/
LIBS=-lhts

demulti: demulti_10x.c 
	gcc -o demulti_10x demulti_10x.c $(CFLAGS) $(LIBS)

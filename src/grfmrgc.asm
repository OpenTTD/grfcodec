section .data
global _grfmrg,_grfmrgsize,grfmrg,grfmrgsize

	align 4
grfmrgsize:
_grfmrgsize dd _grfmrg_end-_grfmrg

grfmrg:
_grfmrg:	incbin "grfmrgc.bin"
_grfmrg_end:

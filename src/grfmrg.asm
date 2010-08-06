ifdef M32
.386
.model flat
else
.model compact
endif
.data
public _grfmrg,_grfmrgsize

_grfmrg = $
include grfmrg.ah

end
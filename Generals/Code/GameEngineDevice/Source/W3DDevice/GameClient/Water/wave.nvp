;
;	Command & Conquer Generals(tm)
;	Copyright 2025 Electronic Arts Inc.
;
;	This program is free software: you can redistribute it and/or modify
;	it under the terms of the GNU General Public License as published by
;	the Free Software Foundation, either version 3 of the License, or
;	(at your option) any later version.
;
;	This program is distributed in the hope that it will be useful,
;	but WITHOUT ANY WARRANTY; without even the implied warranty of
;	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;	GNU General Public License for more details.
;
;	You should have received a copy of the GNU General Public License
;	along with this program.  If not, see <http://www.gnu.org/licenses/>.
;

;////////////////////////////////////////////////////////////////////////////////
;//																																						 //
;//  (c) 2001-2003 Electronic Arts Inc.																				 //
;//																																						 //
;////////////////////////////////////////////////////////////////////////////////

; Waves pixel shader
; Kenny Mitchell April 2001
; Modified July 2001 Mark Wilczynski

; Declare pixel shader version 1.1
ps.1.1

; Define t0 as a standard 3-vector from bumpmap
tex t0

; Perform EMBM to get a local normal bump reflection.
texbem t1, t0       ; compute new (u,v) values

#ifndef DO_WATER_ALPHA_TEXTURE

; result goes in output color multiplied by diffuse
mul r0, t1, v0

#else

;alternate version which uses a third texture which provides per-pixel alpha.
; result goes in output color multiplied by diffuse
tex t2	; get alpha texture
mul r0.rgb, t1, v0
+mul r0.a, t2, v0

#endif




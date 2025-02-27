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

; Motion blur pixel shader
; Created:   John Ahlquist, April 2002
; Currently unused prototype code.  jba.

; Declare pixel shader version 1.1
ps.1.1

tex t0
; Define t0 as a standard 3-vector from bumpmap
tex t1

; Perform EMBM to get a local normal bump reflection.
texbem t2, t1       ; compute new (u,v) values
tex t3
; result goes in output color multiplied by diffuse
;mul r0, t2, v0

mul r0.rgb, v0, t0
+add r0.a, v0, t0
mul r1, t2, c0
add r0.rgb, r0, r1
+mul r0.a, r0, t3


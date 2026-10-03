pico-8 cartridge // http://www.pico-8.com
version 32
__lua__
-- probe -- pico-8 reference readings
-- of 16.16 fixed-point values
--
-- everything is read as raw bits
-- through tostr(value,true): pico-8
-- is 16.16 fixed point, its numbers
-- ARE integers. decimal is
-- ambiguous, hexadecimal is not.
--
-- writes pico8_reference_probe_out.txt and shows
-- a summary on screen.

local first=true
function o(s)
 printh(s,"pico8_reference_probe_out",first)
 first=false
end

function h(x)
 return tostr(x,true)
end

cls(0)
print("probe: running...",0,0,7)
flip()

o("# probe v1")
o("# pico-8 version: <to fill in>")

-- ================================
-- block A: the 54 decimal
-- literals the game uses
-- (0b1010101010101010.1 left
--  out: a fillp binary pattern)
-- ================================
o("## a literals")
local lits={
 {"0.001",0.001},{"0.0032",0.0032},
 {"0.004",0.004},{"0.005",0.005},
 {"0.0067",0.0067},{"0.007",0.007},
 {"0.009",0.009},{"0.01",0.01},
 {"0.025",0.025},{"0.031",0.031},
 {"0.05",0.05},{"0.0525",0.0525},
 {"0.06",0.06},{"0.07",0.07},
 {"0.08",0.08},{"0.0977",0.0977},
 {"0.1",0.1},{"0.12",0.12},
 {"0.125",0.125},{"0.1255",0.1255},
 {"0.14",0.14},{"0.15",0.15},
 {"0.18",0.18},{"0.19",0.19},
 {"0.2",0.2},{"0.25",0.25},
 {"0.27",0.27},{"0.3",0.3},
 {"0.333",0.333},{"0.34",0.34},
 {"0.4",0.4},{"0.45",0.45},
 {"0.5",0.5},{"0.55",0.55},
 {"0.6",0.6},{"0.75",0.75},
 {"0.8",0.8},{"0.85",0.85},
 {"0.9",0.9},{"0.97",0.97},
 {"0.98",0.98},{"0.99",0.99},
 {"1.02",1.02},{"1.08",1.08},
 {"1.15",1.15},{"1.2",1.2},
 {"1.25",1.25},{"1.5",1.5},
 {"2.5",2.5},{"2.75",2.75},
 {"3.2",3.2},{"4.5",4.5},
 {"7.5",7.5},{"10.5",10.5}
}
for p in all(lits) do
 o(p[1].." "..h(p[2]))
end

-- ================================
-- block B: rounding
-- ================================
o("## b rounding")

-- B1: flr on negative numbers
local fs={-1.5,-0.5,-0.9,-1,-2.5,
          1.5,0.5,0.9,1,0}
for v in all(fs) do
 o("flr "..h(v).." -> "..h(flr(v)))
end

-- B2: the actual rounding of spr(),
-- read back from the pixels. sprite
-- 0 gets one lit pixel, at local
-- offset (7,0): both possible
-- answers (truncation or flr) then
-- stay on screen.
--   at -0.5: truncation -> column 7
--            flr        -> column 6
sset(7,0,7)

function sprat(x,y)
 cls(0)
 spr(0,x,y)
 for j=0,127 do
  for i=0,127 do
   if pget(i,j)==7 then
    return i..","..j
   end
  end
 end
 return "none"
end

o("spr(-0.5,4) -> "..sprat(-0.5,4))
o("spr(-0.9,8) -> "..sprat(-0.9,8))
o("spr(-1.5,12) -> "..sprat(-1.5,12))
o("spr(3.5,16) -> "..sprat(3.5,16))
o("spr(3.9,20) -> "..sprat(3.9,20))
o("spr(4,-0.5) -> "..sprat(4,-0.5))
o("spr(4,3.5) -> "..sprat(4,3.5))

-- B3: fractional table keys
local t={}
t[1.5]="a"
t[2]="b"
o("t[1.5]=a; t[1] -> "..tostr(t[1]))
o("t[1.5]=a; t[1.5] -> "..tostr(t[1.5]))
o("t[2]=b; t[2.5] -> "..tostr(t[2.5]))

-- ================================
-- block C: decimal tostr
-- (the form the game prints)
-- next to the exact hexadecimal
-- ================================
o("## c tostr")
local ds={-1.5,1,0,0.15,0.27,
          0.1,2.5,-0.0001,
          100,1000,-32768,32767}
for v in all(ds) do
 o("d ["..tostr(v).."] x "..h(v))
end
o("d [1/3 ="..tostr(1/3).."] x "..h(1/3))
o("d [2/3 ="..tostr(2/3).."] x "..h(2/3))

-- ================================
-- block D: ^0.75
-- (p8_fixed_pow075)
-- ================================
o("## d pow075")
local ps={0.25,0.5,0.75,1,1.5,2,3,
          4,6,8,12,16,24,32,64,128}
for v in all(ps) do
 o("p "..h(v).." -> "..h(v^0.75))
end

-- ================================
-- block E: sqrt / sin / cos
-- test vectors of p8_fixed_sqrt
-- and of the sin/cos table.
-- ================================
o("## e sqrt")
for v in all(ps) do
 o("s "..h(v).." -> "..h(sqrt(v)))
end
o("## e sin")
for i=0,63 do
 o("sin "..i.." "..h(sin(i/64)))
end
o("## e cos")
for i=0,63 do
 o("cos "..i.." "..h(cos(i/64)))
end

-- ================================
-- block F: rnd
-- RECORDED, READ BY NO TEST.
-- ================================
o("## f rnd")
srand(1)
for i=0,63 do
 o("r1 "..i.." "..h(rnd(1)))
end
srand(0)
for i=0,15 do
 o("r0 "..i.." "..h(rnd(1)))
end
srand(1)
o("rnd(0) "..h(rnd(0)))
o("rnd(10) "..h(rnd(10)))
o("rnd() "..h(rnd()))

o("# end")

-- ================================
-- screen summary (a fallback if
-- the file cannot be found: the
-- main readings of block B fit
-- on the screen)
-- ================================
cls(0)
print("probe done",0,0,11)
print("-> pico8_reference_probe_out.txt",0,6,6)
print("",0,12)
print("flr(-1.5) "..h(flr(-1.5)),0,18,7)
print("flr(-0.5) "..h(flr(-0.5)),0,24,7)
print("spr(-0.5,4) "..sprat(-0.5,4),0,30,7)
print("spr(-0.9,8) "..sprat(-0.9,8),0,36,7)
print("spr(4,-0.5) "..sprat(4,-0.5),0,42,7)
print("t[1] "..tostr(t[1]),0,48,7)
print("0.27 "..h(0.27),0,54,7)
print("0.1  "..h(0.1),0,60,7)
print("1/3  "..h(1/3),0,66,7)
print("tostr(1/3) "..tostr(1/3),0,72,7)

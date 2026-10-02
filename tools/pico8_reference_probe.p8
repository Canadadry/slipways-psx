pico-8 cartridge // http://www.pico-8.com
version 32
__lua__
-- probe -- releve de reference pico-8
-- des valeurs fixes 16.16
--
-- tout est releve en bits bruts via
-- tostr(x,true) : pico-8 est en
-- virgule fixe 16.16, ses nombres
-- SONT des entiers. le decimal est
-- ambigu, l'hexa ne l'est pas.
--
-- ecrit pico8_reference_probe_out.txt et affiche
-- un resume a l'ecran.

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
-- bloc a : les 54 litteraux
-- decimaux de pico8/lua.lua
-- (0b1010101010101010.1 ligne 549
--  exclu : motif binaire fillp)
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
-- bloc b : arrondi
-- ================================
o("## b rounding")

-- b1 : flr sur negatifs
local fs={-1.5,-0.5,-0.9,-1,-2.5,
          1.5,0.5,0.9,1,0}
for v in all(fs) do
 o("flr "..h(v).." -> "..h(flr(v)))
end

-- b2 : arrondi reel de spr(), relu
-- au pixel. sprite 0 recoit un seul
-- pixel allume, a l'offset local
-- (7,0) : les deux reponses
-- possibles (troncature vs flr)
-- restent alors a l'ecran.
--   x=-0.5 : trunc -> col 7
--            flr   -> col 6
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

-- b3 : cles de table fractionnaires
local t={}
t[1.5]="a"
t[2]="b"
o("t[1.5]=a; t[1] -> "..tostr(t[1]))
o("t[1.5]=a; t[1.5] -> "..tostr(t[1.5]))
o("t[2]=b; t[2.5] -> "..tostr(t[2.5]))

-- ================================
-- bloc c : tostr decimal
-- (la forme que le jeu affiche)
-- en regard de l'hexa exact
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
-- bloc d : ^0.75
-- (economy_formulas.c:196)
-- ================================
o("## d pow075")
local ps={0.25,0.5,0.75,1,1.5,2,3,
          4,6,8,12,16,24,32,64,128}
for v in all(ps) do
 o("p "..h(v).." -> "..h(v^0.75))
end

-- ================================
-- bloc e : sqrt / sin / cos
-- vecteurs de pico8_fixed_sqrt
-- et de la table sin/cos.
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
-- bloc f : rnd
-- CAPTURE, PAS EXPLOITE (idem).
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
-- resume ecran (secours si le
-- fichier est introuvable : le
-- bloc b tient a l'ecran, c'est
-- lui qui debloque les 76 casts)
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

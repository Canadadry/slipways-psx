-- slipways
-- by @krajzeg / @gruber_music
function ft(rc,nt,pu)rc[nt]=rc[nt]or {}
add(rc[nt],pu)end
function mv(e,rm,...)local fn=e and e[rm]
if type(fn)=="function" then
return fn(e,...)end
return fn
end
function qj(o,mr)for k,v in pairs(mr or {})do
o[k]=v
end
return o
end
function mw(o)
return qj({},o)end
function ho(rc,mr,gp)
return function(ri)for i,p in pairs(mr)do
ri[p]=gu(gp,p)and r[ri[i]]or ri[i]
end
rc[ri.px]=ri
end
end
function rk(y1,y2,...)rectfill(0,y1,127,y2 or y1,...)end
function eq(x1,y1,x2)rect(x1,y1,x2+1,y1+12,0)rectfill(x1+1,y1+1,x2,y1+11,5)spr(70,x1,y1,1,2)spr(71,x2-2,y1,1,2)end
function io(t,x,y,c,a)
if (a)x-=a*4*#t
for _,d in pairs(er)do
print(t,x+d.x,y+d.y,band(d.c,c))end
end
function jp(qk,ow)local f={}
for e in all(qk)do
if (ow(e))add(f,e)
end
return f
end
function nv(qk,kp)local m={}
for e in all(qk)do
add(m,kp(e)or nil)end
return m
end
function gu(qk,kb)for e in all(qk)do
if (e==kb)return true
end
end
function la(a,b)local t={}
local kg=function(e)add(t,e)end
foreach(a,kg)foreach(b,kg)
return t
end
function oo(fn,a)
return fn
and fn(a[1],a[2],a[3],a[4],a[5])or a
end
function ob(qs,mr)local kn,s,n,lm=
{},1,1,0
fi(qs,function(c,i)local sc,rq=sub(qs,s,s),i+1
if c=="(" then
lm+=1
elseif c==")" then
lm-=1
elseif lm==0 then
if c=="=" then
n,s=sub(qs,s,i-1),rq
elseif c=="," and s<i then
kn[n]=sc=='"'and sub(qs,s+1,i-2)or sub(qs,s+1,s+1)=="("and oo(pi[sc],ob(sub(qs,s+2,i-2)..","))or sc!="f"and band(sub(qs,s,i-1)+0,0xffff.fffe)s=rq
if (type(n)=="number")n+=1
elseif sc!='"'and c==" " or c=="\n" then
s=rq
end
end
end)
return qj(kn,mr)end
function fi(qs,fn)local rs={}
for i=1,#qs do
add(rs,fn(sub(qs,i,i),i)or nil)end
return rs
end
gt="abcdefghijklmnopqrstuvwxyz0123456789 ().,=-+_/\"'?%\n"function unstash(pd)local s=""repeat
local i=peek(pd)s=s..sub(gt,i,i)
pd+=1
until i==0
return s
end
kv={}
function kv:jr(rj)rj=ob(rj or "")rj.iq,rj.ns,kv[rj.fz or ""]=
self,{__index=rj},rj
return setmetatable(rj,{__index=self,__call=function(self,ob)ob=setmetatable(mw(ob),rj.ns)local ko,iy=rj
while ko do
if ko.pq and ko.pq~=iy then
iy=ko.pq
iy(ob)end
ko=ko.iq
end
return ob
end
})end
km={}
km.__index=km
function km:__add(b)
return v(self.x+b.x,self.y+b.y)end
function km:__sub(b)
return v(self.x-b.x,self.y-b.y)end
function km:__mul(m)
return v(self.x*m,self.y*m)end
function km:__div(d)
return v(self.x/d,self.y/d)end
function km:__unm()
return v(-self.x,-self.y)end
function km:rg(v2)
return self.x*v2.x+self.y*v2.y
end
function km:nn()
return self/self:qx()end
function km:pa()
return v(-self.y,self.x)end
function km:qx()
return sqrt(#self)end
function km:__len()
return self:rg(self)end
function v(x,y)
return setmetatable({x=x,y=y
},km)end
function qm(fy,lx)
return v(cos(lx),sin(lx))*fy
end
function qz(xl,yt,xr,yb)
return function(p)
return mid(xl,xr,p.x)==p.x
and mid(yt,yb,p.y)==p.y
end
end
pi={b=qz,v=v,br=rk,qr=io,rf=eq,s=spr}
function cu()local a=0x5000
for p=0,15 do
for c=0,15 do
poke(a,bor(sget(p,c),c==3 and 0x80))
a+=1
end
end
end
function dv(no)memcpy(0x5f00,0x5000+shl(flr(no),4),16)end
er=ob([[
o(x=-1,y=-1,c=0),
o(x=0,y=-1,c=0),
o(x=1,y=-1,c=0),
o(x=-1,y=0,c=0),
o(x=1,y=0,c=0),
o(x=-1,y=1,c=0),
o(x=0,y=1,c=0),
o(x=1,y=1,c=0),
o(x=0,y=0,c=15),
]])function jl()hv,cv,cg={},{},{}
mo,je,ok=
qb(),nf(),ui()end
function lt(e)add(hv,e)for p in all(bm)do
if (e[p])ft(cv,p,e)
end
for t in all(e.nu)do
ft(cg,t,e)end
return e
end
function he(e)del(hv,e)for p in all(bm)do
if (e[p])del(cv[p],e)
end
for t in all(e.nu)do
del(cg[t],e)end
end
bm=ob([[
"kj","ev",
"hd",
"hk","hb",
]])function dg()for _,qc in pairs(hv)do
local fn=qc[qc.me]
if fn then
fn(qc,qc.t)end
if qc.p_ then
he(qc)end
qc.t+=1
end
end
ke=kv:jr([[
me="ph",t=0,
]])ke.pq=lt
function ke:ks(me)self.me,self.t=me,0
end
ci=ob([[0.5,1,1,1,1,1,1,1,1,0,0,0,0,0,0,]])
function dc(nt)
local fl={}
for _,qc in pairs(cv[nt])do
ft(fl,qc.fa,qc)
end
for o=1,15 do
if (nt=="kj" and fl[o])mo:md(ci[o])
for _,qc in pairs(fl[o]or {})do
qc[nt](qc,qc.ra)
end
end
end
nf=ke:jr([[
l=o(),r=o(),ra=v(0,0),
]])function nf:pq()poke(0x5f2d,1)end
function nf:ph()self.ra=v(stat(32),stat(33))bu(self.l,1)bu(self.r,6)end
function bu(qn,oa)local on,ia=
band(stat(34),oa)>0,qn.on
qn.on,qn.jn,qn.hm=
on,on and not ia,ia and not on
end
function gk(e,mp,rp)
return e.kq((e.fa>=10 and rp or mp)-e.ra
)end
ui=ke:jr([[
ql=58,
fa=15,
le=o(),
hw=b(10,10,118,118),
]])function ui:lj()local le=self.le
self.qa=je.ra+mo.ra
self:gw()if #le==0 or gu(le,self.o_)then
self:bh("l")end
self:bh("r")if je.l.jn and self.le==le then
self:cj()end
self.gj=self.o_
end
function ui:ev()spr(self.rheld and 9 or self.o_ and self.o_.ql or self.ql,je.ra.x,je.ra.y)end
function ui:gw()self.ic,self.o_={}
for e in all(cv.hd)do
if mv(e,"hd",self.qa,je.ra)then
ft(self.ic,e.fa,e)self.o_=e
end
end
(self.gj or {}).m_=false
(self.o_ or {}).m_=true
poke(0x5f80,self.o_
and self.o_.fg
or max(peek(0x5f80)-1,0))end
function ui:bh(bt)local b,ni=
je[bt],bt.."held"if b.jn then
local pw=self:jz(bt.."down",self.qa,je.ra)if pw and type(pw.kn)=="table" then
self[ni]=pw.kn
end
end
local held=self[ni]
mv(held,"hq",self.qa,je.ra)if b.hm and held then
local nw=self:jz("gv",held)mv(held,"ey",nw and nw.kv)self[ni]=nil
end
end
function ui:jz(rm,...)for mf=15,0,-1 do
for h in all(self.ic[mf])do
local kn=mv(h,rm,...)if kn then
return {kv=h,kn=kn}
end
end
end
end
function ui:cj()nv(self.le,he)self.le={}
end
js=ke:jr([[
fa=11,
]])js.hd=gk
function js:pq()self.lq=lq(mv(self,"lq"))self.mb=self.lq.w+2
self.kq=qz(0,-1,self.mb,11)self.h_=self.h_ and lq(self.h_)self.jb=self.jb and jb(self.jb)end
function js:ph()if self.qf then
local d=self.qf-self.ra
self.ra+=(#d<=1 and d or d*0.4)
end
end
function js:ev()if self.jb and self.m_ then
self.jb:ol(127,1)end
end
function js:kj(p)local qt,dn,c=
p.x+self.mb-1,p.y+9,self.my
rect(p.x-1,p.y,qt+1,dn+1,1)rectfill(p.x,p.y,qt,dn,c and (self.m_ and 12 or 13)or 5)rectfill(p.x,p.y,qt,p.y,c and 6 or 5)self.lq:ol(p+v(1,2))local c=self.h_
if c then
rectfill(qt+3,p.y+1,qt+c.w+4,dn,c.bg or 2)c:ol(p+v(self.mb+3,2))end
end
function js:ldown()rn(self.my and 60)mv(self,"my")
return true
end
function pz(p,bs)if p.ep then
bs=la({{lq={p.ep,c=13}}},bs)end
rn(p.eo or 62)local ra=p.hi or p.ra
local bp=ra+v(0,0)ok:cj()ok.le=nv(bs,function(b)b.ra,b.qf,b.fa=
ra,bp,p.ca
bp+=p.gq
return js(b)end)
if (p.ff)ok.le={}
end
lq=kv:jr()function lq:pq()local w,s,fs,f,fw=0,0
self.fs=nv(self,function(f)f,fw,fs=self:mn(f)local x=w
w+=fw+fs
return {fn=f,d=v(x,0)}
end)self.w=w-fs
end
function lq:ol(p,a)
if (a)p-=v(a*self.w,0)
for f in all(self.fs)do
f.fn(p+f.d)end
end
function lq:mn(f)local w,qw,qv,fn=6,0
if type(f)=="table" then
if (f.qe)f={f.qe,5}
f,w,qw,qv=f[1],f[2],f[3]or qw,f[4]
end
if type(f)=="string" then
local c=self.c or 6
return function(p)io(f,p.x+2,p.y+1,c)end,#f*4+3,1
elseif f then
fn=function(p)dv(qw)spr(f,p.x,p.y)dv()end
end
return fn or function()end,w,qv or 2
end
pi.ld=lq.ol
jb=kv:jr()function jb:pq()self.ls=nv(self,function(ln)
return lq(type(ln)=="string" and {ln}or ln)end)end
function jb:ol(om,a)local h=#self*8+2
om-=h*a
local dn=om+h
fillp(0b0000111100001111)rk(om,dn,0x10)fillp()for l in all(self.ls)do
l:ol(v(64,om+2),0.5)
om+=8
end
end
fq=ke:jr([[
fa=14,
hd=1,
]])function fq:ldown()self.p_=mv(self,"my")
return true
end
function rn(no)
if (no and not ih)sfx(no,3)
end
function jf(n_,b)
return function()local v=rnd(1-abs(b))+max(b,0)
b+=(0.5-v)*n_
return v
end
end
function ne(d,c,ku)if d>=0 then
d="+"..d
else
c=8
end
return d..(ku or ""),c
end
function gd(e)
return abs(e.ra.x-mo.ra.x-64)<=76 and abs(e.ra.y-mo.ra.y-64)<=76
end
function ma(p,e)e.ma=max(0,e.ma-0.1)
return p+qm(e.ma,e.t*0.2)end
function ij(p,j_)local mk,nd=32767
for o in all(j_)do
local d=#(p-o.p)if p~=o.p and d<mk then
nd,mk=o,d
end
end
return mk,nd
end
bg=ke:jr([[
fa=1,
hd=1,
pl=o(0,0,0,72,73,74,75,88,89,90,91,88,91,89,91),
]])function bg:pq()local vs,vh,oh,m=
{},0,jf(0.1,0)for y=0,31 do
for x=0,31 do
vh=oh()*8-4+
(vh+(vs[x]or 1))*0.5
vs[x]=vh
mset(x,y,rnd()<0.004
and 122+rnd(5)or bg.pl[flr(vh+rnd(4))])end
end
end
function bg:kj()map(0,0,0,0,32,32)fillp(0b1010010110100101)for xy=3,262,32 do
rectfill(xy,0,xy,262,1)rectfill(0,xy,262,xy)end
fillp()rect(0,0,262,262,5)end
function bg:ldown(mp)
return (#(cg.g_ or {})>0 and mz or g_)()end
function bg:rdown(mp,rp)self.mq=rp
return self
end
function bg:hq(mp,rp)mo.v=self.mq-rp
self.mq=rp
end
jo=ke:jr([[
nu=o("jo","np","prober"),
me="unknown",
hf=1,hs=1,
fa=6,
kq=b(-7,-7,7,7),
ma=0,
ka=o(2,3,9,12),
dz=o(
o("unhappy",c=8),
o("content",c=3),
o("prosperous",c=9),
o("rich",c=12),
),
cq=o(12,36,24,25),
kw=o(mu=0),
gq=v(0,11),ca=9,
em=o(f,3,0,-1),
]])jo.hd=gk
function jo:pq()self.oe={}
if self.hp then
self:lz(gb[self.hp])end
end
function jo:hh()self.nk=
jo.ir()<0.45
and df.ll
or jo.oc()self.ep=self.nk.d
self:ks(self.nk.me or "known")self:kd()end
function jo:ldown()if self.me=="known" then
self:fe()
return true
elseif self.me=="ly" then
return jc({qu=self})end
end
function jo:fe()pz(self,nv(bj(self),function(b)local od=b:od()
return {lq=mv(b,"lq"),h_=od>0 and {od.."$",c=13},my=function()if fm:lu(od)then
self:hu(b)end
end,jb=b:jb()}
end
))end
function jo:gh()local bs={{lq=
self.px or
jo.dz[self.mi]
}}
bs[1].h_=self.kr
and {ne(self.kr).."$",bg=self.kr>=0 and 1}
local ji={"receives:"}
local jx=jp(self.jy,function(i)for j=1,i.gc do
add(ji,i)end
return i.gc==0
end)if jx[1]then
add(bs,{lq=la({"needs:"},jx)})else
if self.mi<4 and #self.jy>0 and self.kw.qe~=r.w then
add(bs,{lq=la({"wants more:"},self.qh
and {self.jy[#self.jy]}
or self.jy
)})end
end
add(bs,ji[2]and {lq=ji})if self.kw.mu>0 then
local lb=gu(jo.cq,self.kw.qe)and {"makes:",tostr(self.kw.mu),self.kw}
or {"exports:",#self.by.."/"..self.kw.mu,self.kw}
add(bs,{lq=lb})end
for i=2,#bs do
add(bs[i].lq,{false,1})end
pz(self,bs)end
function jo:gg()
return self.hf and self.me=="ly"end
function jo:hu(hp)local fp=2
if hp.cw then
self.p_=true
self=kv[hp.cw]({p=self.p,ra=self.ra,id=self.id
})elseif hp.go then
self.nk=df[hp.go]
else
self:ks("ly")self:lz(hp)self.fx=true
fp=ov("autoassemblers",3,2)end
fm:nx(fp)rn(hp.rn or 55)self:kd()end
function jo:lz(hp)self.hn,self.ep=
hp
self.jy=nv(hp.i,mw)self.kw={qe=hp.o,mu=0}
end
function jo:kf(sw)add(self.oe,sw)
if (sw:eb(self).fx)self.fx=true
self:kd()end
function jo:kd()self.ma=1.25
end
function jo:ll(t)self.p_=t>60
end
function jo:gv(s)if self.me=="ly" then
if s.qu==self then
self:gh()else
return true
end
end
end
function jo:cn(lw)local t=lw.t
if t.qe==r.e
and not self.jh then
for i in all(self.jy)do
if i.gc==0 and i.qe~=r["?"]then
del(self.jy,i)break
end
end
self.jy,self.jh=
la({{qe=r.e}},self.jy),1
end
for i in all(self.jy)do
if i.qe==r["?"]and gu(self.ba,t.qe)then
i.qe=t.qe
end
if t.qe==i.qe and i.qe~=r["?"]then
lw:kf(t:kg(self))
return
end
end
end
function jo:ib()local ib=self.by or {}
local qe,mu=
self.kw.qe,self.kw.mu
for w in all(self.oe)do
local lw={t=trade({lp={self},rl={w},qe=qe
}),mu=mu-#ib,kf=function(self,a)if not gu(ib,a)then
add(ib,a)
self.mu-=1
end
end
}
if (lw.mu==0)break
w:eb(self):cn(lw)end
return ib
end
function jo:hk()if self.kr and self.kr~=0 and gd(self)then
iv({ra=self.ra,ne(self.kr,10,"$")})end
end
function jo.ex(is,o,jq,mj,d
)
if (not o)return
local l,lc,ml=
{},#is>=3 and 1 or 0,o==r.w and {25,23}or {o}
local pe=jq*#ml>=3
for i in all(is)do
add(l,{i.qe,3-lc,i.gc~=0 and 0 or 7
})end
if #is>0 then
add(l,{false,d+lc})add(l,(d==1 or pe)and {35,2+d}or {32,5})end
if jq>5 then
add(l,tostr(jq))add(l,{false,-4})l.c=13
jq=1
end
for qq in all(ml)do
for i=1,max(jq,1)do
add(l,{qq,6,(i<=jq and i>mj)and 0 or 7,pe and -4 or -3
})end
add(l,jo.em)end
return lq(l)end
function jo:hb()self.lq=jo.ex(self.jy,self.kw.qe,self.kw.mu,#self.by,0
)end
function jo:kj(p)
if (not gd(self))return
p=ma(p,self)if self.me=="ll" and rnd(30)<self.t-30 then
return
end
circfill(p.x,p.y,7,0)dv(self.qw)palt(3,false)palt(0,true)spr(self.nk and self.nk.s or self.s or 43,p.x-4,p.y-4,2,2)dv()if self:gg()then
circ(p.x,p.y,6,jo.ka[self.mi])spr(self.mi+1,p.x-3,p.y-6)end
mv(self.lq,"ol",p+v(0,7),0.5)if self.fo then
fillp(0b1010101010101010.1)circfill(p.x,p.y,sqrt(self.fo.dw)*128,2)fillp()self.fo=nil
end
end
function bv(self,mp,rp)self.ra,self.p=mp,mp/128
local nq,c=ij(self.p,cg.np)self.et=
nq<self.dw and c or
jp(cg.jc,function(sw)
return be(sw.qu.p,sw.re.p,self.p,0.0032)end)[1]or
not ui.hw(rp)and {}
if (self.et)self.et.fo=self
end
function dl(self)if self.me=="kz" then
local mp,t=je.ra
if self.et then
t=self.et.qs
if (not t)spr(31,mp.x-3,mp.y-8)
elseif self.od then
t=self.od.."$"end
io(t or "",mp.x,mp.y-8,8,0.5)end
end
function d_(self)if not self.et
and fm:lu(self.od)then
self:ks(self.dq or "ph")fm:nx(self.fp or 0)rn(self.ik or 54)
return true
else
rn(57)self.p_=true
end
end
function na(self,t)
self.ra.y+=sin(t*0.005)*0.06
end
mh=ke:jr([[
nu=o("mh"),
fa=14,
rh=5,
]])function mh:pq()
if (kc)kc.p_=true
kc=self
end
function mh:ev()local t=self.t/self.rh-10
if t>=0 and mv(self,"lh")then
self.t,t=self.rh*10,0
end
local nc=-t^3/9
bl(self,nc)camera()if t>10.5 then
self.p_,kc=true
end
end
function bl(rb,nc)for e in all(rb)do
if nc then
camera(nc,0)nc=-nc
end
oo(pi[e.fn],e)end
end
trade=kv:jr()function trade:to()
return self.lp[#self.lp]
end
function trade:kg(np,jc)
return trade({lp=la(self.lp,{np}),rl=la(self.rl,{jc}),qe=self.qe
})end
function trade.ns.__eq(l,r)
return l.lp[1]==r.lp[1]
and l:to()==r:to()end
jc=ke:jr([[
nu=o("jc"),
me="kz",
fa=2,
fp=f,ik=48,
dw=0.004,
k_=o(0,0,7,7,10,10,9,9,9,9,9,9,9,9,4,4,4,2,2,1),
nh=20,
]])function jc:pq()rn(50)self.ts,self.hg={},{}
end
function jc:hq(mp)self.re=ok.o_.gv and ok.o_ or {ra=mp,p=mp/128}
self.od=
dj(self.qu,self.re)self.mt=
self.qu.mt or self.re.mt
self.et=self:ce()if self.et then
self.et.fo=
self
end
self.fa=
self.mt and 2 or 4
end
function jc:ey(re)if not re
or not d_(self)then
if (stat(19)==50)rn(-1)
self.p_=true
return
end
self.nh,self.re=1,re
self.qu:kf(self)re:kf(self)fm:nx(1)end
function jc:ce()local sp,dp=self.qu.p,self.re.p
if #(sp-dp)>ov("space folding",0.5,1.08)then
return {qs="-too far-"}
end
for w in all(self.qu.oe)do
if (w:eb(self.qu)==self.re)return w
end
local ds=la(jp(cg.np,function(n)
return be(sp,dp,n.p,0.0032)end),jp(cg.jc,function(sw)
return sw.mt==self.mt and
cl(sp,dp,sw.qu.p,sw.re.p)end))
return ds[1]
end
function jc:eb(jo)
return jo==self.qu and self.re or self.qu
end
function jc:ch(f,t)if rnd()<0.08 and self.hg[t.id]then
local d=t.ra-f.ra
local v=d:nn()add(self.ts,{ra=f.ra+v:pa(),v=v,l=d:qx()})end
end
function jc:lg(l,h,s,c)local qu,re=
self.qu.ra+qm(1.5,self.t/126),self.re.ra+qm(1.5,self.t/176)for dx=l,h,s do
for dy=l,h,s do
line(qu.x+dx,qu.y+dy,re.x+dx,re.y+dy,c)end
end
end
function jc:kj()if gd(self.qu)or gd(self.re)then
local jv=
self.k_[self.nh]
if (self.nh<20)self.nh+=1
local c,bc,tc=
(self.et or self.fo)and 2 or self.mt and 0 or jv,self.mt and jv or 0,self.mt and 5 or 13
self:lg(-1,2,3,bc)self:lg(0,1,1,c)for _,t in pairs(self.ts)do
t.ra+=t.v
t.l-=1
if (t.l<=0)del(self.ts,t)
local x,y=t.ra.x,t.ra.y
rectfill(x,y,x+1,y+1,0)pset(x,y,tc)end
self:ch(self.qu,self.re)self:ch(self.re,self.qu)end
self.fo=nil
end
function jc:ev()
if (self.qu~=self.re)dl(self)
end
function f_(self)self.qy={}
end
function ez(self,n)while rnd()<n do
add(self.qy,self:mm())
n-=1
end
end
function ei(self)
if (self.fa<10 and not gd(self))return
local cx,cy,qo,qg,ro,q_,cs=
self.ra.x,self.ra.y,self.qo,self.qg,self.ro,self.q_,self.k_
for _,p in pairs(self.qy)do
local pv=p.v
p.p+=pv
p.v.x=qo*pv.x+ro*pv.y
p.v.y=qg*pv.x+q_*pv.y
if (p.a)p.v+=p.a
p.l-=0.14
if p.l<=1 then
del(self.qy,p)else
pset(cx+p.p.x,cy+p.p.y,cs[flr(p.l)])end
end
ez(self,self.fc)end
g_=ke:jr([[
nu=o("g_","prober"),
fx=1,
dq="ly",
od=0,fp=f,
fa=2,
dw=0.025,
py=0.06,ki=9,
pn=3,pk=1,
fc=1,
k_=o(1,1,5,13,6),
qo=1.02,ro=-0.0525,
qg=0.0525,q_=1.02,
du=2,
]])g_.hq=bv
g_.ey=d_
g_.ev=dl
g_.pq=f_
g_.kj=ei
function g_:mm()local a=(flr(rnd(self.pn))+rnd(0.5))/self.pn+self.t*0.001
local p=qm(self.du+rnd(),a)local v=p*self.py
return {p=p,v=self.pk and v:pa()or v,l=self.ki
}
end
function gn(hr,og,r)local oc,nm={},{}
for e in all(hr)do
for n=1,e.r*og do
add(oc,e)end
end
return function()local e
repeat
e=oc[flr(r()*#oc+1)]
until not gu(nm,e)del(oc,e)nm[3-#oc%3]=e
return e
end
end
function bw(n)for id=1,n do
local p
repeat
p=v(rnd(2.75)+0.125,rnd(2.75)+0.125
)until ij(p,cg.jo)>0.05
jo({id="p"..id,p=p,ra=p*128})end
jo.oc,jo.ir=
gn(df,2,jf(0.5,dk.en)),jf(0.8,0.5)end
mz=ke:jr([[
nu=o("mz"),
me="kz",dw=0,
dq="pr",
spawn_snd=52,ik=49,
oi=0,
od=3,fp=1,
fa=5,
kk=o(7,7,6,13,5,1,1,1,0),
jj=o(qs="-too far-"),
]])mz.hq=bv
mz.ev=dl
function mz:pq()rn(53)self.ox=jp(hv,function(p)
return p.fx
end)end
function mz:hq(...)bv(self,...)local nq
nq,self.ij=ij(self.p,self.ox)if nq>ov("space folding",0.19,0.27)then
self.et=mz.jj
end
end
function mz:kz(t)self.oi=ov("space folding",29,35)*
(min(t/15,1)+sin(t/40)*0.1)end
function mz:ey()self.p_=
self.t<15 or not d_(self)end
function mz:pr(t)local sr=self:ej(t)/128+0.031
for pt in all(cg.jo)do
if pt.me=="unknown"and (pt.p-self.p):qx()<sr then
pt:hh()end
end
self.p_=t>25
end
function mz:kj(p)if self.me=="kz" then
if not self.et then
circ(p.x,p.y,self.oi,1)end
local cp=self.ij.ra
line(cp.x,cp.y,p.x,p.y,1)spr(7,p.x-4,p.y-4)else
for dt=0,8,2 do
if self.t>dt then
local sr=self:ej(self.t-dt)circ(p.x,p.y,sr,self.kk[flr(sr/self.oi*8)])end
end
end
end
function mz:ej(t)
return min(sqrt(t/15),1)*self.oi
end
ef=ob("\"\74\65\78\",\"\70\69\66\",\"\77\65\82\",\"\65\80\82\",\"\77\65\89\",\"\74\85\78\",\"\74\85\76\",\"\65\85\71\",\"\83\69\80\",\"\79\67\84\",\"\78\79\86\",\"\68\69\67\",\"\85\78\68\",\"\68\85\79\",\"\84\69\82\",")bz=ob([[
o(26,86,0,fn="br"),
o(51,59,1,fn="br"),
o(76,77,1,fn="br"),
o(24,25,1,fn="br"),
o(33,41,1,fn="br"),
o("",score=0,ra=v(64,21),c=5),
o(55,"planets:   ",
score=1,ra=v(64,34)),
o(19,"population:",
score=2,ra=v(64,43)),
o(24,"technology:",
score=3,ra=v(64,52)),
o(o(27,5),o(f,0),f,o(25,5),"happiness",
score=4,ra=v(64,61)),
o("total:","",
score=5,ra=v(64,74)),
o(score=6,ra=v(64,82)),
]])pj=ke:jr([[
fa=10,
ea=o(
o(42,43,1,fn="br"),
o("",64,40,6,0.5,fn="qr"),
),
gs=o(
o("last year!",64,48,9,0.5,fn="qr"),
o("2 years remain",64,48,2,0.5,fn="qr"),
f,f,
o("5 years remain",64,48,2,0.5,fn="qr"),
),
cr=o(
hi=v(0,11),gq=v(0,11),ca=10,
),
i_=o(
o(126,127,1,fn="br"),
o(0,1,1,fn="br"),
o(8,118,34,fn="rf"),
o(0,-3,26,fn="rf"),
o(81,-3,117,fn="rf"),
),
ou=o(
o(f,13,121,9,fn="qr"),
o(f,37,121,2,fn="qr"),
o(f,13,2,9,fn="qr"),
o(f,29,2,2,fn="qr"),
o(f,59,2,2,fn="qr"),
o(f,85,2,13,fn="qr"),
o(f,99,2,13,fn="qr"),
),
ed=1,
]])function pj:hb()local cf=ne(fm.ge)local ou={fm.nz.."$",cf,fm.jk,"+"..fm.po[r.k]+1,fm.happiness.."%",ef[fm.lo],fm.yr
}
for i,p in pairs(ou)do
pj.ou[i][1]=p
end
eg()end
function pj:kj()bl(la(pj.i_,pj.ou))if self.gz then
self.gz+=0.2
local h=min(self.gz,3.2)^2
rk(0,h,0)rk(127-h,127)end
end
function pj:hk(yr)local iw,pp=
pj.ea,pj.gs[3426-yr]
iw[2][1],iw[3]=
"year "..yr,pp or nil
if pp then
local t=90
function iw.lh()
t-=1
return t>0
end
end
mh(iw)end
function pj:ph()if fm.hl and not self.gz then
fq()js(ob([[
fa=15,
ra=v(51,100),
lq=o(" again? "),
]],{my=_init}))js(ob([[
fa=15,
ra=v(39,100),
lq=o(o(10,7)),
]],de))self.c_,self.gz,self.fa=
true,0,13
end
if self.c_ and kc~=self.ed then
local s=ky()local iw=nv(bz,function(st)if st.score then
st[3]=tostr(s[st.score])if st.score==0 then
st[1]=(fm.hl or "current score").." ("..dk.px..")"end
if st.score==6 then
for i=1,5 do
st[i]=i<=s[6]and 56 or 57
end
end
return {lq(st),st.ra,0.5,fn="ld"}
else
return st
end
end)iw.rh=3
function iw.lh()
return self.c_
end
self.ed=mh(iw)end
end
de={my=function()ju.c_ = not ju.c_
return true
end
}
qh=jo:jr([[
nu=o("qh","jo","np","prober"),
hp=1,px=o("laboratory",c=13),
me="kz",dq="ly",
hf=f,qh=1,
od=15,fp=2,
dw=0.025,
mi=1,
s=174,
ba=o(28,18,20,21,22,48),
]])qh.hq=bv
qh.ly=na
qh.ev=dl
qh.ey=d_
ie=ke:jr([[
kq=b(0,0,9,8),
fa=12,
id=0,
ra=v(999,999),
eh=v(118,120),
]])ie.hd=gk
function ie:hb()if not self.kh and oj(self.ke.rr)then
self.ra,self.kh=
ie.gy,true
ie.gy-=v(10,0)
end
end
function ie:ldown(mp)
ie.id+=1
return self.ke({id="s"..ie.id
})end
function ie:kj(p)if self.kh then
palt(3,false)spr(self.ke.s,p.x,p.y)if self.m_ then
dl(self.ke)self.jb:ol(0,0)end
palt(3,true)end
end
iv=ke:jr([[
fa=9,
l=0,v=v(0,-0.34),
]])function iv:pq()self.ra=self.ra or
je.ra+mo.ra-v(0,5)end
function iv:ph()
self.ra+=self.v
self.l+=0.1
self.p_=self.l>=5
end
function iv:kj(p)dv(self.l)io(self[1],p.x,p.y,self[2],0.5)dv()end
function cr()local bs,rd,gr=
{},{},#fm.invented
local tl=fm.ew
local qi=ek[tl-1]or 0
local bs=nv(cb(true),function(t)
return {lq={t.nk,t.px,c=6},h_={{24,5},tostr(gi(t))},jb=t.oy,my=t.e_ and function()fm:kt(t)end
}
end
)if gr>0 then
add(bs,{lq={30,gr.." invented",c=3},jb=fm.invented
})end
pj.cr.ep=
"level "..tl.." ("..(gr-qi).."/"..(ek[tl]-qi)..")"pz(pj.cr,bs)end
ix=ob([[
o(
ra=v(118,0),
lq=o(o(15,7)),
),o(
ra=v(0,0),
lq=o(o(37,7)),
),o(
ra=v(47,0),
lq=o(o(25,7)),
),o(
ra=v(0,117),
lq=o(o(8,7)),
),
]])ix[1].my=function()fm.lo=1
fm:hk()end
ix[2].my=cr
ix[2].hb=function(self)self.lq=lq({#cb()>0 and 24 or 37
})end
ix[3].my=function()ju.c_=true
fq(de)end
ha=ob([[
o(lq=o(31,"restart     ")),
o(lq=o(42,"toggle music")),
o(lq=o(40,"toggle sfx  ")),
o(lq=o(26,"how to play?"),fg=5,),
hi=v(0,107),gq=v(0,-11),
ca=14,
]])ix[4].my=function()for i,fn in pairs({_init,function()music(-sgn(stat(20)))end,function()ih=not ih end,function()end
})do
ha[i].my=fn
end
pz(ha,ha)end
hj=qh:jr([[
nu=o("hj","jo","np","prober"),
rr="ascension",
hp=2,qh=f,
px=o("ascension nr",c=13),
od=60,
s=166,
py=0.12,ki=9,
pn=5,pk=1,
fc=0.5,
k_=o(1,13,12,12,12,13,13,5,1,1,1),
du=7.5,
qo=0.98,ro=-0.07,
qg=0.07,q_=0.98,
]])hj.pq=f_
hj.mm=g_.mm
function hj:kj(p)if self.mi and self.mi>=2 then
ei(self)end
jo.kj(self,p)end
pm=hj:jr([[
fz="pm",
nu=o("jo","np","prober"),
hp=3,px=o("protostar",c=13),
me="ly",s=98,
py=0.06,ki=9,
pn=8,pk=f,
ng=v(1,0),
du=2,
fc=1,
k_=o(1,2,4,9,10,7),
qo=1.02,ro=-0.0525,
qg=0.0525,q_=1.02,
]])function pm:pq()ga(ob([[
qo=0.97,ro=-0.1255,
qg=0.1255,q_=0.97,
k_=o(1,2,8,14,14,14,14,8,8,2,2,1,1),
iu=-0.009,py=9,ms=1.5,
]],{ra=self.ra+self.ng}))end
mx=hj:jr([[
nu=o("jo","np","prober"),
rr="void synthesis",
hp=5,s=66,px=o("synthesizer",c=13),
od=40,
ht=o(28,20,21,22),
py=0.12,ki=9,
pn=5,pk=1,
fc=0.3,
k_=o(1,1,5,5,3,3,3,5,5,1),
du=5,
qo=0.98,ro=-0.07,
qg=0.07,q_=0.98,
]])function mx:kf(sw)local cp=sw:eb(self)for i in all(cp.jy)do
if i.gc==0
and gu(self.ht,i.qe)then
self.kw.qe,self.ht=
i.qe,{}
end
end
jo.kf(self,sw)end
nl=qh:jr([[
nu=o("jo","np","prober"),
rr="trade league",qh=f,nl=1,
hp=6,s=68,px=o("trading hub",c=13),
od=20,qw=15,
ba=o(28,48,18,20,21,22),
]])function nl:ly(t)na(self,t)if self.mi>=2 then
self.qw=8+t%42/6
end
end
ga=jo:jr([[
fz="ga",
nu=o(),
hf=f,
by=o(),
kq=b(0,0,-1,-1),
fa=3,
fc=0,
qo=0.9,ro=0,
qg=0,q_=0.9,
k_=o(1,2,4,9,9,10,10,7,7,7,7,7),
iu=-0.06,py=6,ms=3,
]])function ga:pq()f_(self)ez(self,140)mo.ma=3
end
function ga:mm()local qd=rnd(1.5)+0.5
if (rnd()<0.6)qd=flr(qd/0.5)*0.5
local p=qm(qd,rnd())
return {p=p,v=p*self.ms,a=p*self.iu,l=rnd(self.py)+self.py
}
end
function ga:kj(p)local pt
for i=1,3 do
pt=self.qy[i]
if pt then
circ(p.x,p.y,pt.p:qx(),self.k_[flr(pt.l/2)])end
end
ei(self)self.p_=not pt
end
os=qh:jr([[
nu=o("jo","np","prober"),
hp=4,px=o("processor",c=13),
qh=f,s=100,
od=15,
hs=0.5,
]])nr=qh:jr([[
nu=o("nr","np","prober"),
od=10,fp=1,
kq=b(-4,-4,5,5),
s=170,
hz=o(170,170,170,168),
dw=0.01,
rr="slipgates",
]])function nr:gv(s)
return s.qu~=self and #self.oe<3
end
function nr:ldown()
return self:gv({})and jc({qu=self})end
function nr:hb()self.s=self.hz[#self.oe+1]
end
function nr:cn(lw)local t=lw.t
for w in all(self.oe)do
if lw.mu>0 and not gu(t.rl,w)then
local gf=mw(lw)gf.t=t:kg(self,w)w:eb(self):cn(gf)lw.mu=gf.mu
end
end
end
fr=nr:jr([[
mt=1,
rr="infraspace",
od=25,
s=78,hz=o(78,78,78,76),
]])qb=ke:jr([[
v=v(0,0),
ma=0,
p=v(192,192),
ra=v(192,192),
pc=o(v(-1,0),v(1,0),v(0,-1),v(0,1)),
]])function qb:ph()local ip=v(0,0)for b=0,3 do
if btn(b)or btn(b,1)then
ip+=self.pc[b+1]*3
end
end
self.p+=self.v
self.v+=(ip-self.v)*0.2
self.p.x=mid(-64,320,self.p.x)self.p.y=mid(-64,320,self.p.y)self.ra=ma(self.p,self)end
function qb:md(fy)local cp=self.ra*(fy or 1)camera(cp.x,cp.y)end
fv=g_:jr([[
fa=10,ra=v(64,28),
hi=v(34,56),
ca=11,
gq=v(0,11),
eo=-1,
ff=1,
fc=5,
k_=o(0,1,1,2,4,9,10,9,4,2,1,1,1),
qo=1.02,ro=-0.007,
qg=0.007,q_=1.02,
pn=5,du=15,
py=0.0067,pk=1,
ki=17,
kq=b(-40,78,40,98),
ql=11,fg=5,
rb=o(
o(192,0,11,16,4,fn="s"),
o(172,20,107,2,2,fn="s"),
o("need help?",40,110,13,fn="qr"),
o("slipways.net/help",40,116,5,fn="qr"),
),
]])fv.hd=gk
function fv:pq()for i,d in pairs(bo)do
d.my=eu
end
pz(self,bo)end
function fv:kj(p)ei(self)bl(self.rb)end
fb=ob([[
2,2,5,5,7,7,10,10,14,14,14,19,19,19,
]])function eg()local ps=#jp(cg.jo,jo.gg)/2
for i,ny in pairs(fb)do
local pd=0x3101+4*ny
poke(pd,peek(pd)%128+(ps<i and 128 or 0))end
end
function _init()cu()jl()ie.gy,kc=
ie.eh
fv()music(0)end
function eu(nj)dk=nj
jl()bg()bw(90)fm,ju=hy(),pj()nv(ix,js)local gx={qh,os,nr,fr,nl,mx,hj
}
for i,b in pairs(gx)do
ie({ke=b,jb=jb(dr[i])})end
fm:lj()end
function _update60()dg()ok:lj()end
function _draw()cls()dv()dc("kj")dc("ev")end
bo=ob([[
o(lq=o(49,"forgiving   "),px="forgiving",
trade=7,en=-0.99),
o(lq=o(50,"reasonable  "),px="reasonable",
trade=6,en=-0.6),
o(lq=o(51,"challenging "),px="challenging",
trade=5,en=-0.4),
o(lq=o(52,"tough       "),px="tough",
trade=4,en=-0.2),
]])hy=kv:jr()function hy:pq()qj(self,ob([[
nz=100,ge=0,
jk=0,ew=1,
invented=o(),
lo=1,yr=3401,
]]))end
function hy:nx(hx)local kx=ov("time compression",12,15)
self.lo+=hx
self:lj()if self.lo>kx then
self.lo-=kx
self:hk()end
end
function hy:hk()
self.nz+=self.ge
self.jk+=self.po[r.k]+1
self.yr+=1
self:lj()for e in all(cv.hk)do
e:hk(self.yr)end
end
function hy:lu(jt)if self.nz<jt then
iv({"-no money-",8})
return
end
self.nz-=jt
if jt~=0 then
iv({ne(-jt,10,"$")})end
return true
end
function hy:lj()repeat
bd()until not bb()self.po,self.fu=
bx(),ck()local dd=0
for p in all(cg.jo)do
p.kr=flr(db(p)-
da(p))
dd+=p.kr
end
self.ge=flr(dd*ov("skill implants",1,1.15))- cz()self.happiness=
bf()self.hl=
self.yr>=3426 and "final score"or self.ge<=0 and self.nz<3 and "bankrupt"for e in all(cv.hb)do
e:hb()end
end
function cm(jm)local p={}
for _,qe in pairs(r)do
p[qe]=jm
end
return p
end
function bx()local p=cm(0)for pt in all(cg.jo)do
if pt.jy then
p[pt.kw.qe]+=pt.kw.mu
end
end
return p
end
function ck()local d=cm(1)for l in all(cg.qh)do
d[l.jy[#l.jy].qe]-=0.15
end
return d
end
bn,bq=
6,0.0977
function dj(pb,to)local ec=
(#(to.p-pb.p)/bq)^0.75
return flr(bn*bk()*max(0.5,ec))end
function bk()
return max(1,sqrt(#(cg.jc or {}))*0.27-0.1
)end
function bd()for pt in all(cg.jo)do
pt.cc={}
end
for pt in all(cg.jo)do
pt.by=pt:ib()for e in all(pt.by)do
for i,w in pairs(e.rl)do
w.hg[e.lp[i+1].id]=true
end
add(e:to().cc,e)end
end
end
function bb()local jg
for pt in all(cg.jo)do
local jw=dh(pt)if not pt.mi or jw>pt.mi then
if pt.mi==1 and pt.hn.dm then
add(pt.jy,{qe=pt.hn.dm})end
pt.mi,jg=
jw,true
end
if pt.mi~=0 then
local po=pt.hn:po()pt.kw.mu=
po[min(pt.mi,#po)]+
(pt.jh or 0)end
end
return jg
end
function dh(pt)
if (pt.me~="ly")return 0
local jd=1
for i in all(pt.jy)do
i.gc=
#jp(pt.cc,function(im)
return im.qe==i.qe
end)
jd*=i.gc
end
if (jd==0)return 1
if pt.nl then
return min(#jp(pt.jy,function(i)
return i.gc>0
end),3)end
if not pt.hf then
local li=pt.jy[#pt.jy]
return li and min(li.gc+1,4)or 2
end
return min(4,1+mid(#pt.by,#pt.cc,1))end
es=ob([[1,0,1,2,]])
function da(pt)
return pt.qh
and #cg.qh
or pt:gg()
and es[pt.mi]
or 0
end
function cz()
local mu=#jp(cg.jo,jo.gg)
return flr(0.18*mu^2)
end
ct=
ob([[-0.333,0,0.25,0.5,]])
function db(pt)
local total=0
for e in all(pt.by)do
local oz=e:to()
if oz.hf then
total+=dk.trade*pt.hs*(1+ct[pt.mi]+ct[oz.mi])
end
end
if pt.kw.qe==r.w then
total+=pt.kw.mu*dk.trade*1.5
end
return total
end
b_=ob([[0,100,200,400,]])
function ky()
local di=0
for pt in all(cg.jo)do
if pt:gg()then
di+=b_[pt.mi]
end
end
local gl=
fm.po[r.p]*40
local el=
flr((#fm.invented*1.2)^2)*10
local total=flr((
di+
gl+
el
)*0.01*fm.happiness)
return {
di,
gl,
el,
fm.happiness.."%",
total,
min(flr(total/2000),5)
}
end
co=ob([[-5,0,1,1,]])
function bf()
local h=100+2*(
fm.po[r.h]+
fm.po[r.w]
)
for pt in all(cg.jo)do
if pt:gg()then
h+=co[pt.mi]
end
end
return h
end
r,iz=ob(unstash(8192)),ob(unstash(8259))df=ob([[
o("ll",45,0,me="ll"),
o("e",160,4.5,"earth-like"),
o("f",128,6,"forgeworld"),
o("m",134,4,"mineral"),
o("o",130,3,"ocean"),
o("r",140,2.5,"remnant"),
o("x",162,3,"xeno"),
o("j",136,2,"jungle"),
o("i",138,2,"iceball"),
o("s",142,2,"barren"),
o("g",96,4,"gas giant"),
]])foreach(df,ho(df,ob([[
"px","s","r","d",
]])))hc=ob(unstash(8383))fd=ob(unstash(8494))function bj(jo)
return jp(gb,function(b)if oj(b.rr)then
local mg
fi(b.lr,function(k)mg=mg or k==jo.nk.px
end)
return mg
end
end)end
hp=kv:jr([[
mr=o("px","lr","fj","ja","i","o","il","dm"),
gp=o("o","dm"),
fk=o(f,2),
]])function hp:pq()ho(gb,hp.mr,hp.gp)(self)if self.ii then
self.lq={self.px,32,{self.ii,8}}
end
self.i=fi(self.i or "",function(c)
return {qe=r[c]}
end)end
function hp:lq()local l=
jo.ex(self.i,self.o,max(self:po()[1],1),0,1
)add(l,hp.fk)add(l,self.px)
return l
end
function hp:jb()
if (self.ii)return
local it
for i in all(self.i)do
it=(it and it.."," or "")..iz[tostr(i.qe)]
end
return {{it,13,iz[tostr(self.o)]}}
end
function hp:po()local qp=hc[self.ja]
return self.il and oj(self.il)and hc[qp.mc]
or qp
end
function hp:od()local od=self.fj
if self.il and oj(self.il)then
od*=1.5
end
if not self.ii and oj("nanomaterials")then
od*=0.85
end
return flr(od)end
gb,rr={}
for ri in all(fd)do
if type(ri)=="string" then
rr=ri
else
ri.rr=rr
add(gb,hp(ri))end
end
dr=ob(unstash(9807))nb=ob(unstash(10457))foreach(nb,ho(nb,ob([["px","lv","nk","od","oy",]]),
{"nk"}
))
ek=ob([[2,4,7,12,]])
function hy:kt(t)
self.jk-=gi(t)
add(self.invented,t.px)
if self.ew<4
and #self.invented>=ek[self.ew]then
self.ew+=1
end
self:lj()
end
function cb(bi)
return jp(nb,function(t)
t.e_=gi(t)<=fm.jk
return t.lv==fm.ew
and not oj(t.px)
and (t.e_
or bi)
end)
end
function oj(px)
return not px or gu(fm.invented,px)
end
function ov(px,lk,mc)
return oj(px)
and mc or lk
end
function gi(t)
return flr(t.od*max(0.55,
fm.fu[t.nk]))
end
function cl(p1,p2,q1,q2)
local oq1,oq2,op1,op2=
ee(p1,p2,q1),ee(p1,p2,q2),
ee(q1,q2,p1),ee(q1,q2,p2)
return oq1 and oq2 and op1 and op2
and oq1~=oq2 and op1~=op2
end
function ee(a,b,c)
local kl=
(b.y-a.y)*(c.x-b.x)-
(b.x-a.x)*(c.y-b.y)
return kl~=0 and sgn(kl)
end
function be(a,b,c,fh)
local d=b-a
local l_=d:rg(c-a)/#d
return l_>0.01
and l_<0.99
and #(a+d*l_-c)<fh
end
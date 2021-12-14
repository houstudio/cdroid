--[[
	plimage demo
--]]

-- initialise Lua bindings for PLplot examples.
dofile("plplot_examples.lua")

XDIM = 260
YDIM = 220

dbg = 0
nosombrero = 0
nointeractive = 0
f_name=""


-- Transformation function
function mypltr(x,  y)
  local x0 = (stretch["xmin"] + stretch["xmax"])*0.5
  local y0 = (stretch["ymin"] + stretch["ymax"])*0.5
  local dy = (stretch["ymax"]-stretch["ymin"])*0.5
  local tx = x0 + (x0-x)*(1 - stretch["stretch"]*math.cos((y-y0)/dy*math.pi*0.5))
  local ty = y

  return tx, ty
end


-- read image from file in binary ppm format
function read_img(fname)
  -- naive grayscale binary ppm reading. If you know how to, improve it
  local fp = io.open(fname, "rb")
  if fp==nil then
    return 1
  end

  -- version
  local ver = fp:read("*line")

  if ver~="P5" then -- I only understand this!
    fp:close()
    return 1
  end

  while fp:read(1)=="#" do
    local com = fp:read("*line")
    if com==nil then
      fp:close()
      return 1
    end
  end
  fp:seek("cur", -1)

  local w, h, num_col = fp:read("*number", "*number", "*number")
  if w==nil or h==nil or num_col==nil then -- width, height, num colors
    fp:close()
    return 1
  end

  -- read the rest of the line (only EOL)
  fp:read("*line")

  local img = fp:read(w*h)
  fp:close()
  if string.len(img)~=(w*h) then
    return 1
  end

  local imf = {}

  for i = 1, w do
    imf[i] = {}
    for j = 1, h do
      imf[i][j] = string.byte(img, (h-j)*w+i) -- flip image up-down
    end
  end

  return 0, imf, w, h, num_col
end


-- save plot
function save_plot(fname)
  local cur_strm = pl.gstrm() -- get current stream
  local new_strm = pl.mkstrm() -- create a new one

  pl.sdev("psc") -- new device type. Use a known existing driver
  pl.sfnam(fname) -- file name

  pl.cpstrm(cur_strm, 0) -- copy old stream parameters to new stream
  pl.replot()	-- do the save
  pl.end1() -- close new device

  pl.sstrm(cur_strm)	-- and return to previous one
end


--  get selection square interactively
function get_clip(xi, xe, yi, ye)
  return 0, xi, xe, yi, ye
end


-- set gray colormap
function gray_cmap(num_col)
  local r = { 0, 1 }
  local g = { 0, 1 }
  local b = { 0, 1 }
  local pos = { 0, 1 }

  pl.scmap1n(num_col)
  pl.scmap1l(1, pos, r, g, b)
end


x = {}
y = {}
z = {}
r = {}
img_f = {}
cgrid2 = {}


-- Parse and process command line arguments
pl.parseopts(arg, pl.PL_PARSE_FULL)

-- Initialize plplot
pl.init()

z={}

-- view image border pixels
if dbg~=0 then
  pl.env(1, XDIM, 1, YDIM, 1, 1) -- no plot box

  -- build a one pixel square border, for diagnostics
  for i = 1, XDIM do
    z[i] = {}
    z[i][1] = 1 -- left
    z[i][YDIM] = 1 -- right
  end

  for i = 1, YDIM do
    z[1][i] = 1 -- top
    z[XDIM][i] = 1 -- botton
  end

  pl.lab("...around a blue square."," ","A red border should appear...")

  pl.image(z, 1, XDIM, 1, YDIM, 0, 0, 1, XDIM, 1, YDIM)
end

-- sombrero-like demo
if nosombrero==0 then
  r = {}
  pl.col0(2) -- draw a yellow plot box, useful for diagnostics! :(
  pl.env(0, 2*math.pi, 0, 3*math.pi, 1, -1)

  for i = 1, XDIM do
    x[i] = (i-1)*2*math.pi/(XDIM-1)
  end
  for i = 1, YDIM do
    y[i] = (i-1)*3*math.pi/(YDIM-1)
  end

  for i = 1, XDIM do
    r[i] = {}
    z[i] = {}
    for j=1, YDIM do
      r[i][j] = math.sqrt(x[i]^2+y[j]^2)+1e-3
      z[i][j] = math.sin(r[i][j])/r[i][j]
    end
  end

  pl.lab("No, an amplitude clipped \"sombrero\"", "", "Saturn?")
  pl.ptex(2, 2, 3, 4, 0, "Transparent image")
  pl.image(z, 0, 2*math.pi, 0, 3*math.pi, 0.05, 1, 0, 2*math.pi, 0, 3*math.pi)

  -- save the plot
  if f_name~="" then
    save_plot(f_name)
  end
end

-- read Chloe image
-- Note we try two different locations to cover the case where this
-- examples is being run from the test_c.sh script
status, img_f, width, height, num_col = read_img("Chloe.pgm")
if status~=0 then
  status, img_f, width, height, num_col = read_img("../Chloe.pgm")
  if status~=0 then
    pl.abort("No such file")
    pl.plend()
    os.exit()
  end
end

-- set gray colormap
gray_cmap(num_col)

-- display Chloe
pl.env(1, width, 1, height, 1, -1)

if nointeractive==0 then
  pl.lab("Set and drag Button 1 to (re)set selection, Button 2 to finish."," ","Chloe...")
else
  pl.lab(""," ","Chloe...")
end

pl.image(img_f, 1, width, 1, height, 0, 0, 1, width, 1, height)

-- selection/expansion demo
if nointeractive==0 then
  xi = 25
  xe = 130
  yi = 235
  ye = 125

  status, xi, xe, yi, ye = get_clip(xi, xe, yi, ye)
  if status~=0 then  -- get selection rectangle
    pl.plend()
    os.exit()
  end

  pl.spause(0)
  pl.adv(0)

  -- display selection only
  pl.image(img_f, 1, width, 1, height, 0, 0, xi, xe, ye, yi)

  pl.spause(1)

  -- zoom in selection
  pl.env(xi, xe, ye, yi, 1, -1)
  pl.image(img_f, 1, width, 1, height, 0, 0, xi, xe, ye, yi)
end

-- Base the dynamic range on the image contents.
img_max, img_min = pl.MinMax2dGrid(img_f)

-- Draw a saturated version of the original image.  Only use the middle 50%
-- of the image's full dynamic range.
pl.col0(2)
pl.env(0, width, 0, height, 1, -1)
pl.lab("", "", "Reduced dynamic range image example")
pl.imagefr(img_f, 0, width, 0, height, 0, 0, img_min + img_max*0.25, img_max - img_max*0.25)

-- Draw a distorted version of the original image, showing its full dynamic range.
pl.env(0, width, 0, height, 1, -1)
pl.lab("", "", "Distorted image example")

stretch = {}
stretch["xmin"] = 0
stretch["xmax"] = width
stretch["ymin"] = 0
stretch["ymax"] = height
stretch["stretch"] = 0.5

-- In C / C++ the following would work, with plimagefr directly calling
-- mypltr. For compatibilty with other language bindings the same effect
-- can be achieved by generating the transformed grid first and then
-- using pltr2.
-- pl.imagefr(img_f, width, height, 0., width, 0., height, 0., 0., img_min, img_max, mypltr, (PLPointer) &stretch)

cgrid2 = {}
cgrid2["xg"] = {}
cgrid2["yg"] = {}
cgrid2["nx"] = width+1
cgrid2["ny"] = height+1

for i = 1, width+1 do
  cgrid2["xg"][i] = {}
  cgrid2["yg"][i] = {}
  for j = 1, height+1 do
    xx, yy = mypltr(i, j)
    cgrid2["xg"][i][j] = xx
    cgrid2["yg"][i][j] = yy
  end
end

--pl.imagefr(img_f, 0, width, 0, height, 0, 0, img_min, img_max, "pltr2", cgrid2)
pl.imagefr(img_f, 0, width, 0, height, 0, 0, img_min, img_max, "mypltr")

pl.plend()
